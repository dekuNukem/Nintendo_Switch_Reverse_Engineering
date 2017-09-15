

# Bluetooth HID Information

## OUTPUT 0x01

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

## OUTPUT 0x03

MCU FW Update packet

## OUTPUT 0x10

Rumble only. See OUTPUT 0x01 and "Rumble data" below.

## OUTPUT 0x11

Command to MCU.

## OUTPUT 0x12

Unknown.

## Rumble data

A timing byte, then 4 bytes of rumble data for left Joy-Con, followed by 4 bytes for right Joy-Con.
[00 01 40 40 00 01 40 40] (320Hz 0.0f 160Hz 0.0f) is neutral.
The rumble data structure contains 2 bytes High Band data, 2 byte Low Band data.
The values for HF Band frequency and LF amplitude are encoded.

|  Byte #  |        Range                             | Remarks |
|:--------:|:----------------------------------------:| ------------------------------------------------------------------------ |
| 0, 4     | `04` - `FC` (81.75Hz - 313.14Hz)         | High Band Lower Frequency. Steps `+0x0004`.                              |
| 0-1, 4-5 | `00 01` - `FC 01` (320.00Hz - 1252.57Hz) | Byte `1`,`5` LSB enables High Band Higher Frequency. Steps `+0x0400`.    |
| 1, 5     | `00 00` - `C8 00` (0.0f - 1.0f)          | High Band Amplitude. Steps `+0x0200`. Real max: `FE`.                    |
| 2, 6     | `01` - `7F` (40.87Hz - 626.28Hz)         | Low Band Frequency.                                                      |
| 3, 7     | `40` - `72` (0.0f - 1.0f)                | Low Band Amplitude. Safe max: `00 72`.                                   |
| 2-3, 6-7 | `80 40` - `80 71` (0.01f - 0.98f)        | Byte `2`,`6` +0x80 enables intermediate LF amplitude. Real max: `80 FF`. |
 
For a rumble values table, example and the algorithm for frequency, check rumble_data_table.md.

The byte values for frequency raise the frequency in Hz exponentially and not linearly.

Don't use real maximum values for Amplitude. Otherwise, they can damage the linear actuators.
These safe amplitude ranges are defined by Switch HID library.


## INPUT 0x3F

This input packet is pushed to the host when a button is pressed or released, and provides the "normal controller" interface for the OS.

|  Byte # |        Sample value       | Remarks                   |
|:-------:|:-------------------------:|:-------------------------:|
|  0      | `3F`                      | Header, same as report ID |
|  1-2    | `28 CA`                   | Button status             |
|  3      | `08`                      | Stick hat data            |
|  4-11   | `00 80 00 80 00 80 00 80` | Filler data               |

### Stick hat data

Hold your controller sideways so that SL, SYNC, and SR line up with the screen. Pushing the stick towards a direction in this table will cause that value to be sent.

| SL | SYNC | SR |
| --:|:----:|:-- |
| 7  |  0   |  1 |
| 6  |  8   |  2 |
| 5  |  4   |  3 |

### Button status format

| Byte | Bit `01` | `02`  |`04`        |`08`         |`10`  |`20`     |`40`   |`80`     |
|:----:|:--------:|:-----:|:-----------|:-----------:|:----:|:-------:|:-----:|:-------:|
| 1    | Down     | Right | Left       | Up          | SL   | SR      | --    | --      |
| 2    | Minus    | Plus  | Left Stick | Right Stick | Home | Capture | L / R | ZL / ZR |

## INPUT 0x21

Standard input reports used for subcommand replies.

## INPUT 0x23

MCU update state report?

## INPUT 0x30

Standard full mode - input reports with IMU data instead of subcommand replies. Pushes current state @60Hz, or @120Hz if Pro Controller.

If 6-axis sensor is enabled, the IMU data in an 0x30 input report is packaged like this (assuming the packet ID is located at byte -1):

