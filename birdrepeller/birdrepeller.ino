#include <Encoder.h>
#include <TM1637Display.h>
#include <EEPROM.h>

// Пины индикатора TM1637
#define DISP_CLK 7
#define DISP_DIO 6

// Пины энкодера
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_BUTTON 4

// Пины нагрузки
#define VALVE 10
#define FIRE 11
#define SIRENA 12

Encoder encoder(ENC_DT, ENC_CLK);
TM1637Display display(DISP_CLK, DISP_DIO);

long oldPosition  = -999; // Позиция энкодера
byte step_encoder = 2; // Коэффициент для вычисления Шага ЭНКОДЕРА 
unsigned long oldTime = 0; // Хранение стартового времени для вычислений между действиями(миллисекунд)

byte eeprom_delay_maximum = 0; //Адрес ПЗУ для большой задержки между выхлопами
byte eeprom_delay_minimum = 1; //Адрес ПЗУ для малой задержки для наполнения
byte eeprom_delay_sirena = 2; //Адрес ПЗУ для малой задержки для наполнения

byte var_delay_maximum = 0;   // Переменная для большой задержки между выхлопами
byte var_delay_minimum = 0;  // Переменная для малой задержки для наполнения
byte var_delay_sirena = 0; // Задержка для работы сирены
byte rezim = 1; // Режим для изменения: 1 - большой задержки(пауза между выхлопами), 2 - малой задержки (наполнения камеры), 3 - задержка сирены(сирена включена)


void setup() {
  pinMode(ENC_BUTTON, INPUT_PULLUP);
  display.setBrightness(0x0f);
  
  pinMode(VALVE, OUTPUT);
  digitalWrite(VALVE, LOW);

  pinMode(FIRE, OUTPUT);
  digitalWrite(FIRE, LOW);

  pinMode(SIRENA, OUTPUT);
  digitalWrite(SIRENA, LOW);

  var_delay_maximum = EEPROM.read(eeprom_delay_maximum);
  var_delay_minimum = EEPROM.read(eeprom_delay_minimum);
  var_delay_sirena  = EEPROM.read(eeprom_delay_sirena);

  var_delay_maximum = constrain(var_delay_maximum, 1, 60);
  var_delay_minimum = constrain(var_delay_minimum, 1, 15);
  var_delay_sirena = constrain(var_delay_sirena, 1, 15);

  delay(50);
  oldPosition = encoder.read()/step_encoder;

  oldTime = millis();

} //setup()

void loop() {
  long newPosition = encoder.read()/step_encoder; // Делим, чтобы шаг был адекватным
  bool buttonPressed = !digitalRead(ENC_BUTTON);
  unsigned long newTime = millis(); // текущее время

  /////   Обслуживание ЭНКОДЕРА   /////
  if (rezim == 1){
    if(newPosition != oldPosition){
      //Вращение УВЕЛИЧЕНИЕ
      if(newPosition > oldPosition){
        var_delay_maximum = var_delay_maximum + 1;
        if(var_delay_maximum >= 60){var_delay_maximum = 60;}
      }
      //Вращение УМЕНЬШЕНИЕ
      if(newPosition < oldPosition){
        var_delay_maximum = var_delay_maximum - 1;
        if(var_delay_maximum <= 1){var_delay_maximum = 1;}
      }
      oldPosition = newPosition;
    }
    showFormattedValueDisplayDefault(rezim, var_delay_maximum);
  }

  if (rezim == 2){
    if(newPosition != oldPosition){
      //Вращение УВЕЛИЧЕНИЕ
      if(newPosition > oldPosition){
        var_delay_minimum = var_delay_minimum + 1;
        if(var_delay_minimum >= 15){var_delay_minimum = 15;}
      }
      //Вращение УМЕНЬШЕНИЕ
      if(newPosition < oldPosition){
        var_delay_minimum = var_delay_minimum - 1;
        if(var_delay_minimum <= 1){var_delay_minimum = 1;}
      }
      oldPosition = newPosition;
    }
    showFormattedValueDisplayDefault(rezim, var_delay_minimum);
  }


  if (rezim == 3){
    if(newPosition != oldPosition){
      //Вращение УВЕЛИЧЕНИЕ
      if(newPosition > oldPosition){
        var_delay_sirena = var_delay_sirena + 1;
        if(var_delay_sirena >= 15){var_delay_sirena = 15;}
      }
      //Вращение УМЕНЬШЕНИЕ
      if(newPosition < oldPosition){
        var_delay_sirena = var_delay_sirena - 1;
        if(var_delay_sirena <= 1){var_delay_sirena = 1;}
      }
      oldPosition = newPosition;
    }
    showFormattedValueDisplayDefault(rezim, var_delay_sirena);
  }    

  /////     Обслуживание КНОПКИ   /////
  if(buttonPressed){
    delay(1000);
    buttonPressed = !digitalRead(ENC_BUTTON);
    if(buttonPressed){
      if(rezim == 1){
        showFormattedValueDisplaySafe(rezim, var_delay_maximum);
        EEPROM.update(eeprom_delay_maximum, var_delay_maximum);
        delay(1000);
      }
      if(rezim == 2){
        showFormattedValueDisplaySafe(rezim, var_delay_minimum);
        EEPROM.update(eeprom_delay_minimum, var_delay_minimum);
        delay(1000);
      }
      if(rezim == 3){
        showFormattedValueDisplaySafe(rezim, var_delay_sirena);
        EEPROM.update(eeprom_delay_sirena, var_delay_sirena);
        delay(1000);
      }      
    } else {
      if(rezim == 1){
        rezim = 2;
        delay(1000);
      }else if(rezim == 2){
        rezim = 3;
        delay(1000);
      }else if(rezim == 3){
        rezim = 1;
        delay(1000);
      }
    } //else
  } //if(buttonPressed)

  /////   Логика наполнеия газом и искра   /////
  if((newTime - oldTime) > (var_delay_maximum * 60000)){
    digitalWrite(VALVE, HIGH);
    delay(var_delay_minimum * 1000);
    digitalWrite(VALVE, LOW);

    digitalWrite(SIRENA, HIGH);
    delay(var_delay_sirena * 1000);
    digitalWrite(SIRENA, LOW);

    digitalWrite(FIRE, HIGH);
    delay(1000);
    digitalWrite(FIRE, LOW);

    oldTime = millis();
  }

} //loop()



