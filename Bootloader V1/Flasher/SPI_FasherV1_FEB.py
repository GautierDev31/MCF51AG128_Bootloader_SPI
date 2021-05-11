# 11/05 Flasher SPI avec protocole de communication BRUT adapté communicant en réseau avec la FEB
# Communique avec la version V1 du bootloader

import sys
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog, QPushButton, QLabel,QProgressBar
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot
import time
import spidev

from PyQt5 import QtCore, QtGui, QtWidgets
spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 976000

TIME_LIMIT = 100

def construct_trame(input) :

    input = '{:x}'.format(int(input))
    inputvalue = ""
    finalvalue = ""
    for i in range (8 - len(input)) :
        inputvalue = inputvalue + "0"
    inputvalue += input
    finalvalue += inputvalue[6]
    finalvalue += inputvalue[7]
    finalvalue += inputvalue[4]
    finalvalue += inputvalue[5]
    finalvalue += inputvalue[2]
    finalvalue += inputvalue[3]
    finalvalue += inputvalue[0]
    finalvalue += inputvalue[1]
    inputvalue = input
    for i in range (8 - len(input)) :
        inputvalue = inputvalue + "0"
    MESSAGE = Header + finalvalue + Footer
    print(MESSAGE)
    MESSAGE= codecs.decode(MESSAGE, "hex")
    return MESSAGE

def SPI_send(input):
    time.sleep(0.02)
    MESSAGE = construct_trame(str(input))
    sock = socket.socket(socket.AF_INET, # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

def SPI_write(self) :
    length = int(self.Size_File_16bits/4)
    self.progress.setMaximum(length - 1)
    f = open("BrutMemory.txt", "r")
    SPI_send(80)
    
    SPI_send(length - 115*2)
    self.cpttemp = 0
    print(cmd)
    for i in range (int(length)):
            Brut_memory = f.read(4)
            Brut_memory_int = int(Brut_memory, 16)
            SPI_send(Brut_memory_int)
            time.sleep(0.03)
            self.progress.setValue(i)
            self.cpttemp += 1
    SPI_send(81)
    f.close()

def File_conversion(self):
    self.Size_File_16bits = 0
    f = open(self.fileName, "r")
    f2 = open("BrutMemory.txt", "w")
    flag = 0
    print("Converting S19 file to brut memory...")

    # HEADER + VECTORS
    f.read(11)
    for i in range (115):
        f.read(12)
        self.Size_File_16bits += 8
        f2.write(f.read(8))
        f.read(3)
        
    # MEMORY BLOCK
    while(flag != 1):
        f.read(2)
        nb = int(f.read(2), 16)
        nb_caracters = (nb * 2) - 10
        self.Size_File_16bits += nb_caracters
        print(nb_caracters)
        if nb_caracters <= 1 :
            flag = 1
        f.read(8)
        if flag != 1:
            f2.write(f.read(nb_caracters))
        f.read(3)

    print(self.Size_File_16bits)
    print("File converted")
    print("Size : " + str(self.Size_File_16bits))
    f.close()
    f2.close()

class App(QWidget):

    def __init__(self):
        super().__init__()
        self.title = 'MCF51AG128 Flasher SPI'
        self.left = 10
        self.top = 10
        self.width = 640
        self.height = 480
        self.initUI()
    
    def initUI(self):
        self.resize(750, 500)
        self.centralwidget = QtWidgets.QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        self.progress = QtWidgets.QProgressBar(self.centralwidget)
        self.progress.setGeometry(QtCore.QRect(150, 130, 411, 31))
        # self.progressBar.setProperty("value", 24)
        self.progress.setObjectName("progressBar")
        self.pushButton = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton.setGeometry(QtCore.QRect(20, 180, 111, 31))
        self.pushButton.setObjectName("pushButton")
        self.pushButton_2 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_2.setGeometry(QtCore.QRect(20, 130, 111, 31))
        self.pushButton_2.setObjectName("pushButton_2")
        self.label = QtWidgets.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(20, 20, 211, 31))
        self.label.setObjectName("label")
        self.label_2 = QtWidgets.QLabel(self.centralwidget)
        self.label_2.setGeometry(QtCore.QRect(150, 90, 381, 21))
        self.label_2.setObjectName("label_2")
        self.pushButton_3 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_3.setGeometry(QtCore.QRect(20, 80, 111, 31))
        self.pushButton_3.setObjectName("pushButton_3")
        self.label_3 = QtWidgets.QLabel(self.centralwidget)
        self.label_3.setGeometry(QtCore.QRect(20, 250, 211, 31))
        self.label_3.setObjectName("label_3")
        self.pushButton_4 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_4.setGeometry(QtCore.QRect(20, 300, 111, 31))
        self.pushButton_4.setObjectName("pushButton_4")
        self.textbox = QLineEdit(self)
        self.textbox.setGeometry(QtCore.QRect(150, 300, 171, 31))
        self.textbox.setObjectName("textEdit")
        self.pushButton_5 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_5.setGeometry(QtCore.QRect(20, 370, 111, 31))
        self.pushButton_5.setObjectName("pushButton_5")
        self.pushButton_6 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_6.setGeometry(QtCore.QRect(140, 370, 111, 31))
        
        self.pushButton_6.setObjectName("pushButton_6")
        self.pushButton.setText("Launch Program")
        self.pushButton_2.setText("Flash memory")
        self.label.setText("FLASHER SPI")
        self.label_2.setText("File")
        self.pushButton_3.setText("Select File")
        self.label_3.setText("SPI Communication")
        self.pushButton_4.setText("Send SPI")
        self.pushButton_5.setText("LED ON")
        self.pushButton_6.setText("LED OFF")
        
        
        self.pushButton_3.clicked.connect(self.on_click)
        self.pushButton_2.clicked.connect(self.on_click_send_SPI)
        self.pushButton.clicked.connect(self.on_click_jump)
        self.pushButton_4.clicked.connect(self.on_click_SEND_SPI)
        self.pushButton_5.clicked.connect(self.on_click_LED_ON)
        self.pushButton_6.clicked.connect(self.on_click_LED_OFF)
        self.show()
    
    def openFileNameDialog(self):
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        self.fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if self.fileName:
            self.label_2.setText(self.fileName)

            
    @pyqtSlot()
    def on_click(self):
        self.openFileNameDialog()
    
    def on_click_send_SPI(self):
        File_conversion(self)
        SPI_write(self)

            
    def on_click_jump(self):
        SPI_send(130)
        
    def on_click_LED_ON(self):
        SPI_send(10)
        
    def on_click_LED_OFF(self):
        SPI_send(1)
        
    def on_click_SEND_SPI(self):
        SPI_send(int(self.textbox.text()))

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())