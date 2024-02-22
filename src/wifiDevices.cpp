#include "wifiDevices.h"

ClsColWifiDevices::ClsColWifiDevices(String _fileName)
  {
    fileName=_fileName;
    arrayNewMac="";
    rssiThreshold = -80;
    countDevices=0;
    
  }
bool ClsColWifiDevices::checkFreeSpace()
  {
    return (countDevices<NMAXWIFIDEVICES);
  }
wifiDeviceType * ClsColWifiDevices::item(int i)
  {
    // Serial.printf("Item = %d\n",i);
    return  &wifiDevices[i];
  }
wifiDeviceType * ClsColWifiDevices::item(String mac)
  {
    bool found=false;
    size_t i=0;
    if(countDevices>0)
      {
            
        do
          {
            // Serial.print(i);
            // Serial.print(" ");
            if((strcmp(wifiDevices[i].mac,mac.c_str()))==0)
              {
                Serial.printf("Found: %s at position %d ",mac.c_str(),i);
                found=true;
                return item(i);
              }  
            i++;
          } while(!found and i!=countDevices);
          }

    Serial.printf("Creation d'un nouveau device en position %02d, mac = %s ",i,mac.c_str());
    strcpy(wifiDevices[i].mac,mac.c_str());
    countDevices++;
    return  &wifiDevices[i];
  }
bool ClsColWifiDevices::contains(String mac)
  {
    bool result=false;
    for (size_t i = 0; i < countDevices; i++)
      {
        if((strcmp(wifiDevices[i].mac,mac.c_str()))==0)
          {
            result=true;
          }
      }
    return result;
    
  }
void ClsColWifiDevices::deleteMe(String mac)
  {

  }
void ClsColWifiDevices::deleteMe(size_t index)
  {
    // Serial.printf("Effacement de mac: %s\n", this->item(index)->mac);
    // Serial.printf("ArrayNewMac avant: %s\n",arrayNewMac.c_str());
    if(index<countDevices)
      {
        for (size_t i = index; i < countDevices-1; i++)
          {
            swap(i,i+1);
            // yield();
          }
        wifiDeviceType * temp=item(countDevices-1);
        // Serial.printf("mac: %s, countDetection: %d\n",temp->mac,temp->countDetection);
        temp->countDetection=0;
        temp->avrgRssi=0;
        temp->ddlsl=false;
        strcpy(temp->history,"");
        strcpy(temp->mac,"");
        // Serial.printf("ArrayNewMac après: %s\n",arrayNewMac.c_str());

        countDevices--;
      }
    else
      {
        Serial.println("Impossible d'effacer un device qui n'existe pas!");
      }

    
  }

void ClsColWifiDevices::swap(int i, int j)
  {
    wifiDeviceType  temp;
    wifiDeviceType  *thisDevice1=item(i);//*this->item(i);
    wifiDeviceType  *thisDevice2=item(j);
    strcpy(temp.mac,thisDevice1->mac);
    strcpy(thisDevice1->mac,thisDevice2->mac);
    strcpy(thisDevice2->mac, temp.mac);
    temp.avrgRssi=thisDevice1->avrgRssi;
    thisDevice1->avrgRssi=thisDevice2->avrgRssi;
    thisDevice2->avrgRssi=temp.avrgRssi;

    temp.countDetection=thisDevice1->countDetection;
    thisDevice1->countDetection=thisDevice2->countDetection;
    thisDevice2->countDetection=temp.countDetection;

    temp.ddlsl=thisDevice1->ddlsl;
    thisDevice1->ddlsl=thisDevice2->ddlsl;
    thisDevice2->ddlsl=temp.ddlsl;

    strcpy(temp.history,thisDevice1->history);
    strcpy(thisDevice1->history,thisDevice2->history);
    strcpy(thisDevice2->history, temp.history);
  }
void ClsColWifiDevices::list()
  {
    for (size_t i = 0; i < countDevices; i++)
    {
      Serial.printf("mac: %s, détection: %02d  avgRssi:%5.2f\n", this->item(i)->mac, this->item(i)->countDetection, item(i)->avrgRssi);
    }
  }

void ClsColWifiDevices::sort()
  {
    // Serial.println("Avant tri:");
    if(countDevices>1)
      {
      // list();
      for (size_t k = countDevices-1;k>1; k--)
        {
          bool change=false;        
          for (size_t i = 0; i < k; i++)
          {
            if(item(i)->avrgRssi<item(i+1)->avrgRssi)
              {
                swap(i,i+1);
                change=true;
              }
          }
          if(!change){break;}
        }
      // Serial.println("Après tri: ");
      // list();
    }
  }
void ClsColWifiDevices::save()
  {
    File file = getFile("w");
    for (size_t i = 0; i < countDevices; i++)
    {
      char buffer[180];
      sprintf (buffer, "%s,%5.2f,%03d,%s\n", item(i)->mac, item(i)->avrgRssi,item(i)->countDetection, item(i)->history);
      file.print(buffer);
      Serial.printf("%02d: ",i);
      Serial.print(buffer);
      item(i)->countDetection=0;
    }
    file.close();
    Serial.println("File saved!");
    
  }
void ClsColWifiDevices::read()
  {

  }


File ClsColWifiDevices::getFile(const char* mode)
  {
    File myFile;
    if(strcmp(mode,"r")==0)
      {
        myFile = SD.open(fileName + ".txt", FILE_READ);
      }
    else
      {
        myFile = SD.open(fileName + ".txt",FILE_WRITE);
        myFile.seek(0);
      }
    return myFile;
  }