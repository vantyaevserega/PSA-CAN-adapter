
#include "CanFrameLog.h"



CanFrameLog::CanFrameLog(SoftwareSerial* additionalSerial)
{  
  aSerial = additionalSerial;
}

void CanFrameLog::logMessage(struct can_frame *message) 
{
    Serial.print(message->can_id, HEX); // ID
    Serial.print("\t");
    Serial.print(message->can_dlc, HEX); // размер
    for (int i = 0; i<message->can_dlc; i++)  
    {
      Serial.print("\t");
      Serial.print(message->data[i], HEX);
    }

    Serial.println("");

    aSerial->print(message->can_id, HEX); // ID
    aSerial->print("\t");
    aSerial->print(message->can_dlc, HEX); // размер
    for (int i = 0; i<message->can_dlc; i++)  
    {
      aSerial->print("\t");
      aSerial->print(message->data[i], HEX);
    }

    aSerial->println("");
}