#include <Encoder.h>
#include <TM1637Display.h>

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


void setup() {
  pinMode(ENC_SW, INPUT_PULLUP);
  display.setBrightness(0x0f);
  
  pinMode(VALVE, OUTPUT);
  digitalWrite(VALVE, LOW);

  pinMode(FIRE, OUTPUT);
  digitalWrite(FIRE, LOW);

  pinMode(SIRENA, OUTPUT);
  digitalWrite(SIRENA, LOW);

  
}

void loop() {
  long newPos = encoder.read() / 2; // Делим, чтобы шаг был адекватным
  if(newPos > 5) {digitalWrite(VALVE, HIGH); display.showNumberDec(newPos);}
  if(newPos > 10) {digitalWrite(FIRE, HIGH); display.showNumberDec(newPos);}
  if(newPos > 15) {digitalWrite(SIRENA, HIGH); display.showNumberDec(newPos);}

}
