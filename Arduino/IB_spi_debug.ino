#include <SPI.h>

//SPIClass SPI(1);

const int slaveAPin = 10;

//16 bits words
uint16_t Address = 0x0;
uint16_t word1 = 0x0;
uint16_t word2 = 0x0;
int my_delay_SPI = 50;
bool debug = false;

int Nb_mots = 1; // mettre le nombre de mots (1 - 3)
// Choisir un, deux ou trois mots de 16 bits à lire//
// pour l'envoie du message depuis l'arduino vers la carte évaluation maison//
// rq: 3 mots ne fonctionne pas avec la carte arduino uno "stroull". Par défaut les mots manquants prennent la valeur 0x0//

union variable {
  float fltvalue;
  unsigned long longvalue;
  unsigned short shtvalue[2];
  unsigned char chrvalue[4];
};

union variable var_IB;

//get vals
uint16_t stat, val1, val2;
String msg;

// set up the speed, data order and data mode
SPISettings settingsA(1000000, MSBFIRST, SPI_MODE1); // identique sur l'IB

void setup() {
  Serial.begin(115200); 
  // set the Slave Select Pins as outputs:
  pinMode (slaveAPin, OUTPUT);
  // initialize SPI:
  SPI.begin();
}


void loop() {

  serialCheck();
  delay(100);

}


void serialCheck()
{
                                  //Exemples de mots envoyés (premier mot)://
                //température carte IB = 0x8010 la valeur remonté est fixe (99.0°C)//
       //HK_I_A_6_FLT = 0x813C. C'est la valeur du potentiomètre de gauche sur la carte évaluation maison//
      //HK_I_HVPA_1_FLT = 0x8094. C'est la valeur du potentiomètre de droite sur la carte évaluation maison//
      
      if(Serial.available()>0)
    {
      msg = Serial.readString();
      Serial.println("Message reçu :");
      Serial.println(msg);
      Serial.println("Je vais demander a l'IB :");
      Serial.print(msg);
      if (Nb_mots == 1) word1 = 0x0000;
      if (Nb_mots != 3) word2 = 0x0000;
      if (Nb_mots == 1) Serial.print("00000000");
      if (Nb_mots == 2) Serial.print("0000");
      Serial.print('\n');
      
      //Address =  StrToHex(msg);
 
      //if (Nb_mots == 3) Address = (strtoull(msg.c_str(),  0, 0) & 0xFFFF00000000) >> 32;
      //if (Nb_mots == 3) word1 = (strtoull(msg.c_str(),  0, 0) & 0xFFFF0000) >> 16;
      //if (Nb_mots == 3) word2 = (strtoull(msg.c_str(),  0, 0) & 0xFFFF);
      if (Nb_mots == 2) Address = (strtoul(msg.c_str(),  0, 0) & 0xFFFF0000) >> 16;
      if (Nb_mots == 2) word1 = (strtoul(msg.c_str(),  0, 0) & 0xFFFF);
      if (Nb_mots == 1) Address = (strtoul(msg.c_str(),  0, 0) & 0xFFFF);
      //Serial.println(String(Address, HEX));
      if(debug) Serial.println("-------------------");
      if(debug) Serial.println(msg.c_str());
      if(debug) Serial.println(strtoul(msg.c_str(),  0, 16), HEX);
      if(debug) Serial.println(String(Address, HEX));
      if(debug) Serial.println(String(word1, HEX));
      if(debug) Serial.println(String(word2, HEX));
      if(debug) Serial.println("-------------------");
      //Serial.print('\n');

      send_to_IB(Address, word1, word2);   
      
    }
}

void send_to_IB(uint16_t my_address, uint16_t word1, uint16_t word2){

  if(debug) Serial.println("Prepare to send to IB : ");
  if(debug) Serial.println(my_address, HEX);
  if(debug) Serial.println(word1, HEX);
  if(debug) Serial.println(word2, HEX);
  if(debug) Serial.println("Sending...");
  // read three bytes from device A
  //SPI.beginTransaction(settingsA);
  SPI.beginTransaction(settingsA);
  digitalWrite (slaveAPin, LOW);
  // reading only, so data sent does not matter
  stat = SPI.transfer16(my_address);
  delayMicroseconds(my_delay_SPI);
  val1 = SPI.transfer16(word1);
  delayMicroseconds(my_delay_SPI);
  val2 = SPI.transfer16(word2);
  delayMicroseconds(my_delay_SPI);
  digitalWrite (slaveAPin, HIGH);
  SPI.endTransaction();
  delay(20);

  if(debug) Serial.println(stat, HEX);
  if(debug) Serial.println(val1, HEX);
  if(debug) Serial.println(val2, HEX);
  var_IB.shtvalue[1] = val1;
  var_IB.shtvalue[0] = val2;
  //var = val1 << 16 + val2;
  Serial.println("Reponse de l'IB en float");
  Serial.println(var_IB.fltvalue);
  Serial.println(String(stat, HEX)  +"#"+  String(val1, HEX)  +"#"+  String(val2, HEX) + "$");
  Serial.print('\n');
}
