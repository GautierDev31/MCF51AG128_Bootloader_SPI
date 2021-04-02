import sys
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog, QPushButton, QLabel,QProgressBar
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot
import time
import spidev

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 976000

TIME_LIMIT = 100

def SPI_send(input):
    msb = input >> 8
    lsb = input & 0xFF
    response = spi.xfer([msb, lsb])
    Value = response[1] + (response[0] << 8);
    print("Value sended : ", input)
    print("Response :", Value)

def SPI_write(self) :
    length = self.Size_File_16bits/4
    self.progress.setMaximum(length - 1)
    f = open("BrutMemory.txt", "r")
    cmd = int(80)
    msb = int(cmd) >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    
    cmd = length
    msb = int(cmd) >> 8
    lsb = int(cmd) & 0xFF
    response = spi.xfer([msb, lsb])
    
    for i in range (int(length)):
            Brut_memory = f.read(4)
            Brut_memory_int = int(Brut_memory, 16)
            cmd = Brut_memory_int
            msb = cmd >> 8
            lsb = cmd & 0xFF
            response = spi.xfer([msb, lsb])
            Value = response[1] + (response[0] << 8);
            print("Value sended : ", cmd)
            print("Response :", Value)
            time.sleep(0.01)
            self.progress.setValue(i)
    cmd = int(81)
    msb = int(cmd) >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    f.close()

def File_conversion(self):
    self.Size_File_16bits = 0
    f = open(self.fileName, "r")
    f2 = open("BrutMemory.txt", "w")
    flag = 0
    print("Converting S19 file to brut memory...")

    # HEADER
    f.read(10)
    for i in range (115):
        f.read(23)

    # MEMORY BLOCK
    while(flag != 1):
        f.read(3)
        nb = int(f.read(2), 16)
        nb_caracters = (nb * 2) - 10
        self.Size_File_16bits += nb_caracters
        if nb_caracters <= 1 :
            flag = 1
        f.read(8)
        if flag != 1:
            f2.write(f.read(nb_caracters))
        f.read(2)

    print(self.Size_File_16bits)
    print("File converted")
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
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        
        self.progress = QProgressBar(self)
        self.progress.setGeometry(0, 0, 300, 25)
        self.progress.setMaximum(100)
        self.progress.move(10, 200)
        
        self.textbox = QLineEdit(self)
        self.textbox.move(130, 80)
        self.textbox.resize(40,25)

        self.label = QLabel(self)
        self.label.setText('.....................................................')
        self.label.move (100, 15)
        
        button = QPushButton('Select S19 file', self)
        button.move(10,10)
        button.clicked.connect(self.on_click)
        
        button = QPushButton("Flash memory", self)
        button.move(10,50)
        button.clicked.connect(self.on_click_send_SPI)
        
        button = QPushButton("Launch program", self)
        button.move(10,80)
        button.clicked.connect(self.on_click_jump)
        
        self.show()
    
    def openFileNameDialog(self):
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        self.fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if self.fileName:
            self.label.setText(self.fileName)

            
    @pyqtSlot()
    def on_click(self):
        self.openFileNameDialog()
    
    def on_click_send_SPI(self):
        File_conversion(self)
        SPI_write(self)

            
    def on_click_jump(self):
        textboxValue = self.textbox.text()
        SPI_send(130)
        #SPI_send(int(textboxValue))

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
