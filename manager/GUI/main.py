from gui import Ui_MainWindow
from PyQt5 import QtWidgets, QtCore
import sys
from collections import deque
import pyqtgraph as pg
import numpy as np
import threading
import paho.mqtt.client as mqtt 
import re
import time

LINE = 1
SIZE = 2
MQTT_HOST = 'localhost'
MQTT_PORT = 1883

def parseFloat(string, message):
    try:
        ms = re.search(string+"=[-+]?\d*\.\d+", message)
        data = float(message[ms.span()[0]+(len(string)+1):ms.span()[1]])
    except:
        pass
        return None
    
    return data

class BlimpData():
    def __init__(self, id, L):
        self._x = deque([], maxlen=L)
        self._y = deque([], maxlen=L)
        self._z = deque([], maxlen=L)
        self._alpha = deque([], maxlen=L)
        self._vx = deque([], maxlen=L)
        self._vy = deque([], maxlen=L)
        self._vz = deque([], maxlen=L)
        self._valpha = deque([], maxlen=L)
        self._x_ref = deque([], maxlen=L)
        self._y_ref = deque([], maxlen=L)
        self._z_ref = deque([], maxlen=L)
        self._alpha_ref = deque([], maxlen=L)
        self._vx_ref = deque([], maxlen=L)
        self._vy_ref = deque([], maxlen=L)
        self._vz_ref = deque([], maxlen=L)
        self._valpha_ref = deque([], maxlen=L)
        self._fx = deque([], maxlen=L)
        self._fy = deque([], maxlen=L)
        self._fz = deque([], maxlen=L)
        self._malpha = deque([], maxlen=L)
        self._t = deque([], maxlen=L)
        self._m1 = deque([], maxlen=L)
        self._m2 = deque([], maxlen=L)
        self._m3 = deque([], maxlen=L)
        self._m4 = deque([], maxlen=L)
        self._m5 = deque([], maxlen=L)
        self._m6 = deque([], maxlen=L)
        
        self._data = np.zeros(6)
        self._lock = threading.Lock()    
                
        self._id = id
        self._t0 = time.time()
        
        self._client = mqtt.Client(client_id='GUI'+id)
        self._client.username_pw_set(username="testtest",password="testtest")
        self._client.connect(MQTT_HOST, MQTT_PORT, 60)
        print("Added blimp %s to GUI" % self._id)
        self._client.loop_start()
        
        self._client.message_callback_add("manager/+/" + id + "/feedback", self.add_data)
        self._client.subscribe("manager/+/" + id + "/feedback")
                
    def add_data(self, client, userdata, msg):
        with self._lock:
    
            message = msg.payload.decode()
            
            x = parseFloat('x', message)
            y = parseFloat('y', message)
            z = parseFloat('z', message)
            alpha = parseFloat('alpha', message)
            vx = parseFloat('vx', message)
            vy = parseFloat('vy', message)
            vz = parseFloat('vz', message)
            valpha = parseFloat('valpha', message)
            
            x_ref = parseFloat('x_ref', message)
            y_ref = parseFloat('y_ref', message)
            z_ref = parseFloat('z_ref', message)
            alpha_ref = parseFloat('alpha_ref', message)
            vx_ref = parseFloat('vx_ref', message)
            vy_ref = parseFloat('vy_ref', message)
            vz_ref = parseFloat('vz_ref', message)
            valpha_ref = parseFloat('valpha_ref', message)
            
            fx = parseFloat('fx', message)
            fy = parseFloat('fy', message)
            fz = parseFloat('fz', message)
            malpha = parseFloat('malpha', message)
            
            m1 = parseFloat('m1', message)
            m2 = parseFloat('m2', message)
            m3 = parseFloat('m3', message)
            m4 = parseFloat('m4', message)
            m5 = parseFloat('m5', message)
            m6 = parseFloat('m6', message)
            
            self._x.append(x)
            self._y.append(y)
            self._z.append(z)
            self._alpha.append(alpha * 180 / np.pi)
            self._vx.append(vx)
            self._vy.append(vy)
            self._vz.append(vz)
            self._valpha.append(valpha * 180 / np.pi)
            self._x_ref.append(x_ref)
            self._y_ref.append(y_ref)
            self._z_ref.append(z_ref)
            self._alpha_ref.append(alpha_ref * 180 / np.pi)
            self._vx_ref.append(vx_ref)
            self._vy_ref.append(vy_ref)
            self._vz_ref.append(vz_ref)
            self._valpha_ref.append(valpha_ref * 180 / np.pi)
            self._fx.append(fx)
            self._fy.append(fy)
            self._fz.append(fz)
            self._malpha.append(malpha)
            self._m1.append(m1)
            self._m2.append(m2)
            self._m3.append(m3)
            self._m4.append(m4)
            self._m5.append(m5)
            self._m6.append(m6)
            
            self._t.append(time.time() - self._t0)
            self._data_available = True 
                        
    def get_data(self):
        with self._lock:
            self._data_available = False
            return self._x, self._y, self._z, self._alpha, self._vx, self._vy, self._vz, self._valpha, self._x_ref, self._y_ref, self._z_ref, self._alpha_ref, self._vx_ref, self._vy_ref, self._vz_ref, self._valpha_ref, self._fx, self._fy, self._fz, self._malpha, self._m1, self._m2, self._m3, self._m4, self._m5, self._m6, self._t
        
    def data_available(self):
        return self._data_available      
        
    def stop(self):
        self._run = False

