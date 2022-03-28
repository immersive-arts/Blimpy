import paho.mqtt.client as mqtt
import time
import re
from threading import Thread
import signal as sig
import queue
import numpy as np
from osc4py3.as_comthreads import osc_method, osc_udp_server, osc_startup, osc_process, osc_terminate
from enum import Enum
import sys, getopt
import yaml
from pyquaternion import Quaternion

def signal_handler(sig, frame):
    del sig, frame

    global run
    run = False

sig.signal(sig.SIGINT, signal_handler)

def parseFloat(key, message):
    ms = re.search(key + "=[-+]?\d*\.\d+" + "|" + key + "=[-+]?\d+", message)
    if ms is None:
        return
    else:
        data = float(message[ms.span()[0]+(len(key)+1):ms.span()[1]])
        return data

def parseString(key, message):
    ms = re.search(key + "=[a-zA-Z0-9]*", message)
    if ms is None:
        return
    else:
        data = message[ms.span()[0]+(len(key)+1):ms.span()[1]]
        return data

def quaternion_to_euler(qw, qx, qy, qz):
    t0 = +2.0 * (qw * qx + qy * qz)
    t1 = +1.0 - 2.0 * (qx * qx + qy * qy)
    roll = np.arctan2(t0, t1)
    t2 = +2.0 * (qw * qy - qz * qx)
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    pitch = np.arcsin(t2)
    t3 = +2.0 * (qw * qz + qx * qy)
    t4 = +1.0 - 2.0 * (qy * qy + qz * qz)
    yaw = np.arctan2(t3, t4)
    return [yaw, pitch, roll]

def constrain_angle_difference(angle_difference):
    if angle_difference > np.pi:
        angle_difference = angle_difference - 2 * np.pi
    elif angle_difference < -np.pi:
        angle_difference = angle_difference + 2 * np.pi
    
    return angle_difference

class State(Enum):
    unavailable = 0
    untracked = 1
    ready = 2
    moving = 3
    parking = 4
    holding = 5
    unmanaged = 6

