# Bluetooth HID Information

## Output reports

### OUTPUT 0x01

Rumble and subcommand.

The OUTPUT 1 report is how all normal subcommands are sent. It also includes rumble data.

Sample C code for sending a subcommand:

```
uint8_t buf[0x40]; bzero(buf, 0x40);
buf[0] = 1; // 0x10 for rumble only
buf[1] = GlobalPacketNumber; // Increment by 1 for each packet sent. It loops in 0x0 - 0xF range.
memcpy(buf + 2, rumbleData, 8);
buf[10] = subcommandID;
memcpy(buf + 11, subcommandData, subcommandDataLen);
hid_write(handle, buf, 0x40);
```

You can send rumble data and subcommand with `x01` command, otherwise only rumble with `x10` command.

See "Rumble data" below.

### OUTPUT 0x03

NFC/IR MCU FW Update packet.

### OUTPUT 0x10

Rumble only. See OUTPUT 0x01 and "Rumble data" below.

### OUTPUT 0x11

Request specific data from the NFC/IR MCU. Can also send rumble.

### OUTPUT 0x12

Unknown. Does the same thing with 0x28 subcmd.

### Rumble data

A timing byte, then 4 bytes of rumble data for left Joy-Con, followed by 4 bytes for right Joy-Con.
[00 01 40 40 00 01 40 40] (320Hz 0.0f 160Hz 0.0f) is neutral.
The rumble data structure contains 2 bytes High Band data, 2 byte Low Band data.
The values for HF Band frequency and LF amplitude are encoded.

|  Byte #  |        Range                               | Remarks                                                                  |
|:--------:|:------------------------------------------:| ------------------------------------------------------------------------ |
| 0, 4     | `x04` - `xFC` (81.75Hz - 313.14Hz)         | High Band Lower Frequency. Steps `+0x0004`.                              |
| 0-1, 4-5 | `x00 01` - `xFC 01` (320.00Hz - 1252.57Hz) | Byte `1`,`5` LSB enables High Band Higher Frequency. Steps `+0x0400`.    |
| 1, 5     | `x00 00` - `xC8 00` (0.0f - 1.0f)          | High Band Amplitude. Steps `+0x0200`. Real max: `FE`.                    |
| 2, 6     | `x01` - `x7F` (40.87Hz - 626.28Hz)         | Low Band Frequency.                                                      |
| 3, 7     | `x40` - `x72` (0.0f - 1.0f)                | Low Band Amplitude. Safe max: `00 72`.                                   |
| 2-3, 6-7 | `x80 40` - `x80 71` (0.01f - 0.98f)        | Byte `2`,`6` +0x80 enables intermediate LF amplitude. Real max: `80 FF`. |
 
For a rumble values table, example and the algorithm for frequency, check rumble_data_table.md.

The byte values for frequency raise the frequency in Hz exponentially and not linearly.

Don't use real maximum values for Amplitude. Otherwise, they can damage the linear actuators.
These safe amplitude ranges are defined by Switch HID library.

## Input reports

### INPUT 0x3F

This input packet is pushed to the host when a button is pressed or released, and provides the "normal controller" interface for the OS.

|  Byte # |        Sample value        | Remarks                   |
|:-------:|:--------------------------:|:-------------------------:|
|  0      | `x3F`                      | Input report ID           |
|  1-2    | `x28 CA`                   | Button status             |
|  3      | `x08`                      | Stick hat data            |
|  4-11   | `x00 80 00 80 00 80 00 80` | Filler data               |

#### Stick hat data

Hold your controller sideways so that SL, SYNC, and SR line up with the screen. Pushing the stick towards a direction in this table will cause that value to be sent.

| SL | SYNC | SR |
| --:|:----:|:-- |
| 7  |  0   |  1 |
| 6  |  8   |  2 |
| 5  |  4   |  3 |

#### Button status format

| Byte | Bit `x01` | `x02`  |`x04`        |`x08`         |`x10`  |`x20`     |`x40`   |`x80`     |
|:----:|:---------:|:------:|:------------|:------------:|:-----:|:--------:|:------:|:--------:|
| 1    | Down      | Right  | Left        | Up           | SL    | SR       | --     | --       |
| 2    | Minus     | Plus   | Left Stick  | Right Stick  | Home  | Capture  | L / R  | ZL / ZR  |

### INPUT 0x21

Standard input reports used for subcommand replies.

### INPUT 0x23

NFC/IR MCU FW update input report.

### INPUT 0x30

Standard full mode - input reports with IMU data instead of subcommand replies. Pushes current state @60Hz, or @120Hz if Pro Controller.

### INPUT 0x31

NFC/IR MCU mode. Pushes large packets with standard input report + NFC/IR MCU data input report.

### INPUT 0x32

Unknown. Sends standard input reports.

### INPUT 0x33

Unknown. Sends standard input reports.

### Standard input report format

The 3rd byte belongs entirely to the Right Joy-Con, while the 5th byte belongs entirely to the Left Joy-Con.
The middle byte is shared between the controllers.

(Note: in the following table, the byte with the packet ID is included and located at byte "0".)

