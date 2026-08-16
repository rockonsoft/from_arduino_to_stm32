# Lesson 4 - HC-SR04 Ultrasonic Distance Measurement and Interrupt-Driven Proximity Indicator

This lesson builds a small vehicle-reversing warning system. An HC-SR04 ultrasonic sensor measures the distance to an object, and four LEDs show the risk level:

| Distance | LED | Meaning |
| --- | --- | --- |
| Less than `20 cm` | Red | Very close / highest educational warning level |
| `20 cm` to less than `50 cm` | Amber | Object is getting close |
| `50 cm` to less than `100 cm` | Blue | Moderate distance |
| `100 cm` or more, or no valid reading | Green | Far away or measurement invalid |

These thresholds are arbitrary educational values, not calibrated automotive safety limits. They are constants in each implementation so they can easily be changed.

The same application is implemented three times:

1. Arduino Uno R3 with the ATmega328P's Timer1 and external interrupt INT0
2. Arduino Uno R4 WiFi with the Renesas RA4M1's `FspTimer` and external interrupt
3. STM32 Nucleo-F401RE with STM32 TIM4 input capture and HAL interrupts

The objective is to learn how one embedded requirement maps onto different timer, GPIO, and interrupt architectures.

## Learning Objectives

- Explain the HC-SR04 trigger and Echo signals.
- Measure an Echo pulse width with a hardware timer.
- Convert sound travel time into distance.
- Configure GPIO, timers, input capture, and interrupts.
- Handle timer overflow and measurement timeout.
- Build a non-blocking state machine.
- Keep interrupt service routines short and use `volatile` shared state correctly.
- Separate the sensor driver, distance classification, and LED controller.
- Port the same architecture across AVR, Renesas RA4M1, and STM32 MCUs.

## How the HC-SR04 Works

The MCU sends a trigger pulse of approximately `10 us` to `TRIG`. The sensor emits an ultrasonic burst, drives `ECHO` high, and holds it high for the round-trip travel time. The MCU measures that pulse width.

```text
MCU TRIG:  ____|----------|________________  about 10 us high
HC ECHO:   __________|----------------|____  high for round-trip time
                       object and back
```

The fundamental relationship is:

```text
distance = (echo_time * speed_of_sound) / 2
```

The division by two is required because the sound travels to the object and back. At approximately `20 C`, use `343 m/s`, or `0.0343 cm/us`:

```text
distance_cm = echo_time_us * 0.0343 / 2
distance_cm = echo_time_us / 58.2
```

Temperature changes the speed of sound, so this is an approximation. The sensor also has a finite minimum range and a practical maximum range. The code treats a timeout or invalid pulse as a green, far-away indication.

## Common State Machine

Every implementation follows the same conceptual states, even though the peripheral APIs differ:

```text
IDLE
  |
  v
TRIGGER -- timer schedules the end of the 10 us pulse
  |
  v
WAIT_FOR_ECHO_RISING -- input interrupt or capture detects the rising edge
  |
  v
MEASURE_ECHO -- timer timestamp is recorded at the falling edge
  |       \
  |        \ timeout from the timer
  v         v
DISTANCE_READY   INVALID_READING
       \         /
        v       v
          IDLE
```

The main loop starts measurements and processes completed results. It never waits in a loop for Echo and never calls `delay()`, `HAL_Delay()`, `sleep()`, `pulseIn()`, or an equivalent blocking function.

```mermaid
flowchart LR
  T[TRIG GPIO output] --> S[HC-SR04 sensor]
  S --> E[ECHO pulse]
  E --> C[Timer / input capture]
  C --> I[Interrupt]
  I --> TS[Timestamp pulse edges]
  TS --> SM[Non-blocking state machine]
  SM --> D[Distance conversion]
  D --> CL[Threshold classification]
  CL --> L[Red / amber / blue / green LEDs]
```

## Hardware and Wiring

Use a separate current-limiting resistor for every LED. `330 ohm` is a suitable starting value. The LED anode is the longer leg and connects toward the GPIO through the resistor; the cathode connects to GND.

