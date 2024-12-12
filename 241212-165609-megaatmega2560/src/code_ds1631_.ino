#include <Arduino.h>
#include <Wire.h>  // Inclut la bibliothèque pour la communication I2C
#define DS1631_ADDR (0x98 >> 1)  // Adresse I2C du capteur DS1631, décalée d'un bit pour correspondre au format de l'adresse I2C

void setup() {
  Serial.begin(9600);  // Initialise la communication série à 9600 bauds
  Serial.println();  // Saut de ligne
  Serial.println("In setup, configurations...");  // Affiche un message indiquant que la configuration a lieu
  Serial.println();
  Serial.println("**********************************");
  Serial.println("** DS1631 Capteur de température**");  // Affiche le titre du capteur
  Serial.println("**********************************");
  Serial.println();

  Wire.begin();  // Initialise la communication I2C

  // Initialisation de la communication avec le capteur DS1631
  Wire.beginTransmission(DS1631_ADDR);  
  Wire.write(0x22);  // Envoie une commande pour initialiser le capteur
  int error = Wire.endTransmission();  // Termine la transmission et capture l'éventuelle erreur

  if (error != 0) {  // Si une erreur est détectée dans la transmission
    Serial.print("Erreur détectée, valeur: ");  // Affiche l'erreur
    Serial.print(error);  // Affiche le code d'erreur
    Serial.println(" le composant ...");
    delay(10000);  // Attend 10 secondes avant de réessayer
  }

  // Envoi de la commande pour accéder au registre de configuration
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0xAC);  // Code commande @Access Config
  Wire.endTransmission();  // Fin de la transmission

  // Lecture de la configuration actuelle du capteur
  Wire.requestFrom(DS1631_ADDR, 1);  // Demande de lire 1 octet du capteur
  int ancienne_config = Wire.read();  // Lit la configuration actuelle
  Serial.print("Configuration actuelle avant modification : 0x");  // Affiche la configuration actuelle
  Serial.print(ancienne_config, HEX);  // Affiche en format hexadécimal
  Serial.println();  // Fin de ligne

  // Modification de la configuration du capteur (ici, activation du mode de conversion continue)
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0xAC);  // Code commande @Access Config
  Wire.write(0x0C);  // Valeur de configuration à écrire (modifie le comportement du capteur)
  Wire.endTransmission();

  // Lecture de la configuration après modification
  Wire.requestFrom(DS1631_ADDR, 1);  // Demande de lire 1 octet du capteur
  int config_actuelle = Wire.read();  // Lit la configuration après modification
  Serial.print("Configuration actuelle après modification : 0x");  // Affiche la nouvelle configuration
  Serial.print(config_actuelle, HEX);  // Affiche en format hexadécimal
  Serial.println();  // Fin de ligne

  // Démarre la conversion de température sur le capteur
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0x51);  // Code commande de StartConvert (démarre la conversion de température)
  Wire.endTransmission();
}

void loop() {
  Serial.println("___________________________________________");  // Séparateur pour afficher un nouveau cycle

  // Demande de lecture de la température depuis le capteur
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0xAA);  // Code commande pour lire la température
  Wire.endTransmission();  // Fin de la transmission

  // Demande de lecture de 2 octets contenant la température
  Wire.requestFrom(DS1631_ADDR, 2);
  Serial.print("Octets en attente de lecture : ");
  Serial.println(Wire.available());  // Affiche le nombre d'octets disponibles à lire

  // Lecture des octets contenant la température
  int T_MSB = Wire.read();  // Lit le poids fort (MSB) de la température
  int T_LSB = Wire.read();  // Lit le poids faible (LSB) de la température

  // Affichage des valeurs binaires brutes lues
  Serial.println("Valeurs binaires \"brutes\" lues :");
  Serial.print("T_MSB = ");
  Serial.println(T_MSB, BIN);  // Affiche le poids fort en binaire
  Serial.print("T_LSB = ");
  Serial.println(T_LSB, BIN);  // Affiche le poids faible en binaire

  // Calcul de la partie entière de la température (valeur en °C)
  int partie_entiere = T_MSB & 0b01111111;  // Extrait la partie entière de la température (7 bits)
  float partie_decimale = 0.0;  // Initialise la partie décimale à 0

  // Calcul de la partie décimale en fonction des bits du LSB
  if ((T_LSB & 0b01100000) == 0b01100000)
    partie_decimale += 0.5;
  if ((T_LSB & 0b01000000) == 0b01000000)
    partie_decimale += 0.25;
  if ((T_LSB & 0b00100000) == 0b00100000)
    partie_decimale += 0.125;
  if ((T_MSB & 0b00010000) == 0b00010000)
    partie_decimale += 0.0625;

  // Affiche la température calculée avec la partie entière et décimale
  Serial.print("Valeur décimale correspondante : T_dec = ");  // Affiche le message
  if (T_MSB >= 0x80)  // Si le bit de signe est 1 (température négative)
    Serial.print('-');  // Affiche un signe négatif

  // Affiche la température finale avec 4 chiffres après la virgule
  Serial.print((T_MSB & 0b01111111) + partie_decimale, 4);  
  Serial.println();  // Nouvelle ligne pour séparer les affichages

  delay(2000);  // Attends 2 secondes avant de refaire une nouvelle lecture
}
