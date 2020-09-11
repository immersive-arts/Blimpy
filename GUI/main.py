from gui import Ui_MainWindow
from PyQt5 import QtWidgets, QtCore
import sys
from collections import deque
import pyqtgraph as pg
import numpy as np
import socket
import struct
import threading
import time

class BlimpData():
    def __init__(self, L, group, port):
        self._x = deque([], maxlen=L)
        self._y = deque([], maxlen=L)
        self._z = deque([], maxlen=L)
        self._a = deque([], maxlen=L)
        self._vx = deque([], maxlen=L)
        self._vy = deque([], maxlen=L)
        self._vz = deque([], maxlen=L)
        self._va = deque([], maxlen=L)
        self._x_ref = deque([], maxlen=L)
        self._y_ref = deque([], maxlen=L)
        self._z_ref = deque([], maxlen=L)
        self._a_ref = deque([], maxlen=L)
        self._vx_ref = deque([], maxlen=L)
        self._vy_ref = deque([], maxlen=L)
        self._vz_ref = deque([], maxlen=L)
        self._va_ref = deque([], maxlen=L)
        self._fx = deque([], maxlen=L)
        self._fy = deque([], maxlen=L)
        self._fz = deque([], maxlen=L)
        self._ma = deque([], maxlen=L)
        self._t = deque([], maxlen=L)
        self._m1 = deque([], maxlen=L)
        self._m2 = deque([], maxlen=L)
        self._m3 = deque([], maxlen=L)
        self._m4 = deque([], maxlen=L)
        self._m5 = deque([], maxlen=L)
        self._m6 = deque([], maxlen=L)
        
        self._group = group
        self._port = port
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('', port))
        mreq = struct.pack("4sl", socket.inet_aton(group), socket.INADDR_ANY)
        self._sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self._data = np.zeros(6)
        
        self._lock = threading.Lock()
        self._run = True
        
        self._thread = threading.Thread(target=self.subscribe)
        self._thread.start()
        self._data_available = False
        
        self.t0 = time.time()
                
    def subscribe(self):
        while self._run:
            try:
                self._sock.settimeout(1.0)
                data, address = self._sock.recvfrom(10000)
                with self._lock:
                    data = struct.unpack('<26f',data)
                    self._x.append(data[0])
                    self._y.append(data[1])
                    self._z.append(data[2])
                    self._a.append(data[3])
                    self._vx.append(data[4])
                    self._vy.append(data[5])
                    self._vz.append(data[6])
                    self._va.append(data[7])
                    self._x_ref.append(data[8])
                    self._y_ref.append(data[9])
                    self._z_ref.append(data[10])
                    self._a_ref.append(data[11])
                    self._vx_ref.append(data[12])
                    self._vy_ref.append(data[13])
                    self._vz_ref.append(data[14])
                    self._va_ref.append(data[15])
                    self._fx.append(data[16])
                    self._fy.append(data[17])
                    self._fz.append(data[18])
                    self._ma.append(data[19])
                    self._m1.append(data[20])
                    self._m2.append(data[21])
                    self._m3.append(data[22])
                    self._m4.append(data[23])
                    self._m5.append(data[24])
                    self._m6.append(data[25])

                    self._t.append(time.time() - self.t0)
                    self._data_available = True    
            except:
                pass
                        
    def get_data(self):
        with self._lock:
            self._data_available = False
            return self._x, self._y, self._z, self._a, self._vx, self._vy, self._vz, self._va, self._x_ref, self._y_ref, self._z_ref, self._a_ref, self._vx_ref, self._vy_ref, self._vz_ref, self._va_ref, self._fx, self._fy, self._fz, self._ma, self._m1, self._m2, self._m3, self._m4, self._m5, self._m6, self._t
        
    def data_available(self):
        return self._data_available      
        
    def stop(self):
        self._run = False


