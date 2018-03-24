#SPI Flash Memory Information

## SPI memory map

| Section Name            |  Offset | Size    | Remarks                                                              |
| ----------------------- |:-------:|:-------:| -------------------------------------------------------------------- |
| Initial PatchRAM        | `x0000` | `x1000` | Stores static data like BT address, OTA DS 1 offset, etc             |
| Failsafe                | `x1000` | `x1000` | Stores OTA Signature Magic and OTA Dynamic Section 2 offset          |
| Pairing info            | `x2000` | `x1000` | Stores last connected device, data for Sticks and Sensor calibration |
| Pairing info (factory)  | `x3000` | `x1000` | Empty. (Dev-units use it?)                                           |
| Pairing info (factory2) | `x4000` | `x1000` | Empty. (Dev-units use it?)                                           |
| Shipment                | `x5000` | `x1000` | Only first byte is used                                              |
| Config and calibration  | `x6000` | `xA000` | Stores Factory configuration and calibration, user calibration       |
| PatchRAM section        | `x10000`| `x70000 | Stores Broadcom PatchRAMs, by default @0x10000 and @0x28000          |

## x0000: Initial PatchRAM

Initial PatchRAM section starts at `x0000` and ends at `x03B0`. Has a total of 13 records and does not end with the usual PatchRAM EOF footer (xFE 0000).

Includes Magic numbers, OTA FW DS1 address and code that loads the PatchRAM @0x10000 or @0x28000 by checking the 0xx1ff4 address.

|    Range        |        Sample                      | Remarks                                             |
|:---------------:|:----------------------------------:| --------------------------------------------------- |
| `x0000`-`x0010` | `01 08 00 F0 00 00 62 08 C0 5D 89` | Loader Magic or it sends x895DC008 @x620000F0 MMIO  |
| `x0012`-`x001A` | `40 0600 x5730EA8ABB7C`            | BD_ADDR type record, in LE (7C:BB:8A:EA:30:57)      |
| `x001B`-`x03AF` | ---                                | Initial code that loads one of the main PatchRAM    |
| `x03B0`-`x03B7` | `02 0A 00000100 00200000 0010`     | DS1 Uint32LE (`x010000`) and 2 values (x200, x1000) |

## x1000 Failsafe mechanism

The failsafe mechanism has 2 functions.

Bootloader checks OTA Magic and if it's OK, it loads the OTA FW in DS2 offset, otherwise from DS1.

To update OTA FW in DS1 location, it needs a valid OTA Signature Magic which is a result from a verified OTA FW in DS2. Then proceeds to update OTA FW 1 and erases the whole failsafe section, so it can load OTA FW from DS1.

|    Range        |        Sample              | Remarks                                                    |
|:---------------:|:--------------------------:| ---------------------------------------------------------- |
| `x1FF4`-`x1FFB` | `xAA 5 5F0 0F 68 E5 97 D2` | OTA Signature Magic, reversed (`xD2 97 E5 68 0F F0 55 AA`) |
| `x1FFC`-`x1FFF` | `x00800200`                | Dynamic Section Offset 2 for OTA FW, reversed (`x028000`) |

No data before `x1FF4`.

## x2000 Pairing info

Used internally to store current and previous Host Bluetooth addresses and Long Term Keys.

It can store entries until it fills  the whole section.

The current used data has `x95` magic. If `x00`, this data defines the previous paired connection and the next section defines the current one.

By connecting through USB (Charging grip), it keeps the active section and the current LTK used with Switch can be acquired.

Can be reset with `x07` subcommand.

|  Section Range  | Subsection Range | Remarks                                                         |
|:---------------:|:----------------:| --------------------------------------------------------------- |
| `x2000`-`x2025` | -------------    | Pairing info section 1                                          |
|                 | `x2000`          | Magic. Used=`x95`, Unused=`x00`. If `x00`, checks next section. |
|                 | `x2001`          | Size of pairing data. Always `x22` bytes                        |
|                 | `x2002 - x2003`  | Checksum?                                                       |
|                 | `x2004 - x2009`  | Host Bluetooth address (Big-Endian)                             |
|                 | `x200A - x2019`  | 128-bit Long Term Key (Little-Endian)                           |
|                 | `x201A - x2023`  | Always zeroed                                                   |
|                 | `x2024`          | Host capabilities? Switch=`x68`, PC=`x08`                       |
|                 | `x2025`          | Always zero                                                     |
| `x2026`-`x204B` | -------------    | Pairing info section 2. All `xFF` if section 1 is used          |
|                 | `x2026`          | Magic                                                           |
|                 | `x2027`          | Size of pairing data                                            |
|                 | `x2028 - x2029`  | Checksum?                                                       |
|                 | `x202A - x202F`  | Host Bluetooth address (Big-Endian)                             |
|                 | `x2030 - x203F`  | 128-bit Long Term Key (Little-Endian)                           |
|                 | `x2040 - x2049`  | Always zeroed                                                   |
|                 | `x204A`          | Switch=`x68`, PC=`x08`                                          |
|                 | `x204B`          | Always zero                                                     |
| --              | --               | The layout is repeated according to saved pairings              |

## x3000 Pairing info (factory)

It copies and uses the pairing info from here to 0x2000.

## x4000 Pairing info (factory2)

It copies the pairing info from here to 0x3000, checks if OK and erases it.

## x5000 Shipment

The first byte is probably set to x1 on new Joy-Con. Switch makes sure to set it to 0.

## x6000 Factory Configuration and Calibration

|  Section Range  | Subsection Range | Remarks                                                                 |
|:---------------:|:----------------:| ----------------------------------------------------------------------- |
| `x6000`-`x600F` | -------------    | Serial number in non-extended ASCII. If first byte is >= `80`, no S/N. If a byte is `00` `NUL`, skip. Max 15 chars, if 16 chars last one is skipped.|
| `x6012`         | -------------    | Device type. JC (L): `x01`, JC (R): `x02`, Pro: `x03`. Only the 3 LSB are accounted for. Used internally and for `x02` subcmd. |
| `x6013`         | -------------    | Unknown, seems to always be `xA0`                                       |
| `x601B`         | -------------    | Color info exists if `x01`. If 0, default colors used are ARGB `#55555555`, `#FFFFFFFF`. Used for `x02` subcmd. |
| `x6020`-`x6037` | -------------    | Factory configuration & calibration 1                                   |
|                 | `x6020 - x6037`  | 6-Axis motion sensor Factory calibration                                |
| `x603D`-`x6055` | -------------    | Factory configuration & calibration 2                                   |
|                 | `x603D - x6045`  | Left analog stick calibration                                           |
|                 | `x6046 - x604E`  | Right analog stick calibration                                          |
|                 | `x6050 - x6052`  | Body #RGB color, 24-bit                                                 |
|                 | `x6053 - x6055`  | Buttons #RGB color, 24-bit                                              |
|                 | `x6056 - x6058`  | Left Grip #RGB color, 24-bit (Added in 5.0.0 for Pro)                   |
|                 | `x6059 - x605B`  | Right Grip #RGB color, 24-bit (Added in 5.0.0 for Pro)                  |
| `x6080`-`x6097` | -------------    | Factory Sensor and Stick device parameters                              |
|                 | `x6080`-`x6085`  | 6-Axis Horizontal Offsets. (JC sideways)                                |
|                 | `x6086`-`x6097`  | Stick device parameters 1                                               |
| `x6098`-`x60A9` | -------------    | Factory Stick device parameters 2                                       |
|                 | `x6098`-`x60A9`  | Stick device parameters 2. Normally the same with 1, even in Pro Contr. |
| `x6E00`-`x6EFF` | -------------    | Unknown data values.. Exists only in Joy-Con                            |

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

