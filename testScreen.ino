#include <mcp2515.h>
#include <CanFrameLog.h>
#include <EmulateRadioPSA.h>
#include <EmulateBSIPSA.h>
#include <EmulateButtonsPSA.h>
#include <SPI.h>

#define PIN_VRX  A0
#define PIN_VRY  A1
#define PIN_ok   2
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

CanFrameLog lg;
struct can_frame canMsg;
MCP2515 mcp2515(10);
EmulateRadioPSA radio(&mcp2515, &lg);
EmulateBSIPSA car(&mcp2515, &lg);
EmulateButtonsPSA buttons(&mcp2515, &lg);

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
  pinMode(PIN_ok, INPUT_PULLUP);
  pinMode(PIN_esc, INPUT);
  pinMode(PIN_menu, INPUT);
  pinMode(PIN_mode, INPUT);
  pinMode(PIN_dark, INPUT);

  Serial.begin(115200);
  Serial.println("Start work");
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  
  mcp2515.setNormalMode();
  radio.SetState(true);
  buttons.SetState(true);
  radio.Init();
}

void loop() 
{
  unsigned long now = millis();
  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);
  EmulateButtonsPSA::PSA_BUTTON currentButton = EmulateButtonsPSA::NONE;
  if(x < nullValue - 100)
  { currentButton = EmulateButtonsPSA::LEFT;}
  else if( x > nullValue + 100)
  { currentButton = EmulateButtonsPSA::RIGHT; }
  else if( y < nullValue - 100)
  { currentButton = EmulateButtonsPSA::DOWN; }
  else if( y > nullValue + 100)
  { currentButton = EmulateButtonsPSA::UP; }
  
  int currentOk = debounce(PIN_ok, lastOk); 
  int currentEsc = debounce(PIN_esc, lastEsc); 
  int currentMode = debounce(PIN_mode, lastMode); 
  int currentDark = debounce(PIN_dark, lastDark); 
  int currentMenu = debounce(PIN_menu, lastMenu);   
  if (lastOk == LOW && currentOk == HIGH)
    currentButton = EmulateButtonsPSA::OK;
  if (lastEsc == LOW && currentEsc == HIGH)
    currentButton = EmulateButtonsPSA::ESCAPE;
  if (lastMode == LOW && currentMode == HIGH)
    currentButton = EmulateButtonsPSA::MODE; 
  if (lastDark == LOW && currentDark == HIGH)
    currentButton = EmulateButtonsPSA::DARK; 
  if (lastMenu == LOW && currentMenu == HIGH)
    currentButton = EmulateButtonsPSA::MENU; 
       
  lastOk = currentOk; 
  lastEsc = currentEsc; 
  lastMode = currentMode;     
  lastDark = currentDark; 
  lastMenu = currentMenu; 
  
  // обработка серийного порта, для конфигурации
  if (Serial.available() > 0) 
  {
    char readed = (char)Serial.read();
    Serial.println((int)readed);
    if(readed > 48 && readed <= 57)
    {
      car.StopMessages(now);
      currentButton = readed - '0';
    }
    else if(readed == 65)
    {
      car.StartMessages(now);
    }
    else if(readed == 'B')
    { 
      car.SetState(true);
    }
    else if(readed == 'C')
    { 
      car.SetState(false);
    }    
  }
  
  buttons.SetButton(currentButton);
  radio.DoWork(now);
  car.DoWork(now);
  buttons.DoWork(now);
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
  {     
    lg.logMessage(&canMsg);
  }  
}
