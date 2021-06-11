# Bootloader V3.0

Cette version du bootloader permet d'�crire une application dans la memoire flash au travers du SPI avec contr�le des paquets.
Le protocole de communication mis en place s�appuie sur le S19.

Am�liorations par rapport a la version V2.2:
- Changement du protocole de communication et des commandes reseaux
- Modification de l'algorithme pour s'adapter aux nouvelles commandes reseaux
- Protection de la memoire flash (Bootloader + Vecteurs application)
- Amelioration des codes status

Am�liorations par rapport a la version V2.0:
- Sauvegarde des vecteurs dans la memoire flash
- Chargement des vecteurs � chaque lancement de l'application


### I - Protocole de communication

#### 1. Commandes et status

Commandes :<br>
300 : Write <br>
310 : Read status <br>
330 : Read checksum <br>
500 : Effacer la memoire <br>
700 : Lancer le programme <br>

Status : <br>
30 : Vous avez la main sur le bootloader. Pr�t a lancer l'application ou Jumper <br>
20 : Occup� (En train d'�crire ou en train d�effacer) <br>
10 : En processus de flash, pr�t a continuer <br>
5 : Erreur lors de l'�criture : zone memoire protege

#### 2. Protocole d'�criture

<center>
<img src="../Images/diagramme_seq_write_bootloaderV3.0.PNG"  width="70%"/>
</center>