# Embedded Firmware — STM32F103 Carboxytherapy Controller

[← Back to Project Overview](../README.md)

## Overview

This section contains the original embedded C application and associated STM32CubeMX/Keil project files for the carboxytherapy controller. The target identified by the supplied CubeMX configuration is the **STM32F103C(8–B)Tx family**, with an STM32F103C8-class configuration.

The original application source and included project files were copied without modifying their contents. Generated object files, compiled outputs, archived backups, and unrelated vendor examples are omitted from this repository package.

## System Architecture

The firmware coordinates pressure measurement, motor/valve control, operator commands, display updates, timing, calibration, and protective state changes.

| Software function | Implementation identified in supplied files |
| --- | --- |
| **Pressure feedback** | External ADS1115 ADC acquired through I²C |
| **Valve/motor adjustment** | Timer-based PWM output and GPIO direction controls |
| **Touchscreen interface** | DWIN/DGUS-style UART packet exchange using DMA |
| **User programs** | Mode-indexed selections of pressure, volume, and timing values |
| **Calibration** | Motor adjustment with readiness and calibration-state management |
| **Fault handling** | High-pressure and pressure-change anomaly flags, warnings, stop logic, and time-related error states |
| **Persistent information** | EEPROM functions referenced by the application for settings/counters |

## Design and Implementation

### Application Source

- [`Core/Src/main.c`](stm32f103-controller/Core/Src/main.c) — application logic, user-interface protocol, pressure acquisition, motor control, calibration, and operating state management.
- [`Core/Src/stm32f1xx_it.c`](stm32f103-controller/Core/Src/stm32f1xx_it.c) — interrupt-service logic.
- [`Core/Src/stm32f1xx_hal_msp.c`](stm32f103-controller/Core/Src/stm32f1xx_hal_msp.c) — hardware abstraction layer peripheral support.
- [`Core/Inc/`](stm32f103-controller/Core/Inc/) — board configuration and header files.

### Development Project

- [`STM32CubeMX .ioc configuration`](stm32f103-controller/uart_lcd_i2c_adc_pwm3_tim2_3gpio.ioc) — MCU, peripheral, and pin settings.
- [`Keil MDK-ARM project`](stm32f103-controller/MDK-ARM/uart_lcd_i2c_adc_pwm3_tim2_3gpio.uvprojx) — original project file.
- [`Drivers/`](stm32f103-controller/Drivers/) — included STM32 HAL and CMSIS components used by the project, subject to their own license files.

**External dependency:** The original Keil project references `EEPROM.c` and `EEPROM.h` using a relative `libraries` path outside the supplied firmware archive. Those files were not present in the upload. Consequently, the project is **not claimed to build as supplied** without recovering that external dependency and checking the original toolchain configuration.

## Results and Experimental Evaluation

The developer reports implementing and testing the embedded controller in an integrated device. The provided archive contains application code and interface logic, but no independent software-verification record or medical-device validation report. The source is presented as evidence of embedded development, not as verified firmware for clinical deployment.

## Related Documentation

- [Hardware Development](../hardware/README.md)
- [HMI Development](../hmi/README.md)
