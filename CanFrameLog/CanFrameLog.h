#ifndef _CanFrameLog_H_
//#include <Arduino.h>
#include <mcp2515.h>
#define _CanFrameLog_H_

class CanFrameLog
{
  public:
//    CanFrameLog();
    
    void logMessage(struct can_frame *message);  
};

#endif