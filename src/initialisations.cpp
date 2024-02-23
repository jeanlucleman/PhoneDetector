//MyFunctions.cpp
// -------------------INCLUDES ------------------------
  
  #include <Arduino.h>
  #include "global.h"
  
  // #include <ESPAsyncWebServer.h>
  // #include <time.h>
  // #include "functions.h"

  #ifdef ESP_8266
  #include "FS.h"
  #elif defined ESP_32
  #include <SPIFFS.h>
  #endif

  String ESPName="Central Unit"; // the receiver will know from which device it receives a string     
  const uint8_t rxPin = 4; // D2 // -> the rx pin of the sender                             
  const uint8_t txPin = 5; // D1 // -> the tx pin of the sender                              
  int baudRate=38400; // the baud rate of the sender (the same to be used on the receiver)   
  SerialCom* myCom=SerialCom::getInstance(ESPName,baudRate,rxPin,txPin); // the objet to be used to communicate  

  Settings commonData("commonSettings");
  Settings colCertifiedPhones("certifiedPhones");
  Settings colStationaryDevices("StationaryDevices"); // MAC of permanent system such as router
  // Settings colSniffedDevices("sniffedMac");
  ClsColWifiDevices wifiDevices("liste");
bool initialisations()
  {
    try
      {
        // Serial.begin(74880);
        debugln("\n");
        debugln("Début des initialisations...");
        delay(500);
      }
    catch(...)
      {
        return false;
      } 
    #ifdef ESP_8266
    debugln("ESP board = ESP8266");
    #elif defined ESP_32
    debugln("ESP board = ESP32");
    #endif
    const int chipSelect = D8; 
    Serial.print("\r\nWaiting for SD card to initialise...");
    if (!SD.begin(chipSelect)) { // CS is D8 in this example
      Serial.println("Initialising failed!");
      return false;
    }
    commonData.ini();  // L'initialisation de cet objet n'est pas fait lors de l'instantiation. La raison est que celle-ci se déroule 
    // dès la déclaration du type de l'objet (Settins myData;) et qu'à ce stade toutes les initialisations de l'ESP ne sont pas terminées
    // ce qui crée un plantage.
    debugln("Listing des settings:");
    commonData.list();
    colCertifiedPhones.ini();
    colStationaryDevices.ini();
    colCertifiedPhones.list();
    colStationaryDevices.list();
    // Serial.println("I1");
    // colSniffedDevices.ini();
    // Serial.println("I2");

    // colSniffedDevices.list();

    return true;
  }
    

  