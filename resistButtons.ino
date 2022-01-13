#include <mcp2515.h>
#include <SPI.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);

const int startR  = 3; 
const int stepR = 4;
const int CS_XC = 2;
const int INC_XC = 3;
const int UD_XC = 4;

const int REVERSEPIN = 8;
const int ILLUMINATEPIN = 9;

int prevButton = 0;
int currentButton = 0;
int prevScroll = 0;
int prevScroll2 = 0;
int prevScroll3 = 0;
bool prevReverse;
bool prevIlluminate;
void setup() 
{
  Serial.begin(115200);
  Serial.println("Start");
  mcp2515.reset();
  mcp2515.setConfigMode();
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
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
  {    
    bool isHandled = false;
    if(canMsg.can_id == 0x21F && canMsg.can_dlc == 3)
    {
      isHandled = true;
      if(currentButton < 13)
      {
        currentButton = 0;
      }
      
      if(canMsg.data[1] > prevScroll || prevScroll - canMsg.data[1] > 200)
      {
        currentButton = 9;
      }  

      if(canMsg.data[1] < prevScroll || prevScroll - canMsg.data[1] < -200)
      {
        currentButton = 10;
      }  

      prevScroll = canMsg.data[1];
      if(canMsg.data[0] & 0b00000001)
      {
        currentButton = 1;      
      }

      if(canMsg.data[0] & 0b00000010)
      {
        currentButton = 2;      
      }

      if(canMsg.data[0] & 0b00000100)
      {
        currentButton = 3;      
      }

      if(canMsg.data[0] & 0b00001000)
      {
        currentButton = 4;      
      }

      if(canMsg.data[0] & 0b00001100)
      {
        currentButton = 11;      
      }

      if(canMsg.data[0] & 0b00010000)
      {
        currentButton = 5;      
      }

      if(canMsg.data[0] & 0b00100000)
      {
        currentButton = 6;      
      }

      if(canMsg.data[0] & 0b01000000)
      {
        currentButton = 7;      
      }

      if(canMsg.data[0] & 0b10000000)
      {
        currentButton = 8;      
      }

      if(canMsg.data[0] & 0b11000000)
      {
        currentButton = 12;      
      }
      
      log(canMsg, true);
    }
    
    if(canMsg.can_id == 0x0A2 && canMsg.can_dlc == 6)
    {
      isHandled = true;
      if(currentButton > 12)
      {
        currentButton = 0;
      }
      
      if(canMsg.data[0] > prevScroll2 || prevScroll2 - canMsg.data[0] > 200)
      {
        currentButton = 13;
      }  

      if(canMsg.data[0] < prevScroll2 || prevScroll2 - canMsg.data[0] < -200)
      {
        currentButton = 14;
      }  

      prevScroll2 = canMsg.data[0];       

      if(canMsg.data[3] > prevScroll3 || prevScroll3 - canMsg.data[3] > 200)
      {
        currentButton = 15;
      }  

      if(canMsg.data[3] < prevScroll3 || prevScroll3 - canMsg.data[3] < -200)
      {
        currentButton = 16;
      }  

      prevScroll3 = canMsg.data[3];
      log(canMsg, true);
    }    

    // задний ход
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
    }    

    // время
    if(canMsg.can_id == 0x276 && canMsg.can_dlc > 4)
    {  
            isHandled = true;    
      log(canMsg, true);

      float coef = (180 - ((canMsg.data[1] & 0b00001111)-1)*30+(canMsg.data[2] & 0b00011111))/180.0; 
      coef = coef < 0 ? -coef : coef;
      float currentTime = canMsg.data[3] + canMsg.data[4]/60.0;
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
    }    

    if(!isHandled)
    {
      log(canMsg, true);
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
  digitalWrite(UD_XC, LOW); // выбираем понижение
  digitalWrite(CS_XC, LOW); // выбираем потенциометр X9C
  for (int i=0; i<100; i++) 
  { // т.к. потенциометр имеет 100 доступных позиций
    digitalWrite(INC_XC, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_XC, HIGH);
    delayMicroseconds(1);
  }

  // Поднимаем сопротивление до нужного:
  digitalWrite(UD_XC, HIGH);
  for (int i=0; i<percent; i++) 
  {
    digitalWrite(INC_XC, LOW);
    delayMicroseconds(1);
    digitalWrite(INC_XC, HIGH);
    delayMicroseconds(1);
  }

  digitalWrite(CS_XC, HIGH);
}
