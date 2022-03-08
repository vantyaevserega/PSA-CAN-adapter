#ifndef _EmulateRadioPSA_H_
#include <CanFrameLog.h>
//#include <mcp2515.h>
#define _EmulateRadioPSA_H_

class EmulateRadioPSA
{
  public:
    EmulateRadioPSA(MCP2515* _sender, CanFrameLog* _logger);
    
    void DoWork(unsigned long now);

    void Init();

    void SetText(char* text);

    void SetState(bool inWork);  
  private:
    bool active;
    
    char* text;
    
    unsigned long last100;
    
    unsigned long last500;
    
    unsigned long last1000;
    
    struct can_frame radioState;

    struct can_frame radioLevel;
    
    struct can_frame radioSettings;

    MCP2515* sender;
    
    CanFrameLog* loger;
};

#endif