|   Byte #           |        Sample         | Remarks                                                                             |
|:------------------:|:---------------------:| ----------------------------------------------------------------------------------- |
| 0                  | `x21`, `x30`, `x31`   | Input report ID                                                                     |
| 1                  | `x00` - `xFF`         | Timer. Increments very fast. Can be used to estimate excess Bluetooth latency.      |
| 2 high nibble      | `0` - `9`             | Battery level. 8=full, 6=medium, 4=low, 2=critical, 0=empty. LSB=Charging.          |
| 2 low nibble       | `x0`, `x1`, `xE`      | Connection info. `(con_info >> 1) & 3` - 3=JC, 0=Pro/ChrGrip. `con_info & 1` - 1=Switch/USB powered. |
| 3, 4, 5            | `x41 00 82`           | Button status (see below table)                                                     |
| 6, 7, 8            | --                    | Left analog stick data                                                              |
| 9, 10, 11          | --                    | Right analog stick data                                                             |
| 12                 | `x70`, `xC0`, `xB0`   | Vibrator input report. Decides if next vibration pattern should be sent.            |
| 13  (ID `x21`)     | `x00`, `x80`, `x90`, `x82`| ACK byte for subcmd reply. ACK: MSB is `1`, NACK: MSB is `0`. If reply is ACK and has data, `byte12 & 0x7F` gives as the type of data. If simple ACK or NACK, the data type portion is `x00` |
| 14  (ID `x21`)     | `x02`, `x10`, `x03`   | Reply-to subcommand ID. The subcommand ID is used as-is.                            |
| 15-49  (ID `x21`)  | --                    | Subcommand reply data. Max 35 bytes (excludes 2 byte subcmd ack above).             |
| 13-49  (ID `x23`)  | --                    | NFC/IR MCU FW update input report. Max 37 bytes.                                    |
| 13-48  (ID `x30`, `x31`, `x32`, `x33`) | -- | 6-Axis data. 3 frames of 2 groups of 3 Int16LE each. Group is Acc followed by Gyro. |
| 49-361  (ID `x31`) | --                    | NFC/IR data input report. Max 313 bytes.                                            |

