/* Voir settings.h*/
  #include <Arduino.h>
  #include "settings.h"
  #include <vector>
  #include "functions.h"
  #include "global.h"
  using namespace std;

  #ifdef ESP_8266
  #include "FS.h"
  #include "LittleFS.h"					   
  #include <esp8266WiFi.h>
  #elif defined ESP_32
  #include <WiFi.h>
  #include <SPIFFS.h>
  #endif




Settings::Settings(String fileName) 
  { 
    _ini=false; // this private boolean will be set to true when ini() function is done
    _fileName=fileName;
  }
File Settings::getFile(const char* mode)
  {
    File myFile;
    #ifdef ESP_8266
        // Serial.println("getFile1");

    // if(mode=="r")
    if(strcmp(mode,"r")==0)
      // myFile = LittleFS.open("/" + _fileName + ".txt", mode);
      {
                // Serial.println("getFile2");

        myFile = SD.open(_fileName + ".txt", FILE_READ);
                // Serial.println("getFile3");

      }
    else
      {
                // Serial.println("getFile4");

        myFile = SD.open(_fileName + ".txt", FILE_WRITE);
        // Serial.println("getFile5");

      }
    #elif defined ESP_32
      myFile = SPIFFS.open("/" + _fileName + ".txt",mode);
    #endif
    return myFile;
  }


void Settings::ini(){

  #ifdef ESP_8266
    // if(!LittleFS.begin()) // Ouverture de la class LittleFS
    if(!SD.begin(D8))
  #elif defined ESP_32
    if(!SPIFFS.begin(true)) // Ouverture de la class SPIFFS
  #endif
    
      {
        debuglnDated(F("An Error has occurred while mounting SD"));
        return;
      }
    // Serial.println("S1");
    File file = getFile("w");
    // Serial.println("S2");

    // #ifdef ESP_8266
    //   file = SPIFFS.open("/ESP8266.txt","r");
    // #elif defined ESP_32
    //   file = SPIFFS.open("/ESP32.txt");
    // #endif

    // File file= _file();
    if(!file)
      {// Check that the settings file exists
        debuglnDated(F("There was an error opening the file"));
      return;
      }
    file.close();
    _listKeyValues=_readItems();// download in memory the settings in a structure (key, value, description)
    _ini=true;
  }
vector <keyValue> Settings::_readItems()
  {
    vector <keyValue> thisListItems;
    File file = getFile("r");
    // #ifdef ESP_8266
    // File file = SPIFFS.open("/ESP8266.txt","r");
    // #elif defined ESP_32
    // File file = SPIFFS.open("/ESP32.txt");
    // #endif
    // Serial.println("R1");
    
    if(!file){
      debuglnDated(F("There was an error opening the file"));
      return thisListItems;
    }
    String lineRead;
    // Serial.println("R2");
    int maxRecords=60;
    int count=0;
    while (file.available()) {
      // Serial.print(".");
      lineRead = file.readStringUntil('\n'); // one line should be in the form key,value,description
      lineRead.trim(); // to remove cr or other control character beside the string
      Serial.println(lineRead);
      keyValue thisItem; // one item (key, value, description)
      vector<String> result = split(lineRead,",");
      thisItem.key=result[0];
      thisItem.value=result[1];
      if(result.size()> 2) {
        thisItem.description=result[2];
      }else{
        thisItem.description="";
      }
      thisListItems.push_back(thisItem);
      count++;
      if(count>maxRecords){break;}
    }
    // Serial.println("R3");
    file.close();
    return thisListItems;
  }
int Settings::size()
  {
    return _listKeyValues.size();
  }
int Settings::count()
  {
    if(!_ini){ini();}
    File file;
    file = getFile("r");
    // #ifdef ESP_8266
    // File file = SPIFFS.open("/ESP8266.txt","r");
    // #elif defined ESP_32
    // File file = SPIFFS.open("/ESP32.txt");
    // #endif
    
    if(!file)
      {
        debuglnDated(F("There was an error opening the file for writing"));
        return 0;
      }
    int nb=0;
    char buffer[64];
    String lineRead;
    while (file.available())
      {
        int l = file.readBytesUntil('\n', buffer, sizeof(buffer));
        buffer[l] = 0;
        lineRead=String(buffer);
        nb++;
      }
    file.close();
    return nb;
  }
void Settings::sort()
  {
    Serial.println("Avant tri:");
    for (size_t i = 0; i < _listKeyValues.size(); i++)
      {
        Serial.println(_listKeyValues[i].key + " -> " + _listKeyValues[i].value);
      }
    
    for (size_t k = _listKeyValues.size()-1;k>1; k--)
      {
        bool change=false;        
        for (size_t i = 0; i < k; i++)
        {
          keyValue temp=_listKeyValues[i];
          // Serial.println("Comparaison de " + _listKeyValues[i].key + " -> " + _listKeyValues[i].value + " et " + _listKeyValues[i+1].key + " -> " + _listKeyValues[i+1].value);

          if(temp.value.toInt()<_listKeyValues[i+1].value.toInt())
            {
              _listKeyValues[i].value=_listKeyValues[i+1].value;
              _listKeyValues[i+1].value=temp.value;
              _listKeyValues[i].description=_listKeyValues[i+1].description;
              _listKeyValues[i+1].description=temp.description;
              change=true;
              // Serial.println("Change");
            }
        }
        if(!change){break;}
      }
    Serial.println("Après tri: ");
    for (size_t i = 0; i < _listKeyValues.size(); i++)
      {
        Serial.println(_listKeyValues[i].key + " -> " + _listKeyValues[i].value);
      }

  }
