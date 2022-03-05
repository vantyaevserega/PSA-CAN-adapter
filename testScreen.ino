#include <mcp2515.h>
#include <SPI.h>

#define PIN_VRX  A0
#define PIN_VRY  A1
#define PIN_ok   3
#define PIN_esc  4
#define PIN_menu 5
#define PIN_mode 6
#define PIN_dark 7

bool lastOk   = LOW;
bool lastEsc  = LOW;
bool lastMode = LOW;
bool lastMenu = LOW;
bool lastDark = LOW;
int nullValue = 512;

struct can_frame canMsg;
MCP2515 mcp2515(10);
unsigned long pr;
unsigned long prevChangeButtonState;
unsigned long prevInfoState;
unsigned long prevF6;
unsigned long prev36;
bool emulate = false;
struct can_frame data1;
struct can_frame data2;
struct can_frame data3;
struct can_frame data_info;
int currentButton = 0;
int infos;

bool debounce(int pinValue, int lastButton) 
{
  bool current = digitalRead(pinValue);
  if (current != lastButton) 
  {                  
    delay(5);                              
    current = digitalRead(pinValue);      
  }
  
  return current;
}
  
void setup() 
{
  pinMode(PIN_esc, INPUT);
  pinMode(PIN_menu, INPUT);
  pinMode(PIN_mode, INPUT);
  pinMode(PIN_dark, INPUT);
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
  data_info.data[1] = 0x01;
  data_info.data[2] = 0x80;
  data_info.data[3] = 0x00;
  data_info.data[4] = 0x00;
  data_info.data[5] = 0x00;
  data_info.data[6] = 0x00;
  data_info.data[7] = 0x00;
  
  Serial.begin(115200);
  Serial.println("Start work");
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();
  infos = -1;
}

void loop() 
{
  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);
  if(x < nullValue - 100)
  { currentButton = 4;}
  else if( x > nullValue + 100)
  { currentButton = 6; }
  else if( y < nullValue - 100)
  { currentButton = 2; }
  else if( y > nullValue + 100)
  { currentButton = 8; }
  else 
  { 
    int currentOk = debounce(PIN_ok, lastOk); 
    if (lastOk == LOW && currentOk == HIGH)
    { currentButton = 5; }
    lastOk = currentOk; 

    int currentEsc = debounce(PIN_esc, lastEsc); 
    if (lastEsc == LOW && currentEsc == HIGH)
    { currentButton = 7; }
    lastEsc = currentEsc; 

    int currentMode = debounce(PIN_mode, lastMode); 
    if (lastMode == LOW && currentMode == HIGH)
    { currentButton = 7; }
    lastMode = currentMode; 

    int currentDark = debounce(PIN_dark, lastDark); 
    if (lastDark == LOW && currentDark == HIGH)
    { currentButton = 1; }
    lastDark = currentDark; 

    int currentMenu = debounce(PIN_menu, lastMenu); 
    if (lastMenu == LOW && currentMenu == HIGH)
    { currentButton = 9; }
    lastMenu = currentMenu; 
  }
  
  // обработка серийного порта, для конфигурации
  if (Serial.available() > 0) 
  {
    char readed = (char)Serial.read();
    Serial.println((int)readed);
    if(readed > 48 && readed <= 57)
    {
      currentButton = readed - '0';
    }
    else
    {
      if(readed == 65)
      {
        infos = 0;
      }
      else
      if(readed == 'B')
      { 
        emulate = true;
      }
      else
      if(readed == 'C')
      { 
        emulate = false;
      }
    }    
  }

  if(currentButton > 0)
  {
    infos = -1;
  }

  if(emulate && millis() - pr >= 1000)
  {
  
  struct can_frame dat;
  dat.can_id  = 0x276;
  dat.can_dlc = 7;
  dat.data[0] = 0x95;
  dat.data[1] = 0x03;
  dat.data[2] = 0x05;
  dat.data[3] = 0x08;
  dat.data[4] = 0x10;
  dat.data[5] = 0x1B;
  dat.data[6] = 0x10;

    data_info.data[1] = infos;
    if(millis()-prevInfoState > 5000){++infos; prevInfoState = millis();}
    mcp2515.sendMessage(&data_info);
    mcp2515.sendMessage(&dat);
 
    pr = millis();
  }
  
  if(emulate && millis() - prev36 >= 100 )
  {
    mcp2515.sendMessage(&data1);
    prev36 = millis();
  }

  if(emulate && millis() - prevF6 >= 500)
  {
    mcp2515.sendMessage(&data2);
    prevF6 = millis();
  }
  
  if(millis() - prevChangeButtonState >= 99)
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
if(currentButton > 0)
      Serial.println(currentButton);
      mcp2515.sendMessage(&data3);
      currentButton = 0;
      data3.data[0] = 0;
      data3.data[1] = 0;
      data3.data[2] = 0;
      data3.data[3] = 0;
      data3.data[4] = 0;
      data3.data[5] = 0;
      //mcp2515.sendMessage(&data3);
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
