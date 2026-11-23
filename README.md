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


## Mechanical Build

### 3D print files and original design credit

I obtained the original WALL-E 3D printer files from **Simon Bluett (chillibasket)** and built this robot around his WALL-E Robot Replica design. The original mechanical design is his work; my contribution includes the custom electronics, motherboard, ESP32 firmware, wiring, and adaptations for this build.

- [chillibasket's WALL-E replica repository](https://github.com/chillibasket/walle-replica)
- [Original build guide and model downloads](https://wired.chillibasket.com/3d-printed-wall-e/)

I 3D printed the body and alternate remix parts before final assembly. Printing alternate parts early helped reduce duplicate work and made it easier to adapt the mechanisms during the build.

![3D printer producing Wall-E parts](media/3DPrinter.avif)

## Controller PCB Revisions

I designed the controller boards in EasyEDA to collect the ESP32, audio, motor, servo, power, and battery connections on one board. I refer to the assembled board as Revision 1 and the smaller empty board as Revision 2 based on the order I fabricated them. Their silkscreen labels read `WALL-E PCB V2` and `WALL-E PCB V3` respectively.

![WALL-E Drop In V4 schematic](media/wall-e-drop-in-v4-schematic.png)

![WALL-E Drop In V4 motherboard — EasyEDA 3D render](media/wall-e-drop-in-v4-pcb-3d.png)

### Revision 1: assembled and working

I fully assembled and tested the first board. It worked without functional problems, but it was larger than I wanted for the limited space inside Wall-E. The board carries the ESP32, DFPlayer Mini, boost-converter module, screw terminals, servo and motor connections, and the discrete power-protection parts.


### Revision 2: smaller but not assembled

The second board is much smaller and places plug-in modules on both sides. I decided not to assemble it because several modules would be held above the board mainly by their headers. I did not like having those parts mechanically unsupported inside a moving robot, so this revision was manufactured but never electrically tested.


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
| Dual-channel bidirectional brushed ESC, advertised 40 A × 2, 6–15 V, mixed control | 1 | $6.33 displayed listing price; original paid price unconfirmed | AliExpress; original item listing unavailable |
| 5 V 7 A UBEC | 1 | $5.21 each | [AliExpress](https://www.aliexpress.com/item/3256809278446011.html) |
| DFPlayer Mini | 1 | $5.77 per 5-pack | [AliExpress](https://www.aliexpress.com/item/3256808023570782.html) |
| 40 mm 4 Ω 3 W speaker | 1 | $2.85 per 2-pack | [AliExpress](https://www.aliexpress.com/item/3256806925518415.html) |
| 1.3-inch SH1106 I²C OLED, white, 128×64 | 1 | $2.16 each | [AliExpress](https://www.aliexpress.com/item/3256807385478388.html) |
| 2S 1500 mAh 45C LiPo, JST variant | 1 | $8.93 each | [AliExpress](https://www.aliexpress.com/item/3256806995815859.html) |
| 100 kΩ and 6.8 kΩ divider resistors | 1 each | $4.47 per 600-piece assortment | [AliExpress](https://www.aliexpress.com/item/3256810191764673.html) |
| KF301 two-pin screw terminals | As installed | $1.60 per 10-pack | [AliExpress](https://www.aliexpress.com/item/3256806456550715.html) |
| 2.54 mm male/female breakaway headers | Cut to fit | $1.92 per assorted package | [AliExpress](https://www.aliexpress.com/item/2255800687544049.html) |
| Dupont housings and crimp contacts | As installed | $2.71 per 620-piece kit | [AliExpress](https://www.aliexpress.com/item/3256802459986084.html) |
| Custom WALL-E motherboard | 1 order | $10.00 paid for fabrication; board count not recorded | [JLCPCB](https://jlcpcb.com/) |
| Filament for body, tracks, wheels, gears, head, arms and chest panel | 2 rolls | Estimated $20.00 per roll; $40.00 total | Printed locally; filament brand not recorded |
| M2.5 hex screws, bolts and nuts kit | 1 kit | $6.00 paid | Purchase link not recorded |
| Wire, solder, insulation and cable ties | As installed | Estimated $5.00 allowance | Workshop stock |
| microSD card for DFPlayer audio | 1 | Estimated $8.00 allowance | Existing card; model not recorded |
| Xbox wireless controller | 1 | Estimated $60.00 allowance | [Xbox controllers](https://www.xbox.com/accessories/controllers) |

No magnets were used. Filament, microSD, controller, connector consumption, and assembly-supply estimates are budget allowances rather than verified purchase prices.

### Estimated build cost

| Cost group | Amount |
| --- | ---: |
| Main electronics, motors, battery and seven servos | $64.58 |
| JLCPCB fabrication order | $10.00 |
| Two rolls of filament, estimated | $40.00 |
| M2.5 hardware kit | $6.00 |
| Connector/header allowance, estimated | $3.00 |
| Two divider resistors, estimated allocation | $0.02 |
| Wire, solder and assembly supplies, estimated | $5.00 |
| microSD card, estimated | $8.00 |
| **Robot total excluding Xbox controller** | **$136.60** |
| Xbox controller, estimated | $60.00 |
| **Total including controller** | **$196.60** |

The electronics subtotal allocates seven servos from a 10-pack, one DFPlayer from a 5-pack, and one speaker from a 2-pack. Connector costs use a $3.00 consumption allowance rather than charging the robot for entire kits. The fabrication amount is the reported $10 order cost, since the number of boards was not recorded. Filament assumes two rolls at $20 each; roll weight and brand were not recorded. Totals combine historical item prices with estimates and exclude separately unrecorded shipping and taxes.

Mechanical model references retained with the local build files include the [head-center modification](https://www.thingiverse.com/thing:4583962), [linkage arms](https://www.thingiverse.com/thing:4707426), [magnetic door addons](https://www.thingiverse.com/thing:4734863), [grub-screw pinion](https://www.thingiverse.com/thing:4932959), and [realistic hands](https://www.thingiverse.com/thing:5391045).

Purchasing full assortments costs more than the fractions consumed in one robot. The ESC specifications and $6.33 listing price are taken from the identified product screenshot; they are seller claims rather than independently measured ratings.

## Portfolio

Portfolio page: [DIY Wall-E Robot](https://wchowellarchive.web.app/Projects/Walle/Walle.html)