String Settings::getValue(String key, String defaultValue)
  {
    if(!_ini){ini();}
    String value=defaultValue;
    bool found=false;
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        keyValue item = _listKeyValues[i];
        //Serial.println(item.key + " -> " + item.value);
        if (item.key==key)
          {
            value=item.value;
            found=true;
            break;
          }
      }
    if(!found)
      {
        // Serial.println("Not found in getvalue!");
        setValue(key,value,false);
      }
    return value;
  }
String Settings::getKey(unsigned int i)
  {
    if(!_ini){ini();}   
    return  _listKeyValues[i].key;
  }
String Settings::getValue(String key)
  {
    if(!_ini){ini();}
    String value="";
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        keyValue item = _listKeyValues[i];
        //Serial.println(item.key + " -> " + item.value);
        if (item.key==key)
          {
            value=item.value;
            break;
          }
      }
    return value;
  }
String Settings::getDescription(String key)
  {
    if(!_ini){ini();}
    String description="";
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        keyValue item = _listKeyValues[i];
        if (item.key==key)
          {
            description=item.description;
            break;
          }
      }
    return description;
  }
bool Settings::contains(String key)
  {
    if(!_ini){ini();}
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        keyValue item = _listKeyValues[i];
        if (item.key==key)
          {
            return true;
          }
      }
    return false;
  }
bool Settings::setValue(String key, String value, String description)
{
  bool autosave=false;
  return (setValue(key, value, description, autosave));
}
bool Settings::setValue(String key, String value)
{
    bool autosave=false;
    return (setValue(key, value, autosave));
}
bool Settings::setValue(String key, String value, bool autosave)
  {
    if(!_ini){ini();}
    String description;
    if(contains(key))
      {
        description=getDescription(key);
      }
    return (setValue(key, value, description, autosave));
  }
bool Settings::setValue(String key, String value, String description, bool autosave)
  {
    if(!_ini){ini();}
    bool found=false;
    bool change=false;
    // Serial.print("Nombre de clé/valeur: ");_listKeyValues
    // Serial.println(_listKeyValues.size());
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        //keyValue item = _listKeyValues[i];
        //Serial.println("On compare " + key + " à " + _listKeyValues[i].key);
        if (_listKeyValues[i].key==key)
          {
            // Serial.println("La clé de cet élément ("+key+") existe déjà");
            found=true;
            if (_listKeyValues[i].value!=value)
              {
                _listKeyValues[i].value=value;
                change=true;
              }
            if (_listKeyValues[i].description!=description)
              {
              _listKeyValues[i].description=description;
              change=true;
              }
            // Serial.println(change?"changed":"Not changed");
            break;
          }
      }
    if (!found)
      {
        // Serial.println ("Not found in setValue!");
        keyValue item;
        item.key=key;
        item.value=value;
        item.description=description;
        _listKeyValues.push_back(item);
        change=true;
      }

    if (change && autosave)
      {  
        // debugln(F("Enregistrement demandé des données modifiées..."));
        save();
        change=false;
      }
    else
      {
        // debugDated(F("Aucune donnée modifiée pour la clé: "));
        // debugln(key + " (valeur: " + value + ")");
      }
    return change;
  }
void Settings::save()
  {
    int n=_listKeyValues.size();
    String line="";
    for (int i=0;i<n;i++)
      {
        line+=_listKeyValues[i].key+","+_listKeyValues[i].value+","+ _listKeyValues[i].description;
        if (i<n-1)
          {
            line += "\n";
          }
      }
    debuglnDated(F("Contenu à enregistrer:"));
    debugln(line);
    SD.remove(_fileName + ".txt");
    File file = getFile("w");

    // #ifdef ESP_8266
    // File file = SPIFFS.open("/ESP8266.txt","w");
    // #elif defined ESP_32
    // File file = SPIFFS.open("/ESP32.txt");
    // #endif



    //file = SPIFFS.open("/settings.txt","w");
    if(!file)
      {
        debuglnDated(F("There was an error opening the file"));
        return;
      }
    int bytesWritten = file.print(line);
    debugDated(F("Nb de bytes écrits: "));
    debugln(bytesWritten);
    if (bytesWritten > 0) 
      {
        debugDated(F("File was written ("));
        debug(bytesWritten);
        debugln(" bytes)");
    
      }
    else
      {
        debuglnDated(F("File write failed"));
      }

    file.close();
    return;
  }
String Settings::list()
  {
    if(!_ini){ini();}
    String result;
    Serial.printf("------------Contenu de %s -----------\n",_fileName.c_str());
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        keyValue item = _listKeyValues[i];
        debugln("   key        : " + item.key);
        debugln("   value      : " + item.value);
        debugln("   description: " + item.description);
        result += item.description + ": " + item.value + "\r\n";
      }
    return result;
  }
void Settings::erase()
  {
    if(!_ini){ini();}
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        _listKeyValues[i].value="";
      }
    save();
    return;    
  }
bool Settings::deleteMe(String key, bool autosave)
  {
    for (unsigned int i=0;i<_listKeyValues.size();i++)
      {
        if (_listKeyValues[i].key==key)
          {
            _listKeyValues.erase(_listKeyValues.begin() + i);
            if(autosave){save();}
            return true;
          }
      }
    return false;
  }