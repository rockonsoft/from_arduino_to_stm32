#include "FspTimer.h"

const byte redLedPin = 8;
const byte blueLedPin = 7;
volatile bool timerTick = false;
bool redIsOn = false;
FspTimer timer;

void timerCallback(timer_callback_args_t *) {
  timerTick = true;
}

void setup() {
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, LOW);

  uint8_t timerType;
  int8_t timerChannel = FspTimer::get_available_timer(timerType);
  if (timerChannel < 0) {
    while (true) {
    }
  }

  timer.begin(TIMER_MODE_PERIODIC, timerType, timerChannel, 1.0f, 0.0f, timerCallback);
  timer.setup_overflow_irq();
  timer.open();
  timer.start();
}

void loop() {
  if (timerTick) {
    timerTick = false;
    redIsOn = !redIsOn;
    digitalWrite(redLedPin, redIsOn ? HIGH : LOW);
    digitalWrite(blueLedPin, redIsOn ? LOW : HIGH);
  }
}