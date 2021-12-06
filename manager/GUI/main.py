from gui import Ui_MainWindow
from PyQt5 import QtWidgets, QtCore
import sys
from collections import deque
import numpy as np
import threading
import paho.mqtt.client as mqtt 
import re
import time
import copy

LINE = 1
SIZE = 2
MQTT_HOST = 'localhost'
MQTT_PORT = 1883

def parseFloat(key, message):
    ms = re.search(key + "=[-+]?\d*\.\d+", message)
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

class Container():
    def __init__(self, L, k_p_xy, k_d_xy, k_p_z, k_d_z, k_i_z, tau_att_x, tau_att_y, tau_att_z, tau_p, tau_q, tau_r):
        self._x = deque([], maxlen=L)
        self._y = deque([], maxlen=L)
        self._z = deque([], maxlen=L)
        self._vx = deque([], maxlen=L)
        self._vy = deque([], maxlen=L)
        self._vz = deque([], maxlen=L)
        self._qw = deque([], maxlen=L)
        self._qx = deque([], maxlen=L)
        self._qy = deque([], maxlen=L)
        self._qz = deque([], maxlen=L)
        self._p = deque([], maxlen=L)
        self._q = deque([], maxlen=L)
        self._r = deque([], maxlen=L)
        self._x_ref = deque([], maxlen=L)
        self._y_ref = deque([], maxlen=L)
        self._z_ref = deque([], maxlen=L)
        self._vx_ref = deque([], maxlen=L)
        self._vy_ref = deque([], maxlen=L)
        self._vz_ref = deque([], maxlen=L)
        self._qw_ref = deque([], maxlen=L)
        self._qx_ref = deque([], maxlen=L)
        self._qy_ref = deque([], maxlen=L)
        self._qz_ref = deque([], maxlen=L)
        self._p_ref = deque([], maxlen=L)
        self._q_ref = deque([], maxlen=L)
        self._r_ref = deque([], maxlen=L)
        self._fx = deque([], maxlen=L)
        self._fy = deque([], maxlen=L)
        self._fz = deque([], maxlen=L)
        self._mx = deque([], maxlen=L)
        self._my = deque([], maxlen=L)
        self._mz = deque([], maxlen=L)
        self._t = deque([], maxlen=L)
        self._m1 = deque([], maxlen=L)
        self._m2 = deque([], maxlen=L)
        self._m3 = deque([], maxlen=L)
        self._m4 = deque([], maxlen=L)
        self._m5 = deque([], maxlen=L)
        self._m6 = deque([], maxlen=L)
        
        self._state = ''
        self._queue_size = ''
        self._missed_ticks = ''

        self._k_p_xy = k_p_xy
        self._k_d_xy = k_d_xy
        self._k_p_z = k_p_z
        self._k_d_z = k_d_z
        self._k_i_z = k_i_z
        self._tau_att_x = tau_att_x
        self._tau_att_y = tau_att_y
        self._tau_att_z = tau_att_z
        self._tau_p = tau_p
        self._tau_q = tau_q
        self._tau_r = tau_r
        self._battery_charge = 0.0

