# 26/06 Flasher SPI avec protocole de communication S19 adapté communicant en réseau avec la FEB
# Code et protocole de communication (README.md) : https://github.com/GautierDev31/MCF51AG128_Bootloader_SPI

import sys
import time
import socket
import codecs
import threading 
 
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QLineEdit, QFileDialog, QPushButton, QLabel,QProgressBar
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot, QThread, pyqtSignal, Qt
from PyQt5 import QtCore, QtGui, QtWidgets

class Network_communication (QThread, threading.Thread):

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


    def Run_send_and_receive_one_word(self, input):
        """Lancement des thread de lecture et d'écriture pour ecrire un seul mot.
        """
        th1 = threading.Thread(target=self.Send_one_word, args=(input, 0))
        th2 = threading.Thread(target=self.Trame_reception, args=(th1, "SIMPLE_RESPONSE",))
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
        """

        UDP_IP = "192.168.1.205"
        UDP_PORT = 2000

        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.bind((UDP_IP, UDP_PORT))
        MESSAGE_STR = "0x"
        CODE_STATUS = ""
        a = self.nb_lines*2
        if type == "SIMPLE_RESPONSE" :
            data, addr = sock.recvfrom(1024)
            MESSAGE= codecs.encode(data, "hex")
            MESSAGE_STR += chr(MESSAGE[34])
            MESSAGE_STR += chr(MESSAGE[35])
            MESSAGE_STR += chr(MESSAGE[32])
            MESSAGE_STR += chr(MESSAGE[33])
            self.response_spi_received = int(MESSAGE_STR, 16)
            self.value_received_spi.emit(self.response_spi_received)
            print(self.response_spi_received)


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
        print ("Sendind : " + str(input))
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


class App(QWidget):

    def __init__(self):
        super().__init__()
        self.title = 'MCF51AG128 Flasher SPI'
        self.left = 10
        self.top = 10
        self.initUI()
        self.net = Network_communication()
        self.STOP = "FALSE"
    
    def initUI(self):
        #Declaration des éléments
        self.resize(750, 350)
        self.centralwidget = QtWidgets.QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        #Flasher SPI title   
        self.label = QtWidgets.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(20, 20, 211, 31))
        self.label.setObjectName("label")
        self.label.setText("FLASHER SPI")
        #Select file
        self.pushButton_SelectFile = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_SelectFile.setGeometry(QtCore.QRect(20, 80, 111, 31))
        self.pushButton_SelectFile.setObjectName("pushButton_SelectFile")
        self.pushButton_SelectFile.setText("Select File")
        self.pushButton_SelectFile.clicked.connect(self.on_click)
        #Text file
        self.text_file = QtWidgets.QLabel(self.centralwidget)
        self.text_file.setGeometry(QtCore.QRect(150, 90, 381, 21))
        self.text_file.setObjectName("text_file")
        self.text_file.setText("File")
        #Flash memory
        self.pushButton_FlashMemory = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_FlashMemory.setGeometry(QtCore.QRect(20, 130, 111, 31))
        self.pushButton_FlashMemory.setObjectName("pushButton_FlashMemory")
        self.pushButton_FlashMemory.setText("Flash memory")
        self.pushButton_FlashMemory.clicked.connect(self.on_click_send_SPI)
        #Progress bar
        self.progress = QtWidgets.QProgressBar(self.centralwidget)
        self.progress.setGeometry(QtCore.QRect(150, 130, 411, 31))
        self.progress.setMaximum(100) # A modifier
        self.progress.setObjectName("progressBar")
        #Action
        self.text_Action = QtWidgets.QLabel(self.centralwidget)
        self.text_Action.setGeometry(QtCore.QRect(570, 135 , 381, 21))
        self.text_Action.setObjectName("text_Action")
        self.text_Action.setText("")
        #Launch program
        self.pushButton = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton.setGeometry(QtCore.QRect(20, 180, 111, 31))
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("Launch Program")
        #Textbox Jump Address
        self.textbox_jump = QLineEdit(self)
        self.textbox_jump.setGeometry(QtCore.QRect(140, 180, 100, 31))
        self.textbox_jump.setObjectName("textbox_jump")
        self.textbox_jump.setText("000022C0")
        self.pushButton.clicked.connect(self.on_click_jump)
        #Check status
        self.pushButton_CheckStatus = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_CheckStatus.setGeometry(QtCore.QRect(20, 230, 111, 31))
        self.pushButton_CheckStatus.setObjectName("pushButton_CheckStatus")
        self.pushButton_CheckStatus.setText("Check status")
        self.pushButton_CheckStatus.clicked.connect(self.on_click_check_status)
        #Reset microcontroler
        self.pushButton_Reset_micro = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_Reset_micro.setGeometry(QtCore.QRect(20, 280, 211, 31))
        self.pushButton_Reset_micro.setObjectName("pushButton_Reset_micro")
        self.pushButton_Reset_micro.setText("Reset IB (Only IB V5)")
        self.pushButton_Reset_micro.clicked.connect(self.on_click_Reset_micro)

        self.show()
    
    def openFileNameDialog(self):
        """Fenetre de recherche de fichier

        Selection du fichier S19 utilisé pour reflasher le microcontroleur
        """
        global filename
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        self.net.fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if self.net.fileName :
            self.text_file.setText(self.net.fileName)

    def Flash_write_from_file(self) :
        """Envoie du fichier S19 pour reflasher le microcontroleur

            Envoie les données du S19 en respectant le protocole de communication
            La fonction fonctionne en parrallèle de la fonction Trame_reception pour lire les reponses du microcontroleur.

            La checksum calculé pour chaque ligne d'envoie est a somme des bytes a 1 :
            - Du nombre de mots de 32 bits envoyés
            - De l'adresse envoyé
            - Des mots de la mémoire envoyé
            Le calcul de la checksum coté client est le même que coté microcontroleur.
            Après l'envoie d'une ligne, la checksum calculé par le microcontroleur est demandé et comparé avec celle calculé coté client 
            pour savoir si le microcontroleur a correctement reçu toutes les données.
        """


        # Check de la validité du fichier S19 selectionné
        f = open(self.net.fileName, "r")
        if (f.read(2) != "S0") :
            f.read(9)
            if(f.read(2) != "S3") :
                print("Le fichier selectionné n'est pas un fichier S19 ou n'est pas compatible avec le microcontroleur")
                return
        f.close()

        self.net.File_analyse() #Calcul du nombre de mot a envoyer

        f = open(self.net.fileName, "r")
        f.read(11) #Fin de l'entete
        flag = 0
        checksum = 0
        self.net.i_data = 0

        # Check de la connexion avec le microcontroleur
        print("----- Check Status -----") 
        self.net.Run_send_and_receive_one_word(410)
        self.net.Run_send_and_receive_one_word(410)
        # self.net.Wait_until_trame_reception(1)
        if (self.net.response_spi_received != 30) :
            if (self.net.response_spi_received != 10):
                print("Impossible to communicate with the uC")
                return
        print("Status Ready")

        # Effacement de la mémoire flash de l'addresse 0x2000 à 0x1F800
        print("----- Erase Flash -----") 
        self.text_Action.setText("(1/2) Erasing Flash...")
        print("Erasing...")
        self.progress.setMaximum(118)
        self.net.i_data = 0
        address = 0x2000
        while (address!= 0x1F800):
            address_msb = str(int((address & 0xFFFF0000)/65536))
            address_lsb = str(int((address & 0x0000FFFF)))
            self.net.Send_one_word (305, 1) #CMD Write
            self.net.Send_one_word (address_msb, 1) #address msb
            self.net.Send_one_word (address_lsb, 1) #address msb
            address += 0x400
            time.sleep(0.1)
            self.net.i_data += 1
            print(self.net.i_data)
            self.progress.setValue(self.net.i_data)

        # Check de l'état du microcontroleur après l'effacement de la mémoire
        print("----- Check Status -----") 
        self.net.Run_send_and_receive_one_word(410)
        self.net.Run_send_and_receive_one_word(410)
        self.net.Wait_until_trame_reception(1)
        self.net.Wait_until_trame_reception(1)
        if (self.net.response_spi_received != 10) :
            print("Impossible to communicate with the uC")
            return
        print("Status Ready")
        print("Flash erased")

        # Ecriture de la mémoire flash a partir du fichier S19
        self.text_Action.setText("(2/2) Programming Flash...")        
        self.net.i_data = 0
        self.progress.setValue(self.net.i_data)
        self.progress.setMaximum(self.net.nb_memory)
        while flag != 1 :
            #L'envoie ne s'arrête que lorsque la dernière ligne commence par "S7"
            if (f.read(2) == "S7") :
                flag = 1
            else :
                print("----- Write Flash -----") 
                nb_value = f.read(2)
                length = int((int(nb_value, 16)*2 - 2)/8)-1

                self.net.Send_one_word (300, 1) #Envoie de la commande d'écriture
                self.net.Send_one_word(length, 1) #Envoie de la taille
                checksum += self.net.chcksum(length) #Ajout du nombre de mots a envoyer dans le calcul de la checksum

                address_msb = int(f.read(4), 16) 
                checksum += self.net.chcksum(address_msb) 
                self.net.Send_one_word(address_msb, 1) #Envoie de l'adresse msb
                address_lsb = int(f.read(4), 16)
                checksum += self.net.chcksum(address_lsb) 
                self.net.Send_one_word(address_lsb, 1) #Envoie de l'adresse lsb

                #Envoie de la memoire
                for i in range (length*2) :
                    value_to_send = int(f.read(4), 16)
                    self.net.Send_one_word(value_to_send, 1)
                    checksum += self.net.chcksum(value_to_send) 
                    self.net.i_data += 0.5
                    self.progress.setValue(self.net.i_data)
                self.net.Send_one_word(310, 1)  #Ecriture en flash

                self.net.Run_send_and_receive_one_word(420)
                self.net.Run_send_and_receive_one_word(420)
                # self.net.Wait_until_trame_reception(1)
                if (self.net.response_spi_received == checksum ):
                    print("Checksum OK!")
                else :
                    print("Checksum calculated :" + str(checksum))
                    print("Checksum received :" + str(self.net.response_spi_received ))
                    print("Checksum KO")
                    print("Abandon de l'envoie")
                    return
                checksum = 0
                

                print("----- Check Status -----") 
                self.net.Run_send_and_receive_one_word(410)
                self.net.Run_send_and_receive_one_word(410)
                # self.net.Wait_until_trame_reception(1)
                if (self.net.response_spi_received != 10) :
                    print("Impossible to communicate with the uC")
                    return

                f.read(3) #Checksum (not usable)
        print("------ END ------")        
        f.close()
            
    @pyqtSlot()
    def on_click(self):
        """ Cette action de click permet d'ouvrir la fenetre de selection de fichier S19
        """
        self.openFileNameDialog()  

    def on_click_Reset_micro(self):
        """Cette action de click permet de faire un reset du microcontroleur de la carte IB
        """
        print("----- Reset IB microcontroler -----")
        UDP_IP = "192.168.1.21"
        UDP_PORT = 1234
        MESSAGEHEX = "aaaaaaaa307E000004000000aaaaaaaa"
        MESSAGE = codecs.decode(MESSAGEHEX, "hex")
        sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    
    def on_click_send_SPI(self):
        """ Cette action de click permet de lancer le reflash du microcontroleur si un fichier S19 a bien été selectionné
        """

        if self.net.fileName != "" :
            self.pushButton.setEnabled(False)
            self.pushButton_FlashMemory.setEnabled(False)
            self.pushButton_Reset_micro.setEnabled(False)
            self.pushButton_SelectFile.setEnabled(False)
            self.pushButton_CheckStatus.setEnabled(False)
            self.Flash_write_from_file()
            self.pushButton.setEnabled(True)
            self.pushButton_FlashMemory.setEnabled(True)
            self.pushButton_Reset_micro.setEnabled(True)
            self.pushButton_SelectFile.setEnabled(True)
            self.pushButton_CheckStatus.setEnabled(True)
        else :
            print("Merci de joindre un fichier")

    def on_click_check_status(self):
        """ Cette action de click permet d'envoyer une commande "check status" et d'afficher sur le terminal sa valeur
        """
        self.net.Send_one_word (410, 1) 
        self.net.Run_send_and_receive_one_word(410)


    def on_click_jump(self):
        """ Cette action de click permet d'envoyer la commande "Jump to application" avec la valeur de l'adresse dans la textbox
        """
        address = self.textbox_jump.text()
        address_msb = int(address[0:4], 16)
        address_lsb = int(address[4:8], 16)
        self.net.Send_one_word (600, 1) #CMD Write
        self.net.Send_one_word(700, 1)
        self.net.Send_one_word(address_msb, 1)
        self.net.Send_one_word(address_lsb, 1)

if __name__ == '__main__':    
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
