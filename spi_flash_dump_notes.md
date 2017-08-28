Please add your findings to this document!

## Memory map

| Section Name       |  Offset | Size    | Remarks                                                                   |
| ------------------ |:-------:|:-------:| ------------------------------------------------------------------------- |
| Static             | `x0000` | `x1000` | Stores static data like BT address, OTA DS 1 offset, etc                  |
| Failsafe           | `x1000` | `x1000` | Stores OTA Signature Magic and OTA Dynamic Section 2 offset               |
| Volatile           | `x2000` | `x1000` | Stores last connected device, data for Sticks and Sensor calibration      |
| Volatile (Back up) | `x3000` | `x1000` | Empty                                                                     |
| Dynamic            | `x4000` | `x7C000`| Stores Factory configuration and calibration, user calibration and OTA FW |

## x0000: Static data

Static section data starts at `x0000` and ends at `x03B0`.

Includes Magic numbers, OTA FW 1 offset? and other. 

|    Range        |        Sample        | Remarks                                                    |
|:---------------:|:--------------------:| ---------------------------------------------------------- |
| `x0015`-`x001A` | `x57 30 EA 8A BB 7C` | Bluetooth MAC Address, reversed (7C:BB:8A:EA:30:57)        |
| `x03B3`-`x03B7` | `x00000100`          | Dynamic Section Offset 1 for OTA FW, reversed (`x010000`) |

## x1000 Failsafe mechanism

The failsafe mechanism has 2 functions.

Bootloader checks OTA Magic and if it's OK, it loads the OTA FW in DS2 offset, otherwise from DS1.

To update OTA FW in DS1 location, it needs a valid OTA Signature Magic which is a result from a verified OTA FW in DS2. Then proceeds to update OTA FW 1 and erases the whole failsafe section, so it can load OTA FW from DS1.

|    Range        |        Sample              | Remarks                                                    |
|:---------------:|:--------------------------:| ---------------------------------------------------------- |
| `x1FF4`-`x1FFB` | `xAA 5 5F0 0F 68 E5 97 D2` | OTA Signature Magic, reversed (`xD2 97 E5 68 0F F0 55 AA`) |
| `x1FFC`-`x1FFF` | `x00800200`                | Dynamic Section Offset 2 for OTA FW, reversed (`x028000`) |

No data before `x1FF4`.

## x2000 Volatile data

User Motion Sensor and Analog stick calibration data is located around `x8000` in the SPI flash, while calibrating the control stick, data changed at Volatile Section also.

Can be reset with `x07` subcommand.

```
00002000: 0022 0c96[98b6 e92a b0cb]b7ed f1ab b4a6  ; Last connected console (at x2004)
00002010: 39e2 4876 4615 62f2 0be8 0000 0000 0000
00002020: 0000 0000 6800[9522 634f 3402 8662 b86b  ; Marked section also changed after calibrating control sticks
00002030: 3623 5b57 8ff0 16b4 a05f fbc0 b99d 6227
00002040: 0000 0000 0000 0000 0000 0800]ffff ffff
```

## x6000 Factory Configuration and Calibration

|  Section Range  | Subsection Range | Remarks                                         |
|:---------------:|:----------------:| ----------------------------------------------- |
| `x6000`-`x600F` | -------------    | Serial number in non-extended ASCII. If first byte is >= `80`, no S/N. If a byte is `00` `NUL`, skip. Max 15 chars, if 16 chars last one is skipped.|
| `x6012`-`x6013` | -------------    | Left Joy-Con: `x01 A0`, Right Joy-Con: `x02 A0` |
| `x6020`-`x6037` | -------------    | Factory configuration & calibration 1           |
|                 | `x6020 - x6037`  | 6-Axis motion sensor Factory calibration        |
| `x603D`-`x6055` | -------------    | Factory configuration & calibration 2           |
|                 | `x603D - x6045`  | Left analog stick calibration                   |
|                 | `x6046 - x604E`  | Right analog stick calibration                  |
|                 | `x6050 - x6052`  | Body #RGB color, 24-bit                         |
|                 | `x6053 - x6055`  | Buttons #RGB color, 24-bit                      |
| `x6080`-`x6097` | -------------    | Factory Sensor and Stick device parameters      |
|                 | `x6080`-`x6085`  | 6-Axis Horizontal Offsets. (JC sideways)        |
|                 | `x6086`-`x6097`  | Stick device parameters 1                       |
| `x6098`-`x60A9` | -------------    | Factory Stick device parameters 2               |
|                 | `x6098`-`x60A9`  | Stick device parameters 2. Normally the same with 1, even in Pro Contr. |
| `x6E00`-`x6EFF` | -------------    | Unknown data values.. Exist only in Joy-Con     |

Above data ends at `x6F00`.

## x8000 User Calibration

This section holds user generated 6-Axis Motion Sensor and Analog Sticks calibration data.