///// Функция вывода на дисплей в обычном режиме   /////
void showFormattedValueDisplayDefault(byte rezim, int var) {
  uint8_t data[4];

  if(rezim == 1){
    if (var < 10) {
      // Режим "1--x"
      data[0] = display.encodeDigit(1);
      data[1] = 0x40; // '-'
      data[2] = 0x40; // '-'
      data[3] = display.encodeDigit(var);
    } else {
      // Режим "1-xx"
      int tens = var / 10;
      int units = var % 10;

      data[0] = display.encodeDigit(1);
      data[1] = 0x40; // '-'
      data[2] = display.encodeDigit(tens);
      data[3] = display.encodeDigit(units);
    }
  }
  if(rezim == 2){
    if (var < 10) {
       // Режим "2--x"
      data[0] = display.encodeDigit(2);
      data[1] = 0x40; // '-'
      data[2] = 0x40; // '-'
      data[3] = display.encodeDigit(var);
    } else {
       // Режим "2-xx"
      int tens = var / 10;
      int units = var % 10;

        data[0] = display.encodeDigit(2);
        data[1] = 0x40; // '-'
        data[2] = display.encodeDigit(tens);
        data[3] = display.encodeDigit(units);
    }
  }
  if(rezim == 3){
    if (var < 10) {
       // Режим "3--x"
      data[0] = display.encodeDigit(3);
      data[1] = 0x40; // '-'
      data[2] = 0x40; // '-'
      data[3] = display.encodeDigit(var);
    } else {
       // Режим "3-xx"
      int tens = var / 10;
      int units = var % 10;

        data[0] = display.encodeDigit(3);
        data[1] = 0x40; // '-'
        data[2] = display.encodeDigit(tens);
        data[3] = display.encodeDigit(units);
    }
  }  
display.setSegments(data);
} 

///// Функция вывода на дисплей в режиме СОХРАНЕНИЯ  /////
void showFormattedValueDisplaySafe(byte rezim, int var) {
  uint8_t data[4];

  if(rezim == 1){
    if (var < 10) {
      // Режим "1HHx"
      data[0] = display.encodeDigit(1);
      data[1] = 0x76; // это символ 'H'
      data[2] = 0x76; // это символ 'H'
      data[3] = display.encodeDigit(var);
    } else {
      // Режим "1Hxx"
      int tens = var / 10;
      int units = var % 10;

      data[0] = display.encodeDigit(1);
      data[1] = 0x76; // это символ 'H'
      data[2] = display.encodeDigit(tens);
      data[3] = display.encodeDigit(units);
    }
  }
  if(rezim == 2){
    if (var < 10) {
       // Режим "2HHx"
      data[0] = display.encodeDigit(2);
      data[1] = 0x76; // это символ 'H'
      data[2] = 0x76; // это символ 'H'
      data[3] = display.encodeDigit(var);
    } else {
       // Режим "2Hxx"
      int tens = var / 10;
      int units = var % 10;

        data[0] = display.encodeDigit(2);
        data[1] = 0x76; // это символ 'H'
        data[2] = display.encodeDigit(tens);
        data[3] = display.encodeDigit(units);
    }
  }
  if(rezim == 3){
    if (var < 10) {
       // Режим "3HHx"
      data[0] = display.encodeDigit(3);
      data[1] = 0x76; // это символ 'H'
      data[2] = 0x76; // это символ 'H'
      data[3] = display.encodeDigit(var);
    } else {
       // Режим "3Hxx"
      int tens = var / 10;
      int units = var % 10;

        data[0] = display.encodeDigit(3);
        data[1] = 0x76; // это символ 'H'
        data[2] = display.encodeDigit(tens);
        data[3] = display.encodeDigit(units);
    }
  }  
display.setSegments(data);
} 