## x010000 Broadcom PatchRAM (OTA FW update)
Valid areas for PatchRAM are DS1 `@x10000` and DS2 `@x28000`.

Which one is loaded is decided by matching the OTA Signature Magic at `x1FF4`. If it matches, it checks the DS2 offset at `x1FFC` and loads OTA in DS2, otherwise it loads DS1 which is pointed by `x03B3` at static section.

Normally the PatchRAM size for Joy-Con is max 96KB, but if the code fits RAM, it can go higher and itt can be updated through UART and Bluetooth.

It patches ROM and RAM and keeps all the customized code for Joy-Con proprietary protocol, hw configuration, internal communication, patches from Broadcom, etc. 

It uses records defined, by a uint8_t record type, a uint16LE size and data[size]. There's no checksum, so, user can modify it and it will load, if the modifications were correct, otherwise it will bootloop. The currently known records are 0x40 (BD_ADDR), 0x08 (patches ROM) and 0x0A (patches RAM).

The PatchRAM (OTA FW) is not to be confused with the actual ROM firmware (848KB) of BCM20734.

For a parser or an ida loader check [shuffle2's nxpad](https://github.com/shuffle2/nxpad).


## Analog stick factory and user calibration

3 groups of 3 bytes. 

The general code to decode each 3 byte group into uint16_t is:
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
uint16_t rstick_y_min = rstick_center_y - data[3];
uint16_t rstick_y_max = rstick_center_y + data[5];
```

## 6-Axis sensor factory and user calibration

4 groups of 3 Int16LE.

Each group defines the X Y Z axis.

1st two groups are Acc cal and the 2nd two groups are Gyro cal.

Sample (Big-Endian):

| int16_t #  | Sample XYZ       | Remarks                                                                |
|:----------:|:----------------:| ---------------------------------------------------------------------- |
| `0` - `2`  | `FFB0 FEB9 00E0` | Acc XYZ origin position when completely horizontal and stick is upside |
| `3` - `5`  | `4000 4000 4000` | Acc XYZ sensitivity special coeff, for default sensitivity: ±8G.       |
| `6` - `8`  | `000E FFDF FFD0` | Gyro XYZ origin position when still                                    |
| `9` - `11` | `343B 343B 343B` | Gyro XYZ sensitivity special coeff, for default sensitivity: ±2000dps. |

For the sensitivities conversion check [here](imu_sensor_notes.md#convert-to-basic-useful-data-using-spi-calibration).

Reference code for converting from uint16_t to int16_t for doing the above calculations:

```
int16_t uint16_to_int16(uint16_t a) {
	int16_t b;
	char* aPointer = (char*)&a, *bPointer = (char*)&b;
	memcpy(bPointer, aPointer, sizeof(a));
	return b;
}
```

## 6-Axis and Stick device parameters

These follow the same encoding with sensor and stick calibration accordingly.

#### 6-Axis Horizontal Offsets:

3 Int16LE.
Define the accelerator additional offset from origin position when the Joy-Con is on a flat surface. The trigger bumps on the Joy-Con and Pro controller change the origin position and use can use these to level it.

Default values:

| Axis | Joy-Con (L)  | Joy-Con (R)    | Pro Controller |
|:----:|:------------:|:--------------:|:--------------:|
| X    | `x15E` (350) | `x15E` (350)   | `FD50` (-688)  |
| Y    | `x00` (0)    | `x00` (0)      | `x00` (0)      |
| Z    | `xFF1` (4081)| `xF00F` (-4081)| `xFC6` (4038)  |

#### Stick Parameters 1 & 2:
18 bytes that produce 12 uint16_t.
Define maximum/minimum ranges that the analog stick hardware supports and dead-zones.

Each section is for different stick.

| uint16_t # | Sample    | Remarks                   |
|:----------:|:---------:| ------------------------- |
| `0`, `1`   | `019 4CD` | Seems that this is unused |
| `2`        | `AE`      | Dead-zone                 |
| `3`        | `E14`     | Range ratio               |
| `4`, `5`   | `2EE 2EE` | X/Y: Unknown              |
| `6`, `7`   | `2EE 2EE` | X/Y: Unknown              |
| `8`, `9`   | `AB4 AB4` | X/Y: Unknown              |
| `10`, `11` | `496 496` | X/Y: Unknown              |

##### Dead-zone:

It is used to all directions, so it isn't divided by 2. It behaves like a radial dead-zone. Changing it as big as a half axis range, produces a circular d-pad style behavior. The default values for Joy-Con translates to ~15% and ~10% for Pro controller.

##### Range ratio:

Making this very small, produces d-pad like movement on the cross but still retains circular directionality. So it probably produces a float coefficient.