(Note2: In the `21` input reports, the byte13 (ACK byte) can be parsed as follows: `byte13 >> 7` tells us if it's an ACK or NACK. If it's an ACK, check `byte13 & 0x7F` to see what type of data it has. If it is a simple ACK, the byte13 is `x80` and thus the type of data is `x00`. If we expect a certain order of received packets, we can hardcode these byte13 values. If it's a NACK, the byte13 is always `x00`)

#### Standard input report - buttons
| Byte       | Bit `x01` | `x02` | `x04`    | `x08`    | `x10` | `x20`    | `x40` | `x80`         |
|:----------:|:---------:|:-----:|:--------:|:--------:|:-----:|:--------:|:-----:|:-------------:|
| 3 (Right)  | Y         | X     | B        | A        | SR    | SL       | R     | ZR            |
| 4 (Shared) | Minus     | Plus  | R Stick  | L Stick  | Home  | Capture  | --    | Charging Grip |
| 5 (Left)   | Down      | Up    | Right    | Left     | SR    | SL       | L     | ZL            |

Note that the button status of the L and R Joy-Cons can be ORed together to get a complete button status.

#### Standard input report - Stick data

The code below properly decodes the stick data:

```
uint8_t *data = packet + (left ? 6 : 9);
uint16_t stick_horizontal = data[0] | ((data[1] & 0xF) << 8);
uint16_t stick_vertical = (data[1] >> 4) | (data[2] << 4);
```

#### Standard input report - 6-Axis sensor data

See [here](imu_sensor_notes.md) for the 6-Axis sensor data format and conversion.

Also, these are **uncalibrated** stick/sensor data and must be converted to useful axes and values using the calibration data in the SPI flash.

See [here](spi_flash_dump_notes.md#analog-stick-factory-and-user-calibration) for the calibration data format.

## Feature reports

### FEATURE 0x02: Get last subcommand reply

[Send] feature Report

Buffer returned contains the latest 0x21 subcommand input report.

You must pass a buffer that can fit a 0x21 input report.

### FEATURE 0x70: Enable OTA FW upgrade

[Send] feature Report

Enables FW update. Unlocks Erase/Write memory commands.

The buffer sent must be exactly one byte. If else, Joy-Con rejects it.

The only possible ways to send it, is a Linux device with patched hidraw to accept 1 byte reports, directly through l2cap or a custom bluetooth development kit. 

| Byte # |  Sample   | Remarks                        |
|:------:|:---------:| ------------------------------ |
| 0      | `x70`     | Feature report ID              |

### FEATURE 0x71: Setup memory read

[Send] feature Report

Prepares the SPI Read report with the requested address and size.

| Byte # |  Sample        | Remarks                        |
|:------:|:--------------:| ------------------------------ |
| 0      | `x71`          | Feature report ID              |
| 1 - 4  | `xF4 1F 00 F8` | UInt32LE address               |
| 5 - 6  | `x08 00`       | UInt16LE size. Max xF9 bytes.  |
| 7      | `x7C`          | Checksum (8-bit 2s Complement) |

The checksum is calculated as `0x100 - Sum of Bytes`.

Memory map:

| Address #   |  Size    | Remarks                |
|:-----------:|:--------:| ---------------------- |
| `x00000000` | `xC8000` | ROM region 1 (800KB)   |
| `x000D0000` | `x10000` | RAM region 1 (64KB)    |
| `x00200000` | `x48000` | RAM region 2 (288KB)   |
| `x00260000` | `xC000`  | ROM region 2 (48KB)    |
| `xF8000000` | `x80000` | SPI (512KB, fully R/W) |

### FEATURE 0x72: Memory read

[Get] feature Report

| Byte # |  Sample   | Remarks                         |
|:------:|:---------:| ------------------------------- |
| 0      | `x72`     | Feature report ID               |
| 1      | `x8E`     | Checksum* (8-bit 2s Complement) |
| 2-EOF  |           |                                 |

*Checksum is optional.

In Get feature report mode it returns the 0x71 requested SPI or Rom data.

If the 0x71 command wasn't sent previously, it will return zeroed data (except ID and CRC).

The data returned has the following structure:

| Byte #  |  Sample        | Remarks                        |
|:-------:|:--------------:| ------------------------------ |
| 0       | `x74`          | Feature report ID              |
| 1 - 4   | `x00 80 02 F8` | UInt32LE address               |
| 5 - 6   | `xF9 00`       | UInt16LE size                  |
| 7-EOF-1 |                | Data requested                 |
| EOF     | `xDC`          | Checksum (8-bit 2s Complement) |

The returned size is header + size in 0x71 ft report + 1. So make sure to get your report with an adequate buffer size.

### FEATURE 0x73: Memory sector erase

[Send] feature Report

Erases specified sector in SPI. Can erase locked sectors.

Should be used only with SPI (0xF8000000 - 0xF807FFFF), because SPI needs to be erased before writing to it.

0x70 command must be sent before using this. Otherwise, Joy-Con will reply with invalid report ID.

| Byte # |  Sample        | Remarks                        |
|:------:|:--------------:| ------------------------------ |
| 0      | `x73`          | Feature report ID              |
| 1 - 4  | `x00 80 02 F8` | UInt32LE address               |
| 5 - 6  | `x00 10`       | UInt16LE size.                 |
| 7      | `x03`          | Checksum (8-bit 2s Complement) |

This command only checks `& 0x00FFF000` to acquire the sector number. Size is also irrelevant, but it's best to use values `x01 - x100`.

#### Warning:

This erases the whole sector. If you send xF35628F8 x0400, it will not erase 4bytes @x2856F3. It will erase the whole x285000 sector.

You need to read the sector, change the values you want and then erase and program them back.

### FEATURE 0x74: Memory write

[Send] feature Report

Writes to SPI. Can write locked sectors.

0x70 command must be sent before using this. Otherwise, Joy-Con will reply with invalid report ID.

| Byte #  |  Sample        | Remarks                        |
|:-------:|:--------------:| ------------------------------ |
| 0       | `x74`          | Feature report ID              |
| 1 - 4   | `x00 80 02 F8` | UInt32LE address               |
| 5 - 6   | `xF9 00`       | UInt16LE size. Max xF9 bytes.  |
| 7-EOF-1 |                | Data to write                  |
| EOF     | `xDC`          | Checksum (8-bit 2s Complement) |

#### Warning:

Check ft report x73 for info in erasing first.

### FEATURE 0x75: Launch (Reboot)

[Send] feature Report

Reboots and executes the firmware rom in the given address.

If address is `x0000` the Host should assume that the device will reboot.

0x70 command must be sent before using this. Otherwise, Joy-Con will reply with invalid report ID.

| Byte #  |  Sample        | Remarks                                  |
|:-------:|:--------------:| ---------------------------------------- |
| 0       | `x75`          | Feature report ID                        |
| 1 - 4   | `x00 80 02 F8` | UInt32LE entry address for firmware jump |
| 5 - 6   | `x04 00`       | UInt16LE size. Always 4.                 |
| 7       | `x00 80 02 F8` | UInt32LE entry address for firmware jump |
| 8       | `xDC`          | Checksum (8-bit 2s Complement)           |

Sending x75 00000000 0400 00000000 CRC will reboot the device and load the bootrom at 0x0. This is a good practice after finishing erasing/writing proccess.

Exchanging the SPI chip with a compatible one, but bigger size, user can use his own modified ROM firmware (must change addresses) and PatchRAM and launch it by using the above command to send the new address. Also, by modifying the initial PatchRAM in SPI at 0x0, you can create a similar logic that will launch the custom rom in every reboot without using x75 cmd. Additionally, you can add button scan to dual boot ROM firmware or PatchRAM.

### FEATURE 0xCC

[Send] feature Report

Unknown parameters needed

### FEATURE 0xFE

[Get] feature Report

Unknown parameters needed

## Subcommands

See [here](bluetooth_hid_subcommands_notes.md) for information about all subcommands supported.