### Arduino Uno R3

| HC-SR04 or LED signal | Uno R3 connection |
| --- | --- |
| VCC | `5V` |
| GND | `GND` |
| TRIG | `D4` |
| ECHO | `D2` / INT0 |
| Red LED | `D8` through `330 ohm` |
| Amber LED | `D7` through `330 ohm` |
| Blue LED | `D6` through `330 ohm` |
| Green LED | `D5` through `330 ohm` |

### Arduino Uno R4 WiFi

| HC-SR04 or LED signal | Uno R4 connection |
| --- | --- |
| VCC | `5V` |
| GND | `GND` |
| TRIG | `D4` |
| ECHO | `D2` / external interrupt-capable pin |
| Red LED | `D8` through `330 ohm` |
| Amber LED | `D7` through `330 ohm` |
| Blue LED | `D6` through `330 ohm` |
| Green LED | `D5` through `330 ohm` |

Check the exact Uno R4 board and core pin mapping before wiring. Do not assume every Uno R3 electrical limit or interrupt mapping is identical on the R4.

### STM32 Nucleo-F401RE

This lesson uses the STM32 pins directly, not the Arduino pin names:

| HC-SR04 or LED signal | Nucleo-F401RE connection |
| --- | --- |
| VCC | `5V` |
| GND | `GND` |
| TRIG | `PB5` GPIO output |
| ECHO | `PB6` / `TIM4_CH1` input capture through level shifter |
| Red LED | `PA9` through `330 ohm` |
| Amber LED | `PA8` through `330 ohm` |
| Blue LED | `PA10` through `330 ohm` |
| Green LED | `PA11` through `330 ohm` |

The exact Nucleo header positions should be checked against the board manual and pinout. Keep all grounds common.

### STM32 Echo Level Shifter

The HC-SR04 Echo output is commonly `5V`. Do not connect it directly to a `3.3V`-only STM32 GPIO. Use a resistor voltage divider:

```text
HC-SR04 ECHO ----  R1 = 10 kohm  ----+---- STM32 PB6
                                      |
                                R2 = 20 kohm
                                      |
                                     GND
```

The resulting voltage is approximately:

```text
Vout = 5 V * 20 kohm / (10 kohm + 20 kohm) = 3.33 V
```

This is close to the STM32 `3.3V` rail. Use components and limits appropriate for the specific sensor and board, and verify the STM32 input absolute maximum rating. The Uno R3 and R4 connections also require checking the exact HC-SR04 module and board electrical specifications rather than assuming every pin is 5V tolerant.

## Part A: Uno R3 - Timer1 and INT0

The ATmega328P is an 8-bit AVR running at `16 MHz`. The sketch in [4_hcsr04/arduino/uno-r3/uno-r3.ino](../4_hcsr04/arduino/uno-r3/uno-r3.ino) uses:

- Timer1 as a free-running 16-bit counter with a `0.5 us` tick (`16 MHz / 8`).
- Timer1 overflow interrupts to extend the counter beyond 16 bits.
- Timer1 compare-A interrupts to finish the non-blocking `10 us` trigger pulse.
- Timer2 CTC at `1 ms` to schedule measurements and enforce the echo timeout.
- INT0 on `D2` to timestamp both Echo edges.

The external interrupt is used because the chosen Echo pin is `D2`, not Timer1's fixed ICP1 pin. A pin-change interrupt could watch more pins, but it requires inspecting a port and determining which bit changed. INT0 gives this lesson a direct rising/falling edge interrupt.

```text
Timer1 tick = 16,000,000 / 8 = 2,000,000 Hz
Timer1 resolution = 1 / 2,000,000 = 0.5 us
```

Timer overflow handling is essential: a long Echo pulse can cross the 16-bit counter boundary. The overflow counter and edge timestamps form a wider software timestamp.

Compile and upload with:

```bash
arduino-cli compile -b arduino:avr:uno 4_hcsr04/arduino/uno-r3
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno 4_hcsr04/arduino/uno-r3
```

