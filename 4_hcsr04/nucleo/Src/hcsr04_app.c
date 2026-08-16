#include "hcsr04_app.h"

#define HC_TRIG_Pin GPIO_PIN_5
#define HC_TRIG_GPIO_Port GPIOB
#define HC_ECHO_Pin GPIO_PIN_6
#define HC_ECHO_GPIO_Port GPIOB
#define AMBER_LED_Pin GPIO_PIN_7
#define AMBER_LED_GPIO_Port GPIOA
#define BLUE_LED2_Pin GPIO_PIN_10
#define BLUE_LED2_GPIO_Port GPIOA
#define GREEN_LED_Pin GPIO_PIN_11
#define GREEN_LED_GPIO_Port GPIOA

static const uint16_t redThresholdCm = 20;
static const uint16_t amberThresholdCm = 50;
static const uint16_t blueThresholdCm = 100;
static const uint16_t measurementPeriodUs = 60000;
static const uint16_t echoTimeoutUs = 30000;

typedef enum {
  HCSR04_IDLE,
  HCSR04_TRIGGER_HIGH,
  HCSR04_WAITING_FOR_RISING,
  HCSR04_MEASURING
} Hcsr04State;

static TIM_HandleTypeDef *sensorTimer;
static volatile Hcsr04State state = HCSR04_IDLE;
static volatile uint16_t riseTick;
static volatile uint16_t pulseTicks;
static volatile uint8_t measurementReady;
static uint16_t triggerTick;
static uint16_t nextMeasurementTick;

static uint16_t timerNow(void) {
  return (uint16_t)__HAL_TIM_GET_COUNTER(sensorTimer);
}

static void setLeds(GPIO_PinState red, GPIO_PinState amber,
                    GPIO_PinState blue, GPIO_PinState green) {
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, red);
  HAL_GPIO_WritePin(AMBER_LED_GPIO_Port, AMBER_LED_Pin, amber);
  HAL_GPIO_WritePin(BLUE_LED2_GPIO_Port, BLUE_LED2_Pin, blue);
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, green);
}

static void showDistance(uint16_t centimetres, uint8_t valid) {
  if (!valid) {
    setLeds(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
  } else if (centimetres < redThresholdCm) {
    setLeds(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
  } else if (centimetres < amberThresholdCm) {
    setLeds(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
  } else if (centimetres < blueThresholdCm) {
    setLeds(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
  } else {
    setLeds(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
  }
}

void Hcsr04_Init(TIM_HandleTypeDef *timer) {
  sensorTimer = timer;
  HAL_GPIO_WritePin(HC_TRIG_GPIO_Port, HC_TRIG_Pin, GPIO_PIN_RESET);
  showDistance(0, 0);
  HAL_TIM_Base_Start(sensorTimer);
  HAL_TIM_IC_Start_IT(sensorTimer, TIM_CHANNEL_1);
  __HAL_TIM_SET_CAPTUREPOLARITY(sensorTimer, TIM_CHANNEL_1,
                                 TIM_INPUTCHANNELPOLARITY_RISING);
  nextMeasurementTick = timerNow();
}

void Hcsr04_Process(void) {
  uint16_t now = timerNow();

  if (state == HCSR04_IDLE &&
      (uint16_t)(now - nextMeasurementTick) >= measurementPeriodUs) {
    nextMeasurementTick = now + measurementPeriodUs;
    HAL_GPIO_WritePin(HC_TRIG_GPIO_Port, HC_TRIG_Pin, GPIO_PIN_SET);
    triggerTick = now;
    state = HCSR04_TRIGGER_HIGH;
  }

  if (state == HCSR04_TRIGGER_HIGH &&
      (uint16_t)(now - triggerTick) >= 10) {
    HAL_GPIO_WritePin(HC_TRIG_GPIO_Port, HC_TRIG_Pin, GPIO_PIN_RESET);
    state = HCSR04_WAITING_FOR_RISING;
  }

  if ((state == HCSR04_WAITING_FOR_RISING || state == HCSR04_MEASURING) &&
      (uint16_t)(now - triggerTick) >= echoTimeoutUs) {
    state = HCSR04_IDLE;
    pulseTicks = 0;
    measurementReady = 1;
  }

  if (measurementReady) {
    uint16_t pulse;
    uint16_t centimetres;

    __disable_irq();
    measurementReady = 0;
    pulse = pulseTicks;
    __enable_irq();

    centimetres = (uint16_t)(pulse / 58U);
    showDistance(centimetres, pulse != 0 && centimetres <= 400);
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *timer) {
  if (timer != sensorTimer || timer->Channel != HAL_TIM_ACTIVE_CHANNEL_1) {
    return;
  }

  if (state == HCSR04_WAITING_FOR_RISING) {
    riseTick = HAL_TIM_ReadCapturedValue(timer, TIM_CHANNEL_1);
    state = HCSR04_MEASURING;
    __HAL_TIM_SET_CAPTUREPOLARITY(timer, TIM_CHANNEL_1,
                                   TIM_INPUTCHANNELPOLARITY_FALLING);
  } else if (state == HCSR04_MEASURING) {
    uint16_t fallTick = HAL_TIM_ReadCapturedValue(timer, TIM_CHANNEL_1);
    pulseTicks = (uint16_t)(fallTick - riseTick);
    state = HCSR04_IDLE;
    measurementReady = 1;
    __HAL_TIM_SET_CAPTUREPOLARITY(timer, TIM_CHANNEL_1,
                                   TIM_INPUTCHANNELPOLARITY_RISING);
  }
}