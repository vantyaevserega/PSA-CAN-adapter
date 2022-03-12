
#include "EmulateBSIPSA.h"

    EmulateBSIPSA::EmulateBSIPSA(MCP2515* _sender, CanFrameLog* _logger)
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

      data_time.can_id  = 0x276;
      data_time.can_dlc = 7;
      data_time.data[0] = 0x95;
      data_time.data[1] = 0x03;
      data_time.data[2] = 0x05;
      data_time.data[3] = 0x08;
      data_time.data[4] = 0x10;
      data_time.data[5] = 0x1B;
      data_time.data[6] = 0x10;

      data_message.can_id  = 0x1A1;
      data_message.can_dlc = 8;
      data_message.data[0] = 0x00;
      data_message.data[1] = 0x00;
      data_message.data[2] = 0x80;
      data_message.data[3] = 0x00;
      data_message.data[4] = 0x00;
      data_message.data[5] = 0x00;
      data_message.data[6] = 0x00;
      data_message.data[7] = 0x00;
    }
    
    void EmulateBSIPSA::StartMessages(unsigned long now)
    {
      SetMessage(now, 0);
      cycleMessages = true;
      previousCycle = now;
    }

    void EmulateBSIPSA::StopMessages(unsigned long now)
    {
      SetMessage(now, -1);
      cycleMessages = false;
    }

    void EmulateBSIPSA::SetMessage(unsigned long now, int _message)
    {
      previousMessage = now;
      data_message.data[0] = _message > -1 && _message < 256 ? 0x80 : 0;
      data_message.data[1] = _message > -1 && _message < 256 ? _message : 0;
      message = _message;
    }

    void EmulateBSIPSA::DoWork(unsigned long now)
    {
      if(!active)
      {
        return;
      }

      if(cycleMessages && now - previousCycle > 5000)
      {
        SetMessage(now, ++message);
        previousCycle = now;
        if(message > 255)
        {
          cycleMessages = false;
        }
      }

      if(now - previousMessage > 4000)
      {
        if(data_message.data[0] == 0x80)
        {
          data_message.data[0] = 0x7F;
        }
        else
        {
          data_message.data[0] = 0;
        }
      }

      if(now - last100 >= 100 )
      {
        sender->sendMessage(&data_message);
        sender->sendMessage(&data1);
        last100 = now;
      }

      if(now - last500 >= 500 )
      {
        sender->sendMessage(&data2);
                sender->sendMessage(&data_time);
        last500 = now;
      }
    }

    void EmulateBSIPSA::Init()
    {
    }

    void EmulateBSIPSA::SetState(bool inWork)
    {
      active = inWork;
    }