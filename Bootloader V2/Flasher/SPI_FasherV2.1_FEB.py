# 31/05 Flasher SPI avec protocole de communication S19 adapté communicant en réseau avec la FEB
# Communique avec la version V2 du Bootloader
# Maj ajout threads avec IHM et check status

import sys
import time
import socket
import codecs
import threading 
import queue as Queue

from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog, QPushButton, QLabel,QProgressBar
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot, QThread, pyqtSignal, Qt
from PyQt5 import QtCore, QtGui, QtWidgets


qfilename = Queue.Queue() #Variable partagé entre les threads de l'adresse du fichier S19
#qstatus = Queue.Queue() #Variable partagé entre les threads du status

class Send_network (QThread, threading.Thread):

    #Variables partagés avec la classe APP
    value_progress = pyqtSignal(int) #Avancement de la barre de progression
    value_max_progress = pyqtSignal(int) #Maximum de la barre de progression
    value_received_spi= pyqtSignal(int) #Valeur d'un signal recu en spi

    def __init__(self):
        super().__init__()
        threading.Thread.__init__(self)
        self.fileName = "" #Lien du fichier S19
        self.checksum_received = 0 #Variable globale de la checksum
        self.response_spi_received = 0 #Variable globale de la reponse SPI
        self.reception_state = "WAITING" #Etat de la reception reseau, WAITING = En attente de reception UDP, RECEIVED = Signal recu 
        self.nb_memory = 0 #Nombre de mot de 32 bits contenu dans la memoire
        self.nb_lines = 0 #Nombre de mot de 32 bits contenu dans la memoire
        self.i_data = 0 #Variable qui s'incrémente à chaque écriture d'un mot de 16 bits dans la memoire

    def listen_network(self, type) :
        """Ecoute du port de la FEB.
            
            Parametres
            ----------
            type : str
            type de communication attendue.

            A chaque reception la fonction change l'état de reception_state à "RECEIVED"
            Pour le protocole de communication :
            Le premier mot recu correspond au status avant ecriture dans la flash
            Le deuxième mot recu correspond au status après écriture dans la flash
            Le troixième mot recu correspond à la chacksum et est stocké dans la variable checksum_received.
        """

        UDP_IP = "192.168.1.205"
        UDP_PORT = 2000

        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.bind((UDP_IP, UDP_PORT))
        i = 0
        MESSAGE_STR = "0x"
        a = self.nb_lines *3
        if type == "WRITING" :
            while a != 0:   
                i += 1
                a -= 1
                data, addr = sock.recvfrom(1024) 
                MESSAGE= codecs.encode(data, "hex")
                MESSAGE_STR += chr(MESSAGE[34])
                MESSAGE_STR += chr(MESSAGE[35])
                MESSAGE_STR += chr(MESSAGE[32])
                MESSAGE_STR += chr(MESSAGE[33])
                #print("received message:" + str(MESSAGE_STR))
                #print("received message:" + str(MESSAGE))
                self.response_spi_received = int(MESSAGE_STR, 16)
                if (i == 3 ) :
                    self.checksum_received = int(MESSAGE_STR, 16)
                    print("checksum:" + str(int(MESSAGE_STR, 16)))
                    i = 0
                    self.reception_state = "RECEIVED"
                MESSAGE_STR = "0x"

        elif type == "SIMPLE_RESPONSE" :
            data, addr = sock.recvfrom(1024)
            MESSAGE= codecs.encode(data, "hex")
            MESSAGE_STR += chr(MESSAGE[34])
            MESSAGE_STR += chr(MESSAGE[35])
            MESSAGE_STR += chr(MESSAGE[32])
            MESSAGE_STR += chr(MESSAGE[33])
            self.response_spi_received = int(MESSAGE_STR, 16)
            self.value_received_spi.emit(self.response_spi_received)
            print(self.response_spi_received)
            #qstatus.put(self.response_spi_received)

    def send_and_receive(self, input):
        th1 = threading.Thread(target=self.SPI_send, args=(input, 0))
        th2 = threading.Thread(target=self.listen_network, args=("SIMPLE_RESPONSE",))
        th1.start()
        th2.start()
        th1.join()
        th2.join()

    def run(self):
        """Lancement des thread de lecture et d'écriture reseau.
        """

        th1 = threading.Thread(target=self.SPI_write)
        th2 = threading.Thread(target=self.listen_network, args=("WRITING",))
        th1.start()
        th2.start()
        th1.join()
        th2.join()

    def chcksum(self,value) :
        """Calcul de la checksum.

            Parametres
            ----------
            value : str
            Ligne du fichier S19 a envoyer

            Prend en entrée la chaine de caractère comprenant la taille, l'addresse et la memoire.
            On convertie la chaine de caractère en binaire et on fait la somme de tout les bits à 1.
        """

        ck = 0
        table = [int(x) for x in bin(value)[2:]]
        for i in range (len(table)) :
            if table[i] == 1 :
                ck += 1
        return ck       

    def construct_trame(self,input, type) :
        """Construction de la trame a envoyer sur le reseau.

            Parametres 
            ----------
            input : str
            Message a envoyer.
            type : int
            Definis la commande d'écriture ou de lecture. 1 : ecriture, 0 : lecture
        """

        Header = "aaaaaaaac0cc000047300101"
        Footer = "aaaaaaaa"
        Header_read = "aaaaaaaac0cc000067400101"
        Footer_read = "aaaaaaaa"
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
        if type == 1 :
            MESSAGE = Header + finalvalue + Footer
        else :
            MESSAGE = Header_read + finalvalue + Footer_read
        #print(MESSAGE)
        MESSAGE= codecs.decode(MESSAGE, "hex")
        return MESSAGE

    def SPI_send(self, input, type):
        """Envoie du message en reseau

            Parametres 
            ----------
            input : str
            Message a envoyer.
            type : int
            Definis la commande d'écriture ou de lecture. 1 : ecriture, 0 : lecture

            Envoie le message en input après construction de la trame.
        """

        UDP_IP = "192.168.1.21"
        UDP_PORT = 1234
        time.sleep(0.03)
        MESSAGE = self.construct_trame(str(input), type)
        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))


    def SPI_write(self) :
        """Envoie du fichier S19 pour reflasher le microcontroleur

            Envoie les données du S19 en respectant le protocole de communication
            La fonction fonctionne en parrallèle de la fonction listen_network pour lire les reponses du microcontroleur.
        """
        self.fileName = qfilename.get()
        self.count_memory_size() #Calcul du nombre de mot a envoyer
        self.value_max_progress.emit(self.nb_memory)
        f = open(self.fileName, "r")
        f.read(11) #Entete
        flag = 0
        checksum = 0
        self.i_data = 0
        self.value_progress.emit(self.i_data)
        while flag != 1 :
            #L'envoie ne s'arrête que lorsque la dernière ligne commence par "S7"
            if (f.read(2) == "S7") :
                flag = 1
            else :
                nb_value = f.read(2)
                length = int((int(nb_value, 16)*2 - 2)/8)-1
                self.i_data += length
                self.value_progress.emit(self.i_data)
                self.SPI_send (300, 1) #Envoie de la commande d'écriture
                self.SPI_send(length, 1) #Envoie de la taille
                checksum += self.chcksum(length) 
                address_msb = int(f.read(4), 16) 
                checksum += self.chcksum(address_msb) 
                self.SPI_send(address_msb, 1) #Envoie de l'adresse msb
                address_lsb = int(f.read(4), 16)
                checksum += self.chcksum(address_lsb) 
                self.SPI_send(address_lsb, 1) #Envoie de l'adresse lsb
                self.SPI_send(310, 0)  #Commande read status

                #Envoie de la memoire
                for i in range (length*2) :
                    value_to_send = int(f.read(4), 16)
                    self.SPI_send(value_to_send, 1)
                    checksum += self.chcksum(value_to_send) 

                self.SPI_send(310, 0) #Commande read status
                self.SPI_send(330, 0) #Commande read checksum

                self.wait_until(1) #En attente de la reception de la checksum

                if (self.checksum_received  == checksum): 
                    print("Checksum OK")
                else :
                    print("Checksum KO")
                f.read(3) #Checksum
                print("checksum calculated : " + str(checksum))
                print("checksum received : " + str(self.checksum_received))
                checksum = 0
        f.close()

    def wait_until(self, timeout):
        """Validation de reception du retour reseau.

            Parametres :
            ----------
            timeout : int
            Attente maximum

            Attend la reception du retour du message reseau
            Sort de la boucle si le timeout est atteint.
        """
        mustend = time.time() + timeout
        while time.time() < mustend:
            if self.reception_state == "RECEIVED": 
                self.reception_state = "WAITING"
                return True
        return False

    def count_memory_size(self):
        """Calcule le nombre de mot dans le fichier S19.
        """

        f = open(self.fileName, "r")
        f.read(11)
        self.nb_memory = 0
        flag = 0
        while flag != 1 :
            if (f.read(2) == "S7") :
                flag = 1
            else :
                nb_value = f.read(2)
                #print(nb_value)
                length = int((int(nb_value, 16)*2 - 2)/8)-1
                self.nb_memory +=  length
                self.nb_lines += 1
                f.read(int(nb_value, 16)*2+1)
        f.close()

    def File_conversion(self):
        """Convertie le fichier S19 en memoire brut.

            Fonction non utilisée.
        """
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
        self.net = Send_network()
    
    def initUI(self):
        self.resize(750, 500)
        self.centralwidget = QtWidgets.QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        self.progress = QtWidgets.QProgressBar(self.centralwidget)
        self.progress.setGeometry(QtCore.QRect(150, 130, 411, 31))
        self.progress.setMaximum(100) # A modifier
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
        self.label_status = QtWidgets.QLabel(self.centralwidget)
        self.label_status.setGeometry(QtCore.QRect(140, 370, 111, 31))
        self.label_status.setObjectName("label_status")
        
        self.pushButton.setText("Launch Program")
        self.pushButton_2.setText("Flash memory")
        self.label.setText("FLASHER SPI")
        self.label_2.setText("File")
        self.label_status.setText("...")
        self.pushButton_3.setText("Select File")
        self.label_3.setText("SPI Communication")
        self.pushButton_4.setText("Send SPI")
        self.pushButton_5.setText("Check status")
        
        
        self.pushButton_3.clicked.connect(self.on_click)
        self.pushButton_2.clicked.connect(self.on_click_send_SPI)
        self.pushButton.clicked.connect(self.on_click_jump)
        self.pushButton_4.clicked.connect(self.on_click_SEND_SPI)
        self.pushButton_5.clicked.connect(self.on_click_check_status)
        self.show()
    
    def openFileNameDialog(self):
        global filename
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        self.net.fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if self.net.fileName :
            qfilename.put(self.net.fileName )
            self.label_2.setText(self.net.fileName)

            
    @pyqtSlot()
    def on_click(self):
        self.openFileNameDialog()
    
    def on_click_send_SPI(self):
        if self.net.fileName != "" :
            # self.net.run()
            self.thread = Send_network()
            self.thread.value_progress.connect(self.setProgressVal)
            self.thread.value_max_progress.connect(self.setProgressMax)
            self.thread.start()
        else :
            print("Merci de joindre un fichier")

    def on_click_check_status(self):
        self.net.send_and_receive(310)

    def setStatus(self, val):
        self.label_status.setText(val)

    def setProgressVal(self, val):
        self.progress.setValue(val)

    def setProgressMax(self, val):
        self.progress.setMaximum(val)

    def on_click_jump(self):
        self.net.SPI_send(700, 1)
        
    def Check_status(self):
        self.net.send_and_receive(700)
        
    def on_click_SEND_SPI(self):
        self.net.send_and_receive(self.textbox.text())

if __name__ == '__main__':    
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
