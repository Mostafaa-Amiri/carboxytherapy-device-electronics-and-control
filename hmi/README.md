# HMI Development — Carboxytherapy Device

[← Back to Project Overview](../README.md)

## Overview

This section documents the touchscreen human–machine interface (HMI) developed for the carboxytherapy device. The available screenshots show the device interface and the DWIN DGUS editing environment used for screen development.

The display connects to the STM32 controller through a UART-based protocol, allowing operator selections, status information, and fault indications to be exchanged with the application firmware.

## System Architecture

The HMI organizes operator interaction into visual program selection, device operation and information screens, and controller-driven status/alert states. The firmware includes packet-based display writes and reception of user control values.

## Design and Implementation

### Screen Design and Navigation

<p align="center">
  <img src="images/carboxy-splash-screen.jpg" alt="Carboxy interface splash screen" width="47%">
  <img src="images/treatment-selection.jpg" alt="Carboxytherapy interface program selection screen" width="47%">
</p>

<p align="center"><em>Branded device interface and program-selection navigation.</em></p>

![Carboxytherapy HMI information screen](images/operational-information.jpg)

*On-screen information categories including sessions, volume, gas pressure, interval, and procedure-related parameters.*

### HMI Editor Screenshots

<p align="center">
  <img src="images/dwin-interface-editor-overview.png" alt="DWIN DGUS design environment with device program screen" width="47%">
  <img src="images/dwin-interface-editor-program.png" alt="DWIN interface editor showing a program-specific screen" width="47%">
</p>

<p align="center"><em>Screen layout and interface development in the DWIN DGUS design environment.</em></p>

The editable native HMI project is not part of the uploaded material; these screenshots document the interface design.

## Results and Experimental Evaluation

The screenshots demonstrate the implemented interface design and device-specific screens. The supplied material does not include independent usability testing or a clinical human-factors evaluation.

## Related Documentation

- [Embedded Firmware](../firmware/README.md)
- [Hardware Development](../hardware/README.md)
- [Results and Media](../results-and-media/README.md)