class DeviceData():
    def __init__(self, identifier, L, k_p_xy, k_d_xy, k_p_z, k_d_z, k_i_z, tau_att_x, tau_att_y, tau_att_z, tau_p, tau_q, tau_r):

        self._container = Container(L, k_p_xy, k_d_xy, k_p_z, k_d_z, k_i_z, tau_att_x, tau_att_y, tau_att_z, tau_p, tau_q, tau_r)
        self._lock = threading.Lock()    
                
        self._id = identifier
        self._t0 = time.time()
        
        self._client = mqtt.Client(client_id='GUI'+identifier)
        self._client.username_pw_set(username="testtest",password="testtest")
        self._client.connect(MQTT_HOST, MQTT_PORT, 60)
        print("Added blimp %s to GUI" % self._id)
        self._client.loop_start()
        
        self._client.message_callback_add("manager/+/" + identifier + "/feedback", self.add_data)
        self._client.subscribe("manager/+/" + identifier + "/feedback")

        self._data_available = False
                
    def add_data(self, client, userdata, msg):
        del client, userdata
        
        with self._lock:
    
            message = msg.payload.decode()
            
            x = parseFloat('x', message)
            y = parseFloat('y', message)
            z = parseFloat('z', message)
            vx = parseFloat('vx', message)
            vy = parseFloat('vy', message)
            vz = parseFloat('vz', message)
            qw = parseFloat('qw', message)
            qx = parseFloat('qx', message)
            qy = parseFloat('qy', message)
            qz = parseFloat('qz', message)
            p = parseFloat('p', message)
            q = parseFloat('q', message)
            r = parseFloat('r', message)

            x_ref = parseFloat('x_ref', message)
            y_ref = parseFloat('y_ref', message)
            z_ref = parseFloat('z_ref', message)
            vx_ref = parseFloat('vx_ref', message)
            vy_ref = parseFloat('vy_ref', message)
            vz_ref = parseFloat('vz_ref', message)
            qw_ref = parseFloat('qw_ref', message)
            qx_ref = parseFloat('qx_ref', message)
            qy_ref = parseFloat('qy_ref', message)
            qz_ref = parseFloat('qz_ref', message)
            p_ref = parseFloat('p_ref', message)
            q_ref = parseFloat('q_ref', message)
            r_ref = parseFloat('r_ref', message)
            
            fx = parseFloat('fx', message)
            fy = parseFloat('fy', message)
            fz = parseFloat('fz', message)
            mx = parseFloat('mx', message)
            my = parseFloat('my', message)
            mz = parseFloat('mz', message)

            m1 = parseFloat('m1', message)
            m2 = parseFloat('m2', message)
            m3 = parseFloat('m3', message)
            m4 = parseFloat('m4', message)
            m5 = parseFloat('m5', message)
            m6 = parseFloat('m6', message)
            
            self._container._x.append(x)
            self._container._y.append(y)
            self._container._z.append(z)
            self._container._vx.append(vx)
            self._container._vy.append(vy)
            self._container._vz.append(vz)
            self._container._qw.append(qw)
            self._container._qx.append(qx)
            self._container._qy.append(qy)
            self._container._qz.append(qz)
            self._container._p.append(p * 180 / np.pi)
            self._container._q.append(q * 180 / np.pi)
            self._container._r.append(r * 180 / np.pi)
            self._container._x_ref.append(x_ref)
            self._container._y_ref.append(y_ref)
            self._container._z_ref.append(z_ref)
            self._container._vx_ref.append(vx_ref)
            self._container._vy_ref.append(vy_ref)
            self._container._vz_ref.append(vz_ref)
            self._container._qw_ref.append(qw_ref)
            self._container._qx_ref.append(qx_ref)
            self._container._qy_ref.append(qy_ref)
            self._container._qz_ref.append(qz_ref)
            self._container._p_ref.append(p_ref * 180 / np.pi)
            self._container._q_ref.append(q_ref * 180 / np.pi)
            self._container._r_ref.append(r_ref * 180 / np.pi)
            self._container._fx.append(fx)
            self._container._fy.append(fy)
            self._container._fz.append(fz)
            self._container._mx.append(mx)
            self._container._my.append(my)
            self._container._mz.append(mz)
            self._container._m1.append(m1)
            self._container._m2.append(m2)
            self._container._m3.append(m3)
            self._container._m4.append(m4)
            self._container._m5.append(m5)
            self._container._m6.append(m6)
      
            self._container._state = parseString('state', message)
            self._container._queue_size = parseString('queue_size', message)
            self._container._missed_ticks = parseString('missed_ticks', message)

            self._container._k_p_xy = parseFloat('k_p_xy', message)
            self._container._k_d_xy = parseFloat('k_d_xy', message)
            self._container._k_p_z = parseFloat('k_p_z', message)
            self._container._k_d_z = parseFloat('k_d_z', message)
            self._container._k_i_z = parseFloat('k_i_z', message)
            self._container._tau_att_x = parseFloat('tau_att_x', message)
            self._container._tau_att_y = parseFloat('tau_att_y', message)
            self._container._tau_att_z = parseFloat('tau_att_z', message)
            self._container._tau_p = parseFloat('tau_p', message)
            self._container._tau_q = parseFloat('tau_q', message)
            self._container._tau_r = parseFloat('tau_r', message)
            self._container._battery_charge = parseFloat('battery_charge', message)
            
            self._container._t.append(time.time() - self._t0)
            
            self._data_available = True 
                        
    def get_data(self):
        with self._lock:
            self._data_available = False
            return self._container
        
    def get_data_without_lock(self):
        self._data_available = False
        return self._container

    def acquire_lock(self):
        self._lock.acquire()

    def release_lock(self):
        self._lock.release()

    def data_available(self):
        return self._data_available
        
    def stop(self):
        self._run = False

