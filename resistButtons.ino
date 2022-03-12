#include <DigiPotX9Cxxx.h>
#include <mcp2515.h>
#include <SPI.h>

struct DashboardState
{
  byte MotorState = 0; // 0 заглушен, 1 запуск, 2 работает, 3 останавливается
  bool GeneratorState = 0; // 1 работает
  byte IgnitionState = 0; // 0 выключено, 1 включено, 2 запуск двигателя
  int CoolantTemperature = 0; // -40 .. +215
  unsigned long Odometer = 0; // в км * 10
  int ExternalTemperature = 0; // -40 .. 87.5
  bool IsReverseGear = false;
  bool PassengerAirbagState = false;
  byte BlinkersState = 0; // 1 правый, 2 левый, 3 оба
  bool WipingState = false; // работают дворники
  byte WheelPosition = 0; // 1 right, 2 left 
  byte DoorsState = 0; // FL FR RL RR  T H RW FC двери, багажник, капот, заднее стекло, крышка бензобака
  bool ThreeDoors = false;
};

struct DashboardState State;
struct can_frame canMsg;
MCP2515 mcp2515(10);
const int buttonDelay = 150;
const int startR  = 3; 
const int stepR = 4;
const int CS_XC = 2;
const int INC_XC = 3;
const int UD_XC = 4;
DigiPot pot(INC_XC,UD_XC,CS_XC);
struct can_frame s10C;
unsigned long prevChangeButtonState;
unsigned long confPressedTime;
bool isConfig = false;
const int REVERSEPIN = 8;
const int ILLUMINATEPIN = 9;
int vv = 0;
int prevButton = 0;
int currentButton = 0;
int prevScroll = 0;
int prevScroll2 = 0;
bool prevReverse;
int pressTime = 0;
bool prevIlluminate;
int prevResist = 0;
byte debug = 0;
unsigned long sT = 0;
void setup() 
{
  s10C.can_id = 0x10C;
  s10C.can_dlc = 7;
  s10C.data[0] = 0x00;
  s10C.data[1] = 0x00;
  s10C.data[2] = 0x10;
  s10C.data[3] = 0x00;
  s10C.data[4] = 0x00;
  s10C.data[5] = 0x00;
  s10C.data[6] = 0x00; //10C — window control; https://www.drive2.ru/l/469794106709639308/

  Serial.begin(115200);
  Serial.println("Start work");
  mcp2515.reset();

  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();

  // инициализация резистивных выводов  
  pot.reset();
  pot.set(99);
  debug = 0
;  pinMode(REVERSEPIN, OUTPUT);
  pinMode(ILLUMINATEPIN, OUTPUT);
  digitalWrite(REVERSEPIN, LOW); 
  digitalWrite(ILLUMINATEPIN, LOW);
}

