# Carboxytherapy Device — Electronics, Firmware and HMI Development

**Independent Engineering Development**  
*STM32 Embedded Control · Controlled CO₂ Delivery · Pressure Feedback · Motorized Valve Regulation · Touchscreen HMI*

## Overview

The **Carboxytherapy Device** is a custom-developed, STM32-based system for controlling the delivery of carbon dioxide (CO₂) through a motor-regulated gas-delivery mechanism. The project encompasses the complete electronic and embedded-system development process, from circuit and PCB design to firmware, touchscreen interface development, device assembly, and functional testing.

The controller coordinates operator-selected **delivery volume, pressure, and timing** with real-time pressure measurements and motorized valve adjustment. Treatment programs provide predefined operating parameters for different application categories, while calibration and fault-monitoring functions support repeatable operation and controlled responses to abnormal conditions.

**Mostafa Amiri independently developed the electronic hardware, embedded C firmware, HMI, and integrated control system.** This repository presents the implementation through STM32 source code, PCB visualizations, interface screenshots, and photographs of the completed device.

## System Architecture

An **STM32F103 microcontroller** serves as the central controller, connecting the gas-delivery mechanism, pressure-measurement circuitry, motor driver, touchscreen, and supporting electrical subsystems.

| Subsystem | Function |
| --- | --- |
| **CO₂ supply and delivery** | Gas cylinder/cartridge, regulated flow path, and delivery interface |
| **Pressure measurement** | Pressure-sensor acquisition through an ADS1115 analog-to-digital converter over I²C |
| **Motorized valve control** | PWM-based motor actuation and directional control for adjusting the gas-delivery mechanism |
| **Delivery management** | Processing of selected pressure and volume settings, operating sequences, and calculated delivery timing |
| **Calibration** | Automated motor/valve adjustment and calibration-state management |
| **Monitoring and protection** | Pressure-limit supervision, detection of abnormal pressure behavior, timeout handling, and fault-driven interruption |
| **Touchscreen interface** | DWIN DGUS HMI communicating with the STM32 through UART |
| **Auxiliary electronics** | Heater, UV-related circuitry, and supporting power and control interfaces |

The control sequence combines operator inputs with pressure feedback. The firmware manages valve adjustment, monitors operating conditions, updates the HMI, and responds to detected faults. This architecture integrates actuator control, measurement, operator interaction, and protective functions in a single embedded platform.

## Hardware Development and Integration

The device uses a **custom-designed electronic control PCB** integrating the microcontroller and interfaces for sensing, motor actuation, communication, power distribution, and auxiliary functions. The hardware was designed and assembled as part of the complete device development.

<p align="center">
  <img src="hardware/images/main-pcb-3d-render.png" alt="3D visualization of the custom carboxytherapy control PCB" width="47%">
  <img src="hardware/images/main-pcb-assembled-top.jpg" alt="Assembled custom carboxytherapy control PCB" width="47%">
</p>

<p align="center"><em>Custom control electronics: PCB design visualization (left) and assembled circuit board (right).</em></p>

The final assembly combines the electronic controller with the CO₂ supply, pressure-sensing path, motor-driven valve mechanism, touchscreen, and internal mechanical and electrical components.

[Explore the hardware design and integration](hardware/README.md).

## Embedded Control and Protective Functions

The firmware, developed in **C for the STM32F103 platform**, implements the device's operating logic, sensor acquisition, actuator commands, calibration, HMI communication, and fault handling.

Key control functions include:

- **Pressure-feedback regulation:** Acquires pressure measurements and adjusts the motorized valve mechanism in response to the selected operating pressure.
- **Configurable operating programs:** Manages user-selected volume and pressure parameters, predefined program settings, and the corresponding delivery timing.
- **Automatic motor calibration:** Executes calibration and readiness sequences to establish the valve-control operating state.
- **Abnormal-pressure detection:** Monitors pressure limits and unexpected pressure behavior, including conditions associated with needle or delivery-path faults.
- **Protective interruption and diagnostics:** Implements pressure-related stop conditions, fault indications, audible alerts, and timeout/error handling. The wider hardware design also incorporates temperature and electrical-fault protection functions.
- **Real-time HMI communication:** Processes touchscreen commands and updates device status, settings, and warning indications through a UART-based interface.