def generatePlot(item, line, color, size, name = ''):
    del name
    if line == 1:
        return item.plot([],  pen={'color': color, 'width': size})
    else:
        return item.plot([], pen=None, 
            symbolBrush=color, symbolSize=size, symbolPen=None)

class Ui(QtWidgets.QMainWindow):
    
    updated = QtCore.pyqtSignal(str)

    def __init__(self):
        super(Ui, self).__init__()
        self.ui = Ui_MainWindow()        
        self.ui.setupUi(self)    
            
        self.plotItem_x = self.ui.positionView.addPlot(row = 0, col = 0)
        self.plotItem_x.setLabel('left', 'X', 'm')
        self.plotItem_x.setXRange(-70, 0)

        self.plotDataItem_x = generatePlot(self.plotItem_x, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_x_ref = generatePlot(self.plotItem_x, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_y = self.ui.positionView.addPlot(row = 1, col = 0)
        self.plotItem_y.setLabel('left', 'Y', 'm')
        self.plotItem_y.setXRange(-70, 0)

        self.plotDataItem_y = generatePlot(self.plotItem_y, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_y_ref = generatePlot(self.plotItem_y, LINE, (11, 220, 13), SIZE)

        self.plotItem_z = self.ui.positionView.addPlot(row = 2, col = 0)
        self.plotItem_z.setLabel('left', 'Z', 'm')
        self.plotItem_z.setXRange(-70, 0)

        self.plotDataItem_z = generatePlot(self.plotItem_z, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_z_ref = generatePlot(self.plotItem_z, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_vx = self.ui.positionView.addPlot(row = 0, col = 1)
        self.plotItem_vx.setLabel('left', 'Vx', 'm/s')
        self.plotItem_vx.setXRange(-70, 0)

        self.plotDataItem_vx = generatePlot(self.plotItem_vx, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vx_ref = generatePlot(self.plotItem_vx, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_vy = self.ui.positionView.addPlot(row = 1, col = 1)
        self.plotItem_vy.setLabel('left', 'Vy', 'm/s')
        self.plotItem_vy.setXRange(-70, 0)

        self.plotDataItem_vy = generatePlot(self.plotItem_vy, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vy_ref = generatePlot(self.plotItem_vy, LINE, (11, 220, 13), SIZE)
                
        self.plotItem_vz = self.ui.positionView.addPlot(row = 2, col = 1)
        self.plotItem_vz.setLabel('left', 'Vz', 'm/s')
        self.plotItem_vz.setXRange(-70, 0)

        self.plotDataItem_vz = generatePlot(self.plotItem_vz, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vz_ref = generatePlot(self.plotItem_vz, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_fx = self.ui.positionView.addPlot(row = 0, col = 2)
        self.plotItem_fx.setLabel('left', 'Fx', '-')
        self.plotItem_fx.setXRange(-70, 0)

        self.plotDataItem_fx = generatePlot(self.plotItem_fx, LINE, (204, 0, 0), SIZE)
        
        self.plotItem_fy = self.ui.positionView.addPlot(row = 1, col = 2)
        self.plotItem_fy.setLabel('left', 'Fy', '-')
        self.plotItem_fy.setXRange(-70, 0)

        self.plotDataItem_fy = generatePlot(self.plotItem_fy, LINE, (204, 0, 0), SIZE)
        
        self.plotItem_fz = self.ui.positionView.addPlot(row = 2, col = 2)
        self.plotItem_fz.setLabel('left', 'Fz', '-')
        self.plotItem_fz.setXRange(-70, 0)

        self.plotDataItem_fz = generatePlot(self.plotItem_fz, LINE, (204, 0, 0), SIZE)

        self.plotItem_qw = self.ui.attitudeView.addPlot(row = 0, col = 0)
        self.plotItem_qw.setLabel('left', 'qw', '-')
        self.plotItem_qw.setLabel('bottom', 'Time', 's')
        self.plotItem_qw.setXRange(-70, 0)

        self.plotDataItem_qw = generatePlot(self.plotItem_qw, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_qw_ref = generatePlot(self.plotItem_qw, LINE, (11, 220, 13), SIZE)

        self.plotItem_qx = self.ui.attitudeView.addPlot(row = 1, col = 0)
        self.plotItem_qx.setLabel('left', 'qx', '-')
        self.plotItem_qx.setLabel('bottom', 'Time', 's')
        self.plotItem_qx.setXRange(-70, 0)

        self.plotDataItem_qx = generatePlot(self.plotItem_qx, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_qx_ref = generatePlot(self.plotItem_qx, LINE, (11, 220, 13), SIZE)

        self.plotItem_qy = self.ui.attitudeView.addPlot(row = 2, col = 0)
        self.plotItem_qy.setLabel('left', 'qy', '-')
        self.plotItem_qy.setLabel('bottom', 'Time', 's')
        self.plotItem_qy.setXRange(-70, 0)

        self.plotDataItem_qy = generatePlot(self.plotItem_qy, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_qy_ref = generatePlot(self.plotItem_qy, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_qz = self.ui.attitudeView.addPlot(row = 3, col = 0)
        self.plotItem_qz.setLabel('left', 'qz', '-')
        self.plotItem_qz.setLabel('bottom', 'Time', 's')
        self.plotItem_qz.setXRange(-70, 0)

        self.plotDataItem_qz = generatePlot(self.plotItem_qz, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_qz_ref = generatePlot(self.plotItem_qz, LINE, (11, 220, 13), SIZE)

        self.plotItem_p = self.ui.attitudeView.addPlot(row = 1, col = 1)
        self.plotItem_p.setLabel('left', 'p', '°/s')
        self.plotItem_p.setLabel('bottom', 'Time', 's')
        self.plotItem_p.setXRange(-70, 0)

        self.plotDataItem_p = generatePlot(self.plotItem_p, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_p_ref = generatePlot(self.plotItem_p, LINE, (11, 220, 13), SIZE)

        self.plotItem_q = self.ui.attitudeView.addPlot(row = 2, col = 1)
        self.plotItem_q.setLabel('left', 'q', '°/s')
        self.plotItem_q.setLabel('bottom', 'Time', 's')
        self.plotItem_q.setXRange(-70, 0)

        self.plotDataItem_q = generatePlot(self.plotItem_q, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_q_ref = generatePlot(self.plotItem_q, LINE, (11, 220, 13), SIZE)

        self.plotItem_r = self.ui.attitudeView.addPlot(row = 3, col = 1)
        self.plotItem_r.setLabel('left', 'r', '°/s')
        self.plotItem_r.setLabel('bottom', 'Time', 's')
        self.plotItem_r.setXRange(-70, 0)

        self.plotDataItem_r = generatePlot(self.plotItem_r, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_r_ref = generatePlot(self.plotItem_r, LINE, (11, 220, 13), SIZE)

        self.plotItem_mx = self.ui.attitudeView.addPlot(row = 1, col = 2)
        self.plotItem_mx.setLabel('left', 'Mx', '-')
        self.plotItem_mx.setLabel('bottom', 'Time', 's')
        self.plotItem_mx.setXRange(-70, 0)

        self.plotDataItem_mx = generatePlot(self.plotItem_mx, LINE, (204, 0, 0), SIZE)

        self.plotItem_my = self.ui.attitudeView.addPlot(row = 2, col = 2)
        self.plotItem_my.setLabel('left', 'My', '-')
        self.plotItem_my.setLabel('bottom', 'Time', 's')
        self.plotItem_my.setXRange(-70, 0)

        self.plotDataItem_my = generatePlot(self.plotItem_my, LINE, (204, 0, 0), SIZE)

        self.plotItem_mz = self.ui.attitudeView.addPlot(row = 3, col = 2)
        self.plotItem_mz.setLabel('left', 'Mz', '-')
        self.plotItem_mz.setLabel('bottom', 'Time', 's')
        self.plotItem_mz.setXRange(-70, 0)

        self.plotDataItem_mz = generatePlot(self.plotItem_mz, LINE, (204, 0, 0), SIZE)

        self.plotItem_motors = self.ui.motorsView.addPlot(row = 0, col = 0)
        self.plotItem_motors.setLabel('left', 'Motor DC', '-')
        self.plotItem_motors.setLabel('bottom', 'Time', 's')
        self.plotItem_motors.addLegend()
        self.plotDataItem_motors = []
        
        col = np.array([[240, 20, 0],
               [30, 244, 10],
               [190, 190, 0], 
               [120, 210, 130],
               [10, 120, 230],
               [150, 70, 200]])
        
        for i in range(6):
            self.plotDataItem_motors.append(generatePlot(self.plotItem_motors, LINE, (col[i,0],col[i,1],col[i,2]), SIZE, 'motor ' + str(i + 1)))

        self.plotItem_motors.setLabel('bottom', 'Time', 's')
        self.plotItem_motors.setXRange(-70, 0)
        self.plotItem_motors.setYRange(-1, 1)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(250) # in milliseconds
        self.timer.start()
        self.timer.timeout.connect(self.updateData)
        self.now = 0
        self.show()

        self.devices = {}
        self.active_device = ''
        self.device_count = 0

        self.client = mqtt.Client(client_id='GUI')
        self.client.username_pw_set(username="testtest",password="testtest")
        self.client.connect(MQTT_HOST, MQTT_PORT, 60)
        print("MQTT client connected to %s on port %d" % (MQTT_HOST, MQTT_PORT))
        self.client.loop_start()

        self.client.message_callback_add("manager/+/+/feedback", self.addDevice)
        self.client.subscribe("manager/+/+/feedback")
    
        self.updated.connect(self.addPushButton)

        self.sem = threading.Semaphore()
        self.t0 = time.time()

        self.ui.hor_pos_slider.valueChanged.connect(lambda: self.sliderChanged('hor_pos'))
        self.ui.hor_vel_slider.valueChanged.connect(lambda: self.sliderChanged('hor_vel'))
        self.ui.ver_pos_slider.valueChanged.connect(lambda: self.sliderChanged('ver_pos'))
        self.ui.ver_vel_slider.valueChanged.connect(lambda: self.sliderChanged('ver_vel'))
        self.ui.ver_int_slider.valueChanged.connect(lambda: self.sliderChanged('ver_int'))
        self.ui.tau_att_x_slider.valueChanged.connect(lambda: self.sliderChanged('tau_att_x'))
        self.ui.tau_att_y_slider.valueChanged.connect(lambda: self.sliderChanged('tau_att_y'))
        self.ui.tau_att_z_slider.valueChanged.connect(lambda: self.sliderChanged('tau_att_z'))
        self.ui.tau_p_slider.valueChanged.connect(lambda: self.sliderChanged('tau_p'))
        self.ui.tau_q_slider.valueChanged.connect(lambda: self.sliderChanged('tau_q'))
        self.ui.tau_r_slider.valueChanged.connect(lambda: self.sliderChanged('tau_r'))

        self.k_p_xy = 0.0
        self.k_d_xy = 0.0
        self.k_p_z = 0.0
        self.k_d_z = 0.0
        self.k_i_z = 0.0
        self.tau_att_x = 0.0
        self.tau_att_y = 0.0
        self.tau_att_z = 0.0
        self.tau_p = 0.0
        self.tau_q = 0.0
        self.tau_r = 0.0
        self.roll_ref = 0.0
        self.pitch_ref = 0.0
        self.yaw_ref = 0.0

    def sliderChanged(self, slider):
        if slider == 'hor_pos':
            self.k_p_xy = self.ui.hor_pos_slider.value()/10
        if slider == 'hor_vel':
            self.k_d_xy = self.ui.hor_vel_slider.value()/10
        if slider == 'ver_pos':
            self.k_p_z = self.ui.ver_pos_slider.value()/10
        if slider == 'ver_vel':
            self.k_d_z = self.ui.ver_vel_slider.value()/10
        if slider == 'ver_int':
            self.k_i_z = self.ui.ver_int_slider.value()/10
        if slider == 'tau_att_x':
            self.tau_att_x = self.ui.tau_att_x_slider.value()/10
        if slider == 'tau_att_y':
            self.tau_att_y = self.ui.tau_att_y_slider.value()/10
        if slider == 'tau_att_z':
            self.tau_att_z = self.ui.tau_att_z_slider.value()/10
        if slider == 'tau_p':
            self.tau_p = self.ui.tau_p_slider.value()/1000
        if slider == 'tau_q':
            self.tau_q = self.ui.tau_q_slider.value()/1000
        if slider == 'tau_r':
            self.tau_r = self.ui.tau_r_slider.value()/1000

        command = "k_p_z=%f k_d_z=%f k_i_z=%f k_p_xy=%f k_d_xy=%f tau_att_x=%f tau_att_y=%f tau_att_z=%f tau_p=%f tau_q=%f tau_r=%f " \
                % (self.k_p_z, self.k_d_z, self.k_i_z, self.k_p_xy, self.k_d_xy, self.tau_att_x, self.tau_att_y, self.tau_att_z, self.tau_p, self.tau_q, self.tau_r)

        self.client.publish('manager/blimps/' + self.active_device +'/config', command, 0, False)
                        
    def addPushButton(self, identifier):

        if not hasattr(self.ui, 'pushButtons'):
            self.ui.pushButtons = {}

        self.ui.pushButtons[identifier] = QtWidgets.QPushButton(self.ui.groupBox)
        self.ui.pushButtons[identifier].setGeometry(QtCore.QRect(10, 30 + (self.device_count - 1)*100, 131, 91))
        self.ui.pushButtons[identifier].setStyleSheet("")
        self.ui.pushButtons[identifier].setFlat(False)
        self.ui.pushButtons[identifier].setObjectName(identifier)
        self.ui.pushButtons[identifier].setText(identifier)
        self.ui.pushButtons[identifier].show()
        self.ui.pushButtons[identifier].clicked.connect(lambda: self.handleButton(self.ui.pushButtons[identifier]))
        self.sem.release()
        
    def addDevice(self, client, userdata, message):
        del client, userdata
        ident = message.topic.split('/')[-2]

        if ident in self.devices:
            pass
        else:
            self.sem.acquire()
            self.k_p_xy = parseFloat('k_p_xy', message.payload.decode())
            self.k_d_xy = parseFloat('k_d_xy', message.payload.decode())
            self.k_p_z = parseFloat('k_p_z', message.payload.decode())
            self.k_d_z = parseFloat('k_d_xy', message.payload.decode())
            self.k_i_z = parseFloat('k_i_z', message.payload.decode())
            self.tau_att_x = parseFloat('tau_att_x', message.payload.decode())
            self.tau_att_y = parseFloat('tau_att_y', message.payload.decode())
            self.tau_att_z = parseFloat('tau_att_z', message.payload.decode())
            self.tau_p = parseFloat('tau_p', message.payload.decode())
            self.tau_q = parseFloat('tau_q', message.payload.decode())
            self.tau_r = parseFloat('tau_r', message.payload.decode())

            self.devices[ident] = DeviceData(ident, 10*60*1, self.k_p_xy, self.k_d_xy, self.k_p_z, self.k_d_z, self.k_i_z, \
                                           self.tau_att_x, self.tau_att_y, self.tau_att_z, self.tau_p, self.tau_q, self.tau_r)
            self.device_count = self.device_count + 1
            
            self.updated.emit(ident)
        
    def updateData(self):
        if hasattr(self.ui, 'pushButtons'): 
            for key, button in self.ui.pushButtons.items():
                if self.devices[key].data_available():
                    charge = '%.2f %%' % (self.devices[key].get_data()._battery_charge*100)
                    button.setText(key + '\n' + 'Charge: ' + charge + '\n State: ' + self.devices[key].get_data()._state + \
                                   '\n Queue:' + self.devices[key].get_data()._queue_size + '\n Missed:' + self.devices[key].get_data()._missed_ticks)
    
                    button.setStyleSheet("")
                    if self.devices[key].get_data()._state != 'unavailable':
                        s = int(255)
                        l = int(255*0.6)
                        h = int(min(max(171 * self.devices[key].get_data()._battery_charge - 51, 0),120))
                        if button.objectName() == self.active_device:
                            style = "background-color:hsl(%i, %i, %i); border:2px solid #000000; border-radius:5px" %(h, s, l)
                        else:
                            style = "background-color:hsl(%i, %i, %i);" %(h, s, l)
    
                        button.setStyleSheet(style)
                if key == self.active_device:
                    self.devices[self.active_device].acquire_lock()
                    data = copy.deepcopy(self.devices[self.active_device].get_data_without_lock())
                    self.devices[self.active_device].release_lock()
    
                    self.ui.k_p_xy.setText(str(data._k_p_xy))
                    self.ui.k_d_xy.setText(str(data._k_d_xy))
                    self.ui.k_p_z.setText(str(data._k_p_z))
                    self.ui.k_d_z.setText(str(data._k_d_z))
                    self.ui.k_i_z.setText(str(data._k_i_z))
                    self.ui.tau_att_x.setText(str(data._tau_att_x))
                    self.ui.tau_att_y.setText(str(data._tau_att_y))
                    self.ui.tau_att_z.setText(str(data._tau_att_z))
                    self.ui.tau_p.setText(str(data._tau_p))
                    self.ui.tau_q.setText(str(data._tau_q))
                    self.ui.tau_r.setText(str(data._tau_r))

                    time_relative = np.array(data._t) - data._t[-1]
                    self.plotDataItem_x.setData(time_relative, data._x)
                    self.plotDataItem_y.setData(time_relative, data._y)
                    self.plotDataItem_z.setData(time_relative, data._z)
                    self.plotDataItem_vx.setData(time_relative, data._vx)
                    self.plotDataItem_vy.setData(time_relative, data._vy)
                    self.plotDataItem_vz.setData(time_relative, data._vz)
                    self.plotDataItem_qw.setData(time_relative, data._qw)
                    self.plotDataItem_qx.setData(time_relative, data._qx)
                    self.plotDataItem_qy.setData(time_relative, data._qy)
                    self.plotDataItem_qz.setData(time_relative, data._qz)
                    self.plotDataItem_p.setData(time_relative, data._p)
                    self.plotDataItem_q.setData(time_relative, data._q)
                    self.plotDataItem_r.setData(time_relative, data._r)
                    self.plotDataItem_x_ref.setData(time_relative, data._x_ref)
                    self.plotDataItem_y_ref.setData(time_relative, data._y_ref)
                    self.plotDataItem_z_ref.setData(time_relative, data._z_ref)
                    self.plotDataItem_vx_ref.setData(time_relative, data._vx_ref)
                    self.plotDataItem_vy_ref.setData(time_relative, data._vy_ref)
                    self.plotDataItem_vz_ref.setData(time_relative, data._vz_ref)
                    self.plotDataItem_qw_ref.setData(time_relative, data._qw_ref)
                    self.plotDataItem_qx_ref.setData(time_relative, data._qx_ref)
                    self.plotDataItem_qy_ref.setData(time_relative, data._qy_ref)
                    self.plotDataItem_qz_ref.setData(time_relative, data._qz_ref)
                    self.plotDataItem_p_ref.setData(time_relative, data._p_ref)
                    self.plotDataItem_q_ref.setData(time_relative, data._q_ref)
                    self.plotDataItem_r_ref.setData(time_relative, data._r_ref)
    
                    self.plotDataItem_fx.setData(time_relative, data._fx)
                    self.plotDataItem_fy.setData(time_relative, data._fy)
                    self.plotDataItem_fz.setData(time_relative, data._fz)
                    self.plotDataItem_mx.setData(time_relative, data._mx)
                    self.plotDataItem_my.setData(time_relative, data._my)
                    self.plotDataItem_mz.setData(time_relative, data._mz)
    
                    self.plotDataItem_motors[0].setData(time_relative, data._m1)
                    self.plotDataItem_motors[1].setData(time_relative, data._m2)
                    self.plotDataItem_motors[2].setData(time_relative, data._m3)
                    self.plotDataItem_motors[3].setData(time_relative, data._m4)
                    self.plotDataItem_motors[4].setData(time_relative, data._m5)
                    self.plotDataItem_motors[5].setData(time_relative, data._m6)
            
    def handleButton(self, btn):       
        self.active_device = btn.objectName()

        data = self.devices[self.active_device].get_data()
        self.ui.k_p_xy.setText(str(data._k_p_xy))
        self.ui.hor_pos_slider.setValue(int(10 * data._k_p_xy))
        self.ui.k_d_xy.setText(str(data._k_d_xy))
        self.ui.hor_vel_slider.setValue(int(10 * data._k_d_xy))
        self.ui.k_p_z.setText(str(data._k_p_z))
        self.ui.ver_pos_slider.setValue(int(10 * data._k_p_z))
        self.ui.k_d_z.setText(str(data._k_d_z))
        self.ui.ver_int_slider.setValue(int(10 * data._k_i_z))
        self.ui.k_d_z.setText(str(data._k_d_z))
        self.ui.ver_vel_slider.setValue(int(10 * data._k_d_z))
        self.ui.tau_att_x.setText(str(data._tau_att_x))
        self.ui.tau_att_x_slider.setValue(int(10 * data._tau_att_x))
        self.ui.tau_att_y.setText(str(data._tau_att_y))
        self.ui.tau_att_y_slider.setValue(int(10 * data._tau_att_y))
        self.ui.tau_att_z.setText(str(data._tau_att_z))
        self.ui.tau_att_z_slider.setValue(int(10 * data._tau_att_z))
        self.ui.tau_p.setText(str(data._tau_p))
        self.ui.tau_p_slider.setValue(int(1000 * data._tau_p))
        self.ui.tau_q.setText(str(data._tau_q))
        self.ui.tau_q_slider.setValue(int(1000 * data._tau_q))
        self.ui.tau_r.setText(str(data._tau_r))
        self.ui.tau_r_slider.setValue(int(1000 * data._tau_r))

    def closeEvent(self, event):
        self.timer.stop()
        for _, device in self.devices.items():
            device.stop()
        event.accept()        

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--line", help="plot lines (0: dots, 1: lines)")
parser.add_argument("--size", help="plot size")
parser.add_argument("--mqtt_host", help="IP of MQTT host")
parser.add_argument("--mqtt_port", help="port of MQTT host")

args = parser.parse_args()
if args.line:
    print("Line plotting turned on")
    LINE = int(args.line)
if args.size:
    print("Set plot size")
    SIZE = int(args.size)  
if args.mqtt_host:
    print("Set mqtt host", args.mqtt_host)
    MQTT_HOST = str(args.mqtt_host)
if args.mqtt_port:
    print("Set mqtt port", args.mqtt_port)
    MQTT_PORT = int(args.mqtt_port)

app = QtWidgets.QApplication(sys.argv)
window = Ui()
app.exec_()
