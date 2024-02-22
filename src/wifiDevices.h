#ifndef WIFIDEVICES_H
  #define WIFIDEVICES_H
  #include <arduino.h>
  #include <Arduino.h>
  #include "FS.h"
  #include "LittleFS.h"	
  #include <SD.h>
      typedef struct 
        {
          char mac[13];
          float avrgRssi;
          int countDetection;
          bool ddlsl; //detection during last short loop
          char history[145]; // 6/hours * 24 hours +1

        } wifiDeviceType;
  class ClsColWifiDevices
    {
      public:
        #define NMAXWIFIDEVICES 30 
        ClsColWifiDevices(String fileName);
        wifiDeviceType wifiDevices[NMAXWIFIDEVICES];
        wifiDeviceType  * item(int i);
        wifiDeviceType * item(String mac);
        // int deviceNumber;

        String fileName;
        File getFile(const char* mode);
        bool contains(String mac);
        void save();
        void sort();
        void read();
        void deleteMe(String mac);
        void deleteMe(size_t i);
        bool checkFreeSpace();
        void swap(int x, int j);
        void list();
        String arrayNewMac;
        int rssiThreshold;
        size_t countDevices;
      private:
        
    };
  
#endif