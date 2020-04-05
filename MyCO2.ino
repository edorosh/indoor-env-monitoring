/**
  Based on the Project "Home meteo station" from AlexGyver
  Author: Evgeny Doros
  License: MIT

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is furnished
  to do so, subject to the following conditions:

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  Original Project Web Page(RU): https://alexgyver.ru/meteoclock/
  GitHub: https://github.com/AlexGyver/MeteoClock
  Author: AlexGyver Technologies, 2018
  http://AlexGyver.ru/
*/

// ------------------------- SETTINGS --------------------
#define DEBUG 0             // вывод лог инициализации датчиков при запуске через порт
#define DISPLAY_ADDR 0x27   // адрес платы дисплея: 0x27 или 0x3f. Если дисплей не работает - смени адрес! На самом дисплее адрес не указан

// адрес BME280 жёстко задан в файле библиотеки Adafruit_BME280.h
// стоковый адрес был 0x77, у китайского модуля адрес 0x76.
// Так что если юзаете НЕ библиотеку из архива - не забудьте поменять

// timers
#define SENS_TIMER 5500      // время обновления показаний сенсоров на экране, миллисекунд. Период обновления MHZ-19B 5сек
#define CHECK_BRIGHT_TIMER 2000
#define BLINK_RED_LED_TIMER 500
#define MELODY_REPEAT_TIMEOUT 5000

// brightness
#define BRIGHT_THRESHOLD 150  // величина сигнала, ниже которой яркость переключится на минимум (0-1023)

#define LCD_BRIGHT_MAX 150    // макс яркость подсветки дисплея (0 - 255)
#define LCD_BRIGHT_MIN 10     // мин яркость подсветки дисплея (0 - 255)

#define LED_BRIGHT_RED_MAX 60    // макс яркость светодиода СО2 (0 - 255)
#define LED_BRIGHT_RED_MIN 1     // мин яркость светодиода СО2 (0 - 255)

#define LED_BRIGHT_GREEN_MAX 10    // макс яркость светодиода СО2 (0 - 255)
#define LED_BRIGHT_GREEN_MIN 1     // мин яркость светодиода СО2 (0 - 255)

#define LED_BRIGHT_YELLOW_MAX 100    // макс яркость светодиода СО2 (0 - 255)
#define LED_BRIGHT_YELLOW_MIN 10     // мин яркость светодиода СО2 (0 - 255)

// pins
#define BACKLIGHT_PIN 10
#define PHOTO_RES_PIN A3

#define MHZ_RX 2
#define MHZ_TX 3

#define LED_G 6
#define LED_Y 5
#define LED_R 9

#define BTN_PIN 4

#define BUZZER_PIN 11
#define BTN_MUTE_PIN 12

#define PRESSSURE_DIVIDER 100.0F

// Счет начинается с 1
#define MAX_MODE_NUM 3

// libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MHZ19_uart.h>
#include <GyverTimer.h>
#include <GyverButton.h>
#include <rtttl.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

GButton button(BTN_PIN);
GButton muteButton(BTN_MUTE_PIN);

LiquidCrystal_I2C lcd(DISPLAY_ADDR, 16, 2);
MHZ19_uart mhz19;
Adafruit_BME280 bme;

GTimer sensorsTimer(MS, SENS_TIMER);
GTimer drawSensorsTimer(MS, SENS_TIMER);
GTimer brightTimer(MS, CHECK_BRIGHT_TIMER);
GTimer redLedBlink(MS, BLINK_RED_LED_TIMER);
GTimer melodyRepeatTimeout(MS);

boolean playMelody = false;

byte LED_OFF = 0;

byte LED_BRIGHT_RED = LED_BRIGHT_RED_MIN;
byte LED_BRIGHT_GREEN = LED_BRIGHT_GREEN_MIN;
byte LED_BRIGHT_YELLOW = LED_BRIGHT_YELLOW_MIN;
byte LCD_BRIGHT = LCD_BRIGHT_MIN;

// цифры //  (с)НР
byte row2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};   // в т.ч. для двустрочной цифры 4 (с)НР
byte row3[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};   // в т.ч. для двустрочной цифры 0, для четырехстрочных цифр 2, 3, 4, 5, 6, 8, 9 (с)НР

uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};   // для двустрочных 7, 0   // для четырехстрочных 2, 3, 4, 5, 6, 8, 9
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};  // для двустрочных 2, 3, 5, 6, 8, 9
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};  // для двустрочных 2, 3, 5, 6, 8, 9
uint8_t LM2[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};  // для двустрочной 4

/*
  0 CO2
  1 Давление
  2 Температура
*/
byte mode = 0;

// переменные для вывода
int dispCO2;
int dispPres;
float dispTemp;

// Мелодия
const char song_P[] PROGMEM = "TakeOnMe:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5";
unsigned long printedTime = -1;
unsigned long printedPeriod = 500;

ProgmemPlayer player(BUZZER_PIN);

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);

#if (DEBUG == 1)
  printDebug();
#else
  // Говнокод
  bme.begin(BME280_ADDRESS_ALTERNATE, &Wire);
