#include <mcp2515.h>
#include <SPI.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);

const int startR  = 3; 
const int stepR = 4;
const int CS_XC = 2;
const int INC_XC = 3;
const int UD_XC = 4;

int confPressedTime;
bool isConfig = false;
const int REVERSEPIN = 8;
const int ILLUMINATEPIN = 9;
int vv = 0;
int prevButton = 0;
int currentButton = 0;
int prevScroll = 0;
int prevScroll2 = 0;
int prevScroll3 = 0;
bool prevReverse;
bool prevIlluminate;
int prevResist = 0;
void setup() 
{
  Serial.begin(115200);
  Serial.println("Start");
  mcp2515.reset();
  //mcp2515.setConfigMode();
  /*
  mcp2515.setFilter(MCP2515::RXF0, false, 0x21F);
  mcp2515.setFilter(MCP2515::RXF1, false, 0x0A2);
  mcp2515.setFilter(MCP2515::RXF2, false, 0x0F6);
  mcp2515.setFilter(MCP2515::RXF3, false, 0x276);
  */  
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();

  // инициализация резистивных выводов  
  pinMode(CS_XC, OUTPUT);
  pinMode(INC_XC, OUTPUT);
  pinMode(UD_XC, OUTPUT);
  digitalWrite(CS_XC, HIGH);
  digitalWrite(INC_XC, HIGH); 
  digitalWrite(UD_XC, HIGH); 

  pinMode(REVERSEPIN, OUTPUT);
  pinMode(ILLUMINATEPIN, OUTPUT);
  digitalWrite(REVERSEPIN, LOW); 
  digitalWrite(ILLUMINATEPIN, LOW); 
}

