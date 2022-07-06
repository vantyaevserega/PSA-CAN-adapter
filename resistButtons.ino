#include <DigiPotX9Cxxx.h>
#include <mcp2515.h>
#include <CanFrameLog.h>
#include <EmulateButtonsPSA.h>
#include <SPI.h>
#include <SoftwareSerial.h>

// используемые пины
#define XC_CS 2
#define XC_INC 3
#define XC_UD 4
#define HC_TXD 5
#define HC_RXD 6
#define REVERSEPIN 8
#define ILLUMINATEPIN 9
#define MCP2515_CS 10

#define DOWN_BUTTON 0
#define UP_BUTTON 1
#define MUTE_BUTTON 2
#define FORWARD_BUTTON 3
#define PREV_BUTTON 4
#define NEXT_BUTTON 5

struct buttonProgram {
  bool currentState;
  bool previousState;
  unsigned long downTime;
  char value;
};

struct button {
  bool currentState;
  bool previousState;
  bool isLong;
  unsigned long downTime;
  uint8_t value;
  uint8_t longValue;
  char name;
  char longName;
};

// обработка кнопок
#define buttonDelay 150
#define buttonDelayLong 500
#define buttonDelayVeryLong 1000
#define buttonConfigDelay 1500

// связь по bluetooth
SoftwareSerial btSerial(HC_TXD, HC_RXD);

// шина для обшения с авто
MCP2515 mcp2515(MCP2515_CS);

// сопротивления для кнопок
DigiPot pot(XC_INC, XC_UD, XC_CS);

// логирование пакетов
CanFrameLog lg(&btSerial);

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
bool hiddenReverse;
bool forceReverse;
int pressTime = 0;
bool prevIlluminate;
byte debug = 0;
int prevButton;

unsigned long setProgram0;
unsigned long set0;
button resistButtons[6];
buttonProgram programButtons[2];
void setup() {
  // инициализация шины bluetooth
  btSerial.begin(9600);

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

  programButtons[0].value = 'O';
  programButtons[1].value = 'E';

  resistButtons[DOWN_BUTTON].value = 1;
  resistButtons[DOWN_BUTTON].longValue = 9;
  resistButtons[DOWN_BUTTON].name = 'v';
  resistButtons[DOWN_BUTTON].longName = 'q';
  resistButtons[UP_BUTTON].value = 2;
  resistButtons[UP_BUTTON].longValue = 12;
  resistButtons[UP_BUTTON].name = 'V';
  resistButtons[UP_BUTTON].longName = 'Q';
  resistButtons[MUTE_BUTTON].value = 3;
  resistButtons[MUTE_BUTTON].longValue = 15;
  resistButtons[MUTE_BUTTON].name = 'm';
  resistButtons[MUTE_BUTTON].longName = 'w';
  resistButtons[FORWARD_BUTTON].value = 4;
  resistButtons[FORWARD_BUTTON].longValue = 18;
  resistButtons[FORWARD_BUTTON].name = 'n';
  resistButtons[FORWARD_BUTTON].longName = 'W';
  resistButtons[PREV_BUTTON].value = 5;
  resistButtons[PREV_BUTTON].longValue = 5;
  resistButtons[PREV_BUTTON].name = 't';
  resistButtons[PREV_BUTTON].longName = 'e';
  resistButtons[NEXT_BUTTON].value = 6;
  resistButtons[NEXT_BUTTON].longValue = 6;
  resistButtons[NEXT_BUTTON].name = 'T';
  resistButtons[NEXT_BUTTON].longName = 'E';

  // инициализация эмуляции кнопок
  buttons.SetState(true);

  btSerial.println("Start work");
  Serial.println("Start work");
}

