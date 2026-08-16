#include "FspTimer.h"

const byte triggerPin = 4;
const byte echoPin = 2;
const byte redLedPin = 8;
const byte amberLedPin = 7;
const byte blueLedPin = 6;
const byte greenLedPin = 5;

const uint16_t redThresholdCm = 30;
const uint16_t amberThresholdCm = 75;
const uint16_t blueThresholdCm = 150;
const uint32_t measurementPeriodTicks = 6000; // 60 ms at 10 us per tick.
const uint32_t echoTimeoutTicks = 3000;       // 30 ms at 10 us per tick.

const bool RED_ON = true;
const bool RED_OFF = false;
const bool AMBER_ON = true;
const bool AMBER_OFF = false;
const bool BLUE_ON = true;
const bool BLUE_OFF = false;    
const bool GREEN_ON = true;
const bool GREEN_OFF = false;

enum MeasurementState : byte {
  Idle,
  TriggerHigh,
  WaitingForRising,
  Measuring
};

FspTimer timer;
volatile uint32_t timerTicks = 0;
volatile MeasurementState measurementState = Idle;
volatile uint32_t triggerStartedAt = 0;
volatile uint32_t echoStartedAt = 0;
volatile uint32_t echoTicks = 0;
volatile bool measurementReady = false;
bool timerAvailable = false;
uint32_t nextMeasurementAt = 0;
bool invalidMeasurement = true;
uint16_t distanceCm = 0;

// Timer callback function to increment the timer tick count
// passed to the timer.begin() function.
void timerCallback(timer_callback_args_t *) {
  timerTicks++;
}

// Interrupt Service Routine for the echo pin change (rising or falling edge)
void echoEdge() {
  uint32_t now = timerTicks;
  bool echoHigh = digitalRead(echoPin) == HIGH;

  // move the state machine forward based on the edge and current state
  if (echoHigh && measurementState == WaitingForRising) {
    echoStartedAt = now;
    measurementState = Measuring;
  } else if (!echoHigh && measurementState == Measuring) {
    echoTicks = now - echoStartedAt;
    measurementState = Idle;
    measurementReady = true;
  }
}

// Set the state of the four LEDs based on the distance measurement.
void setLeds(bool red, bool amber, bool blue, bool green) {
  digitalWrite(redLedPin, red ? HIGH : LOW);
  digitalWrite(amberLedPin, amber ? HIGH : LOW);
  digitalWrite(blueLedPin, blue ? HIGH : LOW);
  digitalWrite(greenLedPin, green ? HIGH : LOW);
}

void showDistance(uint16_t centimetres, bool valid) {
  if (!valid) {
    setLeds(RED_ON, AMBER_OFF, BLUE_OFF, GREEN_ON);
  } else if (centimetres < redThresholdCm) {
    setLeds(RED_ON, AMBER_OFF, BLUE_OFF, GREEN_OFF);
  } else if (centimetres < amberThresholdCm) {
    setLeds(RED_OFF, AMBER_ON, BLUE_OFF, GREEN_OFF);
  } else if (centimetres < blueThresholdCm) {
    setLeds(RED_OFF, AMBER_OFF, BLUE_ON, GREEN_OFF);
  } else {
    setLeds(RED_OFF, AMBER_OFF, BLUE_OFF, GREEN_ON);
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

  uint8_t timerType;
  int8_t timerChannel = FspTimer::get_available_timer(timerType);
  if (timerChannel >= 0 && timer.begin(TIMER_MODE_PERIODIC, timerType,
                                       timerChannel, 100000.0f, 0.0f,
                                       timerCallback/*pass in callback*/)) {
    // Initiated 100 kHz timer, so each tick is 10 us.
    timer.setup_overflow_irq();
    timer.open();
    timer.start();
    attachInterrupt(digitalPinToInterrupt(echoPin), echoEdge, CHANGE);
    nextMeasurementAt = timerTicks;
    timerAvailable = true;
  }
}

void loop() {
  if (!timerAvailable) {
    return;
  }

  uint32_t now = timerTicks;
  // Trigger pin is held high for 10 us, so we can use the timer ticks directly.
  if (measurementState == Idle && (int32_t)(now - nextMeasurementAt) >= 0) {
    nextMeasurementAt = now + measurementPeriodTicks;
    digitalWrite(triggerPin, HIGH);
    triggerStartedAt = now;
    measurementState = TriggerHigh;
  }

  if (measurementState == TriggerHigh && now - triggerStartedAt >= 1) {
    digitalWrite(triggerPin, LOW);
    measurementState = WaitingForRising;
  }

  if ((measurementState == WaitingForRising || measurementState == Measuring) &&
      now - triggerStartedAt >= echoTimeoutTicks) {
    measurementState = Idle;
    echoTicks = 0;
    measurementReady = true;
  }

  if (measurementReady) {
    uint32_t measuredTicks;

    noInterrupts();
    measurementReady = false;
    measuredTicks = echoTicks;
    interrupts();

    // Each timer tick is 10 us; 58.2 us is approximately one cm round trip.
    distanceCm = (uint16_t)((measuredTicks * 10UL) / 58UL);
    invalidMeasurement = measuredTicks == 0 || distanceCm > 400;
    showDistance(distanceCm, !invalidMeasurement);
  }
}