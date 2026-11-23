# DIY Wall-E Robot

Status: Working robot / active improvements

I built this Wall-E robot with an ESP32, two DC drive motors, servo-controlled arms and claws, Bluetooth gamepad control, a DFPlayer Mini, and 3D-printed parts. It drives, responds to the controller, moves its arms and claws, and plays audio.

![DIY Wall-E robot](media/WallEPhoto.avif)

## Overview

I started with a 3D-printed Wall-E body and rebuilt the electronics, wiring, and firmware around it. Everything fits inside the existing body, but the available space is extremely tight. I have built two controller-board revisions to reduce the loose wiring and make the electronics easier to package.

## Main Parts

- Controller: ESP32-family development hardware
- Gamepad input: Bluetooth through Bluepad32
- Servo control: PCA9685 PWM servo driver
- Drive control: L298N motor driver in the current documented build
- Audio: DFPlayer Mini module and speaker
- Power: 3S battery with separate motor and servo power requirements
- Mechanical: 3D-printed body, arms, claws, and mounting parts
- PCB work: EasyEDA schematic and controller-board layout

## Hardware

My documented build includes an ESP32-32S module, PCA9685 servo controller, L298N 12 V DC motor driver, XL6019 boost converter, DFPlayer Mini audio module, 4 Ohm 3 W speaker, 12 V motors, and MG90S metal servos.

![Wall-E electronics parts](media/NewParts.avif)

## Mechanical Build

I 3D printed the body and alternate remix parts before final assembly. Printing alternate parts early helped reduce duplicate work and made it easier to adapt the mechanisms during the build.

![3D printer producing Wall-E parts](media/3DPrinter.avif)

## Controller PCB Revisions

I designed the controller boards in EasyEDA to collect the ESP32, audio, motor, servo, power, and battery connections on one board. I refer to the assembled board as Revision 1 and the smaller empty board as Revision 2 based on the order I fabricated them. Their silkscreen labels read `WALL-E PCB V2` and `WALL-E PCB V3` respectively.

![Wall-E schematic](media/Schematic.avif)

![Wall-E PCB layout](media/PCB.avif)

### Revision 1: assembled and working

I fully assembled and tested the first board. It worked without functional problems, but it was larger than I wanted for the limited space inside Wall-E. The board carries the ESP32, DFPlayer Mini, boost-converter module, screw terminals, servo and motor connections, and the discrete power-protection parts.

![Assembled first controller PCB](media/controller-pcb-rev1-component-side.jpg)

![Solder side of the assembled first controller PCB](media/controller-pcb-rev1-solder-side.jpg)

### Revision 2: smaller but not assembled

The second board is much smaller and places plug-in modules on both sides. I decided not to assemble it because several modules would be held above the board mainly by their headers. I did not like having those parts mechanically unsupported inside a moving robot, so this revision was manufactured but never electrically tested.

![ESP32 and step-up-converter side of the second controller PCB](media/controller-pcb-rev2-esp32-side.jpg)

![DFPlayer and connector side of the second controller PCB](media/controller-pcb-rev2-dfplayer-side.jpg)

## Packaging And Power Notes

The replacement electronics fit inside the existing printed body, but the fit is extremely tight. I also designed a different front panel to make room for the OLED display.

For another compact revision, I would replace the L298N and its boost converter with a dual motor ESC rated for a 3S battery. That would remove two of the larger modules from the build.

The servo supply also needs more current than a small buck-converter module can reliably provide. I recommend a 5 V, 8 A UBEC for the servo rail. I budget up to roughly 2 A for a stalled servo, so the firmware should limit how many high-load servo movements happen at the same time.

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
- controller PCB design, manufacturing, assembly, and revision testing
- debugging across the wiring, firmware, motors, servos, and printed mechanisms

## Testing

The assembled robot operates with its motor, servo, Bluetooth-control, and audio systems. The portfolio page includes the working build, parts, schematic and PCB work, firmware, and a demo video.

The assembled first controller board worked fully in testing, with size as its main drawback. The smaller second board was manufactured but intentionally left unassembled, so I do not claim that revision as electrically tested.