void loop() {
  // начало обработки цикла
  unsigned long now = millis();

  char data;
  bool isRead = false;
  if (btSerial.available() > 0) {
    data = btSerial.read();
    isRead = true;
  } else {
    if (Serial.available() > 0) {
      data = Serial.read();
      isRead = true;
    }
  }

  // обработка серийного порта, для конфигурации
  if (isRead) {
    if (data > '0' && data <= '9') {
      buttons.SetButton(data - '0');
      if (debug > 0) {
        Serial.println("Display button pressed");
        btSerial.println("Display button pressed");
      }
    } else if (data == 'S') {
      debug = 0;
    } else if (data == 'D') {
      debug = 1;
    } else if (data == 'F') {
      debug = 2;
    } else if (data == 's') {
      if (debug > 0) {
        Serial.println("Configuration started");
        btSerial.println("Configuration started");
      }

      isConfig = true;
    } else if (data == 'f') {
      if (debug > 0) {
        Serial.println("Configuration finished");
        btSerial.println("Configuration finished");
      }

      isConfig = false;
      currentButton = 0;
    } else if (data == 'r') {
      forceReverse = false;
      digitalWrite(REVERSEPIN, HIGH);
      if (debug > 0) {
        Serial.println("reverse off");
        btSerial.println("reverse off");
      }
    } else if (data == 'R') {
      forceReverse = true;
      digitalWrite(REVERSEPIN, LOW);
      if (debug > 0) {
        Serial.println("reverse on");
        btSerial.println("reverse on");
      }
    } else if (isConfig && data == 'i') {
      digitalWrite(ILLUMINATEPIN, HIGH);
      if (debug > 0) {
        Serial.println("illuminate off");
        btSerial.println("illuminate off");
      }
    } else if (isConfig && data == 'I') {
      digitalWrite(ILLUMINATEPIN, LOW);
      if (debug > 0) {
        Serial.println("illuminate on");
        btSerial.println("illuminate on");
      }
    } else if (data == 'l') {
      islogenabled = false;
      if (debug > 0) {
        Serial.println("looging on");
        btSerial.println("looging on");
      }
    } else if (data == 'L') {
      islogenabled = true;
      if (debug > 0) {
        Serial.println("looging on");
        btSerial.println("looging on");
      }
    } else {
      if (isConfig) {
        for (int ind = 0; ind < 6; ++ind) {
          if (data == resistButtons[ind].name) {
            confPressedTime = now;
            currentButton = resistButtons[ind].value;
            pressTime = buttonConfigDelay;
          } else if (data == resistButtons[ind].longName) {
            confPressedTime = now;
            currentButton = resistButtons[ind].longValue;
            pressTime = buttonConfigDelay;
          }
        }
      }
    }
  }

  if (isConfig) {
    if (currentButton > 0 && now - confPressedTime >= pressTime) {
      if (debug > 0) {
        Serial.print("config key ");
        Serial.print(currentButton);
        Serial.println(" was up");

        btSerial.print("config key ");
        btSerial.print(currentButton);
        btSerial.println(" was up");
      }

      currentButton = 0;
      prevButton = 0;
    }

    pressButton(currentButton);
    if (debug > 0 && currentButton > 0 && prevButton != currentButton) {
      prevButton = currentButton;
      Serial.print("config key ");
      Serial.print(currentButton);
      Serial.println(" was down");

      btSerial.print("config key ");
      btSerial.print(currentButton);
      btSerial.println(" was down");
    }
  }

  // обработка шины автомобиля
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK && !isConfig) {
    if (debug > 1 || islogenabled) {
      lg.logMessage(&canMsg);
    }

    // блок конпок, основной
    if (canMsg.can_id == 0x21F) {
      for (int ind = 0; ind < 6; ++ind) {
        resistButtons[ind].currentState = false;
      }

      if (canMsg.data[1] > prevScroll || prevScroll - canMsg.data[1] > 200)  // scroll up
      {
        resistButtons[NEXT_BUTTON].currentState = true;
      }

      if (canMsg.data[1] < prevScroll || prevScroll - canMsg.data[1] < -200)  // scroll down
      {
        resistButtons[PREV_BUTTON].currentState = true;
      }

      prevScroll = canMsg.data[1];
      if (canMsg.data[0] == 4)  // volume down
      {
        resistButtons[DOWN_BUTTON].currentState = true;
      }

      if (canMsg.data[0] == 8)  // volume up
      {
        resistButtons[UP_BUTTON].currentState = true;
      }

      if (canMsg.data[0] == 12)  // mute
      {
        resistButtons[MUTE_BUTTON].currentState = true;
      }

      if (canMsg.data[0] == 128)  // forward
      {
        resistButtons[FORWARD_BUTTON].currentState = true;
      }
    } else
      // задний ход
      if (canMsg.can_id == 0xF6 && canMsg.can_dlc == 8) {
        if ((canMsg.data[7] & 0b10000000 && !hiddenReverse) || forceReverse) {
          if (!prevReverse) {
            prevReverse = true;
            digitalWrite(REVERSEPIN, LOW);
          }
        } else {
          if (prevReverse) {
            prevReverse = false;
            digitalWrite(REVERSEPIN, HIGH);
          }
        }
      } else
        // яркость
        if (canMsg.can_id == 0x128 && canMsg.can_dlc > 4) {
          if (canMsg.data[4] & 0b00010000) {
            if (!prevIlluminate) {
              prevIlluminate = true;
              digitalWrite(ILLUMINATEPIN, LOW);
            }
          } else {
            if (prevIlluminate) {
              prevIlluminate = false;
              digitalWrite(ILLUMINATEPIN, HIGH);
            }
          }
        } else
        // кнопки долгие
        if(canMsg.can_id == 0xA2 && canMsg.can_dlc == 5)
        {  
          for (int ind = 0; ind < 2; ++ind) {
            programButtons[ind].currentState = false;
          }
          // data[0] - крутилка
          /* // mode menu
          if(canMsg.data[1]  == 4)
          {     
          } else
          if(canMsg.data[1] == 8)
          {     
          } else*/
          if(canMsg.data[1]  == 16) // esc
          {
            programButtons[1].currentState = true;
          } else 
          if(canMsg.data[1] == 32) // ok
          {   
            programButtons[0].currentState = true;
          }
        }
  } 

  if (!isConfig) {
    for (int ind = 0; ind < 2; ++ind) {
      programButtons[ind] = handleProgramButtonState(now, programButtons[ind]);
    }

    for (int ind = 0; ind < 6; ++ind) {
      resistButtons[ind] = handleButtonState(now, resistButtons[ind]);
    }

    if (setProgram0 > 0 && now >= setProgram0) {
      for (int ind = 0; ind < 2; ++ind) {
        programButtons[ind].currentState = programButtons[ind].previousState = false;
      }

      setProgram0 = 0;
      if (debug > 0) {
        Serial.println("key program was up by delay");
        btSerial.println("key program was up by delay");
      }
    }

    if (set0 > 0 && now >= set0) {
      for (int ind = 0; ind < 6; ++ind) {
        resistButtons[ind].currentState = resistButtons[ind].previousState = false;
      }

      set0 = 0;
      setResistance(0);
      if (debug > 0) {
        Serial.println("key was up by delay");
        btSerial.println("key was up by delay");
      }
    }
  }

  buttons.DoWork(now);
}