## Part B: Uno R4 - RA4M1 FspTimer

The Uno R4 WiFi uses a `48 MHz` Renesas RA4M1 Arm Cortex-M4, not the ATmega328P. Its Arduino core exposes RA4M1 GPT/AGT hardware timers through `FspTimer`. The sketch in [4_hcsr04/arduino/uno-r4/uno-r4.ino](../4_hcsr04/arduino/uno-r4/uno-r4.ino) uses an available FspTimer channel at `100 kHz`, giving a `10 us` software timestamp tick, and uses Arduino's external interrupt attachment only for the Echo edge routing.

`FspTimer` is the hardware timer; `pinMode`, `digitalWrite`, and `attachInterrupt` are Arduino core setup APIs that configure RA4M1 GPIO and interrupt routing. The code does not use an HC-SR04 library or `pulseIn()`.

The R4 core's public `FspTimer` API provides periodic timer interrupts rather than a portable Arduino-level input-capture API. Therefore this implementation uses the timer interrupt as a free-running timestamp and the Echo external interrupt for edge capture. A native RA4M1 project can replace that edge path with GPT input capture while keeping the same driver state machine.

Compile and upload with:

```bash
arduino-cli compile -b arduino:renesas_uno:unor4wifi 4_hcsr04/arduino/uno-r4
arduino-cli upload -p <your_port> --fqbn arduino:renesas_uno:unor4wifi 4_hcsr04/arduino/uno-r4
```

## Part C: Nucleo-F401RE - TIM4 Input Capture

The STM32F401RE runs at `84 MHz`. Configure `TIM4` as a `16-bit` timer with:

| CubeMX setting | Value | Reason |
| --- | --- | --- |
| Clock Source | Internal Clock | Timer counts MCU clock ticks |
| Prescaler | `83` | `84 MHz / (83 + 1) = 1 MHz` |
| Counter Period | `65535` | Free-running 16-bit counter |
| Channel 1 | Input Capture direct mode | Echo is connected to `PB6/TIM4_CH1` |
| Rising polarity | Enabled first | Stores the start timestamp |
| Falling polarity | Switched in callback | Stores the end timestamp |
| NVIC | TIM4 global interrupt enabled | Delivers capture and update interrupts |

The STM32 implementation in [4_hcsr04/nucleo/Src/hcsr04_app.c](../4_hcsr04/nucleo/Src/hcsr04_app.c) is a HAL application module. Add it to a CubeMX-generated Nucleo project, declare the handles and GPIO labels shown in the module, and call its init/process functions from the generated `main.c`. CubeMX owns clock, GPIO, TIM4, and NVIC initialization; the module owns the HC-SR04 state machine and classification.

Start input capture with `HAL_TIM_IC_Start_IT()`. `HAL_TIM_IC_CaptureCallback()` receives the rising and falling edge events. The timer counter itself is the timestamp; unsigned 16-bit subtraction handles a single counter wrap between edges. A timeout based on the same timer counter marks the measurement invalid.

## Driver and Application Separation

```text
HC-SR04 driver
  trigger GPIO + timer/input capture + interrupts
        |
        v
distance measurement in centimetres
        |
        v
distance classifier and thresholds
        |
        v
LED controller
```

The driver reports either a valid distance or an invalid reading. The application does not need to know whether the timestamp came from AVR registers, RA4M1 `FspTimer`, or STM32 input capture.

## Interrupt Architecture

```text
Timer counts --------------------+
                                  |
Echo rising edge -> interrupt -> timestamp start
Echo falling edge -> interrupt -> timestamp end
                                  |
Timer timeout ------------------> invalid measurement
                                  |
                                  v
                         state machine
                                  |
                                  v
                         main-loop processing
```

Only timestamps, state transitions, and flags are updated in interrupt context. Distance conversion and LED writes run in the main loop. Variables shared between an ISR and the main loop are `volatile`; the short copy/clear operations are protected where the MCU can update a multi-byte value concurrently.

## Testing Procedure

