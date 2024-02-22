/* 
Friend Detector by Ricardo Oliveira, forked by Skickar 9/->/2018
See https://github.com/skickar/FriendDetector/tree/master


The function of this code is to read nearby 2.4 GHz Wi-Fi traffic in the form of packets. These packets are compared
to a list of MAC addresses we wish to track, and if the MAC address of a packet matches one on the list, the presence 
in house can be validated*/

#include "esppl_functions.h"
// #include <ESP8266WiFi.h>
#include "global.h"
#include "clock.h"
// #include <NTPClient.h>  // voir https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
#include <SPI.h>
#include <SD.h>
/* pin connexion:
  SD card  ESP8266(wifi)    ESP8266 (hub)
  GND         GND           GND
  VCC         5V
  MISO        D6
  MOSI        D7
  SCK         D5
  CS          D8
              D1            D2
              D2            D1
*/

// #include <espnow.h> // https://randomnerdtutorials.com/esp-now-two-way-communication-esp8266-nodemcu/
//Authentication Variables that will be used by the UDP clients (doorSensores)
// String AP_SSID, AP_PSW, STA_SSID, STA_PSW; 
// String     AP_SSID;                  // WIFI Name of this Access Point giving access to the ESP8266 though AP mode (ex. 192.168.4.1)
// String     AP_PSW;                   // Password to access this AP
//Authentication Variables that will be used by the http clients (users)
// String STA_SSID;                      // WIFI Name of the router allowing access to the ESP8266 through the home router (ex. 192.168.2.98)
// String STA_PSW;                       // PSW of home router
//WiFi settings for the AP mode
// IPAddress APlocal_IP(192, 168, 4, 10);   //Access Point (AP) Local IP Address
// IPAddress APgateway(192, 168, 4, 1);     //Access Point (AP) Gateway Address
// IPAddress APsubnet(255, 255, 255, 0);    //Access Point (AP) Subnet Address
  // void setupWifi();                       // Start wifi in AP and STA modes
  // void startServer();                     // Start the server on this ESP8266
  // void checkWifi();                       // Check wifi status and setup wifi when not connected 
  void printDated(String text);           // Format serial output with date
  void printlnDated(String text);         // Format serial output with date and CR
  bool setupClock();                      // Initialize a NTP client to connect on a NTP server to get time and date
  bool setClockFromOTP();                 // Get time and date from a NTP client and update the clock (see Clock class)
  void maintainClock();                   // Periodically check the clock and update
  // bool startStaMode();
/*--------Time and date management---- */
unsigned long currentMillis;
unsigned long lastTimeRequestMillis;
Clock* myClock=Clock::getInstance(); // Creates a clock. It gives the current date/time by date/time at t0 + millis elapsed since t0.
  // t0 is the date/time given by a call to OTP at t0

// WiFiUDP UdpClock; // It must be another instance of WiFiUDP otherwise after clock setup, the UDP transmission with swithes is lost. 
// Wifi settings for the STA mode: read from the setting file. Settings can be acccessed by a connection on AP mode (ex. 192.168.4.1)
bool staModeOK=false;  // Report success of STA connexion
// REPLACE WITH THE MAC Address of your receiver 
// uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
/* SD card: https://www.youtube.com/watch?v=-PyAEYaJp2U et https://github.com/G6EJD/ESP8266-SD-Card-Reading-Writing/blob/master/ESP8266_D1_MicroSD_Test.ino

*/

File root;
#define LIST_SIZE 2
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {
  {0x2E, 0x7C, 0xA6, 0x26, 0xBC, 0xDC} 
  ,{0x7A, 0xB3, 0x2E, 0xEE, 0x05, 0x35} 
};
/*
 * This is your friend's name list
 * put them in the same order as the MAC addresses
 */
String friendname[LIST_SIZE] = {
   "Jean-Luc"
  ,"Geneviève"
  };