// нажатие кнопки из настроек
void pressButton(int value) {
  if (value == 0) {
    setResistance(0);
  } else {
    setResistance(value);
  }
}
// обработка состояния кнопки
buttonProgram handleProgramButtonState(unsigned long now, buttonProgram state) {
  if (state.currentState && !state.previousState) {
    state.downTime = now;
    if (debug > 0) {
      Serial.print("key ");
      Serial.print(state.value);
      Serial.println(" was down");

      btSerial.print("key ");
      btSerial.print(state.value);
      btSerial.println(" was down");
    }

    state.previousState = true;
  }

  if (!state.currentState && state.previousState) {
    if (now - state.downTime >= buttonDelayVeryLong) {
      if(state.value == 'O')
      {
        hiddenReverse = !hiddenReverse;
      }
      else
      {
        forceReverse = !forceReverse;
      }

      if (debug > 0) {
        Serial.print("key ");
        Serial.print(state.value);
        Serial.println(" was up. very long");

        btSerial.print("key ");
        btSerial.print(state.value);
        btSerial.println(" was up. very long");
      }
    }

    state.previousState = false;
  }

  return state;
}

// обработка состояния кнопки
button handleButtonState(unsigned long now, button state) {
  if (state.currentState && !state.previousState) {
    state.downTime = now;
    if (debug > 0) {
      Serial.print("key ");
      Serial.print(state.value);
      Serial.println(" was down");

      btSerial.print("key ");
      btSerial.print(state.value);
      btSerial.println(" was down");
    }

    state.previousState = true;
  }

  if (!state.currentState && state.previousState) {
    if (now - state.downTime >= buttonDelayLong) {
      pressButton(state.longValue, buttonDelay);
      if (debug > 0) {
        Serial.print("key ");
        Serial.print(state.value);
        Serial.println(" was up. long");

        btSerial.print("key ");
        btSerial.print(state.value);
        btSerial.println(" was up. long");
      }
    } else {
      pressButton(state.value, buttonDelay);
      if (debug > 0) {
        Serial.print("key ");
        Serial.print(state.value);
        Serial.println(" was up.");

        btSerial.print("key ");
        btSerial.print(state.value);
        btSerial.println(" was up.");
      }
    }

    state.previousState = false;
  }

  return state;
}

// нажатие кнопки через шину
void pressButton(int value, int delay) {
  unsigned long now = millis();
  if (now < set0) {
    return;
  }

  if (value == 0) {
    setResistance(0);
  } else {
    setResistance(value);
    set0 = now + delay;
  }
}

// изменить сопротивление для резистивных кнопок
void setResistance(int percent) {
  if (percent > 0) {
    pot.set(percent * 2);
  } else {
    pot.set(99);
  }
}