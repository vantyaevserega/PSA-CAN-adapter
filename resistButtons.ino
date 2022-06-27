#include <DigiPotX9Cxxx.h>
#include <mcp2515.h>
#include <CanFrameLog.h>
#include <EmulateButtonsPSA.h>
#include <SPI.h>

// используемые пины
#define XC_CS 2
#define XC_INC 3
#define XC_UD 4
#define REVERSEPIN 8
#define ILLUMINATEPIN 9
#define MCP2515_CS 10

struct button {
    bool currentState;
    bool previousState;
    bool isLong;
    unsigned long downTime;
};

// обработка кнопок
#define buttonDelay 150
#define buttonDelayLong 500

// шина для обшения с авто
MCP2515 mcp2515(MCP2515_CS);

// сопротивления для кнопок
DigiPot pot(XC_INC,XC_UD,XC_CS);

// логирование пакетов
CanFrameLog lg;

// эмулятор кнопок БК
EmulateButtonsPSA buttons(&mcp2515, &lg);

// пакет шины
struct can_frame canMsg;

unsigned long confPressedTime;
bool isConfig = false;
bool islogenabled = false;
int currentButton = 0;
int prevScroll = 0;
bool prevReverse;
int pressTime = 0;
bool prevIlluminate;
byte debug = 0;
int prevButton;


unsigned long set0;
button forward, up, down, mute, next, prev;

void setup() 
{
  // инициализация шины bluetooth
  Serial.begin(9600);

  // иницилазиация CAN шины
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();

  // инициализация резистивных выводов  
  pot.reset();
  pot.set(99);

  // инициализация управления реле
  pinMode(REVERSEPIN, OUTPUT);
  pinMode(ILLUMINATEPIN, OUTPUT);
  digitalWrite(REVERSEPIN, HIGH); 
  digitalWrite(ILLUMINATEPIN, HIGH);

  // инициализация эмуляции кнопок
  buttons.SetState(true);

  Serial.println("Start work");
}

