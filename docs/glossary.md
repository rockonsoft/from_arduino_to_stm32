# Acronyms and Dictionary

An alphabetic reference for technical terms and abbreviations used in this documentation.

## A

**ADC (analog-to-digital converter)**
: A peripheral that measures an analog voltage and converts it to a numeric value that software can read.

**Analog input**
: A pin or peripheral input that measures a varying voltage, such as the output from a sensor or potentiometer.

**Analog output**
: A voltage output produced by a DAC. A PWM output can approximate an analog voltage but is not a true analog output.

**Anode**
: The positive LED connection. On a typical through-hole LED, it is the longer leg.

**Arduino-compatible header**
: Header sockets laid out to accept Arduino Uno R3 shields. The Nucleo-F401RE includes these headers.

**AVR**
: A family of microcontrollers originally developed by Atmel. The ATmega328P on the Arduino Uno R3 is an 8-bit AVR device.

## B

**Bluetooth Low Energy (BLE)**
: A low-power Bluetooth radio protocol. The Uno R4 WiFi provides it through its ESP32-S3 module.

**Bootloader**
: Small program that accepts new firmware, usually over USB or serial, and writes it into flash memory.

**Breadboard**
: A reusable board with connected holes for building circuits without soldering.

## C

**CAN (Controller Area Network)**
: A robust communication bus commonly used in vehicles and industrial equipment.

**Cathode**
: The negative LED connection. On a typical through-hole LED, it is the shorter leg and is nearest the flat side of the LED body.

**Clock speed**
: The rate at which a processor operates, usually stated in MHz. A higher clock speed does not alone determine program performance.

**Cortex-M4**
: A 32-bit Arm microcontroller processor core used by the Renesas RA4M1 and STM32F401RE.

## D

**DAC (digital-to-analog converter)**
: A peripheral that converts a number produced by software into an analog voltage.

**Data flash**
: Non-volatile flash memory intended for storing data, separate from the main program flash on some microcontrollers.

**Digital I/O**
: General-purpose pins that software can read as logic inputs or drive as logic outputs.

## E

**EEPROM (electrically erasable programmable read-only memory)**
: Non-volatile memory for small pieces of data that must remain after power is removed. The Uno R3 has dedicated EEPROM.

**ESP32-S3**
: The Espressif wireless module on the Uno R4 WiFi that provides Wi-Fi and Bluetooth Low Energy connectivity.

**EXTI (external interrupt)**
: STM32 hardware that lets an input pin trigger immediate software handling when its signal changes.

## F

**Firmware**
: The program stored in a microcontroller's non-volatile memory that controls the board and its connected hardware.

**Flash memory**
: Non-volatile memory used to store firmware. It keeps its contents when power is removed.

**FPU (floating-point unit)**
: Processor hardware that accelerates floating-point arithmetic. The STM32F401RE's Cortex-M4 includes an FPU.

## G

**GND (ground)**
: The circuit's common voltage reference and return path. A circuit needs both a supply connection, such as `5V`, and GND to operate.

**GPIO (general-purpose input/output)**
: A programmable digital pin that can be used as an input or output.

**GPIO output**
: A GPIO configured to drive a logic low or high voltage. In Lesson 2, the output turns an LED off or on through its series resistor.

## H

**HAL (hardware abstraction layer)**
: Software library that provides a standard interface to microcontroller peripherals. STM32Cube HAL is one example.

**Header**
: A row of connector pins or sockets used to attach jumper wires, shields, or expansion hardware.

**HID (human interface device)**
: A USB device class for peripherals such as keyboards, mice, and game controllers.

## I

**I2C (Inter-Integrated Circuit)**
: A two-wire serial bus for communicating with sensors, displays, and other integrated circuits.

**ICSP (in-circuit serial programming)**
: A connector or method for programming a microcontroller directly, typically using SPI signals on the Uno R3.

## L

**LED (light-emitting diode)**
: A diode that emits light when current flows from its anode to cathode. It requires a series current-limiting resistor.

