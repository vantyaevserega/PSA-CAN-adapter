
#include "EmulateButtonsPSA.h"

    EmulateButtonsPSA::EmulateButtonsPSA(MCP2515* _sender, CanFrameLog* _logger)
    {
      sender = _sender;
      loger = _logger;
      data1.can_id  = 0x036;
      data1.can_dlc = 8;
      data1.data[0] = 0x0E;
      data1.data[1] = 0x00;
      data1.data[2] = 0x05;
      data1.data[3] = 0x2F;
      data1.data[4] = 0x21;
      data1.data[5] = 0x80;
      data1.data[6] = 0x00;
      data1.data[7] = 0xA0;

      data2.can_id  = 0x0F6;
      data2.can_dlc = 8;
      data2.data[0] = 0x08;
      data2.data[1] = 0x32;
      data2.data[2] = 0x00;
      data2.data[3] = 0x1F;
      data2.data[4] = 0x00;
      data2.data[5] = 0x0D;
      data2.data[6] = 0x40;
      data2.data[7] = 0x01;
    }
    
    void EmulateButtonsPSA::DoWork(unsigned long now)
    {
      if(!active)
      {
        return;
      }

      struct can_frame data3;  
      data3.can_id  = 0x3E5;
      data3.can_dlc = 6;
      data3.data[0] = 0;
      data3.data[1] = 0;
      data3.data[2] = 0;
      data3.data[3] = 0;
      data3.data[4] = 0;
      data3.data[5] = 0;

      if(now - last100 >= 100)
      {
        switch(current)
        {          
        case UP:
        data3.data[5] = 64;
          break;
        case DOWN:
        data3.data[5] = 16;
          break;
        case LEFT:
        data3.data[5] = 1;
          break;
        case RIGHT:
        data3.data[5] = 4;
          break;
        case OK:
        data3.data[2] = 64;
          break;
        case ESCAPE:
        data3.data[2] = 16;
          break;
        case MENU:
        data3.data[0] = 64;
          break;
        case MODE:
        data3.data[1] = 16;
          break;
        case DARK:
        data3.data[2] = 4;
          break;
      }
  
      sender->sendMessage(&data3);
      last100 = now;
    }
    }

    void EmulateButtonsPSA::SetState(bool inWork)
    {
      active = inWork;
    }

    void EmulateButtonsPSA::SetButton(EmulateButtonsPSA::PSA_BUTTON button)
    {
      current = button;
    }