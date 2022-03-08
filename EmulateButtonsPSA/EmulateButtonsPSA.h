#ifndef _EmulateButtonsPSA_H_
#include <CanFrameLog.h>
//#include <mcp2515.h>
#define _EmulateButtonsPSA_H_

class EmulateButtonsPSA
{
  public:
  enum PSA_BUTTON {
    NONE,
    MODE,
    DOWN,
    DARK,
    LEFT,
    OK,
    RIGHT,
    ESCAPE,
    UP,
    MENU
};

    EmulateButtonsPSA(MCP2515* _sender, CanFrameLog* _logger);
    
    void DoWork(unsigned long now);

    void SetState(bool inWork);

    void SetButton(PSA_BUTTON button);
  
  private:
    bool active;

    struct can_frame data1;

    struct can_frame data2;
    
    unsigned long last100;
    
    PSA_BUTTON current;

    MCP2515* sender;
    
    CanFrameLog* loger;
};

#endif