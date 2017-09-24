# Subcommands

## Subcommand 0x##: All unused subcommands

All subcommands that do nothing, reply back with ACK `x80` `x##` and `x03`

## Subcommand 0x00: Get Only Controller State

Replies with `x80` `x00` `x03`

Can be used to get Controller state only (w/o 6-Axis sensor data), like any subcommand that does nothing

## Subcommand 0x01 (With cmd 0x01 or 0x11): Bluetooth Pairing or Get MCU State

One argument with valid values of `x00` to `x04`.

With some trials, only `x01` and `x02` return some real data. With 1st one, the data is always the same. With 2nd seems random

This subcommand handles some of the BT pairing.

It happens once every BT host change. 

If the command is `x11`, it polls the MCU State? Used with IR Camera or NFC?

## Subcommand 0x02: Request device info

Response data after 02 command byte:

| Byte # | Sample              | Remarks                                                  |
|:------:|:-------------------:| -------------------------------------------------------- |
|  0-1   | `03 48`             | Firmware Version. Latest is 3.48                         |
|  2     | `01`                | 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller.       |
|  3     | `02`                | Unknown. Seems to be always `02`                         |
|  4-9   | `7C BB 8A EA 30 57` | Joy-Con MAC address in Big Endian                        |
|  10    | `01`                | Unknown. Seems to be always `01`                         |
|  11    | `01`                | If `01`, colors in SPI are used. Otherwise default ones. |

## Subcommand 0x03: Set input report mode

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

## Subcommand 0x04: Trigger buttons elapsed time

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

## Subcommand 0x05: Get page

Replies a uint8 with a value of `x01`.

## Subcommand 0x06: Reset connection (Disconnect)

Causes the controller to disconnect the Bluetooth connection.

Takes as argument `x00` or `x01`.

## Subcommand 0x07: Reset pairing info

Initializes the 0x2000 SPI section.

## Subcommand 0x08: Set shipment

Takes as argument `x00` or `x01`.

If `x01` it writes `x01` @`x5000` of SPI flash. With `x00`, it resets to `xFF` @`x5000`.

If `x01` is set, then Switch initiates pairing, if not, initializes connection with the device.

Switch always sends `x08 00` after every initialization.

## Subcommand 0x10: SPI flash read
Little-endian int32 address, int8 size, max size is `x1D`.
Subcommand reply echoes the request info, followed by `size` bytes of data.

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

## Subcommand 0x11: SPI flash Write

Little-endian int32 address, int8 size. Max size `x1D` data to write.
Subcommand reply echoes the address, size, plus a uint8 status. `x00` = success, `x01` = write protected.

## Subcommand 0x12: SPI sector erase

Takes a Little-endian uint32. Erases the whole 4KB in the specified address to 0xFF.
Subcommand reply echos the address, plus a uint8 status. `x00` = success, `x01` = write protected.

## Subcommand 0x20: MCU (Micro-controller for Sensors and Peripherals) reset

## Subcommand 0x21: Write configuration to MCU

Write configuration data to MCU. This data can be IR configuration, NFC configuration or data for the 512KB MCU firmware update.

## Subcommand 0x22: MCU Resume mode

Takes one argument:


| Argument # | Remarks           |
|:----------:| ----------------- |
|   `00`     | Suspend           |
|   `01`     | Resume            |
|   `02`     | Resume for update |

## Subcommand 0x28: Set unknown data?

Replies with ACK `x80` `x28`.

## Subcommand 0x29: Get `x28` data

Replies with ACK `xA8` `x29`. Sometimes these subcmd take arguments

## Subcommand 0x2A: Unknown

Replies with ACK `x00` `x2A`.

`x00` as an ACK, means no data. Some commands if you send wrong arguments reply with this.

## Subcommand 0x2B: Get `x29` data?

Replies with ACK `xA9` `x2B`. Normally these take an address and a size as an argument and give out data.

## Subcommand 0x30: Set player lights

First argument byte is a bitfield:

```
aaaa bbbb
     3210 - keep player light on
3210 - flash player light
```

On overrides flashing. When on USB, flashing bits work like always on bits.


## Subcommand 0x31: Get player lights

Replies with ACK `xB0` `x31` and one byte that uses the same bitfield with `x30` subcommand

`xB1` is the 4 leds trail effect. But it can't be set via `x30`.

## Subcommand 0x38: Set HOME Light

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

## Subcommand 0x40: Enable 6-Axis sensor

One argument of `x00` Disable  or `x01` Enable.

## Subcommand 0x41: Set 6-Axis sensor sensitivity

Sets the 6-axis sensor sensitivity for accelerometer and gyroscope.

Sending x40 x01 (IMU enable) resets your configuration to default ±8G / 2000dps.

Gyroscope sensitivity (Byte 0):

| Arg # | Remarks  |
|:-----:|:--------:|
| `00`  | ±250dps  |
| `01`  | ±500dps  |
| `02`  | ±1000dps |
| `03`  | ±2000dps |

Accelerometer sensitivity (Byte 1):

| Arg # | Remarks |
|:-----:|:-------:|
| `00`  | ±8G     |
| `01`  | ±4G     |
| `02`  | ±2G     |
| `03`  | ±16G    |

## Subcommand 0x42: 6-Axis sensor write

## Subcommand 0x43: Get all 6-Axis sensor values and configuration

It takes 2 uint8t_t.

To view all bytes send `xF20` for the first page and `x2F20` for the second.

The values you can check are acc, gyro, configuration, registers and many more.

| Byte # | Remarks                          |
|:------:| -------------------------------- |
| `00`   | Offset of values                 |
| `01`   | How many values to show. Max x20 |

## Subcommand 0x48: Enable vibration

One argument of `x00` Disable  or `x01` Enable.

## Subcommand 0x50: Get regulated voltage

Replies with ACK `xD0` `x50` and a little-endian uint16. Raises when charging a Joy-Con.

|   Range #            |   Range mV  | Reported battery |
|:--------------------:|:-----------:| ---------------- |
|   `x052B` - `x059F`  | 1323 - 1439 | 2 - Critical     |
|   `x05A0` - `x05DF`  | 1440 - 1503 | 4 - Low          |
|   `x05E0` - `x0617`  | 1504 - 1559 | 6 - Medium       |
|   `x0618` - `x0690`  | 1560 - 1680 | 8 - Full         |

Tests showed charging stops at around 1680mV and the controller turns off at around 1323mV.

## Subcommand 0x51: Set unknown data. Connection status?

Replies with ACK `x80` `x51`.

It takes a uint8. Valid values are 0x00, 0x04, 0x10, 0x14. Other values result to these in groups of 4.

E.g. 0x0->0x3 = 0x0, 0x4->0x7,0xC-0xF = 0x4, 0x8->0xB = 0x0, 0x10-0x13 = 0x10 and so on.

## Subcommand 0x52: Get 0x51 unknown data

Replies with ACK `xD1` `x52` and a uint8. 

If the joy-cons are connected to a charging grip, the reply is `x17`. If you remove it from it, changes to `x14`.

If you only connect or pair it from sleep mode, the reply is `x04`.
