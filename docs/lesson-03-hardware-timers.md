# Lesson 3: Hardware Timer LED Control

Lesson 2 used `delay()` and `HAL_Delay()` to alternate two LEDs. Those functions stop the main program while waiting. In this lesson, a hardware timer creates a one-second event independently of the main loop. The loop can perform other work while the timer measures time.

The red and blue LED wiring remains the same as Lesson 2:

| LED | Arduino Uno R3 and Uno R4 WiFi | STM32 Nucleo-F401RE |
| --- | --- | --- |
| Red | `D8` | `D8`, mapped to `PA9` |
| Blue | `D7` | `D7`, mapped to `PA8` |

## Timer Pattern

All three implementations use the same pattern:

1. Configure a hardware timer to generate an update every second.
2. In the timer interrupt callback, set a `volatile` flag only.
3. In the main loop, check and clear the flag, then swap the LED states.

Do not call slow functions such as `delay()`, `Serial.print()`, or network code inside a timer interrupt. The interrupt should finish quickly.

## Part A: Arduino Uno R3 Timer1 CTC

The Uno R3 uses the ATmega328P. Its 16-bit `Timer1` can run in **CTC** (Clear Timer on Compare Match) mode. In CTC mode, the counter resets automatically when it reaches the value in the `OCR1A` compare register.

At a `16 MHz` CPU clock, dividing by `1024` gives a timer clock of `15,625 Hz`. Setting `OCR1A` to `15,624` produces one compare event each second:

```text
16,000,000 Hz / 1024 = 15,625 Hz
15,625 timer counts = 1 second
OCR1A = 15,625 - 1 = 15,624
```

Upload [uno-r3-ctc.ino](../3_hard_time/arduino/uno-r3-ctc/uno-r3-ctc.ino) after selecting **Arduino Uno**. Timer1 is dedicated to this sketch, so avoid libraries that also require Timer1, such as the standard Servo library.

## Part B: Arduino Uno R4 WiFi FspTimer

The Uno R4 WiFi uses the Renesas RA4M1. Its Arduino core provides `FspTimer`, which selects an available GPT or AGT hardware timer channel and configures it for periodic interrupts. This is board-specific Arduino code: it does not compile for the Uno R3.

Upload [uno-r4-fsptimer.ino](../3_hard_time/arduino/uno-r4-fsptimer/uno-r4-fsptimer.ino) after selecting **Arduino UNO R4 WiFi**. The sketch requests a free timer channel and configures a `1 Hz` periodic event.

## Part C: STM32 Nucleo-F401RE TIM2

The Nucleo-F401RE uses an STM32F401RE. Configure `TIM2` in STM32CubeMX as an internal-clock **Time Base** with **Update interrupt** enabled.

The existing project clock configuration runs the APB1 timer clock at `84 MHz`. These values produce a one-second update:

```text
timer clock = 84,000,000 Hz
prescaler = 8,400 - 1  -> 10,000 Hz counter clock
period = 10,000 - 1    -> 1 second update event
```

In the CubeMX timer configuration:

1. Enable `TIM2` as **Time Base** with **Internal Clock** source.
2. Set **Prescaler** to `8399` and **Counter Period** to `9999`.
3. Enable the `TIM2 global interrupt` in the NVIC configuration.
4. Keep `PA9` (`RED_LED`) and `PA8` (`BLUE_LED`) as GPIO outputs from Lesson 2.
5. Generate the project code.

Add a timer handle and a flag in the user-code sections of `main.c`:

```c
TIM_HandleTypeDef htim2;
volatile uint8_t timerTick = 0;
```

Call `MX_TIM2_Init()` with the other `MX_..._Init()` functions, then start the interrupt timer after initialization:

```c
HAL_TIM_Base_Start_IT(&htim2);
```

Implement the callback in a user-code section:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) {
    timerTick = 1;
  }
}
```

Replace the blocking LED code in `while (1)` with:

```c
if (timerTick) {
  timerTick = 0;
  HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
  HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
}

/* Other application work runs here without a one-second delay. */
```

## Comparison

| Board | Timer implementation | One-second source | Interrupt entry point |
| --- | --- | --- | --- |
| Uno R3 | ATmega328P Timer1 in CTC mode | `OCR1A = 15624`, prescaler `1024` | `ISR(TIMER1_COMPA_vect)` |
| Uno R4 WiFi | RA4M1 GPT or AGT via `FspTimer` | `1.0f` Hz periodic timer | `timerCallback()` |
| Nucleo-F401RE | STM32F401RE TIM2 time base | Prescaler `8399`, period `9999` | `HAL_TIM_PeriodElapsedCallback()` |

## What You Learned

- Hardware timers create periodic events without blocking the main loop.
- CTC mode on the Uno R3 clears the Timer1 counter after each compare match.
- `FspTimer` manages RA4M1 timer allocation on the Uno R4 WiFi.
- CubeMX can configure STM32 timer peripherals and generate interrupt setup code.
- A `volatile` flag moves substantive work out of the timer interrupt and into the main loop.