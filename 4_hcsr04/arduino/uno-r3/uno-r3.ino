#include <avr/interrupt.h>

const byte triggerPin = 4;
const byte echoPin = 2; // INT0
const byte redLedPin = 8;
const byte amberLedPin = 7;
const byte blueLedPin = 6;
const byte greenLedPin = 5;

const uint16_t redThresholdCm = 30;
const uint16_t amberThresholdCm = 75;
const uint16_t blueThresholdCm = 150;
const uint16_t measurementPeriodMs = 60;
const uint16_t echoTimeoutMs = 30;

enum MeasurementState : byte {
  Idle,
  WaitingForRising,
  Measuring
};

volatile MeasurementState measurementState = Idle;
volatile uint32_t timer1OverflowCount = 0;
volatile uint32_t echoStartTicks = 0;
volatile uint16_t echoPulseTicks = 0;
volatile bool measurementReady = false;
volatile uint16_t measurementAgeMs = 0;
volatile bool triggerHigh = false;
bool invalidMeasurement = true;
uint16_t distanceCm = 0;

uint32_t timer1Timestamp() {
  uint32_t overflows = timer1OverflowCount;
  uint16_t counter = TCNT1;

  if ((TIFR1 & (1 << TOV1)) && counter < 0x8000) {
    overflows++;
  }

  return (overflows << 16) | counter;
}

void startMeasurementFromTimer2() {
  if (measurementState != Idle) {
    return;
  }

  measurementState = WaitingForRising;
  measurementAgeMs = 0;
  triggerHigh = true;
  PORTD |= (1 << PD4);

  OCR1A = TCNT1 + 20; // 20 Timer1 ticks at 0.5 us per tick = 10 us.
  TIFR1 |= (1 << OCF1A);
  TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_OVF_vect) { //Interrupt Service Routine for Timer1 overflow
  timer1OverflowCount++;
}

ISR(TIMER1_COMPA_vect) { //Interrupt Service Routine for Timer1 compare match
  if (triggerHigh) {
    PORTD &= ~(1 << PD4);
    triggerHigh = false;
    TIMSK1 &= ~(1 << OCIE1A);
  }
}

ISR(TIMER2_COMPA_vect) { //Interrupt Service Routine for Timer2 compare match
  static uint16_t periodAgeMs = measurementPeriodMs;

  if (periodAgeMs < measurementPeriodMs) {
    periodAgeMs++;
  }
  if (measurementState != Idle) {
    measurementAgeMs++;
  }

  if (measurementState == Idle && periodAgeMs >= measurementPeriodMs) {
    periodAgeMs = 0;
    startMeasurementFromTimer2();
  }

  if (measurementState != Idle && measurementAgeMs >= echoTimeoutMs) {
    measurementState = Idle;
    echoPulseTicks = 0;
    measurementReady = true;
  }
}

ISR(INT0_vect) { //Interrupt Service Routine for external interrupt 0 (INT0) D2
  uint32_t timestamp = timer1Timestamp();

  bool echoHigh = (PIND & (1 << PD2)) != 0;

  if (echoHigh && measurementState == WaitingForRising) {
    echoStartTicks = timestamp;
    measurementState = Measuring;
    measurementAgeMs = 0;
  } else if (!echoHigh && measurementState == Measuring) {
    echoPulseTicks = (uint16_t)(timestamp - echoStartTicks);
    measurementState = Idle;
    measurementReady = true;
  }
}

void setLeds(bool red, bool amber, bool blue, bool green) {
  digitalWrite(redLedPin, red ? HIGH : LOW);
  digitalWrite(amberLedPin, amber ? HIGH : LOW);
  digitalWrite(blueLedPin, blue ? HIGH : LOW);
  digitalWrite(greenLedPin, green ? HIGH : LOW);
}

void showDistance(uint16_t centimetres, bool valid) {
  if (!valid) {
    setLeds(false, false, false, true);
  } else if (centimetres < redThresholdCm) {
    setLeds(true, false, false, false);
  } else if (centimetres < amberThresholdCm) {
    setLeds(false, true, false, false);
  } else if (centimetres < blueThresholdCm) {
    setLeds(false, false, true, false);
  } else {
    setLeds(false, false, false, true);
  }
}

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(amberLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  showDistance(0, false);

  noInterrupts();

  // Timer1: normal mode, free-running, 16 MHz / 8 = 0.5 us per tick.
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << TOIE1);

  // Timer2: CTC mode, 16 MHz / 64 / (249 + 1) = 1 kHz.
  TCCR2A = (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = 249;
  TIMSK2 |= (1 << OCIE2A);

  // INT0 on D2 reacts to both Echo edges.
  EICRA = (1 << ISC00);
  EIMSK = (1 << INT0);

  interrupts();
}

void loop() {
  if (measurementReady) {
    uint16_t pulseTicks;

    noInterrupts();
    measurementReady = false;
    pulseTicks = echoPulseTicks;
    interrupts();

    if (pulseTicks == 0 || pulseTicks > 60000) {
      invalidMeasurement = true;
    } else {
      // Timer1 ticks are 0.5 us, so convert to microseconds before cm.
      uint32_t echoMicroseconds = pulseTicks / 2;
      distanceCm = echoMicroseconds / 58;
      invalidMeasurement = distanceCm == 0 || distanceCm > 400;
    }

    showDistance(distanceCm, !invalidMeasurement);
  }
}