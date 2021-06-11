# 07/06 Flasher SPI avec protocole de communication S19 adapté communicant en réseau avec la FEB
# Code et protocole de communication (README.md) : https://github.com/GautierDev31/MCF51AG128_Bootloader_SPI

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

    def run(self):
        """Lancement des thread de lecture et d'écriture de la flash.
        """

        th1 = threading.Thread(target=self.Flash_write_from_file)
        th2 = threading.Thread(target=self.Trame_reception, args=(th1, "WRITING",))
        th1.start()
        th2.start()
        th1.join()
        th2.join()

    def Run_send_and_receive_one_word(self, input):
        """Lancement des thread de lecture et d'écriture pour ecrire un seul mot.
        """
        th1 = threading.Thread(target=self.Send_one_word, args=(input, 0))
        th2 = threading.Thread(target=self.Trame_reception, args=(th1, "SIMPLE_RESPONSE",))
        th1.start()
        th2.start()
        th1.join()
        th2.join()

    def Run_send_one_trame(self, address, memory):
        """Lancement des thread de lecture et d'écriture pour ecrire une seule trame.
        """
        self.nb_lines = 1
        th1 = threading.Thread(target=self.Flash_write_one_trame, args=(address,memory))
        th2 = threading.Thread(target=self.Trame_reception, args=(th1, "WRITING",))
        th1.start()
        th2.start()
        th1.join()
        th2.join()

    def Trame_reception(self, thread, type) :
        """Ecoute du port de la FEB.
            
            Parametres
            ----------
            type : str
            type de communication attendue.
            
            A chaque reception la fonction change l'état de reception_state à "RECEIVED"
            Pour le protocole de communication (type WRITTING):
            Le premier mot recu correspond au status avant ecriture dans la flash
            Le deuxième mot recu correspond au status après écriture dans la flash
            Le troixième mot recu correspond à la chacksum et est stocké dans la variable checksum_received.
        """

        UDP_IP = "192.168.1.205"
        UDP_PORT = 2000

        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.bind((UDP_IP, UDP_PORT))
        MESSAGE_STR = "0x"
        CODE_STATUS = ""
        a = self.nb_lines*2
        if type == "WRITING" :
            while thread.isAlive() :   
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

                """
                if (self.response_spi_received == 5):
                    CODE_STATUS = "Erreur : Zone memoire protege"
                elif (self.response_spi_received == 10):
                    CODE_STATUS = "OK"
                elif(self.response_spi_received == 20):
                    CODE_STATUS = "Occupé"
                elif(self.response_spi_received == 30):
                    CODE_STATUS = "Erreur : Processus d'écriture décallé, redemarrez le microcontroleur"
                else :
                    CODE_STATUS = "Erreur : Code de retour inconnu"
                """
                self.checksum_received = int(MESSAGE_STR, 16)
                print("checksum:" + str(int(MESSAGE_STR, 16)))
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

    def Flash_write_from_file(self) :
        """Envoie du fichier S19 pour reflasher le microcontroleur

            Envoie les données du S19 en respectant le protocole de communication
            La fonction fonctionne en parrallèle de la fonction Trame_reception pour lire les reponses du microcontroleur.
        """
        self.fileName = qfilename.get()
        self.File_analyse() #Calcul du nombre de mot a envoyer
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
                print("----- SEND TRAME -----") 
                nb_value = f.read(2)
                length = int((int(nb_value, 16)*2 - 2)/8)-1

                self.value_progress.emit(self.i_data)
                self.Send_one_word (300, 1) #Envoie de la commande d'écriture
                self.Send_one_word(length, 1) #Envoie de la taille
                checksum += self.chcksum(length) 

                address_msb = int(f.read(4), 16) 
                checksum += self.chcksum(address_msb) 
                self.Send_one_word(address_msb, 1) #Envoie de l'adresse msb
                address_lsb = int(f.read(4), 16)
                checksum += self.chcksum(address_lsb) 
                self.Send_one_word(address_lsb, 1) #Envoie de l'adresse lsb

                #Envoie de la memoire
                for i in range (length*2) :
                    value_to_send = int(f.read(4), 16)
                    self.Send_one_word(value_to_send, 1)
                    checksum += self.chcksum(value_to_send) 
                    self.value_progress.emit
                    self.i_data += 0.5

                self.Send_one_word(420, 1)  #Demande Checksum
                self.Send_one_word(420, 0)  #Read checksum
                self.Wait_until_trame_reception(1)
                if (self.checksum_received == checksum ):
                    print("Checksum OK!")
                else :
                    print("Checksum calculated :" + str(checksum))
                    print("Checksum received :" + str(self.checksum_received ))
                    print("Checksum KO !!!")
                checksum = 0
                self.Send_one_word(310, 1)  #Ecriture en flash
                f.read(3) #Checksum
        print("------ END ------")        
        f.close()

    def Send_one_word(self, input, type):
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
        time.sleep(0.005)
        print ("Send : " + str(input))
        MESSAGE = self.construct_trame(str(input), type)
        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

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

        """ Bootloader V2.1
        Header = "aaaaaaaac0cc000047300101"
        Footer = "aaaaaaaa"
        Header_read = "aaaaaaaac0cc000067400101"
        Footer_read = "aaaaaaaa"
        """

        Header = "aaaaaaaaC0CC000087300101"
        Footer = "aaaaaaaa"
        Header_read = "aaaaaaaaC0CC0000A7400141"
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

    def Wait_until_trame_reception(self, timeout):
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

    def File_analyse(self):
        """Calcule le nombre de mots et de lignes dans le fichier S19.
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

    def Flash_write_one_trame(self, address, memory):
        address_msb = int(address[0:4], 16)
        address_lsb = int(address[4:8], 16)
        memory_msb = int(memory[0:4], 16)
        memory_lsb = int(memory[4:8], 16)
        print("------- SEND ONE TRAME -------")
        print( "address : " + str (address_msb) + " " + str (address_lsb))
        print( "memory value : " + str (memory_msb) + str (memory_lsb))
        self.Send_one_word (300, 1) #CMD Write
        self.Send_one_word (1, 1) #Length
        self.Send_one_word (address_msb, 1) #address msb
        self.Send_one_word (address_lsb, 1) #address msb
        self.Send_one_word (memory_msb, 1) #memory msb
        self.Send_one_word (memory_lsb, 1) #memory msb
        self.Send_one_word (310, 1) #Flash memory sector


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
        #Declaration des éléments
        self.resize(750, 500)
        self.centralwidget = QtWidgets.QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        #Flasher SPI title   
        self.label = QtWidgets.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(20, 20, 211, 31))
        self.label.setObjectName("label")
        #Select file
        self.pushButton_SelectFile = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_SelectFile.setGeometry(QtCore.QRect(20, 80, 111, 31))
        self.pushButton_SelectFile.setObjectName("pushButton_SelectFile")
        #Text file
        self.text_file = QtWidgets.QLabel(self.centralwidget)
        self.text_file.setGeometry(QtCore.QRect(150, 90, 381, 21))
        self.text_file.setObjectName("text_file")
        #Flash memory
        self.pushButton_FlashMemory = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_FlashMemory.setGeometry(QtCore.QRect(20, 130, 111, 31))
        self.pushButton_FlashMemory.setObjectName("pushButton_FlashMemory")
        #Progress bar
        self.progress = QtWidgets.QProgressBar(self.centralwidget)
        self.progress.setGeometry(QtCore.QRect(150, 130, 411, 31))
        self.progress.setMaximum(100) # A modifier
        self.progress.setObjectName("progressBar")
        #Stop transfert
        self.pushButton_Stop = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_Stop.setGeometry(QtCore.QRect(570, 130, 111, 31))
        self.pushButton_Stop.setObjectName("pushButton_Stop ")  
        #Launch program
        self.pushButton = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton.setGeometry(QtCore.QRect(20, 180, 111, 31))
        self.pushButton.setObjectName("pushButton")
        #Textbox Jump Address
        self.textbox_jump = QLineEdit(self)
        self.textbox_jump.setGeometry(QtCore.QRect(140, 180, 100, 31))
        self.textbox_jump.setObjectName("textbox_jump")
        self.textbox_jump.setText("000022C0")
        #Check status
        self.pushButton_CheckStatus = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_CheckStatus.setGeometry(QtCore.QRect(20, 230, 111, 31))
        self.pushButton_CheckStatus.setObjectName("pushButton_CheckStatus")
        #Send trame title
        self.label_trame_title = QtWidgets.QLabel(self.centralwidget)
        self.label_trame_title.setGeometry(QtCore.QRect(20, 270, 211, 31))
        self.label_trame_title.setObjectName("label_trame_title")
        #Title address
        self.label_address = QtWidgets.QLabel(self.centralwidget)
        self.label_address.setGeometry(QtCore.QRect(20, 300, 211, 31))
        self.label_address.setObjectName("label_address")
        #Title memory
        self.label_memory = QtWidgets.QLabel(self.centralwidget)
        self.label_memory.setGeometry(QtCore.QRect(140, 300, 211, 31))
        self.label_memory.setObjectName("label_memory")
        #Textbox address
        self.textbox_address = QLineEdit(self)
        self.textbox_address.setGeometry(QtCore.QRect(20, 340, 100, 31))
        self.textbox_address.setObjectName("textbox_address")
        self.textbox_address.setText("00002000")
        #Textbox memory
        self.textbox_memory = QLineEdit(self)
        self.textbox_memory.setGeometry(QtCore.QRect(140, 340, 100, 31))
        self.textbox_memory.setObjectName("textbox_memory")
        self.textbox_memory.setText("00000000")
        #Send SPI
        self.pushButton_SendTrame = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_SendTrame.setGeometry(QtCore.QRect(20, 380, 111, 31))
        self.pushButton_SendTrame.setObjectName("pushButton_4")
        #Read at this address
        self.pushButton_Read = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_Read.setGeometry(QtCore.QRect(20, 420, 111, 31))
        self.pushButton_Read.setObjectName("pushButton_Read")


        #Texte
        self.pushButton_Stop.setText("Unlock")
        self.pushButton.setText("Launch Program")
        self.pushButton_FlashMemory.setText("Flash memory")
        self.label.setText("FLASHER SPI")
        self.text_file.setText("File")
        self.pushButton_SelectFile.setText("Select File")
        self.label_trame_title.setText("SEND TRAME")
        self.pushButton_SendTrame.setText("Send trame")
        self.pushButton_CheckStatus.setText("Check status")
        self.label_address.setText("Address (Hex)")
        self.label_memory.setText("Memory (Hex)")
        self.pushButton_Read.setText("Read")

        #Association boutons et fonctions
        self.pushButton_SelectFile.clicked.connect(self.on_click)
        self.pushButton_FlashMemory.clicked.connect(self.on_click_send_SPI)
        self.pushButton.clicked.connect(self.on_click_jump)
        self.pushButton_SendTrame.clicked.connect(self.on_click_Run_send_one_trame)
        self.pushButton_CheckStatus.clicked.connect(self.on_click_check_status)
        self.pushButton_Read.clicked.connect(self.on_click_read)
        self.pushButton_Stop.clicked.connect(self.Stop_sending)
        self.show()
    
    def openFileNameDialog(self):
        global filename
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        self.net.fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if self.net.fileName :
            qfilename.put(self.net.fileName )
            self.text_file.setText(self.net.fileName)

            
    @pyqtSlot()
    def on_click(self):
        self.openFileNameDialog()

    def Stop_sending(self):
        self.pushButton.setEnabled(True)
        self.pushButton_FlashMemory.setEnabled(True)
    
    def on_click_send_SPI(self):
        if self.net.fileName != "" :
            #self.net.Flash_write_from_file()
            self.pushButton.setEnabled(False)
            self.pushButton_FlashMemory.setEnabled(False)
            self.thread = Send_network()
            self.thread.value_progress.connect(self.setProgressVal)
            self.thread.value_max_progress.connect(self.setProgressMax)
            self.thread.start()

        else :
            print("Merci de joindre un fichier")

    def on_click_check_status(self):
        self.net.Run_send_and_receive_one_word(410)

    def setStatus(self, val):
        self.label_status.setText(val)

    def setProgressVal(self, val):
        self.progress.setValue(val)

    def setProgressMax(self, val):
        self.progress.setMaximum(val)

    def on_click_jump(self):
        address = self.textbox_jump.text()
        address_msb = int(address[0:4], 16)
        address_lsb = int(address[4:8], 16)
        self.net.Send_one_word (600, 1) #CMD Write
        self.net.Send_one_word(700, 1)
        self.net.Send_one_word(address_msb, 1)
        self.net.Send_one_word(address_lsb, 1)
        print("----- Launch Application -----")

    def on_click_read(self):
        address = self.textbox_address.text()
        address_msb = int(address[0:4], 16)
        address_lsb = int(address[4:8], 16)
        valuemsb = ""
        valuelsb = ""
        self.net.Run_send_and_receive_one_word(400)
        self.net.Run_send_and_receive_one_word(address_lsb)
        self.net.Run_send_and_receive_one_word(address_msb)
        valuemsb = str(hex(self.net.response_spi_received))
        self.net.Run_send_and_receive_one_word(1)
        valuelsb = str(hex(self.net.response_spi_received))
        print("------ READ MEMORY ADDRESS 0x" + str(address) + " ------")
        print("Value MBS : " + str(valuemsb))
        print("Value LBS : " + str(valuelsb))
        #self.net.Run_send_and_receive_one_word(1)

    def Check_status(self):
        self.net.Run_send_and_receive_one_word(700)
        
    def on_click_SEND_SPI(self):
        self.net.Run_send_and_receive_one_word(self.textbox.text())

    def on_click_Run_send_one_trame(self):
        address = self.textbox_address.text()
        memory = self.textbox_memory.text()
        self.net.Flash_write_one_trame(address, memory)



if __name__ == '__main__':    
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