class Ui(QtWidgets.QMainWindow):
    def __init__(self):
        super(Ui, self).__init__()
        self.ui = Ui_MainWindow()        
        self.ui.setupUi(self)    
            
        self.plotItem_x = self.ui.graphicsView.addPlot(row = 0, col = 0)
        self.plotItem_x.setLabel('left', 'X', 'm')
        self.plotItem_x.setXRange(-70, 0)

        self.plotDataItem_x = self.plotItem_x.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_x_ref = self.plotItem_x.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
        
        self.plotItem_y = self.ui.graphicsView.addPlot(row = 1, col = 0)
        self.plotItem_y.setLabel('left', 'Y', 'm')
        self.plotItem_y.setXRange(-70, 0)

        self.plotDataItem_y = self.plotItem_y.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_y_ref = self.plotItem_y.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
                
        self.plotItem_z = self.ui.graphicsView.addPlot(row = 2, col = 0)
        self.plotItem_z.setLabel('left', 'Z', 'm')
        self.plotItem_z.setXRange(-70, 0)

        self.plotDataItem_z = self.plotItem_z.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_z_ref = self.plotItem_z.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
                
        self.plotItem_a = self.ui.graphicsView.addPlot(row = 3, col = 0)
        self.plotItem_a.setLabel('left', 'Alpha', '°')
        self.plotItem_a.setLabel('bottom', 'Time', 's')
        self.plotItem_a.setXRange(-70, 0)

        self.plotDataItem_a = self.plotItem_a.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_a_ref = self.plotItem_a.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
        
        self.plotItem_vx = self.ui.graphicsView.addPlot(row = 0, col = 1)
        self.plotItem_vx.setLabel('left', 'Vx', 'm/s')
        self.plotItem_vx.setXRange(-70, 0)

        self.plotDataItem_vx = self.plotItem_vx.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_vx_ref = self.plotItem_vx.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
        
        self.plotItem_vy = self.ui.graphicsView.addPlot(row = 1, col = 1)
        self.plotItem_vy.setLabel('left', 'Vy', 'm/s')
        self.plotItem_vy.setXRange(-70, 0)

        self.plotDataItem_vy = self.plotItem_vy.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_vy_ref = self.plotItem_vy.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
                
        self.plotItem_vz = self.ui.graphicsView.addPlot(row = 2, col = 1)
        self.plotItem_vz.setLabel('left', 'Vz', 'm/s')
        self.plotItem_vz.setXRange(-70, 0)

        self.plotDataItem_vz = self.plotItem_vz.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_vz_ref = self.plotItem_vz.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
                
        self.plotItem_va = self.ui.graphicsView.addPlot(row = 3, col = 1)
        self.plotItem_va.setLabel('left', 'Va', '°/s')
        self.plotItem_va.setLabel('bottom', 'Time', 's')
        self.plotItem_va.setXRange(-70, 0)

        self.plotDataItem_va = self.plotItem_va.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        self.plotDataItem_va_ref = self.plotItem_va.plot([], pen=None, 
            symbolBrush=(11,220,13), symbolSize=5, symbolPen=None)
        
        self.plotItem_fx = self.ui.graphicsView.addPlot(row = 0, col = 2)
        self.plotItem_fx.setLabel('left', 'Fx', '-')
        self.plotItem_fx.setXRange(-70, 0)

        self.plotDataItem_fx = self.plotItem_fx.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        
        self.plotItem_fy = self.ui.graphicsView.addPlot(row = 1, col = 2)
        self.plotItem_fy.setLabel('left', 'Fy', '-')
        self.plotItem_fy.setXRange(-70, 0)

        self.plotDataItem_fy = self.plotItem_fy.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        
        self.plotItem_fz = self.ui.graphicsView.addPlot(row = 2, col = 2)
        self.plotItem_fz.setLabel('left', 'Fz', '-')
        self.plotItem_fz.setXRange(-70, 0)

        self.plotDataItem_fz = self.plotItem_fz.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)

        self.plotItem_ma = self.ui.graphicsView.addPlot(row = 3, col = 2)
        self.plotItem_ma.setLabel('left', 'Ma', '-')
        self.plotItem_ma.setLabel('bottom', 'Time', 's')
        self.plotItem_ma.setXRange(-70, 0)

        self.plotDataItem_ma = self.plotItem_ma.plot([], pen=None, 
            symbolBrush=(204,0,0), symbolSize=5, symbolPen=None)
        
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
            self.plotDataItem_motors.append(self.plotItem_motors.plot([], pen=None, symbolSize=5, symbolBrush=(col[i,0],col[i,1],col[i,2]), symbolPen=None, name='motor ' + str(i + 1)))
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
        
        self.blimps = []
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7001))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7002))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7003))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7004))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7005))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7006))
        self.blimps.append(BlimpData(10*60*1, '224.1.1.1', 7007))
        
        self.active_blimp = 1
        
        self.ui.pushButton.clicked.connect(self.handleButton1)
        self.ui.pushButton_2.clicked.connect(self.handleButton2)
        self.ui.pushButton_3.clicked.connect(self.handleButton3)
        self.ui.pushButton_4.clicked.connect(self.handleButton4)
        self.ui.pushButton_5.clicked.connect(self.handleButton5)
        self.ui.pushButton_6.clicked.connect(self.handleButton6)
        self.ui.pushButton_7.clicked.connect(self.handleButton7)
        
    def updateData(self):
        self.now = pg.ptime.time()

        x, y, z, alpha, vx, vy, vz, valpha, x_ref, y_ref, z_ref, alpha_ref, vx_ref, vy_ref, vz_ref, valpha_ref, fx, fy, fz, malpha, m1, m2, m3, m4, m5, m6, t = self.blimps[self.active_blimp-1].get_data()
        if t:
            self.plotDataItem_x.setData(t, x)
            self.plotDataItem_x.setPos(-t[-1], 0)
            self.plotDataItem_y.setData(t, y)
            self.plotDataItem_y.setPos(-t[-1], 0)
            self.plotDataItem_z.setData(t, z)
            self.plotDataItem_z.setPos(-t[-1], 0)
            self.plotDataItem_a.setData(t, alpha)
            self.plotDataItem_a.setPos(-t[-1], 0)
            
            self.plotDataItem_x_ref.setData(t, x_ref)
            self.plotDataItem_x_ref.setPos(-t[-1], 0)
            self.plotDataItem_y_ref.setData(t, y_ref)
            self.plotDataItem_y_ref.setPos(-t[-1], 0)
            self.plotDataItem_z_ref.setData(t, z_ref)
            self.plotDataItem_z_ref.setPos(-t[-1], 0)
            self.plotDataItem_a_ref.setData(t, alpha_ref)
            self.plotDataItem_a_ref.setPos(-t[-1], 0)
            
            self.plotDataItem_vx.setData(t, vx)
            self.plotDataItem_vx.setPos(-t[-1], 0)
            self.plotDataItem_vy.setData(t, vy)
            self.plotDataItem_vy.setPos(-t[-1], 0)
            self.plotDataItem_vz.setData(t, vz)
            self.plotDataItem_vz.setPos(-t[-1], 0)
            self.plotDataItem_va.setData(t, valpha)
            self.plotDataItem_va.setPos(-t[-1], 0)
            
            self.plotDataItem_vx_ref.setData(t, vx_ref)
            self.plotDataItem_vx_ref.setPos(-t[-1], 0)
            self.plotDataItem_vy_ref.setData(t, vy_ref)
            self.plotDataItem_vy_ref.setPos(-t[-1], 0)
            self.plotDataItem_vz_ref.setData(t, vz_ref)
            self.plotDataItem_vz_ref.setPos(-t[-1], 0)
            self.plotDataItem_va_ref.setData(t, valpha_ref)
            self.plotDataItem_va_ref.setPos(-t[-1], 0)
            
            self.plotDataItem_fx.setData(t, fx)
            self.plotDataItem_fx.setPos(-t[-1], 0)
            self.plotDataItem_fy.setData(t, fy)
            self.plotDataItem_fy.setPos(-t[-1], 0)
            self.plotDataItem_fz.setData(t, fz)
            self.plotDataItem_fz.setPos(-t[-1], 0)
            self.plotDataItem_ma.setData(t, malpha)
            self.plotDataItem_ma.setPos(-t[-1], 0)
            
            self.plotDataItem_motors[0].setData(t, m1)
            self.plotDataItem_motors[1].setData(t, m2)
            self.plotDataItem_motors[2].setData(t, m3)
            self.plotDataItem_motors[3].setData(t, m4)
            self.plotDataItem_motors[4].setData(t, m5)
            self.plotDataItem_motors[5].setData(t, m6)

            self.plotDataItem_motors[0].setPos(-t[-1], 0)
            self.plotDataItem_motors[1].setPos(-t[-1], 0)
            self.plotDataItem_motors[2].setPos(-t[-1], 0)
            self.plotDataItem_motors[3].setPos(-t[-1], 0)
            self.plotDataItem_motors[4].setPos(-t[-1], 0)
            self.plotDataItem_motors[5].setPos(-t[-1], 0)

    def handleButton1(self):
        self.active_blimp = 1
        self.clearButtons()
        self.ui.pushButton.setStyleSheet("background-color:rgb(11, 220, 13);")
            
    def handleButton2(self):
        self.active_blimp = 2
        self.clearButtons()
        self.ui.pushButton_2.setStyleSheet("background-color:rgb(11, 220, 13);")
                
    def handleButton3(self):
        self.active_blimp = 3
        self.clearButtons()
        self.ui.pushButton_3.setStyleSheet("background-color:rgb(11, 220, 13);")
                
    def handleButton4(self):
        self.active_blimp = 4
        self.clearButtons()
        self.ui.pushButton_4.setStyleSheet("background-color:rgb(11, 220, 13);")
                
    def handleButton5(self):
        self.active_blimp = 5
        self.clearButtons()
        self.ui.pushButton_5.setStyleSheet("background-color:rgb(11, 220, 13);")
            
    def handleButton6(self):
        self.active_blimp = 6
        self.clearButtons()
        self.ui.pushButton_6.setStyleSheet("background-color:rgb(11, 220, 13);")
                
    def handleButton7(self):
        self.active_blimp = 7
        self.clearButtons()
        self.ui.pushButton_7.setStyleSheet("background-color:rgb(11, 220, 13);")
        
    def clearButtons(self):
        self.ui.pushButton.setStyleSheet("")
        self.ui.pushButton_2.setStyleSheet("")
        self.ui.pushButton_3.setStyleSheet("")
        self.ui.pushButton_4.setStyleSheet("")
        self.ui.pushButton_5.setStyleSheet("")
        self.ui.pushButton_6.setStyleSheet("")
        self.ui.pushButton_7.setStyleSheet("")
    
    def closeEvent(self, event):
        self.timer.stop()
        for blimp in self.blimps:
            blimp.stop()
        event.accept()        

app = QtWidgets.QApplication(sys.argv)
window = Ui()
app.exec_()