The application uses STM32 timer/PWM peripherals, GPIO control, I²C pressure acquisition, and UART/DMA communications. Its state-based logic coordinates delivery, calibration, operator commands, and protective responses.

[Explore the STM32 firmware and development project](firmware/README.md).

## Touchscreen HMI Development

A custom **DWIN DGUS touchscreen interface** provides program selection, parameter configuration, operational status, and device information. The HMI was designed alongside the controller firmware, with bidirectional communication between the display and STM32 microcontroller.

<p align="center">
  <img src="hmi/images/treatment-selection.jpg" alt="Carboxytherapy touchscreen program selection" width="47%">
  <img src="hmi/images/operational-information.jpg" alt="Carboxytherapy touchscreen operating information" width="47%">
</p>

<p align="center"><em>Custom touchscreen interface: program selection (left) and operating information (right).</em></p>

The repository includes screenshots of the interface and its development environment. Editable PCB/schematic files and native HMI project files are intentionally not distributed.

[Explore the HMI design](hmi/README.md).

## Integrated Prototype and Testing

The electronics, firmware, gas-control mechanism, and HMI were integrated into a complete working prototype and subjected to functional testing during development. Testing included device operation, motor/valve response, pressure-feedback behavior, calibration, and fault-handling functions.

<p align="center">
  <img src="results-and-media/images/assembled-device.jpg" alt="Completed carboxytherapy device with touchscreen interface" width="47%">
  <img src="hardware/images/internal-device-assembly.jpg" alt="Internal mechanical and electrical integration of the carboxytherapy device" width="47%">
</p>

<p align="center"><em>Completed device (left) and internal electronic and mechanical integration (right).</em></p>

This repository documents the engineering prototype and its implementation; it does not include clinical validation records, independently verified delivery-accuracy measurements, or regulatory approval documentation.

## Engineering Contributions

- **End-to-end electronic development:** Circuit design, custom PCB layout, board assembly, power and signal interfaces, and system-level integration.
- **Embedded firmware engineering:** STM32F103 C programming, peripheral configuration, actuator coordination, operating-state logic, and communication protocols.
- **Closed-loop actuation:** Pressure-sensor acquisition and motorized valve adjustment, including automatic calibration procedures.
- **Fault detection and protective control:** Pressure monitoring, abnormal-condition detection, operating interruptions, and diagnostic/status reporting.
- **Human–machine interface:** DWIN touchscreen screen design, program navigation, parameter exchange, and controller-driven feedback.
- **Prototype realization:** Mechanical/electronic integration, system bring-up, troubleshooting, and functional evaluation of the assembled device.

## Repository Sections

| Section | Description |
| --- | --- |
| **[Hardware](hardware/README.md)** | Custom PCB design visualization, assembled electronics, pressure-sensing and motor-control interfaces, and integration photographs. |
| **[Embedded Firmware](firmware/README.md)** | STM32F103 embedded C application, CubeMX configuration, Keil project, and relevant HAL/CMSIS components. |
| **[HMI Development](hmi/README.md)** | DWIN DGUS interface screens, operator navigation, and screenshots from the HMI development environment. |
| **[Results and Media](results-and-media/README.md)** | Photographs of the completed prototype and internal mechanical/electrical assembly. |

---

## Copyright and Citation

Copyright © 2026 Mostafa Amiri. All rights reserved.

This repository is publicly accessible to document the engineering development and demonstrate the completed project. **No general open-source license is granted for the original project materials.** Third-party software components retain their respective licenses.

For reuse permissions, see [Copyright and Permissions](COPYRIGHT.md). For attribution, see [CITATION.cff](CITATION.cff).

---

**Electronics, embedded firmware, HMI, and system integration:** [Mostafa Amiri](https://github.com/Mostafaa-Amiri).