void loop() 
{
  if (Serial.available() > 0) 
  {
    int data = Serial.parseInt();
    if(data == 100)
    {
      isConfig = true;
    }

    if(data == 200)
    {
      isConfig = false;
    }

    if(currentButton > 0 && millis() - confPressedTime > 5000)
    {
      currentButton = 0;
    }
    
    if(data > 0 && data < 15)
    {
      confPressedTime = millis();
      currentButton = data;
    }    
  }
  
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK && !isConfig) 
  {    
    bool isHandled = false;
    if(canMsg.can_id == 0x21F && canMsg.can_dlc == 3)
    {
      isHandled = true;
      if(currentButton & 0b111 > 0)
      {
        currentButton = (currentButton >> 3) << 3;
      }
      
      if(canMsg.data[1] > prevScroll || prevScroll - canMsg.data[1] > 200) // scroll up
      {
        currentButton = 5;
      }  

      if(canMsg.data[1] < prevScroll || prevScroll - canMsg.data[1] < -200) // scroll down
      {
        currentButton = 6;
      }  

      prevScroll = canMsg.data[1];
      if(canMsg.data[0]  == 4) // volume down
      {
        currentButton = 1;      
      }

      if(canMsg.data[0] == 8) // volume up
      {
        currentButton = 2;      
      }

      if(canMsg.data[0] == 12) // mute
      {
        currentButton = 3;      
      }

      if(canMsg.data[0] == 128) // forward
      {
        currentButton = 4;      
      }
            
      log(canMsg, true);
      Serial.print("Btn "); Serial.println(currentButton);  
      Serial.print("Scr1 "); Serial.println(prevScroll); 
    }
    
    if(canMsg.can_id == 0xA2 && canMsg.can_dlc == 5)
    {
      isHandled = true;
      if(currentButton > 7)
      {
        currentButton = currentButton & 0b111;
      }
      
      if(canMsg.data[0] > prevScroll2 || prevScroll2 - canMsg.data[0] > 200)
      {
        currentButton = 8;
      }  

      if(canMsg.data[0] < prevScroll2 || prevScroll2 - canMsg.data[0] < -200)
      {
        currentButton = 9;
      }  

      prevScroll2 = canMsg.data[0];       

      //if(canMsg.data[3] > prevScroll3 || prevScroll3 - canMsg.data[3] > 200)
      //{
      //  currentButton = 15;
      //}  

      //if(canMsg.data[3] < prevScroll3 || prevScroll3 - canMsg.data[3] < -200)
      //{
      //  currentButton = 16;
      //}  

      if(canMsg.data[1]  == 4) // 
      {
        currentButton = 10;      
      }

      if(canMsg.data[0] == 8) //
      {
        currentButton = 11;      
      }

      if(canMsg.data[1]  == 16) // 
      {
        currentButton = 12;      
      }

      if(canMsg.data[0] == 32) //
      {
        currentButton = 13;      
      }

      //prevScroll3 = canMsg.data[3];
      log(canMsg, true);
      Serial.print("Btn "); Serial.println(currentButton);  
      Serial.print("Scr3 "); Serial.println(prevScroll3); 
      Serial.print("Scr2 "); Serial.println(prevScroll2); 
    }    

    // задний ход все верно
    if(canMsg.can_id == 0x0F6 && canMsg.can_dlc == 8)
    {
        isHandled = true;      
        if (canMsg.data[7] & 0b10000000) 
        {
          if(!prevReverse)
          {
            prevReverse = true;
            digitalWrite(REVERSEPIN, HIGH);
          }
        }
        else
        {
          if(prevReverse)
          {
            prevReverse = false;
            digitalWrite(REVERSEPIN, LOW);          
          }
        }
                
      log(canMsg, true);    
      Serial.print("Reverse "); Serial.println(prevReverse);
    }    

    // время
    if(canMsg.can_id == 0x3f6 && canMsg.can_dlc == 7)
    {  
      isHandled = true;  
      log(canMsg, true);
      float day = ((canMsg.data[3] & 0b00001111) << 4) + (canMsg.data[4] >> 4);
      float month = canMsg.data[4] & 0b00001111;
      float year = canMsg.data[5] + 2000;
      float coef = (180 - (month-1)*30+day)/180.0; 
      coef = coef < 0 ? -coef : coef;
      float currentTime = ((canMsg.data[0] << 12) + (canMsg.data[1] << 4) + (canMsg.data[2] >> 4))/3600.0;
      if(currentTime < 3.5 + 6.0/coef || currentTime > 21.5 - 5.5/coef)
      {
        if(!prevIlluminate)
        {
          prevIlluminate = true;        
          digitalWrite(ILLUMINATEPIN, HIGH);
        }
      } 
      else
      {
        if(prevIlluminate)
        {
          prevIlluminate = false;        
          digitalWrite(ILLUMINATEPIN, LOW);
        }        
      }

       Serial.print("Illuminate "); Serial.print(prevIlluminate); Serial.print("Date "); Serial.print(day); Serial.print("."); Serial.print(month); Serial.print("."); Serial.print(year); Serial.print(" Time "); Serial.println(currentTime);  
    }
 }
  
  // если необходимо - нажимаем кнопку
  if(currentButton != prevButton)
  {
    pressButton(currentButton);
    prevButton = currentButton;
  }
}

void pressButton(int value)
{
  if(value == 0)
  {
    setResistance(0);
  }
  else
  {
    setResistance(startR + value*stepR);
  }
}

void log(struct can_frame can, bool isParsed)
{
    if(isParsed)
    {
      Serial.print("Defined ");
    }
    
    Serial.print(can.can_id, HEX); // print ID
    Serial.print(can.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<can.can_dlc; i++)  
    {
      Serial.print(can.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println("");
}

void setResistance(int percent) 
{ 
  digitalWrite(CS_XC, LOW); // выбираем потенциометр X9C
    digitalWrite(UD_XC, LOW); // выбираем понижение
  
  for (int i=0; i<99; i++) 
  { 
    // т.к. потенциометр имеет 100 доступных позиций
    digitalWrite(INC_XC, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_XC, HIGH);
    delayMicroseconds(1);
  }

  digitalWrite(UD_XC, HIGH);
  
  for (int i=0; i<percent; i++) 
  { 
    // т.к. потенциометр имеет 100 доступных позиций
    digitalWrite(INC_XC, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_XC, HIGH);
    delayMicroseconds(1);
  }

  digitalWrite(CS_XC, HIGH);
}
