# DIY Wall-E Robot

Status: Working robot / active improvements

I built this Wall-E robot with an ESP32, two DC drive motors, servo-controlled arms and claws, Bluetooth gamepad control, a DFPlayer Mini, and 3D-printed parts. It drives, responds to the controller, moves its arms and claws, and plays audio.

![DIY Wall-E robot](media/WallEPhoto.avif)

## Overview

I started with a 3D-printed Wall-E body and rebuilt the electronics, wiring, and firmware around it. I am now replacing the hand-wired electronics with a controller PCB that combines the main power and signal connections.

## Main Parts

- Controller: ESP32-family development hardware
- Gamepad input: Bluetooth through Bluepad32
- Servo control: PCA9685 PWM servo driver
- Drive control: L298N motor driver in the current documented build
- Audio: DFPlayer Mini module and speaker
- Power: battery-powered robot with buck/boost conversion
- Mechanical: 3D-printed body, arms, claws, and mounting parts
- PCB work: EasyEDA schematic and controller-board layout

## Hardware

My documented build includes an ESP32-32S module, PCA9685 servo controller, L298N 12 V DC motor driver, XL6019 boost converter, 5 V buck converter, DFPlayer Mini audio module, 4 Ohm 3 W speaker, 12 V motors, and MG90S metal servos.

![Wall-E electronics parts](media/NewParts.avif)

## Mechanical Build

I 3D printed the body and alternate remix parts before final assembly. Printing alternate parts early helped reduce duplicate work and made it easier to adapt the mechanisms during the build.

![3D printer producing Wall-E parts](media/3DPrinter.avif)

## Schematic And PCB Work

The working robot currently uses hand-wired electronics. I have also started EasyEDA schematic and PCB layout work toward a cleaner controller board that reduces wiring complexity and organizes power distribution, servo connections, motor-driver wiring, audio, and sensor/controller connections.

![Wall-E schematic](media/Schematic.avif)

![Wall-E PCB layout](media/PCB.avif)

## Firmware

I wrote firmware separately from the original mechanical project. It uses Arduino/C++ and ESP32 controller support. I handle servo calibration before running the full robot program so correct travel limits can be entered into the main control logic.

Firmware source included in this repo:

- [firmware/Walle_code.cpp](firmware/Walle_code.cpp)

## Project Work

- ESP32 embedded firmware
- Bluetooth gamepad control
- PCA9685 servo driving
- DC motor control
- DFPlayer Mini audio control
- battery-powered robotics wiring
- 3D-printed body, arm, and claw assembly
- schematic capture and controller PCB layout
- debugging across the wiring, firmware, motors, servos, and printed mechanisms

## Testing

The assembled robot operates with its motor, servo, Bluetooth-control, and audio systems. The portfolio page includes the working build, parts, schematic and PCB work, firmware, and a demo video.

The controller PCB shown here is still in the design stage and has not been manufactured yet.

## Future Improvements

- Add a complete pin map and PCA9685 channel table.
- Document servo calibration values.
- Add power architecture and battery-protection notes.
- Add measured current draw for motors, servos, and idle electronics.
- Add controller PCB revision files and manufacturing notes when available.
- Add a short demo video or linked media section.

## Portfolio

Portfolio page: [DIY Wall-E Robot](https://wchowellarchive.web.app/Projects/Walle/Walle.html)
