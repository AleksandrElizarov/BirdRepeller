#include <Encoder.h>
#include <TM1637Display.h>

// Пины TM1637
#define DISP_CLK 7
#define DISP_DIO 6

// Пины энкодера
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4

Encoder encoder(ENC_DT, ENC_CLK);

TM1637Display display(DISP_CLK, DISP_DIO);

int lastPos = 1;
int value = 1;
bool buttonPressed = false;
unsigned long lastDebounce = 0;
const int debounceDelay = 50;

void setup() {
  pinMode(ENC_SW, INPUT_PULLUP);
  display.setBrightness(0x0f);
  display.showNumberDec(value);
}

void loop() {
  long newPos = encoder.read() / 4; // Делим, чтобы шаг был адекватным

  if (newPos != lastPos) {
    value = constrain(newPos + 1, 1, 100); // +1, чтобы старт с 1
    display.showNumberDec(value);
    lastPos = newPos;
  }

  // Обработка кнопки
  if (digitalRead(ENC_SW) == LOW && !buttonPressed && millis() - lastDebounce > debounceDelay) {
    buttonPressed = true;
    lastDebounce = millis();
    display.showNumberDec(1111);
    delay(500);
    display.showNumberDec(value);
  }

  if (digitalRead(ENC_SW) == HIGH) {
    buttonPressed = false;
  }
}
