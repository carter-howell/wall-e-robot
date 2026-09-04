# DIY Wall-E Robotics Platform

Status: Active robotics platform / prototype work

This project is an ESP32-based Wall-E inspired robotics platform built to integrate motor control, servo actuation, Bluetooth controller input, audio playback, battery power, mechanical prototyping, and PCB design direction into one mobile robot.

![DIY Wall-E robot](media/WallEPhoto.avif)

## Overview

The project started from a 3D-printed Wall-E mechanical concept and was rebuilt around custom electronics, wiring, and firmware. It is one of Carter Howell's strongest multidisciplinary engineering projects because it combines embedded systems, robotics, power conversion, 3D printing, actuator control, audio, and a path toward a more integrated controller PCB.

## System Architecture

- Controller: ESP32-family development hardware
- Gamepad input: ESP32 Bluetooth / Bluepad32 workflow
- Servo control: PCA9685 PWM servo driver
- Drive control: L298N motor driver in the current documented build
- Audio: DFPlayer Mini module and speaker
- Power: battery-powered robot with buck/boost conversion
- Mechanical: 3D-printed body, arms, claws, and mounting parts
- PCB direction: EasyEDA schematic and PCB layout for cleaner future electronics integration

## Hardware

The documented build includes an ESP32-32S module, PCA9685 servo controller, L298N 12 V DC motor driver, XL6019 boost converter, 5 V buck converter, DFPlayer Mini audio module, 4 Ohm 3 W speaker, 12 V motors, and MG90S metal servos.

![Wall-E electronics parts](media/NewParts.avif)

## Mechanical Build

The body and alternate remix parts were 3D printed before final assembly. Printing alternate parts early helped reduce duplicate work and made it easier to adapt the mechanisms during the build.

![3D printer producing Wall-E parts](media/3DPrinter.avif)

## Schematic And PCB Work

The robot is currently documented as a hand-wired prototype, but the project includes EasyEDA schematic and PCB layout work toward a cleaner controller board. The PCB direction is meant to reduce wiring complexity and organize power distribution, servo connections, motor-driver wiring, audio, and sensor/controller connections.

![Wall-E schematic](media/Schematic.avif)

![Wall-E PCB layout](media/PCB.avif)

## Firmware

The firmware is separate from the original mechanical project. It uses Arduino/C++ and ESP32 controller support. Servo calibration is handled before running the full robot program so correct travel limits can be entered into the main control logic.

Firmware source included in this repo:

- [firmware/Walle_code.cpp](firmware/Walle_code.cpp)

## What This Demonstrates

- ESP32 embedded firmware
- Bluetooth gamepad control
- PCA9685 servo driving
- DC motor control
- audio-module integration
- battery-powered robotics wiring
- 3D-printed mechanical integration
- schematic capture and PCB layout direction
- practical system-level debugging across hardware, firmware, and mechanics

## Validation Status

The portfolio page documents the assembled robot, parts, schematic/PCB work, firmware, and a demo video. More complete engineering documentation should eventually add a full wiring diagram, pin map, servo-channel table, power measurements, motor current notes, battery runtime, and controller PCB revision history.

Do not treat the current PCB documentation as a manufactured and validated controller board unless later evidence confirms that revision.

## Future Improvements

- Add a complete pin map and PCA9685 channel table.
- Document servo calibration values.
- Add power architecture and battery-protection notes.
- Add measured current draw for motors, servos, and idle electronics.
- Add controller PCB revision files and manufacturing notes when available.
- Add a short demo video or linked media section.

## Portfolio

Portfolio page: [DIY Wall-E Robot](https://wchowellarchive.web.app/Projects/Walle/Walle.html)