class Device:
    t0 = 0
    run_thread = False

    x = 0
    y = 0
    z = 0
    vx = 0
    vy = 0
    vz = 0
    roll = 0
    pitch = 0
    yaw = 0
    p = 0
    q = 0
    r = 0

    attitude = Quaternion()
    attitude_ref = Quaternion()
    attitude_rate = 0
    roll_rate = 0
    pitch_rate = 0
    yaw_rate = 0

    x_ref = 0
    y_ref = 0
    z_ref = 0
    vx_ref = 0
    vy_ref = 0
    vz_ref = 0
    p_ref = 0
    q_ref = 0
    r_ref = 0

    fx = 0
    fy = 0
    fz = 0
    mx = 0
    my = 0
    mz = 0

    i_e_z = 0
    c_x_filt = 0
    c_y_filt = 0
    c_z_filt = 0
    c_a_filt = 0

    m1 = 0.0
    m2 = 0.0
    m3 = 0.0
    m4 = 0.0
    m5 = 0.0
    m6 = 0.0

    battery_charge = 0.0
    last_heartbeat = 0.0

    max_command = 1.0
    max_i_e = 3.0

    time_const = 0.1

    t_pos = 0
    t_att = 0

    tracked = False
    missed_ticks = 0
    loop_count = 0
    empty_count = 0
    
    state = State.unavailable
    
    J = np.array([[0.024, 0.0, 0.000],
                  [0.0, 0.039, 0.0],
                  [0.00, 0.0, 0.032]])

    def __init__(self, loop_period, client, manager_base_topic, device_base_topic, device_type, device_name, tracking_id):

        self.tracking_id = tracking_id
        self.manager_base_topic = manager_base_topic
        self.device_base_topic = device_base_topic
        self.device_type = device_type
        self.device_name = device_name
        self.loop_period = loop_period
        self.client = client
        self.client.message_callback_add(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/stack", self.stack)
        self.client.message_callback_add(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/clear", self.clear)
        self.client.message_callback_add(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/config", self.config)
        self.client.message_callback_add(str(device_base_topic) + "/" + str(self.device_name) + "/model", self.model)
        self.client.message_callback_add(str(device_base_topic) + "/" + str(self.device_name) + "/battery", self.battery)
        self.client.subscribe(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/stack")
        self.client.subscribe(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/clear")
        self.client.subscribe(str(manager_base_topic) + "/" + str(device_base_topic) + "/" + str(self.device_name) + "/config")
        self.client.subscribe(str(device_base_topic) + "/" + str(self.device_name) + "/model")
        self.client.subscribe(str(device_base_topic) + "/" + str(self.device_name) + "/battery")

        osc_method("/rigidbody/" + str(self.tracking_id) + "/tracked", self.set_track)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/quat", self.set_attitude)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/position", self.set_position)

        self.command_queue = queue.Queue(1000)
        
        with open('device_types.yml', 'r') as stream:
            data = yaml.safe_load(stream)    
    
        self.k_p_z = data[device_type]['k_p_z']
        self.k_d_z = data[device_type]['k_d_z']
        self.k_i_z = data[device_type]['k_i_z']
        
        self.k_p_xy = data[device_type]['k_p_xy']
        self.k_d_xy = data[device_type]['k_d_xy']

        self.tau_att_x = data[device_type]['tau_att_x']
        self.tau_att_y = data[device_type]['tau_att_y']
        self.tau_att_z = data[device_type]['tau_att_z']
        
        self.tau_p = data[device_type]['tau_p']
        self.tau_q = data[device_type]['tau_q']
        self.tau_r = data[device_type]['tau_r']

        self.velocity_filter_RC = 1/(2*np.pi * 0.1)
        self.velocity_filter_constant = self.loop_period / (self.velocity_filter_RC + self.loop_period)
        self.rate_filter_RC = 1/(2*np.pi * 0.1)
        self.rate_filter_constant = self.loop_period / (self.rate_filter_RC + self.loop_period)

        self.t0 = time.time()
        self.last_heartbeat = self.t0
        self.thread = Thread(target=self.run)
        self.run_thread = True
        self.thread.start()

        print("%s of type %s started" % (self.device_name, self.device_type))

    def add_command(self, multi_command):
        for command in multi_command.split(';'):
            command = command.strip()

            if command.split()[0] == 'clear':
                self.clear_commands()

            if command.split()[0]  == 'move':

                x_ref = parseFloat('x', command)
                if x_ref == None:
                    return
                y_ref = parseFloat('y', command)
                if y_ref == None:
                    return
                z_ref = parseFloat('z', command)
                if z_ref == None:
                    return

                try:
                    self.command_queue.put_nowait(command)
                except queue.Full:
                    pass

            if command.split()[0]  == 'freeze':
                self.clear_commands()
                command = 'hold x=%f y=%f z=%f qw=%f qx=%f qy=%f qz=%f' % (self.x, self.y, self.z, self.attitude.w, self.attitude.x, self.attitude.y, self.attitude.z)
                try:
                    self.command_queue.put_nowait(command)
                except queue.Full:
                    print(self.device_name, "Error: command queue full")

            if command.split()[0]  == 'park':
                xf = parseFloat('x', command)
                if xf == None:
                    return
                xf = xf - self.x

                yf = parseFloat('y', command)
                if yf == None:
                    return
                yf = yf - self.y

                zf = parseFloat('z', command)
                if zf == None:
                    return
                zf = zf - self.z

                q = Quaternion()

                qw = parseFloat('qw', command)
                qx = parseFloat('qx', command)
                qy = parseFloat('qy', command)
                qz = parseFloat('qz', command)

                if qw == None or qx == None or qy == None or qz == None:
                    yaw_ref = parseFloat('alpha', command)
                    pitch_ref = parseFloat('beta', command)
                    roll_ref = parseFloat('gamma', command)

                    if roll_ref == None and pitch_ref == None and yaw_ref == None:
                        yaw_ref = parseFloat('yaw', command)
                        pitch_ref = parseFloat('pitch', command)
                        roll_ref = parseFloat('roll', command)

                    if roll_ref != None or pitch_ref != None or yaw_ref != None:
                        if roll_ref == None:
                            roll_ref = 0.0
                        if pitch_ref == None:
                            pitch_ref = 0.0
                        if yaw_ref == None:
                            yaw_ref = 0.0

                        q_X = Quaternion(axis= [1.0, 0.0, 0.0], radians=roll_ref)
                        q_Y = Quaternion(axis= [0.0, 1.0, 0.0], radians=pitch_ref)
                        q_Z = Quaternion(axis= [0.0, 0.0, 1.0], radians=yaw_ref)

                        q = q_Z * q_Y * q_X

                else:
                    q = Quaternion(qw, qx, qy, qz).normalised

                tf = parseFloat('t', command)
                if tf is None:
                    tf = 0

                vmax = parseFloat('vmax', command)
                if vmax is None:
                    vmax = 0

                if vmax > 0:
                    tf = np.abs(xf/vmax*1.8746174)
                    if tf < np.abs(yf/vmax*1.8746174):
                        tf = np.abs(yf/vmax*1.8746174)
                    if tf < np.abs(zf/vmax*1.8746174):
                        tf = np.abs(zf/vmax*1.8746174)

                t, x, dx = self.compute_min_jerk(xf, tf)
                t, y, dy = self.compute_min_jerk(yf, tf)
                t, z, dz = self.compute_min_jerk(zf, tf)

                for i in range(len(t)):
                    command = 'move x=%f y=%f z=%f vx=%f vy=%f vz=%f qw=%f qx=%f qy=%f qz=%f state=parking' % (x[i] + self.x, y[i] + self.y, z[i] + self.z, dx[i], dy[i], dz[i], q.w, q.x, q.y, q.z)
                    try:
                        self.command_queue.put_nowait(command)
                    except queue.Full:
                        print(self.device_name, "Error: command queue full")

                command = 'hold x=%f y=%f z=%f qw=%f qx=%f qy=%f qz=%f' % (x[-1] + self.x, y[-1] + self.y, z[-1] + self.z, q.w, q.x, q.y, q.z)
                try:
                    self.command_queue.put_nowait(command)
                except queue.Full:
                    print(self.device_name, "Error: command queue full")

    def parse_command(self, command):
        if command.split()[0]  == 'move':
            x_ref = parseFloat('x', command)
            y_ref = parseFloat('y', command)
            z_ref = parseFloat('z', command)

            vx_ref = parseFloat('vx', command)
            if vx_ref == None:
                vx_ref = self.vx_ref + self.velocity_filter_constant * ((x_ref - self.x_ref)/self.loop_period - self.vx_ref)

            vy_ref = parseFloat('vy', command)
            if vy_ref == None:
                vy_ref = self.vy_ref + self.velocity_filter_constant * ((y_ref - self.y_ref)/self.loop_period - self.vy_ref)

            vz_ref = parseFloat('vz', command)
            if vz_ref == None:
                vz_ref = self.vz_ref + self.velocity_filter_constant * ((z_ref - self.z_ref)/self.loop_period - self.vz_ref)

            attitude_ref = Quaternion()

            qw = parseFloat('qw', command)
            qx = parseFloat('qx', command)
            qy = parseFloat('qy', command)
            qz = parseFloat('qz', command)

            if qw == None or qx == None or qy == None or qz == None:
                yaw_ref = parseFloat('alpha', command)
                pitch_ref = parseFloat('beta', command)
                roll_ref = parseFloat('gamma', command)

                if roll_ref == None and pitch_ref == None and yaw_ref == None:
                    yaw_ref = parseFloat('yaw', command)
                    pitch_ref = parseFloat('pitch', command)
                    roll_ref = parseFloat('roll', command)

                if roll_ref != None or pitch_ref != None or yaw_ref != None:
                    if roll_ref == None:
                        roll_ref = 0.0
                    if pitch_ref == None:
                        pitch_ref = 0.0
                    if yaw_ref == None:
                        yaw_ref = 0.0

                    q_X = Quaternion(axis= [1.0, 0.0, 0.0], radians=roll_ref)
                    q_Y = Quaternion(axis= [0.0, 1.0, 0.0], radians=pitch_ref)
                    q_Z = Quaternion(axis= [0.0, 0.0, 1.0], radians=yaw_ref)

                    attitude_ref = q_Z * q_Y * q_X

            else:
                attitude_ref = Quaternion(qw, qx, qy, qz).normalised

            self.set_reference(x_ref, y_ref, z_ref, vx_ref, vy_ref, vz_ref, attitude_ref)

            state = parseString('state', command)
            if state == State.parking.name:
                return State.parking
            else:
                return State.moving

        if command.split()[0]  == 'hold':
            x_ref = parseFloat('x', command)
            y_ref = parseFloat('y', command)
            z_ref = parseFloat('z', command)
            qw = parseFloat('qw', command)
            qx = parseFloat('qx', command)
            qy = parseFloat('qy', command)
            qz = parseFloat('qz', command)

            attitude_ref = Quaternion(qw, qx, qy, qz)

            self.set_reference(x_ref, y_ref, z_ref, 0.0, 0.0, 0.0, attitude_ref)

            try:
                self.command_queue.put_nowait(command)
            except queue.Full:
                print(self.device_name, "Error: command queue full")

            return State.holding

    def clear_commands(self):
        while not self.command_queue.empty():
            try:
                self.command_queue.get(False)
            except queue.Empty:
                continue
            self.command_queue.task_done()

    def stack(self, client, userdata, msg):
        del client, userdata

        self.add_command(msg.payload.decode())

    def config(self, client, userdata, msg):
        del client, userdata

        command = msg.payload.decode()

        ms = re.search("k_p_z=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_p_z = float(command[ms.span()[0]+6:ms.span()[1]])

        ms = re.search("k_d_z=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_d_z = float(command[ms.span()[0]+6:ms.span()[1]])

        ms = re.search("k_i_z=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_i_z = float(command[ms.span()[0]+6:ms.span()[1]])

        ms = re.search("k_p_xy=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_p_xy = float(command[ms.span()[0]+7:ms.span()[1]])

        ms = re.search("k_d_xy=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_d_xy = float(command[ms.span()[0]+7:ms.span()[1]])
        
        self.tau_p = parseFloat('tau_p', command)
        self.tau_q = parseFloat('tau_q', command)
        self.tau_r = parseFloat('tau_r', command)
        self.tau_att_x = parseFloat('tau_att_x', command)
        self.tau_att_y = parseFloat('tau_att_y', command)
        self.tau_att_z = parseFloat('tau_att_z', command)

    def model(self, client, userdata, msg):
        del client, userdata

        command = msg.payload.decode()

        seg = command.split(",")
        self.m1 = float(seg[0])
        self.m2 = float(seg[1])
        self.m3 = float(seg[2])
        self.m4 = float(seg[3])
        self.m5 = float(seg[4])
        self.m6 = float(seg[5])

    def battery(self, client, userdata, msg):
        del client, userdata

        command = msg.payload.decode()

        seg = command.split(",")
        self.battery_charge = float(seg[0])
        self.last_heartbeat = time.time()

    def clear(self, client, userdata, msg):
        del client, userdata, msg

        self.clear_commands()

    def run_state_machine(self):
        if time.time() - self.last_heartbeat > 2.0:
            self.state = State.unavailable
            print(self.device_name, "Error: Not available")
            return

        if self.tracked == False:
            self.state = State.untracked
            print(self.device_name, "Error: Not tracked")
            return

        self.state = State.ready

    def run(self):
        while self.run_thread:
            self.run_state_machine()
            
            if self.state == State.untracked or self.state == State.unavailable:
                self.turn_off()
            else:
                try:
                    command = self.command_queue.get(block=False)
                    self.state = self.parse_command(command)
                    self.empty_count = 0
                except queue.Empty:
                    self.empty_count = self.empty_count + 1
                    print(self.device_name, "Warning: Queue empty Count: ", self.empty_count)
                    if self.empty_count > 10:
                        self.state = State.ready
                        self.turn_off()

                if self.state.value > State.ready.value:
                    #print(self.device_name, "run")
                    #print(self.device_name, "Command: ", command)
                    self.control()

            command = self.state.name
            self.client.publish(self.manager_base_topic + '/' + self.device_base_topic + '/' + self.device_name + '/state', command, 0, False)
            command = "x=%f y=%f z=%f vx=%f vy=%f vz=%f " % (self.x, self.y, self.z, self.vx, self.vy, self.vz)
            command = command + "x_ref=%f y_ref=%f z_ref=%f vx_ref=%f vy_ref=%f vz_ref=%f " % (self.x_ref, self.y_ref, self.z_ref, self.vx_ref, self.vy_ref, self.vz_ref)
            command = command + "qw=%f qx=%f qy=%f qz=%f p=%f q=%f r=%f " % (self.attitude.w, self.attitude.x, self.attitude.y, self.attitude.z, self.p, self.q, self.r)
            command = command + "qw_ref=%f qx_ref=%f qy_ref=%f qz_ref=%f p_ref=%f q_ref=%f r_ref=%f " % (self.attitude_ref.w, self.attitude_ref.x, self.attitude_ref.y, self.attitude_ref.z, self.p_ref, self.q_ref, self.r_ref)
            command = command + "fx=%f fy=%f fz=%f mx=%f my=%f mz=%f " % (self.fx, self.fy, self.fz, self.mx, self.my, self.mz)
            command = command + "m1=%f m2=%f m3=%f m4=%f m5=%f m6=%f " % (self.m1, self.m2, self.m3, self.m4, self.m5, self.m6)
            command = command + "missed_ticks=%d state=%s queue_size=%d " % (self.missed_ticks, self.state.name, self.command_queue.qsize())
            command = command + "k_p_xy=%f k_d_xy=%f k_p_z=%f k_d_z=%f k_i_z=%f " % (self.k_p_xy, self.k_d_xy, self.k_p_z, self.k_d_z, self.k_i_z)
            command = command + "tau_att_x=%f tau_att_y=%f tau_att_z=%f tau_p=%f tau_q=%f tau_r=%f " % (self.tau_att_x, self.tau_att_y, self.tau_att_z, self.tau_p, self.tau_q, self.tau_r)
            command = command + "battery_charge=%f" % (self.battery_charge)
            self.client.publish(self.manager_base_topic + '/' + self.device_base_topic + '/' + self.device_name + '/feedback', command, 0, False)

            t = time.time() - self.t0
            self.loop_count = self.loop_count + 1

            if self.loop_count * self.loop_period - t < 0:
                self.missed_ticks = self.missed_ticks + 1

            #print(self.device_name, "State: ", self.state.name, "Missed ticks: ", self.missed_ticks, "Queue size: ", self.command_queue.qsize())

            time.sleep(max(0.0, self.loop_count * self.loop_period - t))

        print(self.device_name, "stopped")

    def stop(self):
        self.run_thread = False
        self.thread.join()
        self.turn_off()
        self.state = State.unmanaged
        command = self.state.name
        mi = self.client.publish(self.manager_base_topic + '/' + self.device_base_topic + '/' + self.device_name + '/state', command, 0, False)
        mi.wait_for_publish()

    def set_reference(self, x_ref, y_ref, z_ref, vx_ref, vy_ref, vz_ref, attitude_ref):
        self.x_ref = x_ref
        self.y_ref = y_ref
        self.z_ref = z_ref
        self.vx_ref = vx_ref
        self.vy_ref = vy_ref
        self.vz_ref = vz_ref
        self.attitude_ref = attitude_ref

    def turn_off(self):
        command = "0,0,0,0,0,0"
        mi = self.client.publish(str(self.device_base_topic) + "/" + str(self.device_name) + "/motors", command, 0, False)
        mi.wait_for_publish()
        command = "0.0,0.0,0.0,0.0,0.0,0.0"
        mi = self.client.publish(str(self.device_base_topic) + "/" + str(self.device_name) + "/forces", command, 0, False)
        mi.wait_for_publish()

    def control(self):
        
        # xy control 
        f_x = (self.x_ref - self.x) * self.k_p_xy + (self.vx_ref - self.vx) * self.k_d_xy
        f_y = (self.y_ref - self.y) * self.k_p_xy + (self.vy_ref - self.vy) * self.k_d_xy

        # altitude control
        e_z = (self.z_ref - self.z)

        if (np.abs(self.i_e_z + e_z * self.loop_period) < self.max_i_e):
            self.i_e_z = e_z * self.loop_period + self.i_e_z

        f_z = e_z * self.k_p_z + (self.vz_ref - self.vz) * self.k_d_z + self.i_e_z * self.k_i_z
        
        R_IB = np.array(self.attitude.rotation_matrix)

        c = np.dot(R_IB.T, np.array([f_x, f_y, f_z]))
        
        c_x_body = c[0]
        c_y_body = c[1]
        c_z_body = c[2]
        
        if np.abs(c_x_body) > self.max_command:
            c_x_body = np.sign(c_x_body) * self.max_command

        if np.abs(c_y_body) > self.max_command:
            c_y_body = np.sign(c_y_body) * self.max_command

        if np.abs(c_z_body) > self.max_command:
            c_z_body = np.sign(c_z_body) * self.max_command

        self.fx = c_x_body
        self.fy = c_y_body
        self.fz = c_z_body
        
        # attitude control
        attitude_err = self.attitude.inverse * self.attitude_ref

        if attitude_err.w < 0:
            attitude_err = -attitude_err
        
        omega_cmd = 2 * np.sign(attitude_err.w) * np.array([attitude_err.x, attitude_err.y, attitude_err.z])
        omega_cmd[0] = omega_cmd[0] / self.tau_att_x
        omega_cmd[1] = omega_cmd[1] / self.tau_att_y
        omega_cmd[2] = omega_cmd[2] / self.tau_att_z
        
        self.p_ref = omega_cmd[0]
        self.q_ref = omega_cmd[1]
        self.r_ref = omega_cmd[2]
        omega = np.array([self.p, self.q, self.r])
        
        t = np.dot(self.J, np.array([omega_cmd[0], omega_cmd[1], omega_cmd[2]]) - omega)

        t[0] = t[0] / self.tau_p
        t[1] = t[1] / self.tau_q
        t[2] = t[2] / self.tau_r
        
        t = t + np.dot(np.cross(omega, self.J), omega)
        
        c_roll_body = t[0]
        c_pitch_body = t[1]
        c_yaw_body = t[2]

        if np.abs(c_roll_body) > self.max_command:
            c_roll_body = np.sign(c_roll_body) * self.max_command

        if np.abs(c_pitch_body) > self.max_command:
            c_pitch_body = np.sign(c_pitch_body) * self.max_command
        
        if np.abs(c_yaw_body) > self.max_command:
            c_yaw_body = np.sign(c_yaw_body) * self.max_command

        self.mx = c_roll_body
        self.my = c_pitch_body
        self.mz = c_yaw_body

        command = '{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f}'.format(c_x_body, c_y_body, c_z_body, c_roll_body, c_pitch_body, c_yaw_body)
        self.client.publish(str(self.device_base_topic) + "/" + str(self.device_name) + "/forces", command, 0, False)
        #print("Command: fx: %.3f fy: %.3f fz: %.3f mx: %.3f my: %.3f mz: %.3f" % (c_x_body, c_y_body, c_z_body, 0, 0, c_yaw_body))
        #print("State: x: %.3f/%.3f y: %.3f/%.3f z: %.3f/%.3f alpha: %.3f/%.3f" % (self.x, self.x_ref, self.y, self.y_ref, self.z, self.z_ref, self.alpha, self.alpha_ref))

    def set_position(self, x, y, z):
        t_now = time.time()
        dt = t_now - self.t_pos
        self.t_pos = t_now

        if dt >= 0.008 and dt < 0.1:
            self.vx = self.vx + self.velocity_filter_constant * ((x - self.x)/dt - self.vx)
            self.vy = self.vy + self.velocity_filter_constant * ((y - self.y)/dt - self.vy)
            self.vz = self.vz + self.velocity_filter_constant * ((z - self.z)/dt - self.vz)

        self.x = x
        self.y = y
        self.z = z

    def set_attitude(self, qx, qy, qz, qw):
        t_now = time.time()
        dt = t_now - self.t_att
        self.t_att = t_now

        ypr = quaternion_to_euler(qw, qx, qy, qz)
        self.roll = ypr[2]
        self.pitch = ypr[1]
        self.yaw = ypr[0]

        attitude = Quaternion(qw, qx, qy, qz)

        if dt >= 0.008 and dt < 0.1:
            self.attitude_rate = (attitude - self.attitude) / dt

        self.attitude = attitude

        omega = 2 * self.attitude.inverse * self.attitude_rate
        self.p = self.p + self.rate_filter_constant * (omega[1] - self.p)
        self.q = self.q + self.rate_filter_constant * (omega[2] - self.q)
        self.r = self.r + self.rate_filter_constant * (omega[3] - self.r)

    def set_track(self, tracked):
        self.tracked = tracked

    def compute_min_jerk(self, xf, tf):
        A = np.array([[tf**3, tf**4, tf**5],
                      [3*tf**2, 4*tf**3, 5*tf**4],
                      [6*tf, 12*tf**2, 20*tf**3]])

        B = np.array([[xf, 0, 0]]).T

        a = np.dot(np.linalg.inv(A), B)
        a3 = a[0]
        a4 = a[1]
        a5 = a[2]
        N = tf/self.loop_period
        t = np.linspace(0, tf, int(N))
        x = a3*t**3 + a4*t**4 + a5*t**5
        dx = 3*a3*t**2 + 4*a4*t**3 + 5*a5*t**4

        return t, x, dx

def stop_device(device_name):
    global devices

    for device in devices:
        if device.device_name == device_name:
            device.stop()
            devices.remove(device)

def add_device(client, userdata, msg):
    del userdata

    global devices

    message = msg.payload.decode()

    device_base_topic = parseString("device_base_topic", message)
    if device_base_topic is None:
        return

    device_type = parseString("device_type", message)
    if device_type is None:
        return

    device_name = parseString("device_name", message)
    if device_name is None:
        return

    for device in devices:
        if device.device_name == device_name:
            print("Error: device with name '%s' exists" % device_name)
            return

    tracking_id = parseString("tracking_id", message)
    if tracking_id is None:
        return

    for device in devices:
        if device.tracking_id == tracking_id:
            print("Error: device with tracking_id '%s' exists" % tracking_id)
            return

    try:
        devices.append(Device(device_loop_period, client, manager_base_topic, device_base_topic, device_type, device_name, tracking_id))
    except:
        print("Error: device type unknown")

def remove_device(client, userdata, msg):
    del client, userdata

    global devices

    message = msg.payload.decode()

    device_name = parseString("device_name", message)
    if device_name is None:
        return

    thread = Thread(target=stop_device, args=(device_name,))
    thread.start()

def main(mqtt_host, mqtt_port, osc_server, osc_port, manager_base_topic):
    global devices

    client = mqtt.Client(client_id=manager_base_topic)
    client.username_pw_set(username="testtest",password="testtest")
    client.connect(mqtt_host, mqtt_port, 60)
    print("MQTT client connected to %s on port %d" % (mqtt_host, mqtt_port))
    client.loop_start()

    client.message_callback_add(manager_base_topic + "/add", add_device)
    client.subscribe(manager_base_topic + "/add")

    client.message_callback_add(manager_base_topic + "/remove", remove_device)
    client.subscribe(manager_base_topic + "/remove")

    osc_startup()
    osc_udp_server(osc_server, osc_port, "aservername")
    print("OSC server connected to %s on port %d" % (osc_server, osc_port))

    t0 = time.time()
    count = 1
    main_loop_period = 0.001
    while run:
        osc_process()
        t = time.time() - t0
        if count % 100 == 0:
            client.publish(manager_base_topic + "/heartbeat")
        count = count + 1
        time.sleep(max(0, count * main_loop_period - t))

    for device in devices:
        device.stop()

    client.disconnect()
    osc_terminate()
    print("Exit")

devices = []
device_loop_period = 0.1
run = True
mqtt_host = "localhost"
mqtt_port = 1883
osc_server = "localhost"
osc_port = 54321
manager_base_topic = "manager"

if __name__ == "__main__":
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:],"h",["mqtt_host=","mqtt_port=","osc_server=","osc_port=", "manager_base_topic="])
    except getopt.GetoptError:
        print ('manager.py --mqtt_host <host> --mqtt_port <port> --osc_server <server> --osc_port <port> --manager_base_topic <topic>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('manager.py --mqtt_host <host> --mqtt_port <port> --osc_server <server> --osc_port <port> --manager_base_topic <topic>')
            sys.exit()
        elif opt in ("--mqtt_host"):
            mqtt_host = arg
            print(arg)
        elif opt in ("--mqtt_port"):
            mqtt_port = int(arg)
        elif opt in ("--osc_server"):
            osc_server = arg
        elif opt in ("--osc_port"):
            osc_port = int(arg)
        elif opt in ("--manager_base_topic"):
            manager_base_topic = arg

    main(mqtt_host, mqtt_port, osc_server, osc_port, manager_base_topic)