void loop() 
{
  // начало обработки цикла
  unsigned long now = millis();
  
  // обработка серийного порта, для конфигурации
  if (Serial.available() > 0) 
  {
    char data = Serial.read();
    if (data > '0' && data <= '9')
    {
      buttons.SetButton(data - '0');
    }
    else
    if(data == 'S')
    {
      debug = 0;
    }
    else
    if(data == 'D')
    {
      debug = 1;
    }
    else
    if(data == 'F')
    {
      debug = 2;
    }
    else
    if(data == 's')
    {
      if(debug>0)
      {
        Serial.println("Configuration started");
      }
          
      isConfig = true;
    }
    else
    if(data == 'f')
    {
      if(debug>0)
      {
        Serial.println("Configuration finished");
      } 

      isConfig = false;
      currentButton = 0;
    }
    else
    if(isConfig && data == 'r')
    {
      digitalWrite(REVERSEPIN, HIGH);
      if(debug>0)
      {
        Serial.println("reverse off");   
      }   
    }    
    else
    if(isConfig && data == 'R')
    {
      digitalWrite(REVERSEPIN, LOW);
      if(debug>0)
      {
        Serial.println("reverse on");      
      }   
    }  
    else
    if(isConfig && data == 'i')
    {
      digitalWrite(ILLUMINATEPIN, HIGH);
      if(debug>0)
      {
        Serial.println("illuminate off");      
      }  
    }  
    else
    if(isConfig && data == 'I')
    {
      digitalWrite(ILLUMINATEPIN, LOW);
      if(debug>0)
      {
        Serial.println("illuminate on");       
      }  
    }  
    else    
    if(isConfig && data == 'v')
    {
      confPressedTime = now;
      currentButton = 1;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'V')
    {
      confPressedTime = now;
      currentButton = 2;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'm')
    {
      confPressedTime = now;
      currentButton = 3;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'n')
    {
      confPressedTime = now;
      currentButton = 4;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 't')
    {
      confPressedTime = now;
      currentButton = 5;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'T')
    {
      confPressedTime = now;
      currentButton = 6;
      pressTime = 1500;
    }    
    else    
    if(data == 'l')
    {
      islogenabled = false;
    }    
    else    
    if(data == 'L')
    {
      islogenabled = true;
    }    
    else    
    if(isConfig && data == 'q')
    {
      confPressedTime = now;
      currentButton = 11;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'Q')
    {
      confPressedTime = now;
      currentButton = 12;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'w')
    {
      confPressedTime = now;
      currentButton = 13;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'W')
    {
      confPressedTime = now;
      currentButton = 14;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'e')
    {
      confPressedTime = now;
      currentButton = 15;
      pressTime = 1500;
    }    
    else    
    if(isConfig && data == 'E')
    {
      confPressedTime = now;
      currentButton = 16;
      pressTime = 1500;
    }
  }
  
  if(isConfig)
  {
    if(currentButton > 0 && now - confPressedTime >= pressTime)
    {
        if(debug > 0)
        {          
          Serial.print("config key ");
          Serial.print(currentButton);
          Serial.println(" was up");
        }

        currentButton = 0;
        prevButton = 0;
    }
    
    pressButton(currentButton);
    if(debug > 0 && currentButton > 0 && prevButton != currentButton)
    {
      prevButton = currentButton;
      Serial.print("config key ");
      Serial.print(currentButton);
      Serial.println(" was down");
    }
  }

  // обработка шины автомобиля
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK && !isConfig) 
  {    
    if(debug > 1 || islogenabled)
    {
      lg.logMessage(&canMsg);
    }

    // блок конпок, основной
    if(canMsg.can_id == 0x21F)
    {  
      next.currentState = false;
      prev.currentState = false;
      down.currentState = false;
      up.currentState = false;
      mute.currentState = false;
      forward.currentState = false;
      if(canMsg.data[1] > prevScroll || prevScroll - canMsg.data[1] > 200) // scroll up
      {
        next.currentState = true;
      }  

      if(canMsg.data[1] < prevScroll || prevScroll - canMsg.data[1] < -200) // scroll down
      {
        prev.currentState = true;
      }  

      prevScroll = canMsg.data[1];
      if(canMsg.data[0]  == 4) // volume down
      {
        down.currentState = true;   
      }

      if(canMsg.data[0] == 8) // volume up
      {
        up.currentState = true; 
      }

      if(canMsg.data[0] == 12) // mute
      {
        mute.currentState = true;  
      }

      if(canMsg.data[0] == 128) // forward
      {
        forward.currentState = true;   
      }
    }
    else
    // задний ход
    if(canMsg.can_id == 0xF6 && canMsg.can_dlc == 8)
    {   
        if (canMsg.data[7] & 0b10000000) 
        {
          if(!prevReverse)
          {
            prevReverse = true;
            digitalWrite(REVERSEPIN, LOW);
          }
        }
        else
        {
          if(prevReverse)
          {
            prevReverse = false;
            digitalWrite(REVERSEPIN, HIGH);          
          }
        }
    }
    else
    // яркость
    if(canMsg.can_id == 0x128 && canMsg.can_dlc > 4)
    {   
      if (canMsg.data[4] & 0b00010000) 
      {
        if(!prevIlluminate)
        {
          prevIlluminate = true;
          digitalWrite(ILLUMINATEPIN, LOW);
        }
      }
      else
      {
        if(prevIlluminate)
        {
          prevIlluminate = false;
          digitalWrite(ILLUMINATEPIN, HIGH);          
        }
      }
    }    
 }

  if(!isConfig)
  {
    down    = handleButtonState(now, down, 1);
    up      = handleButtonState(now, up, 2);
    mute    = handleButtonState(now, mute, 3);
    forward = handleButtonState(now, forward, 4);
    prev    = handleButtonState(now, prev, 5);
    next    = handleButtonState(now, next, 6);    
  
    if(set0 > 0 && now >= set0)
    {
      set0 = 0;
      setResistance(0);
      if(debug>0)
      {
        Serial.println("key was up by delay"); 
      }  
    }
  }
    
  buttons.DoWork(now);
}

// нажатие кнопки из настроек
void pressButton(int value)
{
  if(value == 0)
  {
    setResistance(0);
  }
  else
  {
    setResistance(value);
  }
}

// обработка состояния кнопки 
button handleButtonState(unsigned long now, button state, byte value)
{
  if(state.currentState && !state.previousState)
  {
    state.downTime = now;
    if(debug>0)
    {
      Serial.print("key ");      
      Serial.print(value);      
      Serial.println(" was down");      
    }  
  }

  if(!state.currentState && state.previousState)
  {
    if(now - state.downTime >= buttonDelayLong)
    {
      pressButton(10 + value, buttonDelay);
      if(debug>0)
      {
        Serial.print("key ");      
        Serial.print(value);      
        Serial.println(" was up. long");   
      }
    }
    else 
    {
      pressButton(value, buttonDelay);
      if(debug>0)
      {
        Serial.print("key ");      
        Serial.print(value);      
        Serial.println(" was up.");   
      }
    }
  }

  state.previousState = state.currentState;  
}

// нажатие кнопки через шину
void pressButton(int value, int delay)
{
  unsigned long now = millis();
  if(now < set0)
  {
    return;
  }

  if(value == 0)
  {
    setResistance(0);
  }
  else
  {
    setResistance(value);
    set0 = now + delay;
  }
}

// изменить сопротивление для резистивных кнопок
void setResistance(int percent) 
{
  if(percent>0)
  {
    pot.set(percent*2); 
  }
  else 
  {
    pot.set(99);
  }
}