/*
There are 3 loops to record wifi signals
- the short loop reports listened device, once by device and by loop. Typically this loop period is 2 min.
  It records the strength of the signal received (rssi) and compute the average stregth for this device. 
  In this loop wifi devices recorded must comply with these conditions:
  - Not to be a stationary device (ex. router, chromecast...) according to a file of stationary device
  - To have a minimum strength signal
  When a wifi device signal is received during this short loop, a boolean is set to true to indicate that this device has been seen.
- The medium loop start at the end of a short loop. Typically the period is 15min. The devices with a low occurrence are rejected. 
  It makes a sort of the devices by decreasing strength, 
  and records and history which devices were present.
  When this is done, the boolean is reset to false to be ready for the next period.
- The long loop is typically once a day. It starts when software start then once a day at midnight.
  Data are saved in a file containing date in the filename. Then data are resetted (could be by restarting ESP).
  At the start of this loop, the date is updated
*/
unsigned long millisLastSniffing;
unsigned long sniffingPeriod=60000; // I listen during one minute
// unsigned long millisLastSaving;
// #define SAVINGSNIFFINGRATIO 5
int maxCountShortLoop=5;
int countShortLoop=0;
// unsigned long savingPeriod=sniffingPeriod*SAVINGSNIFFINGRATIO; // and after x periods of listening I save the result
size_t indexMediumLoop=0;
//https://maclookup.app/
  //  {0x9A, 0x8F, 0xDC, 0x84, 0x01, 0x42}
  //  {0x2E, 0x7C, 0xA6, 0x26, 0xBC, 0xDC} JLL
  // ,{0x7A, 0xB3, 0x2E, 0xEE, 0x05, 0x35} GVV

unsigned long timeLastPresence;
bool presence=false;



// String ESPName="PhoneDetector";
const uint8_t rxPin = 4; // D2
const uint8_t txPin = 5; // D1
// int baudRate=38400;
// SerialCom* myCom=SerialCom::getInstance(ESPName,baudRate,rxPin,txPin); 

bool cbEnabled=true;
void cb(esppl_frame_info *info);
bool maccmp(uint8_t *mac1, uint8_t *mac2);
void printDirectory(File dir, int numTabs) {
// int colcnt =0;
while(true) {
  File entry =  dir.openNextFile();
  if (! entry) {
    // no more files
    break;
   }
   if (numTabs > 0) {
     for (uint8_t i=0; i<=numTabs; i++) {
       Serial.print('\t');
     }
   }
   Serial.print(entry.name());
   if (entry.isDirectory()) {
     Serial.println("/");
     printDirectory(entry, numTabs+1);
   } else
   {
     // files have sizes, directories do not
     Serial.print("\t");
     Serial.println(entry.size(), DEC);
   }
   entry.close();
  }
}
void setup() 
  { 
    Serial.begin(74880);                       //Set Serial Port to 115200 baud
    if (!initialisations())
      {
        debugln(F("Initialization failed!"));
        return;
      }
    Serial.println();
    
    setupClock(); 
    #ifdef OTA
      ArduinoOTA.setHostname("ESP8266");
      ArduinoOTA.onStart([]() {
        debugln("Start");
      });
      ArduinoOTA.onEnd([]() {
        debugln("\nEnd");
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        debugf2("Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) {
        debugf2("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) debugln("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) debugln("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) debugln("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) debugln("Receive Failed");
        else if (error == OTA_END_ERROR) debugln("End Failed");
      });
      ArduinoOTA.begin();
    #endif
    debuglnDated("Setup completed!");
    esppl_init(cb);
    esppl_sniffing_start();
    Serial.println("Setup completed!");
  }

void loop() 
  { 
    currentMillis=millis();
    // Mettre ici  serialComHandle();
    // myCom->readString();
    // if (myCom->dataAvailable())
    //   {
        
    //   }
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) 
      {
        esppl_set_channel(i);
        while (esppl_process_frames()) {}
      }
    if(millis()>timeLastPresence+600000 and presence)
      {
        presence=false;
        // Serial.println("\nAppartement vide, enclenchement de l'alarme");
      }
    if(currentMillis>millisLastSniffing + sniffingPeriod) // short loop
      {
        cbEnabled=false;

        Serial.printf("short loop n° %d\n", countShortLoop);
        wifiDevices.list();
        Serial.println();
        wifiDevices.arrayNewMac="";
        countShortLoop++;
        if (countShortLoop==maxCountShortLoop) // Medium loop
          {
            countShortLoop=0;
            // Serial.println("Avant deleting low occurrence");
            // wifiDevices.list();
            size_t index=0;
            // à chaque boucle, supprimer les lignes avec une très faible occurence de la boucle courante
            do
              {
                if(wifiDevices.item(index)->countDetection==1)
                  {wifiDevices.deleteMe(index);}// This decreases the countDevices
                else
                  {
                    index++;
                  }
              } while (index<wifiDevices.countDevices);
            // Serial.println("Après effacement des faibles occurences:");
            // wifiDevices.list();
            // Serial.println("========================");
            wifiDevices.sort();
            for (size_t i = 0; i < wifiDevices.countDevices; i++)
              {
                char rs[2];
                strcpy(rs,"0");
                if(strlen(wifiDevices.item(i)->history)==0 )
                  {
                    for (size_t j = 0; j < indexMediumLoop; j++)
                    {
                      strcat(wifiDevices.item(i)->history, rs);
                    }
                  }
                if(wifiDevices.item(i)->ddlsl)
                  {
                    strcpy(rs,"1");
                  }
                strcat(wifiDevices.item(i)->history, rs);
                wifiDevices.item(i)->ddlsl=false;
              }
            wifiDevices.save();
            indexMediumLoop++;
            wifiDevices.arrayNewMac="";
            Serial.println("***********************");
          }
        Serial.println("Re start of sniffing...");
        cbEnabled=true;
        millisLastSniffing=millis();
      }
   
  }