| Byte       | Remarks                                                        |
|:----------:| -------------------------------------------------------------- |
|   12-13    | accel_x (Int16LE). This Axis is reversed in Left JC.           |
|   14-15    | accel_y (Int16LE)                                              |
|   16-17    | accel_z (Int16LE). This Axis is reversed in Right JC.          |
|   18-19    | gyro_1 (Int16LE)                                               |
|   20-21    | gyro_2 (Int16LE)                                               |
|   22-23    | gyro_3 (Int16LE)                                               |
|   24-47    | The data is repeated 2 more times. Each with 5ms Δt sampling.  |

The 6-Axis data is repeated 3 times. On Joy-con with a 15ms packet push, this is translated to 5ms difference sampling. E.g. 1st sample 0ms, 2nd 5ms, 3rd 10ms. Using all 3 samples let you have a 5ms precision instead of 15ms.

The axes are defined as follows:

```
_______
|      \
| Front |        +x +z
|   R   |    -y __|/__ +y
|       |        /|
|       |     -z -x
|______/

```
The following equation should scale an int16 IMU value into an acceleration vector component (measured in Gs):

`acc_vector_component = acc_raw_component * G_RANGE / SENSOR_RES / 1000`

where `G_RANGE` is the sensitivity range setting of the accelerometer, as explained [here](http://ozzmaker.com/accelerometer-to-g/).

The Joy-Con are ranged to ±8000 MilliGs (G_RANGE = 16000 MilliGs), the sensitivity calibration is always 16384 MilliGs and the SENSOR_RES is 16bit, so the above equation can be simplified to:

`acc_vector_component = acc_raw_component * 0.00025f`. (16384/65535/1000 = 0.00025)

For Gyro the equation to convert the values into angular velocity (measured in degrees per second):

`gyro_vector_component = gyro_raw_component * G_GAIN / SENSOR_RES / 1000`

where G_GAIN is the degrees per second sensitivity range.

The Joy-Con, based on their calibration, have a G_GAIN of  13371dps, so the above equation can be simplified to:

`gyro_vector_component = gyro_raw_component * 0.000204f`

The [SparkFun library for the LSM6DS3](https://github.com/sparkfun/SparkFun_LSM6DS3_Arduino_Library) will likely be a valuable resource for future IMU hacking.

## INPUT 0x31

NFC Mode. Pushes large packets with standard input reports and subcommand replies.

## INPUT 0x32

Unknown. Sends standard input reports.

## INPUT 0x33

Unknown. Sends standard input reports.

## Standard input report format

The 2nd byte belongs entirely to the Right Joy-Con, while the 4th byte belongs entirely to the Left Joy-Con.
The middle byte is shared between the controllers.

(Note: in the following table, the byte with the packet ID is cut off, located at byte "-1".)

|   Byte #           |        Sample         | Remarks                                                                        |
|:------------------:|:---------------------:|:------------------------------------------------------------------------------:|
| 0                  | `00` - `FF`           | Timer. Increments very fast. Can be used to estimate excess Bluetooth latency. |
| 1 high nibble      | `0` - `9`             | Battery level. 8=full, 6=medium, 4=low, 2=critical, 0=empty. LSB=Charging.     |
| 1 low nibble       | `xE`                  | Connection info. `(con_info >> 1) & 3` - 3=BT, 0=USB. `con_info & 0x1` - 1=Charging Grip.  |
| 2, 3, 4            | `41 00 82`            | Button status (see below table)                                                |
| 5, 6, 7            | --                    | Left analog stick data                                                         |
| 8, 9, 10           | --                    | Right analog stick data                                                        |
| 11                 | `00`, `80`            | ACK acknowledge subcommand reply                                               |
| 12                 | `90`, `82`, `B3`, etc | Reply-to subcommand ID. For packet 0x21, `x80` is added to the subcommand ID. For packet `x31` through `x33`, the subcommand ID is used as-is. |
| 13-39 (`x30` only) | --                    | 6-Axes data. 3 frames of 2 groups of 3 Int16LE each. Group is Acc followed by Gyro. |
| 13-EOF (Other)     | --                    | Subcommand reply data.                                                         |


### Standard input report - buttons
| Byte       | Bit `01` | `02` | `04`    | `08`    | `10` | `20`    | `40` | `80`          |
|:----------:|:--------:|:----:|:-------:|:-------:|:----:|:-------:|:----:|:-------------:|
| 2 (Right)  | Y        | X    | B       | A       | SR   | SL      | R    | ZR            |
| 3 (Shared) | Minus    | Plus | R Stick | L Stick | Home | Capture | --   | Charging Grip |
| 4 (Left)   | Down     | Up   | Right   | Left    | SR   | SL      | L    | ZL            |

Note that the button status of the L and R Joy-Cons can be ORed together to get a complete button status.

### Standard input report - Stick data

The code below properly decodes the stick data:

```
uint8_t *data = packet + (left ? 5 : 8);
uint16_t stick_horizontal = data[0] | ((data[1] & 0xF) << 8);
uint16_t stick_vertical = (data[1] >> 4) | (data[2] << 4);
```

Also, these are **uncalibrated** stick data and must be converted to useful axes using the calibration data in the SPI flash.

See [here](spi_flash_dump_notes.md#analog-stick-factory-and-user-calibration) for the calibration data format.

## Subcommands

### Subcommand 0x##: All unused subcommands

All subcommands that do nothing, reply back with ACK `x80` `x##` and `x03`

### Subcommand 0x00: Get Only Controller State

Replies with `x80` `x00` `x03`

Can be used to get Controller state only, like any subcommand that does nothing

### Subcommand 0x01 (With cmd 0x01 or 0x11): Bluetooth Pairing or Get MCU State

One argument with valid values of `x00` to `x04`.

With some trials, only `x01` and `x02` return some real data. With 1st one, the data is always the same. With 2nd seems random

This subcommand handles some of the BT pairing.

It happens once every BT host change. 

If the command is `x11`, it polls the MCU State? Used with IR Camera or NFC?

### Subcommand 0x02: Request device info

Response data after 02 command byte:

| Byte # | Sample              | Remarks                                           |
|:------:|:-------------------:| ------------------------------------------------- |
|  0-1   | `03 48`             | Firmware Version. Latest is 3.48                  |
|  2     | `01`                | 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller |
|  3     | `02`                | Unknown. Seems to be always 02                    |
|  4-9   | `57 30 EA 8A BB 7C` | Joy-Con MAC address 7C:BB:8A:EA:30:57             |
|  10-1  | `01 01`             | Unknown. Seems to be always 01 01                 |

### Subcommand 0x03: Set input report mode

One argument:

| Argument # | Remarks                                                                                          |
|:----------:| ------------------------------------------------------------------------------------------------ |
|   `00`     | Used with cmd `x11`. Active polling for IR camera data. 0x31 data format must be set first       |
|   `01`     | Same as `00`                                                                                     |
|   `02`     | Same as `00`. Active polling mode for IR camera data. Special IR mode or before configuring it?  |
|   `23`     | MCU update state report?                                                                         |
|   `30`     | Standard full mode. Pushes current state @60Hz                                                   |
|   `31`     | NFC/IR mode. Pushes large packets @60Hz                                                          |
|   `33`     | Unknown mode.                                                                                    |
|   `35`     | Unknown mode.                                                                                    |
|   `3F`     | Simple HID mode. Pushes updates with every button press                                          |

Starts pushing input data at 60Hz.

### Subcommand 0x04: Trigger buttons elapsed time

Replies with 7 little-endian uint16. The values are in 10ms. They reset by turning off the controller.

```
Left_trigger_ms = ((byte[1] << 8) | byte[0]) * 10;
```

| Bytes # | Remarks |
|:-------:|:-------:|
|   1-0   | L       |
|   3-2   | R       |
|   5-4   | ZL      |
|   7-6   | ZR      |
|   9-8   | SL      |
|   10-9  | SR      |
|   12-11 | HOME    |

### Subcommand 0x05: Get page

Replies a uint8 with a value of `x01`.

### Subcommand 0x06: Reset connection (Disconnect)

Causes the controller to disconnect the Bluetooth connection.

Takes as argument `x00` or `x01`.

### Subcommand 0x07: Factory reset

Does a factory reset and initialises the 0x2000 SPI region.

### Subcommand 0x08: Set shipment

Takes as argument `x00` or `x01`.

If `x01` it writes `x01` @`x5000` of SPI flash. With `x00`, it resets to `xFF` @`x5000`.

If `x01` is set, then Switch initiates pairing, if not, initializes connection with the device.

Switch always sends `x08 00` after every initialization.

### Subcommand 0x10: SPI flash read
Little-endian int32 address, int8 size, max size is `x1D`.
Subcommand reply echos the request info, followed by `size` bytes of data.

```
Request:
[01 .. .. .. .. .. .. .. .. .. 10 80 60 00 00 18]
                               ^ subcommand
                                  ^~~~~~~~~~~ address x6080
                                              ^ length = 0x18 bytes
Response: INPUT 21
[xx .E .. .. .. .. .. .. .. .. .. .. 0. 90 80 60 00 00 18 .. .. .. ....]
                                        ^ subcommand reply
                                              ^~~~~~~~~~~ address
                                                       ^ length = 0x18 bytes
                                                          ^~~~~ data
```

### Subcommand 0x11: SPI flash Write

Little-endian int32 address, int8 size. Max size `x1D` data to write.
Subcommand reply echos the address, size, plus a uint8 status. `x00` = success, `x01` = write protected.

### Subcommand 0x12: SPI sector erase

Takes a Little-endian uint32. Erases the whole 4KB in the specified address to 0xFF.
Subcommand reply echos the address, plus a uint8 status. `x00` = success, `x01` = write protected.

### Subcommand 0x20: MCU (Microcontroller for Sensors and Peripherals) reset

### Subcommand 0x21: Write configuration to MCU

Write configuration data to MCU. This data can be IR configuration, NFC configuration or data for the 512KB MCU firmware update.

### Subcommand 0x22: MCU Resume mode

Takes one argument:


| Argument # | Remarks           |
|:----------:| ----------------- |
|   `00`     | Suspend           |
|   `01`     | Resume            |
|   `02`     | Resume for update |

### Subcommand 0x28: Set unknown data?

Replies with ACK `x80` `x28`.

### Subcommand 0x29: Get `x28` data

Replies with ACK `xA8` `x29`. Sometimes these subcmd take arguments

### Subcommand 0x2A: Unknown

Replies with ACK `x00` `x2A`.

`x00` as an ACK, means no data. Some commands if you send wrong arguments reply with this.

### Subcommand 0x2B: Get `x29` data?

Replies with ACK `xA9` `x2B`. Normally these take an address and a size as an argument and give out data.

### Subcommand 0x30: Set player lights

First argument byte is a bitfield:

```
aaaa bbbb
     3210 - keep player light on
3210 - flash player light
```

On overrides flashing. When on USB, flashing bits work like always on bits.


### Subcommand 0x31: Get player lights

Replies with ACK `xB0` `x31` and one byte that uses the same bitfield with `x30` subcommand

`xB1` is the 4 leds trail effect. But it can't be set via `x30`.

### Subcommand 0x38: Set HOME Light

25 bytes argument that control 49 elements.

| Byte #, Nibble | Remarks                                                                  |
|:--------------:| ------------------------------------------------------------------------ |
| `x00`, High    | Number of Mini Cycles. 1-15. If number of cycles is > 0 then `x0` = `x1` |
| `x00`, Low     | Global Mini Cycle Duration. 8ms - 175ms. Value `x0` = 0ms/OFF            |
| `x01`, High    | LED Start Intensity. Value `x0`=0% - `xF`=100%                           |
| `x01`, Low     | Number of Full Cycles. 1-15. Value `x0` is repeat forever, but if also Byte `x00` High nibble is set to `x0`, it does the 1st Mini Cycle and then the LED stays on with LED Start Intensity. |

When all selected Mini Cycles play and then end, this is a full cycle.

The Mini Cycle configurations are grouped in two (except the 15th):

| Byte #, Nibble | Remarks                                                   |
|:--------------:| --------------------------------------------------------- |
| `x02`, High    | Mini Cycle 1 LED Intensity                                |
| `x02`, Low     | Mini Cycle 2 LED Intensity                                |
| `x03`, High    | Fading Transition Duration to Mini Cycle 1 (Uses PWM). Value is a Multiplier of Global Mini Cycle Duration |
| `x03`, Low     | LED Duration Multiplier of Mini Cycle 1. `x0` = `x1` = x1 |
| `x04`, High    | Fading Transition Duration to Mini Cycle 2 (Uses PWM). Value is a Multiplier of Global Mini Cycle Duration |
| `x04`, Low     | LED Duration Multiplier of Mini Cycle 1. `x0` = `x1` = x1 |

The Fading Transition uses a PWM to increment the transition to the Mini Cycle. 

The LED Duration Multiplier and the Fading Multiplier use the same algorithm: Global Mini Cycle Duration ms * Multiplier value. 

Example: GMCD is set to `xF` = 175ms and LED Duration Multiplier is set to `x4`. The Duration that the LED will stay on it's configured intensity is then  175 * 4 = 700ms.

Unused Mini Cycles can be skipped from the output packet.

Table of Mini Cycle configuration:

| Byte #, Nibble | Remarks                                   |
| -------------- | ----------------------------------------- |
| `x02`, High    | Mini Cycle 1 LED Intensity                |
| `x02`, Low     | Mini Cycle 2 LED Intensity                |
| `x03` High/Low | Fading/LED Duration Multipliers for MC 1  |
| `x04` High/Low | Fading/LED Duration Multipliers for MC 2  |
| `x05`, High    | Mini Cycle 3 LED Intensity                |
| `x05`, Low     | Mini Cycle 4 LED Intensity                |
| `x06` High/Low | Fading/LED Duration Multipliers for MC 3  |
| `x06` High/Low | Fading/LED Duration Multipliers for MC 4  |
| ...            | ...                                       |
| `x20`, High    | Mini Cycle 13 LED Intensity               |
| `x20`, Low     | Mini Cycle 14 LED Intensity               |
| `x21` High/Low | Fading/LED Duration Multipliers for MC 13 |
| `x22` High/Low | Fading/LED Duration Multipliers for MC 14 |
| `x23`, High    | Mini Cycle 15 LED Intensity               |
| `x23`, Low     | Unused                                    |
| `x24` High/Low | Fading/LED Duration Multipliers for MC 15 |

### Subcommand 0x40: Enable 6-Axis sensor

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x41: 6-Axis sensor configuration

Two arguments of one byte. LO byte takes `x00` to `x03`, `x00` is error in config data and also sets HI byte to `x00`. HI byte takes `x00` to `x02`, `x00` is error.

### Subcommand 0x42: 6-Axis sensor write

### Subcommand 0x43: Get 6-Axis sensor data?

It takes a 2 byte argument.

Replies with ACK `xC0` `x43`.

If the 2 bytes exceed `x12` and `x0E` accordingly, replies with `x00` `x43`: no data returned or out of range.

### Subcommand 0x48: Enable vibration

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x50: Get regulated voltage

Replies with ACK `xD0` `x50` and a little-endian uint16. Raises when charging a Joy-Con.

|   Range #            |   Range mV  | Reported battery |
|:--------------------:|:-----------:| ---------------- |
|   `x052B` - `x059F`  | 1323 - 1439 | 2 - Critical     |
|   `x05A0` - `x05DF`  | 1440 - 1503 | 4 - Low          |
|   `x05E0` - `x0617`  | 1504 - 1559 | 6 - Medium       |
|   `x0618` - `x0690`  | 1560 - 1680 | 8 - Full         |

Tests showed charging stops at around 1680mV and the controller turns off at arround 1323mV.

### Subcommand 0x51: Set unknown data. Connection status?

Replies with ACK `x80` `x51`.

It takes a uint8. Valid values are 0x00, 0x04, 0x10, 0x14. Other values result to these in groups of 4.

E.g. 0x0->0x3 = 0x0, 0x4->0x7,0xC-0xF = 0x4, 0x8->0xB = 0x0, 0x10-0x13 = 0x10 and so on.

### Subcommand 0x52: Get 0x51 unknown data

Replies with ACK `xD1` `x52` and a uint8. 

If the joy-cons are connected to a charging grip, the reply is `x17`. If you remove it from it, changes to `x14`.

If you only connect or pair it from sleep mode, the reply is `x04`.
