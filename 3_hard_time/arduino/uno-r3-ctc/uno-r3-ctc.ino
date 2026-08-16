#include <avr/interrupt.h>

const byte redLedPin = 8;
const byte blueLedPin = 7;
volatile bool timerTick = false;
bool redIsOn = false;

ISR(TIMER1_COMPA_vect) {
  timerTick = true;
}

void setup() {
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, LOW);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void loop() {
  if (timerTick) {
    noInterrupts();
    timerTick = false;
    interrupts();

    redIsOn = !redIsOn;
    digitalWrite(redLedPin, redIsOn ? HIGH : LOW);
    digitalWrite(blueLedPin, redIsOn ? LOW : HIGH);
  }
}