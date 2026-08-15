# From Arduino to STM32

Practical notes, examples, and reference material for moving Arduino Uno projects to STM32 microcontrollers.

## What This Project Covers

- Mapping familiar Arduino concepts to STM32 hardware and tooling
- Recreating Arduino projects with STM32CubeIDE, STM32CubeMX, or PlatformIO
- Comparing pins, peripherals, libraries, and programming workflows
- Building small, reproducible examples for both platforms

## Getting Started

This site is being built alongside the project source. The first guides and examples will be added here as the repository grows.

For the repository overview, setup information, and current progress, see the [project README](https://github.com/rockonsoft/from_arduino_to_stm32#readme).

## Planned Guides

| Arduino Uno | STM32 equivalent |
| --- | --- |
| `digitalWrite()` | GPIO output |
| `analogRead()` | ADC conversion |
| `Serial` | UART / USART |
| `delay()` | Hardware timer or HAL delay |
| `attachInterrupt()` | External interrupt (EXTI) |

## Repository Layout

Firmware source and examples will be organised by target platform, while this `docs/` directory contains the GitHub Pages documentation.