def generatePlot(item, line, color, size, name = ''):
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
            
        self.plotItem_x = self.ui.graphicsView.addPlot(row = 0, col = 0)
        self.plotItem_x.setLabel('left', 'X', 'm')
        self.plotItem_x.setXRange(-70, 0)

        self.plotDataItem_x = generatePlot(self.plotItem_x, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_x_ref = generatePlot(self.plotItem_x, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_y = self.ui.graphicsView.addPlot(row = 1, col = 0)
        self.plotItem_y.setLabel('left', 'Y', 'm')
        self.plotItem_y.setXRange(-70, 0)

        self.plotDataItem_y = generatePlot(self.plotItem_y, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_y_ref = generatePlot(self.plotItem_y, LINE, (11, 220, 13), SIZE)

        self.plotItem_z = self.ui.graphicsView.addPlot(row = 2, col = 0)
        self.plotItem_z.setLabel('left', 'Z', 'm')
        self.plotItem_z.setXRange(-70, 0)

        self.plotDataItem_z = generatePlot(self.plotItem_z, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_z_ref = generatePlot(self.plotItem_z, LINE, (11, 220, 13), SIZE)
                
        self.plotItem_a = self.ui.graphicsView.addPlot(row = 3, col = 0)
        self.plotItem_a.setLabel('left', 'Alpha', '°')
        self.plotItem_a.setLabel('bottom', 'Time', 's')
        self.plotItem_a.setXRange(-70, 0)

        self.plotDataItem_a = generatePlot(self.plotItem_a, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_a_ref = generatePlot(self.plotItem_a, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_vx = self.ui.graphicsView.addPlot(row = 0, col = 1)
        self.plotItem_vx.setLabel('left', 'Vx', 'm/s')
        self.plotItem_vx.setXRange(-70, 0)

        self.plotDataItem_vx = generatePlot(self.plotItem_vx, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vx_ref = generatePlot(self.plotItem_vx, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_vy = self.ui.graphicsView.addPlot(row = 1, col = 1)
        self.plotItem_vy.setLabel('left', 'Vy', 'm/s')
        self.plotItem_vy.setXRange(-70, 0)

        self.plotDataItem_vy = generatePlot(self.plotItem_vy, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vy_ref = generatePlot(self.plotItem_vy, LINE, (11, 220, 13), SIZE)
                
        self.plotItem_vz = self.ui.graphicsView.addPlot(row = 2, col = 1)
        self.plotItem_vz.setLabel('left', 'Vz', 'm/s')
        self.plotItem_vz.setXRange(-70, 0)

        self.plotDataItem_vz = generatePlot(self.plotItem_vz, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_vz_ref = generatePlot(self.plotItem_vz, LINE, (11, 220, 13), SIZE)
                
        self.plotItem_va = self.ui.graphicsView.addPlot(row = 3, col = 1)
        self.plotItem_va.setLabel('left', 'Va', '°/s')
        self.plotItem_va.setLabel('bottom', 'Time', 's')
        self.plotItem_va.setXRange(-70, 0)

        self.plotDataItem_va = generatePlot(self.plotItem_va, LINE, (204, 0, 0), SIZE)
        self.plotDataItem_va_ref = generatePlot(self.plotItem_va, LINE, (11, 220, 13), SIZE)
        
        self.plotItem_fx = self.ui.graphicsView.addPlot(row = 0, col = 2)
        self.plotItem_fx.setLabel('left', 'Fx', '-')
        self.plotItem_fx.setXRange(-70, 0)

        self.plotDataItem_fx = generatePlot(self.plotItem_fx, LINE, (204, 0, 0), SIZE)
        
        self.plotItem_fy = self.ui.graphicsView.addPlot(row = 1, col = 2)
        self.plotItem_fy.setLabel('left', 'Fy', '-')
        self.plotItem_fy.setXRange(-70, 0)

        self.plotDataItem_fy = generatePlot(self.plotItem_fy, LINE, (204, 0, 0), SIZE)
        
        self.plotItem_fz = self.ui.graphicsView.addPlot(row = 2, col = 2)
        self.plotItem_fz.setLabel('left', 'Fz', '-')
        self.plotItem_fz.setXRange(-70, 0)

        self.plotDataItem_fz = generatePlot(self.plotItem_fz, LINE, (204, 0, 0), SIZE)

        self.plotItem_ma = self.ui.graphicsView.addPlot(row = 3, col = 2)
        self.plotItem_ma.setLabel('left', 'Ma', '-')
        self.plotItem_ma.setLabel('bottom', 'Time', 's')
        self.plotItem_ma.setXRange(-70, 0)

        self.plotDataItem_ma = generatePlot(self.plotItem_ma, LINE, (204, 0, 0), SIZE)
        
        self.plotItem_motors = self.ui.graphicsView.addPlot(row = 0, col = 3, rowspan=4)
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

        self.startTime = pg.ptime.time()

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(250) # in milliseconds
        self.timer.start()
        self.timer.timeout.connect(self.updateData)
        self.now = 0
        self.show()
        
        self.blimps = {}
        
        self.active_blimp = ''
        
        self.client = mqtt.Client(client_id='GUI')
        self.client.username_pw_set(username="testtest",password="testtest")
        self.client.connect(MQTT_HOST, MQTT_PORT, 60)
        print("MQTT client connected to %s on port %d" % (MQTT_HOST, MQTT_PORT))
        self.client.loop_start()
        
        self.client.message_callback_add("manager/+/+/state", self.addBlimp)
        self.client.subscribe("manager/+/+/state")
        
        self.blimpsactive = []
        
        self.updated.connect(self.addPushButton)
        
        self.sem = threading.Semaphore()
        self.t0 = time.time()
                        
    def addPushButton(self, id):

        if not hasattr(self.ui, 'pushButtons'):
            self.ui.pushButtons = []

        i = len(self.ui.pushButtons)
        self.ui.pushButtons.append(QtWidgets.QPushButton(self.ui.groupBox))
        self.ui.pushButtons[i].setGeometry(QtCore.QRect(10, 30 + (len(self.blimpsactive)-1)*90, 131, 81))
        self.ui.pushButtons[i].setStyleSheet("")
        self.ui.pushButtons[i].setFlat(False)
        self.ui.pushButtons[i].setObjectName(id)
        self.ui.pushButtons[i].setText(id)
        self.ui.pushButtons[i].show()
        self.ui.pushButtons[i].clicked.connect(lambda: self.handleButton(self.ui.pushButtons[i]))
        self.sem.release()

        
    def addBlimp(self, client, userdata, msg):
        ident = msg.topic.split('/')[-2]
        
        if ident in self.blimpsactive:
            pass
        else:
            self.sem.acquire()
            self.blimpsactive.append(ident)
            self.blimps[ident] = BlimpData(ident, 10*60*1)
            self.updated.emit(ident)
        
    def updateData(self):
        self.now = pg.ptime.time()
                
        try:
            x, y, z, alpha, vx, vy, vz, valpha, x_ref, y_ref, z_ref, alpha_ref, vx_ref, vy_ref, vz_ref, valpha_ref, fx, fy, fz, malpha, m1, m2, m3, m4, m5, m6, t = self.blimps[self.active_blimp].get_data()
        except:
            return
        
        if t:
            time_relative = np.array(t) - t[-1]
            self.plotDataItem_x.setData(time_relative, x)
            self.plotDataItem_y.setData(time_relative, y)
            self.plotDataItem_z.setData(time_relative, z)
            self.plotDataItem_a.setData(time_relative, alpha)
            
            self.plotDataItem_x_ref.setData(time_relative, x_ref)
            self.plotDataItem_y_ref.setData(time_relative, y_ref)
            self.plotDataItem_z_ref.setData(time_relative, z_ref)
            self.plotDataItem_a_ref.setData(time_relative, alpha_ref)
            
            self.plotDataItem_vx.setData(time_relative, vx)
            self.plotDataItem_vy.setData(time_relative, vy)
            self.plotDataItem_vz.setData(time_relative, vz)
            self.plotDataItem_va.setData(time_relative, valpha)
            
            self.plotDataItem_vx_ref.setData(time_relative, vx_ref)
            self.plotDataItem_vy_ref.setData(time_relative, vy_ref)
            self.plotDataItem_vz_ref.setData(time_relative, vz_ref)
            self.plotDataItem_va_ref.setData(time_relative, valpha_ref)
            
            self.plotDataItem_fx.setData(time_relative, fx)
            self.plotDataItem_fy.setData(time_relative, fy)
            self.plotDataItem_fz.setData(time_relative, fz)
            self.plotDataItem_ma.setData(time_relative, malpha)
            
            self.plotDataItem_motors[0].setData(time_relative, m1)
            self.plotDataItem_motors[1].setData(time_relative, m2)
            self.plotDataItem_motors[2].setData(time_relative, m3)
            self.plotDataItem_motors[3].setData(time_relative, m4)
            self.plotDataItem_motors[4].setData(time_relative, m5)
            self.plotDataItem_motors[5].setData(time_relative, m6)
            
    def handleButton(self, btn):       
        self.active_blimp = btn.text()
            
        for i, button in enumerate(self.ui.pushButtons):
            self.ui.pushButtons[i].setStyleSheet("")
            if button.text() == btn.text():
                self.ui.pushButtons[i].setStyleSheet("background-color:rgb(11, 220, 13);")
    
    def closeEvent(self, event):
        self.timer.stop()
        for key, blimp in self.blimps.items():
            blimp.stop()
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
