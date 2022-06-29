#ifndef _CanFrameLog_H_
#include <SoftwareSerial.h>
#include <mcp2515.h>
#define _CanFrameLog_H_

class CanFrameLog
{
  public:
    CanFrameLog(SoftwareSerial* additionalSerial);
    
    void logMessage(struct can_frame *message); 

    SoftwareSerial* aSerial; 
};

#endif