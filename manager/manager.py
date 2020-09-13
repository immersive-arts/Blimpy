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
import socket
import struct

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
    vx = 0
    vy = 0
    vz = 0
    alpha = 0
    valpha = 0

    x_ref = 0
    y_ref = 0
    z_ref = 0
    vx_ref = 0
    vy_ref = 0
    vz_ref = 0
    alpha_ref = 0
    valpha_ref = 0

    fx = 0
    fy = 0
    fz = 0
    malpha = 0

    i_e_z = 0
    c_x_filt = 0
    c_y_filt = 0
    c_z_filt = 0
    c_a_filt = 0

    k_p_z = 2.0
    k_d_z = 2.0
    k_i_z = 0.2

    k_p_xy = 1.0
    k_d_xy = 0.4

    k_p_a = 1.0
    k_d_a = 1.0

    m1 = 0.0
    m2 = 0.0
    m3 = 0.0
    m4 = 0.0
    m5 = 0.0
    m6 = 0.0

    max_command = 1.0
    max_i_e = 3.0

    time_const = 0.1

    t_pos = 0
    t_att = 0
    b, a = signal.butter(3, 5.0, fs = 120.0)

    tracked = False

    def __init__(self, dt, client, base_topic, blimp_base_topic, blimp_id, tracking_id):

        self.tracking_id = tracking_id
        self.blimp_id = blimp_id
        self.blimp_base_topic = blimp_base_topic
        self.dt = dt
        self.client = client
        self.client.message_callback_add(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/stack", self.stack)
        self.client.message_callback_add(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/clear", self.clear)
        self.client.message_callback_add(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/config", self.config)
        self.client.message_callback_add( str(blimp_base_topic) + "/" + str(self.blimp_id) + "/model", self.model)
        self.client.subscribe(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/stack")
        self.client.subscribe(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/clear")
        self.client.subscribe(str(base_topic) + "/" + str(blimp_base_topic) + "/" + str(self.blimp_id) + "/config")
        self.client.subscribe(str(blimp_base_topic) + "/" + str(self.blimp_id) + "/model")

        osc_method("/rigidbody/" + str(self.tracking_id) + "/tracked", self.set_track)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/quat", self.set_attitude)
        osc_method("/rigidbody/" + str(self.tracking_id) + "/position", self.set_position)

        zi = signal.lfilter_zi(self.b, self.a)
        _, self.vx_zf = signal.lfilter(self.b, self.a, [self.vx], zi=zi*self.vy)
        _, self.vy_zf = signal.lfilter(self.b, self.a, [self.vy], zi=zi*self.vy)
        _, self.vz_zf = signal.lfilter(self.b, self.a, [self.vz], zi=zi*self.vz)
        _, self.valpha_zf = signal.lfilter(self.b, self.a, [self.valpha], zi=zi*self.valpha)

        self.command_queue = queue.Queue(1000)
        self.t0 = time.time()
        self.event = Event()
        self.thread = Thread(target=self.run)
        self.run_thread = True
        self.thread.start()

        self.filt_const = dt / (dt + self.time_const)

        self.group = '224.1.1.1'
        self.port = 7000 + int(self.tracking_id)
        MULTICAST_TTL = 2

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)

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

        if command.split()[0]  == 'hold':

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

        if command.split()[0]  == 'goto':
            ms = re.search("x=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            xf = float(command[ms.span()[0]+2:ms.span()[1]])
            xf = xf - self.x

            ms = re.search("y=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            yf = float(command[ms.span()[0]+2:ms.span()[1]])
            yf = yf - self.y

            ms = re.search("z=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            zf = float(command[ms.span()[0]+2:ms.span()[1]])
            zf = zf - self.z

            ms = re.search("t=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            tf = float(command[ms.span()[0]+2:ms.span()[1]])

            ms = re.search("alpha=[-+]?\d*\.\d+", command)
            if ms is None:
                return
            af = float(command[ms.span()[0]+6:ms.span()[1]])

            t, x, dx = self.compute_min_jerk(xf, tf)
            t, y, dy = self.compute_min_jerk(yf, tf)
            t, z, dz = self.compute_min_jerk(zf, tf)
            for i in range(len(t)):
                command = 'move x=%f y=%f z=%f vx=%f vy=%f vz=%f alpha=%f' % (x[i] + self.x, y[i] + self.y, z[i] + self.z, dx[i], dy[i], dz[i], af)
                self.command_queue.put_nowait(command)

            try:
                self.command_queue.put_nowait(command)
            except queue.Full:
                pass

    def stack(self, client, userdata, msg):
        self.parse_command(msg.payload)
        self.event.set()

    def config(self, client, userdata, msg):
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

        ms = re.search("k_p_a=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_p_a = float(command[ms.span()[0]+6:ms.span()[1]])

        ms = re.search("k_d_a=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_d_a = float(command[ms.span()[0]+6:ms.span()[1]])
        ms = re.search("k_p_xy=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_p_xy = float(command[ms.span()[0]+7:ms.span()[1]])

        ms = re.search("k_d_xy=[-+]?\d*\.\d+", command)
        if ms is None:
            return
        self.k_d_xy = float(command[ms.span()[0]+7:ms.span()[1]])

    def model(self, client, userdata, msg):
        command = msg.payload.decode()

        seg = command.split(",")
        self.m1 = float(seg[0])
        self.m2 = float(seg[1])
        self.m3 = float(seg[2])
        self.m4 = float(seg[3])
        self.m5 = float(seg[4])
        self.m6 = float(seg[5])

        return

        ms = re.search("(.+?)(?:,|$)", command)
        self.m1 = float(ms.group()[:ms.span()[1]-1])
        command = command[ms.span()[1]:]
        ms = re.search("(.+?)(?:,|$)", command)
        self.m2 = float(ms.group()[:ms.span()[1]-1])
        command = command[ms.span()[1]:]
        ms = re.search("(.+?)(?:,|$)", command)
        self.m3 = float(ms.group()[:ms.span()[1]-1])
        command = command[ms.span()[1]:]
        ms = re.search("(.+?)(?:,|$)", command)
        self.m4 = float(ms.group()[:ms.span()[1]-1])
        command = command[ms.span()[1]:]
        ms = re.search("(.+?)(?:,|$)", command)
        self.m5 = float(ms.group()[:ms.span()[1]-1])
        command = command[ms.span()[1]:]
        ms = re.search("(.+?)(?:,|$)", command)
        self.m6 = float(ms.group()[:ms.span()[1]-1])

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
                if command.split()[0]  == 'move' or command.split()[0] == 'hold':
                    ms = re.search("x=[-+]?\d*\.\d+", command)
                    x_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                    ms = re.search("y=[-+]?\d*\.\d+", command)
                    y_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                    ms = re.search("z=[-+]?\d*\.\d+", command)
                    z_ref = float(command[ms.span()[0]+2:ms.span()[1]])
                    ms = re.search("vx=[-+]?\d*\.\d+", command)
                    if ms is not None:
                        vx_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                    else:
                        vx_ref = 0
                    ms = re.search("vy=[-+]?\d*\.\d+", command)
                    if ms is not None:
                        vy_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                    else:
                        vy_ref = 0
                    ms = re.search("vz=[-+]?\d*\.\d+", command)
                    if ms is not None:
                        vz_ref = float(command[ms.span()[0]+3:ms.span()[1]])
                    else:
                        vz_ref = 0

                    ms = re.search("alpha=[-+]?\d*\.\d+", command)
                    if ms is not None:
                        alpha_ref = float(command[ms.span()[0]+6:ms.span()[1]])
                    else:
                        alpha_ref = 0

                    self.set_reference(x_ref, y_ref, z_ref, alpha_ref, vx_ref, vy_ref, vz_ref, 0.0)

                if command.split()[0] == 'hold':
                    self.command_queue.put_nowait(command)

                sample = np.array([self.x, self.y, self.z, self.alpha, self.vx, self.vy, self.vz, self.valpha,
                                   self.x_ref, self.y_ref, self.z_ref, self.alpha_ref, self.vx_ref, self.vy_ref, self.vz_ref,
                                   self.valpha_ref, self.fx, self.fy, self.fz, self.malpha, self.m1, self.m2, self.m3, self.m4, self.m5, self.m6])
                data = struct.pack('<26f',*sample)
                self.sock.sendto(data, (self.group, self.port))
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

    def set_state(self, x, y, z, alpha, vx, vy, vz, valpha):
        self.x = x
        self.y = y
        self.z = z
        self.vx = vx
        self.vy = vy
        self.vz = vz
        self.alpha = alpha
        self.valpha = valpha

    def set_reference(self, x_ref, y_ref, z_ref, alpha_ref, vx_ref, vy_ref, vz_ref, valpha_ref):
        self.x_ref = x_ref
        self.y_ref = y_ref
        self.z_ref = z_ref
        self.alpha_ref = alpha_ref
        self.vx_ref = vx_ref
        self.vy_ref = vy_ref
        self.vz_ref = vz_ref
        self.valpha_ref = valpha_ref

    def turn_off(self):
        command = "0,0,0,0,0,0"
        mi = self.client.publish(str(self.blimp_base_topic) + "/" + str(self.blimp_id) + "/motors", command, 0, False)
        mi.wait_for_publish()

    def control(self):
        c_x = (self.x_ref - self.x) * self.k_p_xy + (self.vx_ref - self.vx) * self.k_d_xy
        self.c_x_filt = self.filt_const * c_x + (1 - self.filt_const) * self.c_x_filt

        c_y = (self.y_ref - self.y) * self.k_p_xy + (self.vy_ref - self.vy) *self.k_d_xy
        self.c_y_filt = self.filt_const * c_y + (1 - self.filt_const) * self.c_y_filt

        # altitude control
        e_z = (self.z_ref - self.z)

        if (np.abs(self.i_e_z + e_z*dt) < self.max_i_e):
            self.i_e_z = e_z*dt + self.i_e_z

        c_z = e_z * self.k_p_z + (self.vz_ref - self.vz) * self.k_d_z + self.i_e_z * self.k_i_z
        self.c_z_filt = self.filt_const * c_z + (1 - self.filt_const) * self.c_z_filt

        # heading control
        e_alpha = self.alpha_ref - self.alpha
        if e_alpha > np.pi:
            e_alpha = e_alpha - 2 * np.pi
        elif e_alpha < -np.pi:
            e_alpha = e_alpha + 2 * np.pi

        c_a = e_alpha * self.k_p_a - self.valpha * self.k_d_a
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

        self.fx = c_x_body
        self.fy = c_y_body
        self.fz = c_z_body
        self.malpha = c_a_body

        command = '{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f}'.format(c_x_body, c_y_body, c_z_body, 0.0, 0.0, c_a_body)
        self.client.publish(str(self.blimp_base_topic) + "/" + str(self.blimp_id) + "/forces", command, 0, False)
        print("Command: fx: %.3f fy: %.3f fz: %.3f mx: %.3f my: %.3f mz: %.3f" % (c_x_body, c_y_body, c_z_body, 0, 0, c_a_body))
        print("State: x: %.3f/%.3f y: %.3f/%.3f z: %.3f/%.3f alpha: %.3f/%.3f" % (self.x, self.x_ref, self.y, self.y_ref, self.z, self.z_ref, self.alpha, self.alpha_ref))

    def set_position(self, x, y, z):
        t_now = time.time()
        dt = t_now - self.t_pos
        self.t_pos = t_now

        if dt >= 0.008 and dt < 0.1:
            vx_filt, self.vx_zf = signal.lfilter(self.b, self.a, [(x - self.x)/dt], zi = self.vx_zf)
            vy_filt, self.vy_zf = signal.lfilter(self.b, self.a, [(y - self.y)/dt], zi = self.vy_zf)
            vz_filt, self.vz_zf = signal.lfilter(self.b, self.a, [(z - self.z)/dt], zi = self.vz_zf)

            self.vx = vx_filt[0]
            self.vy = vy_filt[0]
            self.vz = vz_filt[0]

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
            valpha = alpha - self.alpha
            if valpha > np.pi:
                valpha = valpha - 2 * np.pi
            elif valpha < -np.pi:
                valpha = valpha + 2 * np.pi

            valpha_filt, self.valpha_zf = signal.lfilter(self.b, self.a, [valpha / dt], zi = self.valpha_zf)
            self.valpha = valpha_filt[0]

        self.alpha = alpha

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
        N = tf/0.1
        t = np.linspace(0, tf, int(N))
        x = a3*t**3 + a4*t**4 + a5*t**5
        dx = 3*a3*t**2 + 4*a4*t**3 + 5*a5*t**4

        return t, x, dx

def stop_blimp(blimp_id):
    global blimps

    for blimp in blimps:
        if blimp.blimp_id == blimp_id:
            blimp.stop()
            blimps.remove(blimp)

def add_blimp(client, userdata, msg):
    global blimps
    command = msg.payload.decode()

    ms = re.search("blimp_base_topic=[a-zA-Z0-9/]*", command)
    if ms is None:
        return

    blimp_base_topic = command[ms.span()[0]+17:ms.span()[1]]

    ms = re.search("blimp_id=[a-zA-Z0-9]*", command)
    if ms is None:
        return

    blimp_id = command[ms.span()[0]+9:ms.span()[1]]

    ms = re.search("tracking_id=[0-9]*", command)
    if ms is None:
        return

    tracking_id = command[ms.span()[0]+12:ms.span()[1]]

    blimps.append(Blimp(dt, client, base_topic, blimp_base_topic, blimp_id, tracking_id))

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
mqtt_host = "localhost"
mqtt_port = 1883
osc_server = "localhost"
osc_port = 54321
base_topic = "manager"

if __name__ == "__main__":
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:],"h",["mqtt_host=","mqtt_port=","osc_server=","osc_port=", "base_topic="])
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
