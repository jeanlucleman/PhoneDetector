#ifndef DICTIONARY_H
  #define DICTIONARY_H
  #include <Arduino.h>
      typedef struct 
        {
          String key;
          int value;
        } keyValueType;
  class ClsKeyValues
    {
      public:
        #define NMAXKEYVALUES 40 
        ClsKeyValues();
        keyValueType keyValues[NMAXKEYVALUES];
        keyValueType  * item(int i);
        keyValueType * item(String key);
        bool contains(String key);
        void sort();

        void deleteMe(String key);
        void deleteMe(size_t i);
        bool checkFreeSpace();
        void swap(int x, int j);
        void list();
        // String arrayNewMac;
        // int rssiThreshold;
        size_t countKeyValues;
      private:
        
    };
  
#endif