void loop() 
{
  // обработка серийного порта, для конфигурации
  if ((millis() - sT > 5000 || isConfig) && Serial.available() > 0) 
  {
    int data = Serial.parseInt();
    if(data == 102)
    {
      debug = 0;
    }
    else
    if(data == 103)
    {
      debug = 1;
    }
    else
    if(data == 104)
    {
      debug = 2;
    }
    else
    if(data == 300)
    {
      if(debug>0)
        Serial.println("Send windows up command");
      mcp2515.sendMessage(&s10C); //закрываем окна
      mcp2515.sendMessage(&s10C);
    }
    else
    if(data == 100)
    {
      if(debug>0)
        Serial.println("Configuration started");
      isConfig = true;
    }
    else
    if(data == 200)
    {
      if(debug>0)
        Serial.println("Configuration finished");
      isConfig = false;
      currentButton = 0;
    }
    else
    if(isConfig && data == 16)
    {
      digitalWrite(REVERSEPIN, HIGH);
      if(debug>0)
        Serial.println("reverse high");      
    }    
    else
    if(isConfig && data == 17)
    {
      digitalWrite(REVERSEPIN, LOW);
      if(debug>0)
        Serial.println("reverse low");      
    }     
    else
    if(isConfig && data == 18)
    {
      digitalWrite(ILLUMINATEPIN, HIGH);
      if(debug>0)
        Serial.println("ILUMINATEPIN high");      
    }    
    else
    if(isConfig && data == 19)
    {
      digitalWrite(ILLUMINATEPIN, LOW);
      if(debug>0)
        Serial.println("ILUMINATEPIN low");       
    }    
    else    
    if(data > 0 && data <= 99)
    {
      confPressedTime = millis();
      currentButton = data;
      currentButton = data > 40 ? data - 40 : data;
      pressTime = data > 40 ? 1 : 1500;
    }    

    sT = millis();
  }

  if(currentButton > 0 && millis() - confPressedTime > pressTime)
  {
      currentButton = 0;      
  }

  // обработка шины автомобиля
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK && !isConfig) 
  {    
    if(debug>1)
      log(canMsg);

    // кнопки (?)
    if(canMsg.can_id == 0xED)
    {
      if(canMsg.data[0] > 0)   
      {
        Serial.print("ED "); 
        Serial.println(canMsg.data[0]);
      }
    }
    else
    // состояние дверей
    if(canMsg.can_id == 0x220)
    {  
      State.DoorsState = canMsg.data[0];
      State.ThreeDoors = canMsg.data[1] >> 7 > 0;
    }
    else
    // сообщение
    if(canMsg.can_id == 0x18)
    {  
      State.PassengerAirbagState = (canMsg.data[0] >> 7) == 1;
    }
    else
    // сообщение
    if(canMsg.can_id == 0x1A1)
    {  
    }
    else
    // бортовые компьютеры
    if(canMsg.can_id == 0x221 || canMsg.can_id == 0x261 || canMsg.can_id == 0x2A1)
    {
      //bool trip_btn_pressed = canMsg.can_id == 0x221 && (canMsg.data[0] & 0x0F) == 0x08;   
    }
    else
    // блок конпок, основной
    if(canMsg.can_id == 0x21F)// && canMsg.can_dlc == 3)
    {
      //Serial.println(canMsg.data[0]);
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
        Serial.println(canMsg.data[0]);
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
      if(debug>0)
      {            
        if(debug == 1)
          log(canMsg);
        Serial.print("Btn "); Serial.println(currentButton);  
        Serial.print("Scr1 "); Serial.println(prevScroll); 
      }
    }
    else
    // второй набор кнопок
    if(canMsg.can_id == 0xA2 && canMsg.can_dlc == 5)
    {
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

      if(canMsg.data[1]  == 4)
      {
        currentButton = 10;      
      }

      if(canMsg.data[0] == 8)
      {
        currentButton = 11;      
      }

      if(canMsg.data[1]  == 16)
      {
        currentButton = 12;      
      }

      if(canMsg.data[0] == 32)
      {
        currentButton = 13;      
      }

      if(debug>0)
      {
        if(debug == 1)
          log(canMsg);
        Serial.print("Btn2 "); Serial.println(currentButton);  
        Serial.print("Scr2 "); Serial.println(prevScroll2); 
      }
    }    
    else
    // задний ход все верно
    if(canMsg.can_id == 0xF6 && canMsg.can_dlc == 8)
    {   
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

      if(debug>0) 
      { 
        if(debug == 1)          
          log(canMsg);    
        Serial.print("Reverse "); Serial.println(prevReverse);
      }
    }    
    else
    // время, для обработки подсветки, тут хранится время от начала работы дисплея, параметр бесполезен, чуть более, чем полностью
    if(canMsg.can_id == 0x3f6 && canMsg.can_dlc == 7 && false)
    {
      if(debug == 1)
        log(canMsg);
      int day = ((canMsg.data[3] & 0b00001111) << 4) + (canMsg.data[4] >> 4) + 1;
      int month = canMsg.data[4] & 0b00001111 + 1;
      int year = canMsg.data[5] + 2000 + 1;
      float coef = (180 - (month-1)*30+day)/180.0; 
      coef = coef < 0 ? -coef : coef;
      long tLong = ((canMsg.data[0] << 12) + (canMsg.data[1] << 4) + (canMsg.data[2] >> 4));
      float currentTime = tLong/3600.0;
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

      if(debug>0)
      { 
        Serial.print("Illuminate "); Serial.print(prevIlluminate); Serial.print("Date "); Serial.print(day); Serial.print("."); Serial.print(month); Serial.print("."); Serial.print(year); Serial.print(" Time "); Serial.println(currentTime);  
      }
    }
 }
  
  // если необходимо - нажимаем кнопку
  if(currentButton != prevButton)
  {
    if( millis() - prevChangeButtonState >= buttonDelay)
    {
      pressButton(currentButton);
      if(currentButton > 0)
      {
        prevChangeButtonState = millis();
      }
      else
      {
        prevChangeButtonState = 0;
      }
    
      prevButton = currentButton;
    }
  }
  else
  {
    // долгое нажатие
    if(currentButton > 0)
    {
      prevChangeButtonState = millis();
    }
    else
    {
      prevChangeButtonState = 0;
    }
  }
}

// нажатие кнопки
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

// изменить сопротивление для резистивных кнопок
void setResistance(int percent) 
{ 
     //pot.set(99);
      if(percent>0)pot.set(percent*2); else pot.set(99);
}
