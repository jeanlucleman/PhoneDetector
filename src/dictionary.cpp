#include "dictionary.h"

ClsKeyValues::ClsKeyValues()
  {

    countKeyValues=0;
    
  }
bool ClsKeyValues::checkFreeSpace()
  {
    return (countKeyValues<NMAXKEYVALUES);
  }
keyValueType * ClsKeyValues::item(int i)
  {
    // Serial.printf("Item = %d\n",i);
    return  &keyValues[i];
  }
keyValueType * ClsKeyValues::item(String key)
  {
    bool found=false;
    size_t i=0;
    if(countKeyValues>0)
      {
        do
          {
            if(keyValues[i].key==key)
              {
                // Serial.printf("Found: %s at position %02d ",key.c_str(),i);
                found=true;
                return item(i);
              }  
            i++;
          } while(!found and i!=countKeyValues);
      }

    Serial.printf("Creation d'un nouveau device en position %02d, key = %s ",i,key.c_str());
    keyValues[i].key=key;
    countKeyValues++;
    return  &keyValues[i];
  }
bool ClsKeyValues::contains(String key)
  {
    bool result=false;
    for (size_t i = 0; i < countKeyValues; i++)
      {
        if(keyValues[i].key==key)
          {
            result=true;
          }
      }
    return result;
  }
void ClsKeyValues::deleteMe(String key)
  {

  }
void ClsKeyValues::deleteMe(size_t index)
  {
    if(index<countKeyValues)
      {
        for (size_t i = index; i < countKeyValues-1; i++)
          {
            swap(i,i+1);
            // yield();
          }
        keyValueType * temp=item(countKeyValues-1);
        temp->key="";
        temp->value=0;
        countKeyValues--;
      }
    else
      {
        Serial.println("Impossible d'effacer un device qui n'existe pas!");
      }
  }
void ClsKeyValues::swap(int i, int j)
  {
    keyValueType  temp;
    keyValueType  *thisDevice1=item(i);//*this->item(i);
    keyValueType  *thisDevice2=item(j);
    temp.key=thisDevice1->key;
    thisDevice1->key=thisDevice2->key;
    thisDevice2->key= temp.key;
    temp.value=thisDevice1->value;
    thisDevice1->value=thisDevice2->value;
    thisDevice2->value=temp.value;


  }
void ClsKeyValues::list()
  {
    for (size_t i = 0; i < countKeyValues; i++)
    {
      Serial.printf("key: %s, value: %02d\n", this->item(i)->key.c_str(), this->item(i)->value);
    }
  }
void ClsKeyValues::sort()
  {
    // Serial.println("Avant tri:");
    if(countKeyValues>1)
      {
        for (size_t k = countKeyValues-1;k>1; k--)
          {
            bool change=false;        
            for (size_t i = 0; i < k; i++)
            {
              if(item(i)->value<item(i+1)->value)
                {
                  swap(i,i+1);
                  change=true;
                }
            }
            if(!change){break;}
          }
    }
  }

