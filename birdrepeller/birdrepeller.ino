#include <Encoder.h>
#include <TM1637Display.h>
#include <EEPROM.h>

// Пины индикатора TM1637
#define DISP_CLK 7
#define DISP_DIO 6

// Пины энкодера
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4

// Пины нагрузки
#define VALVE 10
#define FIRE 11
#define SIRENA 12

Encoder encoder(ENC_DT, ENC_CLK);
TM1637Display display(DISP_CLK, DISP_DIO);

byte eeprom_delay_maximum = 0; //Адрес ПЗУ для большой задержки между выхлопами
byte eeprom_delay_minimum = 1; //Адрес ПЗУ для малой задержки для наполнения
byte var_delay_maximum = 0;   // Переменная для большой задержки между выхлопами
byte var_delay_minimum = 0;  // Переменная для малой задержки для наполнения
byte rezim = 0; // Режим для изменения: 0 - большой задержки, 1 - малой задержки


void setup() {
  pinMode(ENC_SW, INPUT_PULLUP);
  display.setBrightness(0x0f);
  
  pinMode(VALVE, OUTPUT);
  digitalWrite(VALVE, LOW);

  pinMode(FIRE, OUTPUT);
  digitalWrite(FIRE, LOW);

  pinMode(SIRENA, OUTPUT);
  digitalWrite(SIRENA, LOW);

  var_delay_maximum = EEPROM.read(eeprom_delay_maximum);
  var_delay_minimum = EEPROM.read(eeprom_delay_minimum);

  var_delay_maximum = constrain(var_delay_maximum, 1, 60);
  var_delay_minimum = constrain(var_delay_minimum, 1, 10);

  encoder.write(var_delay_maximum);
  display.showNumberDec(var_delay_maximum);
  byte rezim = 0; 

}

void loop() {
  long var_encoder = encoder.read(); // Делим, чтобы шаг был адекватным

  if (rezim == 0){
    if(var_encoder != var_delay_maximum){
      display.showNumberDec(var_encoder);

    }


  }

  




}