#endif

 bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_NONE, // humidity
                  Adafruit_BME280::FILTER_OFF   );

  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  
  setLED(0);
  redLedBlink.stop();

  player.setSong(song_P);

  analogWrite(BACKLIGHT_PIN, LCD_BRIGHT_MIN);

  createChars();
  readSensors();
  handleLcd();
}

void loop() {
  // тут ловим нажатия на кнопку и переключаем режимы
  modesTick();

  // Играем мелодию если время пришло
  playAlarmIfNeeded();

  // читаем показания датчиков
  if (sensorsTimer.isReady()) {
    readSensors();
  }

  // яркость
  if (brightTimer.isReady()) {
    checkBrightness(); 
    applyBrightness();
  }
  
  // обработка значения сенсоров
  if (drawSensorsTimer.isReady()) {
    handleLcd();
    handleLeds();
    handleAlarm();
  }

  // Мигание диода
  if (redLedBlink.isReady()) {
    blinkLedRed();
  }
}

void createChars()  {
  lcd.createChar(0, row2);
  lcd.createChar(1, UB);
  lcd.createChar(2, row3);
  lcd.createChar(3, UMB);
  lcd.createChar(4, LMB);
  lcd.createChar(5, LM2);
}

void readSensors() {
  dispCO2 = mhz19.getPPM();
  bme.takeForcedMeasurement();

  dispTemp = bme.readTemperature();
  dispPres = (float)(bme.readPressure() / PRESSSURE_DIVIDER);

  #if (DEBUG == 1)
  Serial.print(dispCO2);
  Serial.print(" ppm");
  Serial.println();

  Serial.print(dispTemp);
  Serial.print(" grad");
  Serial.println();

  Serial.print(dispPres);
  Serial.print(" hPa");
  Serial.println();
  #endif
}

void handleLeds() {
 if (dispCO2 < 1200) {
    redLedBlink.stop();
  }

  if (dispCO2 < 800) {
    setLED(2);
  } else if (dispCO2 < 1200) {
    setLED(3);
  } else if (dispCO2 >= 1200) {
    setLED(0);
    redLedBlink.reset();
    redLedBlink.start();
  }
}

void handleAlarm() {
  playMelody = dispCO2 >= 1200;
}

void handleLcd() {  
  switch (mode) {
    case 0:
      drawPPM(dispCO2, 0, 0);
      
      #if (DEBUG == 1)
      Serial.print("Mode drawPPM");
      Serial.println();
      #endif
      break;
  
    case 1:
      drawTemp(dispTemp, 0, 0);

      #if (DEBUG == 1)
      Serial.print("Mode drawTemp");
      Serial.println();
      #endif
      break;
  
    case 2:
      drawPres(dispPres, 0, 0);
      
      #if (DEBUG == 1)
      Serial.print("Mode drawPres");
      Serial.println();
      #endif
      break;
    
    default:
      drawPPM(dispCO2, 0, 0);
      break;
  }
}

void modesTick() {
  button.tick();
  muteButton.tick();
  
  if (button.isClick()) {

    if ((mode + 1) >= MAX_MODE_NUM) {
      mode = 0;
    } else {
      mode++;
    }

    lcd.clear();

    #if (DEBUG == 1)
    Serial.print("Button Clicked. ");
    Serial.print("Current display mode");
    Serial.print(mode);
    Serial.println();
    #endif

    handleLcd();
  }

  if (muteButton.isHold()) {
    playMelody = false;
    
    #if (DEBUG == 1)
    Serial.print("Mute holded");
    Serial.println();
    #endif
  }
}

void printDebug() {
  boolean status = true;

  Serial.begin(9600);
    
  Serial.print(F("MHZ-19... "));
  mhz19.getStatus();    // первый запрос, в любом случае возвращает -1
  delay(500);
  
  if (mhz19.getStatus() == 0) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("ERROR"));
    status = false;
  }

  Serial.print(F("BME280... "));
  delay(50);
  if (bme.begin(BME280_ADDRESS_ALTERNATE, &Wire)) {
    lcd.print(F("OK"));
    Serial.println(F("OK"));
  } else {
    Serial.println(F("ERROR"));
    status = false;
  }

  if (status) {
    Serial.println(F("All good"));
  } else {
    lcd.print(F("ERROR"));
    Serial.println(F("Check wires!"));
  }
}

void playAlarmIfNeeded() {
 static bool songReadyToPLayFlag = true;

  // Проигрываем мелодию
  if (playMelody && songReadyToPLayFlag) {
    songReadyToPLayFlag = player.pollSong();

    // Повторить следующий раз через х секунд
    if (!songReadyToPLayFlag) {
      melodyRepeatTimeout.setInterval(MELODY_REPEAT_TIMEOUT);
      melodyRepeatTimeout.start();
    }
  } else {
    player.silence();
  }

  if (melodyRepeatTimeout.isReady()) {
    melodyRepeatTimeout.stop();
    melodyRepeatTimeout.reset();
    songReadyToPLayFlag = true;
  }
}

void blinkLedRed() {
 static bool redLedOnFlag = false;
  redLedOnFlag = !redLedOnFlag;
  
  analogWrite(LED_R, redLedOnFlag ? LED_BRIGHT_RED : LED_OFF);
}
