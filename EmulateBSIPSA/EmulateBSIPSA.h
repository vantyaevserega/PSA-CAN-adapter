#ifndef _EmulateBSIPSA_H_
#include <CanFrameLog.h>
//#include <mcp2515.h>
#define _EmulateBSIPSA_H_

class EmulateBSIPSA
{
  public:
    EmulateBSIPSA(MCP2515* _sender, CanFrameLog* _logger);
    
    void DoWork(unsigned long now);

    void Init();

    void SetState(bool inWork);

    void SetMessage(unsigned long now, int _message);

    void StartMessages(unsigned long now);

    void StopMessages(unsigned long now);
  
  private:
    bool active;
    
    unsigned long last100;
    
    unsigned long last500;
    
    unsigned long last1000;
    
    struct can_frame data1;
    
    struct can_frame data2;

    struct can_frame data_message;

    struct can_frame data_time;

    unsigned long previousMessage;
    
    unsigned long previousCycle;
    
    int message;

    bool cycleMessages;

    MCP2515* sender;
    
    CanFrameLog* loger;
};

#endif