## Parts and purchase prices

Prices below are historical USD item prices from my inventory and order records. Package prices are identified explicitly. Shipping and taxes are not included. The list covers the finished ESC-controlled, 2S build.

| Part / selected variant | Build quantity | Recorded price | Purchase link |
| --- | ---: | --- | --- |
| ESP32 Type-C 30-pin development board | 1 | $4.39 each | [AliExpress](https://www.aliexpress.com/item/3256808703453847.html) |
| JGB37-520 gear motor, 12 V, 200 RPM | 2 | $8.72 each | [AliExpress](https://www.aliexpress.com/item/3256805250852226.html) |
| PCA9685 16-channel servo driver | 1 | $2.87 each | [AliExpress](https://www.aliexpress.com/item/3256806112519208.html) |
| MG90S 180° metal-gear servo | 7 | $20.96 per 10-pack | [AliExpress](https://www.aliexpress.com/item/3256805639785749.html) |
| Mini brushed ESC, advertised 30 A, 4–8 V | 2 | $3.03 each | [AliExpress](https://www.aliexpress.com/item/3256809348516540.html) |
| 5 V 7 A UBEC | 1 | $5.21 each | [AliExpress](https://www.aliexpress.com/item/3256809278446011.html) |
| DFPlayer Mini | 1 | $5.77 per 5-pack | [AliExpress](https://www.aliexpress.com/item/3256808023570782.html) |
| 40 mm 4 Ω 3 W speaker | 1 | $2.85 per 2-pack | [AliExpress](https://www.aliexpress.com/item/3256806925518415.html) |
| 1.3-inch SH1106 I²C OLED, white, 128×64 | 1 | $2.16 each | [AliExpress](https://www.aliexpress.com/item/3256807385478388.html) |
| 2S 1500 mAh 45C LiPo, JST variant | 1 | $8.93 each | [AliExpress](https://www.aliexpress.com/item/3256806995815859.html) |
| 100 kΩ and 6.8 kΩ divider resistors | 1 each | $4.47 per 600-piece assortment | [AliExpress](https://www.aliexpress.com/item/3256810191764673.html) |
| KF301 two-pin screw terminals | As installed | $1.60 per 10-pack | [AliExpress](https://www.aliexpress.com/item/3256806456550715.html) |
| 2.54 mm male/female breakaway headers | Cut to fit | $1.92 per assorted package | [AliExpress](https://www.aliexpress.com/item/2255800687544049.html) |
| Dupont housings and crimp contacts | As installed | $2.71 per 620-piece kit | [AliExpress](https://www.aliexpress.com/item/3256802459986084.html) |
| Custom WALL-E motherboard | 1 | Fabrication price not recorded | Custom EasyEDA design |
| Printed body, tracks, wheels, gears, head, arms and chest panel | 1 set | Filament consumption and cost not recorded | Printed locally |
| Screws, nuts, servo horns, magnets and linkage hardware | As installed | Used quantities and allocated cost not recorded | Workshop stock |
| Wire, solder, insulation and cable ties | As installed | Allocated cost not recorded | Workshop stock |
| microSD card for DFPlayer audio | 1 | Purchase price not recorded | Existing card |
| Bluetooth gamepad | 1 | Purchase price not recorded | Existing controller |

Mechanical model references retained with the local build files include the [head-center modification](https://www.thingiverse.com/thing:4583962), [linkage arms](https://www.thingiverse.com/thing:4707426), [magnetic door addons](https://www.thingiverse.com/thing:4734863), [grub-screw pinion](https://www.thingiverse.com/thing:4932959), and [realistic hands](https://www.thingiverse.com/thing:5391045).

A complete project total is not available because fabrication, filament, hardware and workshop consumables were not individually costed. Purchasing a full assortment costs more than the fraction consumed in one robot. The ESC listing above advertises 4–8 V; it is not a documented 3S-rated substitute.

## Portfolio

Portfolio page: [DIY Wall-E Robot](https://wchowellarchive.web.app/Projects/Walle/Walle.html)