bool maccmp(uint8_t *mac1, uint8_t *mac2) 
  {
    for (int i=0; i < ESPPL_MAC_LEN; i++) 
      {
        if (mac1[i] != mac2[i]) 
          {
            return false;
          }
      }
    return true;
  }
void getStringMac(uint8_t addr[],String &shortMac, String &longMac)
  {
    char buffer[3];
    shortMac="";longMac="";
    for (size_t i = 0; i < 6; i++)
      {
        sprintf (buffer, "%02X", addr[i]);
        if(i<3){shortMac+=buffer;};
        longMac+=buffer;
      }
  }
int n=10;
// deviceDataType * getDeviceData(String longMac)
//   {

//   }
void cb(esppl_frame_info *info) 
  {
    if(cbEnabled)
      {
        String shortMac;// 3 first byte of MAC to check if the device belongs to the stationary devices such routers.
        String longMac; // Mac address of the device written as string like 2E169B94F6F2
        getStringMac(info->sourceaddr, shortMac, longMac); // To define the short and long MAC
        if(!colStationaryDevices.contains(shortMac)) // Stationary devices such as router are not considered
          {
            // Serial.print("1");
            
            if(info->rssi>wifiDevices.rssiThreshold) // We look at device with rssi greater than the threshold
              {

                if(wifiDevices.arrayNewMac.indexOf(longMac)==-1) // Device are checked and recorded only once in the sniffingPeriod
                  {
                    if(wifiDevices.checkFreeSpace() || wifiDevices.contains(longMac)) 
                      {
                        String temp=wifiDevices.arrayNewMac+longMac;
                        wifiDevices.arrayNewMac=temp;//wifiDevices.arrayNewMac+longMac; // This array now contains the current MAC so that it will be no more considered in the short loop
                        wifiDeviceType  * thisDevice=wifiDevices.item(longMac); // Creates the device if not existing and returns a pointer
                        thisDevice->ddlsl=true; // detection during last short loop becomes true. Resetted to false in the medium loop
                        thisDevice->countDetection++; // increase the number of detection (resetted in the long loop )
                        // computing average RSSI resetted in the long loop
                        float avrgRssi;
                        int count =thisDevice->countDetection;
                        if (count==1)
                          {avrgRssi=float(info->rssi);}// 
                        else
                          {avrgRssi=thisDevice->avrgRssi;}
            // Serial.print("6");                        
                        thisDevice->avrgRssi=((n-1)*avrgRssi+float(info->rssi))/n;
                        // Serial.print(longMac + " " );
                        Serial.printf(" -> %d ",info->rssi);
                        // Serial.print(" ")   ;
                        // Serial.print(info->ssid);
                        // Serial.print(" ");
                        Serial.printf(" %d %5.2f %02d\n",thisDevice->countDetection,thisDevice->avrgRssi,wifiDevices.countDevices );

                      }


                  }
              }
          }
      

        // if(info->rssi>wifiDevices.rssiThreshold) // We look at device with rssi greater than the threshold
        //   {
        //     getStringMac(info->sourceaddr, shortMac, longMac); // To define the short and long MAC
        //     // getStringMac(info->receiveraddr, shortMacR, longMacR);
        //     // if(arrayNewMac.indexOf(longMac)==-1) // Device are checked and recorded only once in the sniffingPeriod
        //     if(wifiDevices.arrayNewMac.indexOf(longMac)==-1) // Device are checked and recorded only once in the sniffingPeriod
        //       {
        //         if(!colStationaryDevices.contains(shortMac)) // Stationary devices such as router are not considered
        //           {
        //             // arrayNewMac+=longMac; // This array now contains the current MAC so that it will be no more considered in the sniffingPeriod
        //             if(wifiDevices.countDevices<NMAXWIFIDEVICES)
        //               {

        //               }
        //     // Si l'array de new mac est inférieur à la limite, sinon on ne traite pas....        
        //             wifiDevices.arrayNewMac+=longMac; // This array now contains the current MAC so that it will be no more considered in the sniffingPeriod
        //             Serial.print(longMac + " ");
        //             // retrieve or create if new a deviceData
        //             // deviceDataType * thisDeviceData =  getDeviceData(longMac);
        //     //  wifiDevices.item(longMac)->count++;
        //             int count=colSniffedDevices.getDescription(longMac).toInt()+1; 
        //             float avrgRssi;
        //             if (count==1)
        //               {avrgRssi=float(info->rssi);}// 
        //             else
        //               {avrgRssi=colSniffedDevices.getValue(longMac,"0").toFloat();}
        //             avrgRssi=((n-1)*avrgRssi+float(info->rssi))/n;
        //             // long sumRSSI=colSniffedDevices.getValue(longMac,"0").toInt(); 

        //             // Serial.println(prevRssi);
        //             // colSniffedDevices.setValue(longMac, String(sumRSSI+info->rssi+100),String(count),false);
        //             colSniffedDevices.setValue(longMac, String(avrgRssi),String(count),false);

        //             // Serial.printf(" %d %li ",count,avrgRssi );
        //             // Serial.print(" - R: " + longMacR + " ");
        //             Serial.print(info->rssi);   
        //             Serial.print(" ");
        //             Serial.printf(" %d %5.2f %d\n",count,avrgRssi,colSniffedDevices.size() );
        //           }  
        //       }
        for (int i=0; i<LIST_SIZE; i++) 
          {
          if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) 
            {
              // Serial.printf("%s (%d)\n",friendname[i].c_str(),info->rssi);
              if(!presence)
                {
                  presence=true;
                  timeLastPresence=millis();  
                  // Serial.printf("\nPresence (%s). Désactivation de l'alarme\n", friendname[i].c_str());
                }
            }
          } 
      }
  }


/**
 * @brief Prints on the serial console a text with a time prefix. No carriage return at end of line
 * @param text texte to be displayed
 */
void printDated(String text)
  {
    debug(myClock->getNow_YYYY_MM_DD_HH_MM_SS() + " " +  text);
  }
/**
 * @brief Prints on the serial console a text with a time prefix.Carriage return at end of line
 * 
 * @param text texte to be displayed
 */
void printlnDated(String text)
  {
    // debugln(text);
    debugln(myClock->getNow_YYYY_MM_DD_HH_MM_SS() + " " +  text);
  }
bool setupClock()
  {
    debugln("Trying to set myClock");
    // Demander l'heure par la liaison série 
    // timeClient.update();
    // time_t epochTime = timeClient.getEpochTime();
    time_t epochTime=99999999;
    myClock->setClockFromUC(epochTime);
    // debuglnDated(F("Clock has been updated from OTP server"));
    return true;
  }
void maintainClock()
  {
    unsigned long period =3600000;
    if (currentMillis > lastTimeRequestMillis + period)
    {
      setClockFromOTP();
      lastTimeRequestMillis=millis();
    }
  }