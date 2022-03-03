#include <mcp2515.h>
#include <SPI.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);
unsigned long prevChangeButtonState;
unsigned long prevInfoState;
unsigned long prevF6;
unsigned long prev36;
struct can_frame data1;
struct can_frame data2;
struct can_frame data3;
struct can_frame data_info;
int currentButton = 0;
int infos;
void setup() 
{
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

  data3.can_id  = 0x3E5;
  data3.can_dlc = 6;
  data3.data[0] = 0;
  data3.data[1] = 0;
  data3.data[2] = 0;
  data3.data[3] = 0;
  data3.data[4] = 0;
  data3.data[5] = 0;

  data_info.can_id  = 0x1A1;
  data_info.can_dlc = 8;
  data_info.data[0] = 0x80;
  data_info.data[1] = 0;
  data_info.data[2] = 0x46;
  data_info.data[3] = 0x80;
  data_info.data[4] = 0;
  data_info.data[5] = 0;
  data_info.data[6] = 0;
  data_info.data[7] = 0;
  
  Serial.begin(115200);
  Serial.println("Start work");
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();
  infos = -1;
}

void loop() 
{
  // обработка серийного порта, для конфигурации
  if (Serial.available() > 0) 
  {
    char readed = (char)Serial.read();
    if(readed >= 48 && readed < 57)
    {
      currentButton = readed - '0';
    }
    else
    {
      if(readed == 'A')
      {
        infos = 0;
      }
    }    
  }

  if(currentButton > 0)
  {
    infos = -1;
  }

  if(infos > -1 && infos < 256 && millis() - prevInfoState >= 5000)
  {
    data_info.data[1] = infos;
    ++infos;
    mcp2515.sendMessage(&data_info);
  }
  
  if(millis() - prev36 >= 100 )
  {
    mcp2515.sendMessage(&data1);
    prev36 = millis();
  }

  if(millis() - prevF6 >= 500)
  {
    mcp2515.sendMessage(&data2);
    prevF6 = millis();
  }
  
  if(millis() - prevChangeButtonState > 50 && currentButton > 0)
  {
      switch(currentButton)
      {          
        case 8: // up
        data3.data[5] = 64;
          break;
        case 2: //down
        data3.data[5] = 16;
          break;
        case 4: // left
        data3.data[5] = 1;
          break;
        case 6: // right
        data3.data[5] = 4;
          break;
        case 5: // ok
        data3.data[2] = 64;
          break;
        case 7: // esc
        data3.data[2] = 16;
          break;
        case 9: // menu
        data3.data[0] = 64;
          break;
        case 1: // mode
        data3.data[1] = 16;
          break;
        case 3: // dark
        data3.data[2] = 4;
          break;
      }
      
      mcp2515.sendMessage(&data3);
      currentButton = 0;
      data3.data[0] = 0;
      data3.data[1] = 0;
      data3.data[2] = 0;
      data3.data[3] = 0;
      data3.data[4] = 0;
      data3.data[5] = 0;
      mcp2515.sendMessage(&data3);
      prevChangeButtonState = millis();
  }

   if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
   {
      log(canMsg);
   }  
}

// логирование сообщения от шины автомобиля
void log(struct can_frame can)
{
    Serial.print(can.can_id, HEX); // ID
    Serial.print(" ");
    Serial.print(can.can_dlc, HEX); // размер
    for (int i = 0; i<can.can_dlc; i++)  
    {
      Serial.print(" ");
      Serial.print(can.data[i],HEX);
    }

    Serial.println("");
}