1. Test each LED separately with a fixed GPIO output.
2. Confirm the HC-SR04 `TRIG` line receives a roughly `10 us` pulse with a logic analyser or oscilloscope.
3. Place an object in front of the sensor and verify that `ECHO` rises and falls.
4. Test a target at approximately `10 cm`, `30 cm`, `70 cm`, and `120 cm`.
5. Verify red, amber, blue, and green classification.
6. Disconnect Echo or point the sensor at open space and verify timeout selects green.
7. Test exact threshold boundaries: `20 cm`, `50 cm`, and `100 cm`.
8. Compare the same target measurement on all three boards.

## Troubleshooting

- **No Echo:** Check VCC, common ground, trigger wiring, sensor orientation, and the edge-interrupt pin.
- **Always zero:** Confirm the rising edge is detected and that the timer is running.
- **Always maximum/green:** Confirm the falling edge and timeout path; check the STM32 level shifter.
- **Wrong distance:** Check timer clock, prescaler, overflow handling, sensor angle, and the `0.0343 cm/us` sound-speed constant.
- **Noisy measurements:** Use a stable target, average several valid readings, and reject impossible pulse widths.
- **LEDs do not light:** Check polarity, each resistor, GPIO labels, and ground.
- **STM32 resets or is damaged:** Disconnect the `5V` Echo signal and verify the divider before reconnecting.
- **Timer overflow errors:** Test distances near the configured timeout and verify counter-wrap subtraction.
- **Wrong CubeMX pin:** Confirm `PB6` is `TIM4_CH1`, not just a normal GPIO input, and that TIM4 global interrupt is enabled.

## Exercises

- Change the three threshold constants.
- Add a buzzer whose warning frequency increases as the object gets closer.
- Average three or five valid measurements without blocking the main loop.
- Add a second HC-SR04 and schedule its trigger separately.
- Measure the same target with all three boards and compare results.
- Send the distance over serial without printing from an ISR.
- Build a Python oscilloscope/data logger for the Echo pulse.

## Cross-Platform Comparison

| Feature | Uno R3 | Uno R4 WiFi | Nucleo-F401RE |
| --- | --- | --- | --- |
| MCU | ATmega328P | Renesas RA4M1 | STM32F401RE |
| CPU clock | `16 MHz` | `48 MHz` | `84 MHz` |
| Timer architecture | 8/16-bit AVR timers; Timer1 used as free-running counter | RA4M1 GPT/AGT timer allocated through `FspTimer` | STM32 general-purpose TIM4 with input capture |
| Input capture | INT0 edge interrupt plus Timer1 timestamp because Echo is on D2 | External interrupt plus FspTimer timestamp; native GPT capture is possible in an RA4M1 project | TIM4 CH1 rising/falling input capture interrupt |
| Interrupt system | AVR vector table: Timer1, Timer2, INT0 | Renesas FSP IRQ routing behind the Arduino core | NVIC and STM32 HAL callbacks |
| ADC | 10-bit | 14-bit | 12-bit |
| GPIO voltage | `5V` | `5V` | `3.3V`; Echo level shifting required |
| Implementation effort | Direct registers, compact but timing-sensitive | Different MCU core and FSP timer API | CubeMX setup plus HAL callback integration |

The application architecture stayed constant, but the hardware mapping changed. That is the central embedded-systems lesson: identify the timing, GPIO, interrupt, and state-machine responsibilities first, then map each responsibility to the peripherals available on the target MCU.

## What We Learned

- An HC-SR04 Echo pulse is a measurement of round-trip sound time.
- Hardware timers and edge interrupts measure pulse width without blocking the application.
- Timer overflow and no-Echo timeout paths are normal parts of a robust driver.
- The driver, classifier, and LED controller can remain conceptually stable across three MCU architectures.
- STM32 `3.3V` GPIO requires electrical protection from a common `5V` HC-SR04 Echo output.

## Next Lesson

The next lesson can add filtering and a buzzer, then compare interrupt-driven measurements with DMA-based capture on the STM32.