**Logic high and logic low**
: The two digital output states. Arduino calls them `HIGH` and `LOW`; STM32 HAL calls them `GPIO_PIN_SET` and `GPIO_PIN_RESET`.

**Logic voltage**
: The voltage levels a board's digital pins use to represent low and high signals. Uno R3 and Uno R4 WiFi GPIO use `5V`; Nucleo-F401RE GPIO uses `3.3V`.

## M

**MCU (microcontroller unit)**
: A single integrated circuit that combines a processor, memory, and hardware peripherals for embedded systems.

**MHz (megahertz)**
: One million clock cycles per second; a unit used for processor clock speed.

**Morpho header**
: ST's expansion header format on Nucleo boards. These male headers expose more STM32 pins than the Arduino-compatible connectors.

## O

**ohm**
: The unit of electrical resistance, written as `ohm` or the symbol `Omega`. A resistor value of `330 ohm` limits LED current in Lesson 1.

## P

**PCB (printed circuit board)**
: The rigid board that mechanically supports components and electrically connects them through copper tracks.

**Peripheral**
: A hardware function within a microcontroller, such as a timer, ADC, UART, SPI controller, or GPIO port.

**Pinout**
: A diagram or table showing a board's connector pins, names, and functions.

**PlatformIO**
: An embedded development environment and build system that supports Arduino and STM32 projects.

**pinMode()**
: An Arduino function that sets the mode of a pin, such as `OUTPUT` for an LED control pin.

**Polarity**
: The required positive and negative orientation of a component or power connection. LEDs have polarity.

**PWM (pulse-width modulation)**
: A digital output technique that varies the proportion of time a signal is on. It is used for LED brightness, motor control, and approximated analog output.

## Q

**Qwiic**
: A solderless I2C connector system. The Uno R4 WiFi includes a Qwiic connector.

## R

**RA4M1**
: Renesas 32-bit Arm Cortex-M4 microcontroller used as the main processor on the Arduino Uno R4 WiFi.

**Real-time clock (RTC)**
: A peripheral that tracks time and date, often while the main processor is asleep.

**Resistor**
: A component that opposes electrical current. In an LED circuit, it limits current to a safe level.

## S

**Serial**
: A general term for data sent one bit after another. Arduino `Serial` commonly refers to UART communication over USB.

**Shield**
: An add-on board designed to plug into Arduino-compatible headers.

**Silkscreen**
: Printed text and symbols on a PCB that identify components, connectors, and pin labels.

**SPI (Serial Peripheral Interface)**
: A synchronous serial bus commonly used for displays, memory chips, and fast sensors.

**SRAM (static random-access memory)**
: Volatile working memory used while firmware runs. Its contents are lost when power is removed.

**ST-LINK**
: STMicroelectronics' on-board programmer and debugger found on the Nucleo-F401RE.

**STM32CubeIDE**
: STMicroelectronics' integrated development environment for STM32 projects.

**STM32CubeMX**
: STMicroelectronics' configuration tool that generates STM32 project setup code.

**STM32 HAL (hardware abstraction layer)**
: STMicroelectronics' library for controlling STM32 hardware. `HAL_GPIO_WritePin()` sets a GPIO output and `HAL_Delay()` pauses for a specified number of milliseconds.

**STM32F401RE**
: The STM32 microcontroller fitted to the Nucleo-F401RE board; it uses an Arm Cortex-M4 core.

**SWD (Serial Wire Debug)**
: Arm's two-wire hardware debugging and programming interface.

## U

**UART (universal asynchronous receiver-transmitter)**
: A hardware peripheral for asynchronous serial communication using transmit and receive lines.

**Uno R3 form factor**
: The connector layout and board dimensions used by Arduino Uno R3-compatible boards and shields.

**USB (Universal Serial Bus)**
: A cable and communication standard used to power boards and, depending on the board, upload firmware, exchange serial data, or debug.

**USB OTG (USB On-The-Go)**
: USB capability that lets a device operate as either a USB host or peripheral, depending on the application.
