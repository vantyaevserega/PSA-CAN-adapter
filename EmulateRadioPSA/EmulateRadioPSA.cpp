
#include "EmulateRadioPSA.h"

    EmulateRadioPSA::EmulateRadioPSA(MCP2515* _sender, CanFrameLog* _logger)
    {
      sender = _sender;
      loger = _logger;
      radioState.can_id = 0x165;
      radioState.can_dlc = 4;
      radioState.data[0] = 204;
      radioState.data[1] = 128;//+64;//60;
      radioState.data[2] = 64;
      radioState.data[3] = 0;
      
      radioLevel.can_id = 0x1A5;
      radioLevel.can_dlc = 1;
      radioLevel.data[0] = 239;

      // 3F 48 3F 3F 3F 07 0F
      radioSettings.can_id = 0x1E5;
      radioSettings.can_dlc = 7;
      radioSettings.data[0] = 0x3F;
      radioSettings.data[1] = 0x48;
      radioSettings.data[2] = 0x3F;
      radioSettings.data[3] = 0x3F;
      radioSettings.data[4] = 0x3F;
      radioSettings.data[5] = 0x07;
      radioSettings.data[6] = 0x0F;
    }
    
    void EmulateRadioPSA::DoWork(unsigned long now)
    {
      if(!active)
      {
        return;
      }

      if(now - last100 >= 100 )
      {
        sender->sendMessage(&radioState);
        last100 = now;
      }

      if(now - last500 >= 500 )
      {
        sender->sendMessage(&radioSettings);
        sender->sendMessage(&radioLevel);    
        last500 = now;

              struct can_frame radioInit;
      radioInit.can_id = 0x39b;
      radioInit.can_dlc = 5;
      radioInit.data[0] = 0xf4;
      radioInit.data[1] = 0x05;
      radioInit.data[2] = 0x0c;
      radioInit.data[3] = 0x14;
      radioInit.data[4] = 0x1e;
      sender->sendMessage(&radioInit);

      }
    }

    void EmulateRadioPSA::Init()
    {
      struct can_frame radioInit;
      radioInit.can_id = 0x5E0;
      radioInit.can_dlc = 8;
      radioInit.data[0] = 0x20;
      radioInit.data[1] = 0x01;
      radioInit.data[2] = 0x02;
      radioInit.data[3] = 0x06;
      radioInit.data[4] = 0x12;
      radioInit.data[5] = 0x1F;
      radioInit.data[6] = 0x20;
      radioInit.data[6] = 0x11;  
      sender->sendMessage(&radioInit);
    }

    void EmulateRadioPSA::SetText(char* text)
    {

    }

    void EmulateRadioPSA::SetState(bool inWork)
    {
      active = inWork;
    }