|  Section Range  | Subsection Range | Remarks                                        |
|:---------------:|:----------------:| ---------------------------------------------- |
| `x8010`-`x8025` | -------------    | User Analog sticks calibration                 |
|                 | `x8010`, `x8011` | Magic `xB2 xA1` for user available calibration |
|                 | `x8012 - x801A`  | Actual User Left Stick Calibration data        |
|                 | `x801B`, `x801C` | Magic `xB2 xA1` for user available calibration |
|                 | `x801D - x8025`  | Actual user Right Stick Calibration data       |
| `x8026`-`x803F` | -------------    | User 6-Axis Motion Sensor calibration          |
|                 | `x8026`, `x8027` | Magic `xB2 xA1` for user available calibration |
|                 | `x8028`-`x803F`  | Actual 6-Axis Motion Sensor Calibration data   |

```
00008000: ffff ffff ffff ffff ffff ffff ffff ffff
00008010: ffff ffff ffff ffff ffff[ffb2 a1c6 f871  ; Control Stick Calibration data
00008020: 6884 42a3 5442|b2a1 7600 a6fe ea02 0040
00008030: 0040 0040 0e00 fcff e0ff 3b34 3b34 3b34] ; Motion Control Calibration data
```

## x010000 Over-The-Air Firmware
Valid areas for OTA FW updates are DS1 `@x10000` and DS2 `@x28000`.

Which one is loaded is decided by matching the OTA Signature Magic at `x1FF4`. If it matches, it checks the DS2 offset at `x1FFC` and loads OTA in DS2, otherwise it loads DS1 which is pointed by `x03B3` at static section.

Normally the firmware at `x010000` is older.

Maximum size for OTA Firmware is 96KB.

It probably keeps the joycon application code and patches to the actual firmware.

It can be updated through UART and Bluetooth.

The OTA FW is not to be confused with the actual Firmware in the 848KB ROM of BCM20734.


## Analog stick factory and user calibration

3 groups of 3 bytes. 

The general code to decode each 3 byte group is:
```
uint16_t data[6]
data[0] = (stick_cal[1] << 8) & 0xF00 | stick_cal[0];
data[1] = (stick_cal[2] << 4) | (stick_cal[1] >> 4);
data[2] = (stick_cal[4] << 8) & 0xF00 | stick_cal[3];
data[3] = (stick_cal[5] << 4) | (stick_cal[4] >> 4);
data[4] = (stick_cal[7] << 8) & 0xF00 | stick_cal[6];
data[5] = (stick_cal[8] << 4) | (stick_cal[7] >> 4);
```

Left Stick:
| uint16_t #| Sample | Remarks                 |
|:---------:|:------:| ----------------------- |
| `0`       | `x4F7` | X Axis Max above center |
| `1`       | `x424` | Y Axis Max above center |
| `2`       | `x79F` | X Axis Center           |
| `3`       | `x8A0` | Y Axis Center           |
| `4`       | `x510` | X Axis Min below center |
| `5`       | `x479` | Y Axis Min below center |

Right Stick:
| uint16_t #| Sample | Remarks                 |
|:---------:|:------:| ----------------------- |
| `0`       | `x79F` | X Axis Center           |
| `1`       | `x8A0` | Y Axis Center           |
| `2`       | `x510` | X Axis Min below center |
| `3`       | `x479` | Y Axis Min below center |
| `4`       | `x4F7` | X Axis Max above center |
| `5`       | `x424` | Y Axis Max above center |

The minimum and maximum values are then subtracted or added to the center values to get the real Min/Max range.

For example (Right stick):
```
uint16_t rstick_center_x = data[0];
uint16_t rstick_center_y = data[1];
uint16_t rstick_x_min = rstick_center_x - data[2];
uint16_t rstick_x_max = rstick_center_x + data[4];
uint16_t rstick_y_min = rstick_center_x - data[3];
uint16_t rstick_y_max = rstick_center_x + data[5];
```

## 6-Axis sensor factory and user calibration

4 groups of 3 little endian 16-bit float (in Int16LE encoding).

Each group probably defines the X Y Z axis.

Probably Gyroscope.

Sample (Big-Endian):
| 16-bit float #| Sample           | Remarks                                     |
|:-------------:|:----------------:| ------------------------------------------- |
| `0` - `2`     | `FFB0 FEB9 00E0` | Possibly XYZ origin position when on table |
| `3` - `5`     | `4000 4000 4000` | Unknown XYZ                                 |
| `6` - `8`     | `000E FFDF FFD0` | Unknown XYZ                                 |
| `9` - `11`    | `343B 343B 343B` | Unknown XYZ                                 |


## 6-Axis and Stick device parameters

These follow the same encoding with sensor and stick calibration accordingly.

6-Axis Horizontal Offsets:

3 little endian 16-bit float (in Int16LE encoding).
Define the origin position when the Joy-Con are held sideways.

Stick Parameters:
18 bytes that produce 12 uint16_t.
Probably define maximum and minimum ranges that the analog stick hardware supports and deadzones.

| uint16_t # | Sample    | Remarks                   |
|:----------:|:---------:| ------------------------- |
| `0`, `1`   | `019 4CD` | Seems that this is unused |
| `2`        | `AE`      | Unknown                   |
| `3`        | `E14`     | Unknown                   |
| `4`, `5`   | `2EE 2EE` | Unknown                   |
| `6`, `7`   | `2EE 2EE` | Unknown                   |
| `8`, `9`   | `AB4 AB4` | Unknown                   |
| `10`, `11` | `496 496` | Unknown                   |