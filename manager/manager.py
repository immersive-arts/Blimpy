import paho.mqtt.client as mqtt
import time
import re
from threading import Event, Thread
import signal as sig
import math
import queue
import numpy as np
from osc4py3.as_comthreads import osc_method, osc_udp_server, osc_startup, osc_process, osc_terminate
from scipy import signal
import sys, getopt

def signal_handler(sig, frame):
    global run
    run = False
    
sig.signal(sig.SIGINT, signal_handler)
        
class Blimp:
    count = 0
    t0 = 0
    run_thread = False
    
    x = 0
    y = 0
    z = 0
    dx = 0
    dy = 0
    dz = 0
    alpha = 0
    dalpha = 0

    x_ref = 0
    y_ref = 0
    z_ref = 0
    dx_ref = 0
    dy_ref = 0
    dz_ref = 0
    alpha_ref = 0
    dalpha_ref = 0

    i_e_z = 0
    c_x_filt = 0
    c_y_filt = 0
    c_z_filt = 0
    c_a_filt = 0

    k_p_z = 2.0
    k_d_z = 1.5
    k_i_z = 0.2

    k_p_xy = 0.8
    k_d_xy = 0.2

    k_p_a = 0.8
    k_d_a = 2.0

    max_command = 1.0
    max_i_e = 3.0

    time_const = 0.1

    t_pos = 0
    t_att = 0
    b, a = signal.butter(3, 5.0, fs = 120.0)

    tracked = False

    def __init__(self, dt, client, manager_base_topic, blimp_base_topic, blimp_id, tracking_id):

        self.tracking_id = tracking_id
        self.blimp_id = blimp_id
        self.manager_base_topic = manager_base_topic
        self.blimp_base_topic = blimp_base_topic
        self.dt = dt
        self.client = client
        self.client.message_callback_add(str(self.manager_base_topic) + str(self.blimp_id) + "/stack", self.stack)
        self.client.message_callback_add(str(self.manager_base_topic) + str(self.blimp_id) + "/clear", self.clear)
        self.client.subscribe(str(self.manager_base_topic) + str(self.blimp_id) + "/stack")
        self.client.subscribe(str(self.manager_base_topic) + str(self.blimp_id) + "/clear")
        
        osc_method("/rigidbody/" + str(self.tracking_id) + "/tracked", self.set_track)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/quat", self.set_attitude)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/position", self.set_position)

        zi = signal.lfilter_zi(self.b, self.a)
        _, self.dx_zf = signal.lfilter(self.b, self.a, [self.dx], zi=zi*self.dx)
        _, self.dy_zf = signal.lfilter(self.b, self.a, [self.dy], zi=zi*self.dy)
        _, self.dz_zf = signal.lfilter(self.b, self.a, [self.dz], zi=zi*self.dz)
        _, self.dalpha_zf = signal.lfilter(self.b, self.a, [self.dalpha], zi=zi*self.dalpha)

        self.command_queue = queue.Queue(10)
        self.t0 = time.time()
        self.event = Event()
        self.thread = Thread(target=self.run)
        self.run_thread = True
        self.thread.start()
        
        self.filt_const = dt / (dt + self.time_const)

        print("%s started" % self.blimp_id)

    def parse_command(self, payload):
        command = payload.decode()
        if command.split()[0]  == 'move':
            
            ms = re.search("x=[-+]?\d*\.\d+", command)
            if ms is None:
                return

            ms = re.search("y=[-+]?\d*\.\d+", command)
            if ms is None:
                return

            ms = re.search("z=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            try:
                self.command_queue.put_nowait(command)
            except queue.Full:
                pass

    def stack(self, client, userdata, msg):
        self.parse_command(msg.payload)
        self.event.set()
 
    def clear(self, client, userdata, msg):
        self.command_stack = []
        self.event.clear()
        
        while not self.command_queue.empty():
            try:
                self.command_queue.get(False)
            except queue.Empty:
                continue
            self.command_queue.task_done()
        
    def run(self):
        while self.run_thread:
            print(self.blimp_id, "Drone wait")
            try:
                command = self.command_queue.get(block=True, timeout=1.0)
                print(self.blimp_id, "Drone run", time.time() - self.t0, self.command_queue.qsize())
                print(self.blimp_id, command)
                ms = re.search("x=[-+]?\d*\.\d+", command)
                x_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                ms = re.search("y=[-+]?\d*\.\d+", command)
                y_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                ms = re.search("z=[-+]?\d*\.\d+", command)
                z_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                ms = re.search("vx=[-+]?\d*\.\d+", command)
                if ms is not None:
                    dx_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                else:
                    dx_ref = 0
                ms = re.search("vy=[-+]?\d*\.\d+", command)
                if ms is not None:
                    dy_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                else:
                    dy_ref = 0
                ms = re.search("vz=[-+]?\d*\.\d+", command)
                if ms is not None:
                    dz_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                else:
                    dz_ref = 0

                ms = re.search("alpha=[-+]?\d*\.\d+", command)
                if ms is not None:
                    alpha_ref = float(command[ms.span()[0]+6:ms.span()[1]])
                else:
                    alpha_ref = 0

                self.set_reference(x_ref, y_ref, z_ref, alpha_ref, dx_ref, dy_ref, dz_ref, 0.0)

                if self.tracked:
                    self.control()
                else:
                    print("Not tracked")
                    self.turn_off()
                
                t = time.time() - self.t0                
                count = math.ceil(t / self.dt)    
                time.sleep(max(0, (count)*self.dt - t))
            except queue.Empty:
                print(self.blimp_id, "Queue empty")
                self.turn_off()
                continue
            
        print(self.blimp_id, "Drone done")
    
    def stop(self):
        self.run_thread = False
        self.thread.join()
        self.turn_off()
        print("%s stopped" % self.blimp_id)

    def set_state(self, x, y, z, alpha, dx, dy, dz, dalpha):
        self.x = x
        self.y = y
        self.z = z
        self.dx = dx
        self.dy = dy
        self.dz = dz
        self.alpha = alpha
        self.dalpha = dalpha

    def set_reference(self, x_ref, y_ref, z_ref, alpha_ref, dx_ref, dy_ref, dz_ref, dalpha_ref):
        self.x_ref = x_ref
        self.y_ref = y_ref
        self.z_ref = z_ref
        self.alpha_ref = alpha_ref
        self.dx_ref = dx_ref
        self.dy_ref = dy_ref
        self.dz_ref = dz_ref
        self.dalpha_ref = dalpha_ref

    def turn_off(self):
        command = "0,0,0,0,0,0"
        mi = self.client.publish(str(self.blimp_base_topic) + str(self.blimp_id) + "/motors", command, 0, False)
        mi.wait_for_publish()

    def control(self):
        c_x = (self.x_ref - self.x) * self.k_p_xy - (self.dx_ref - self.dx) * self.k_d_xy
        self.c_x_filt = self.filt_const * c_x + (1 - self.filt_const) * self.c_x_filt

        c_y = (self.y_ref - self.y) * self.k_p_xy - (self.dy_ref - self.dy) *self.k_d_xy
        self.c_y_filt = self.filt_const * c_y + (1 - self.filt_const) * self.c_y_filt

        # altitude control
        e_z = (self.z_ref - self.z)

        if (np.abs(self.i_e_z + e_z*dt) < self.max_i_e):
            self.i_e_z = e_z*dt + self.i_e_z

        c_z = e_z * self.k_p_z - self.dz * self.k_d_z + self.i_e_z * self.k_i_z
        self.c_z_filt = self.filt_const * c_z + (1 - self.filt_const) * self.c_z_filt

        # heading control
        e_alpha = self.alpha_ref - self.alpha
        if e_alpha > np.pi:
            e_alpha = e_alpha - 2 * np.pi
        elif e_alpha < -np.pi:
            e_alpha = e_alpha + 2 * np.pi

        c_a = e_alpha * self.k_p_a - self.dalpha * self.k_d_a
        self.c_a_filt = self.filt_const * c_a + (1 - self.filt_const) * self.c_a_filt

        # send commands
        c_x_body = np.cos(self.alpha) * self.c_x_filt + np.sin(self.alpha) * self.c_y_filt
        c_y_body = -np.sin(self.alpha) * self.c_x_filt + np.cos(self.alpha) * self.c_y_filt
        c_z_body = self.c_z_filt
        c_a_body = self.c_a_filt

        if np.abs(c_x_body) > self.max_command:
            c_x_body = np.sign(c_x_body) * self.max_command

        if np.abs(c_y_body) > self.max_command:
            c_y_body = np.sign(c_y_body) * self.max_command

        if np.abs(c_z_body) > self.max_command:
            c_z_body = np.sign(c_z_body) * self.max_command

        if np.abs(c_a_body) > self.max_command:
            c_a_body = np.sign(c_a_body) * self.max_command

        command = '{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f}'.format(c_x_body, c_y_body, c_z_body, 0.0, 0.0, c_a_body)
        self.client.publish(str(self.blimp_base_topic) + str(self.blimp_id) + "/forces", command, 0, False)
        print("Command: fx: %.3f fy: %.3f fz: %.3f mx: %.3f my: %.3f mz: %.3f" % (c_x_body, c_y_body, c_z_body, 0, 0, c_a_body))
        print("State: x: %.3f/%.3f y: %.3f/%.3f z: %.3f/%.3f alpha: %.3f/%.3f" % (self.x, self.x_ref, self.y, self.y_ref, self.z, self.z_ref, self.alpha, self.alpha_ref))

    def set_position(self, x, y, z):
        t_now = time.time()
        dt = t_now - self.t_pos
        self.t_pos = t_now

        if dt >= 0.008 and dt < 0.1:
            dx_filt, self.dx_zf = signal.lfilter(self.b, self.a, [(x - self.x)/dt], zi = self.dx_zf)
            dy_filt, self.dy_zf = signal.lfilter(self.b, self.a, [(y - self.y)/dt], zi = self.dy_zf)
            dz_filt, self.dz_zf = signal.lfilter(self.b, self.a, [(z - self.z)/dt], zi = self.dz_zf)

            self.dx = dx_filt[0]
            self.dy = dy_filt[0]
            self.dz = dz_filt[0]

        self.x = x
        self.y = y
        self.z = z

    def set_attitude(self, qx, qy, qz, qw):
        t_now = time.time()
        dt = t_now - self.t_att
        self.t_att = t_now

        siny_cosp = 2.0 * (qw * qz + qx * qy)
        cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
        alpha = np.arctan2(siny_cosp, cosy_cosp)

        if dt >= 0.008 and dt < 0.1:
            dalpha = alpha - self.alpha
            if dalpha > np.pi:
                dalpha = dalpha - 2 * np.pi
            elif dalpha < -np.pi:
                dalpha = dalpha + 2 * np.pi

            dalpha_filt, self.dalpha_zf = signal.lfilter(self.b, self.a, [dalpha / dt], zi = self.dalpha_zf)
            self.dalpha = dalpha_filt[0]

        self.alpha = alpha

    def set_track(self, tracked):
        self.tracked = tracked

def stop_blimp(blimp_id):
    global blimps

    for blimp in blimps:
        if blimp.blimp_id == blimp_id:
            blimp.stop()
            blimps.remove(blimp)

def add_blimp(client, userdata, msg):
    global blimps, manager_base_topic
    command = msg.payload.decode()

    ms = re.search("manager_base_topic=[a-zA-Z0-9/]*/", command)
    if ms is None:
        return

    manager_base_topic = command[ms.span()[0]+19:ms.span()[1]]

    ms = re.search("blimps_base_topic=[a-zA-Z0-9/]*/", command)
    if ms is None:
        return

    blimps_base_topic = command[ms.span()[0]+18:ms.span()[1]]

    ms = re.search("blimp_id=[a-zA-Z0-9]*", command)
    if ms is None:
        return

    blimp_id = command[ms.span()[0]+9:ms.span()[1]]

    ms = re.search("tracking_id=[0-9]*", command)
    if ms is None:
        return

    tracking_id = command[ms.span()[0]+12:ms.span()[1]]

    blimps.append(Blimp(dt, client, manager_base_topic, blimps_base_topic, blimp_id, tracking_id))

def remove_blimp(client, userdata, msg):
    global blimps
    command = msg.payload.decode()

    ms = re.search("blimp_id=[a-zA-Z0-9]*", command)
    if ms is None:
        return
    blimp_id = command[ms.span()[0]+9:ms.span()[1]]

    thread = Thread(target=stop_blimp, args=(blimp_id,))
    thread.start()

def main(mqtt_host, mqtt_port, osc_server, osc_port, base_topic):
    global blimps

    client = mqtt.Client(client_id=base_topic)
    client.username_pw_set(username="testtest",password="testtest")
    client.connect(mqtt_host, mqtt_port, 60)
    print("MQTT client connected to %s on port %d" % (mqtt_host, mqtt_port))
    client.loop_start()

    client.message_callback_add(base_topic + "/add", add_blimp)
    client.subscribe(base_topic + "/add")

    client.message_callback_add(base_topic + "/remove", remove_blimp)
    client.subscribe(base_topic + "/remove")

    osc_startup()
    osc_udp_server(osc_server, osc_port, "aservername")
    print("OSC server connected to %s on port %d" % (osc_server, osc_port))

    t0 = time.time()

    main_dt = 0.001
    while run:
        osc_process()
        t = time.time() - t0
        count = math.ceil(t / main_dt)
        time.sleep(max(0, count * main_dt - t))

    for blimp in blimps:
        blimp.stop()

    client.disconnect()
    osc_terminate()
    print("Exit")

blimps = []
dt = 0.1
run = True
#mqtt_host = "10.128.96.102"
#mqtt_port = 1883
#osc_server = "10.128.96.176"
#osc_port = 54321
#manager_base_topic = "manager"

if __name__ == "__main__":
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:],"h",["mqtt_host=","mqtt_port=","osc_server=","osc_port=", "base_topic="])
        print(opts, args)
    except getopt.GetoptError:
        print ('manager.py --mqtt_host <host> --mqtt_port <port> --osc_server <server> --osc_port <port>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('manager.py --mqtt_host <host> --mqtt_port <port> --osc_server <server> --osc_port <port>')
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
        elif opt in ("--base_topic"):
            base_topic = arg

    main(mqtt_host, mqtt_port, osc_server, osc_port, base_topic)
