

# Bluetooth HID Information

## OUTPUT 0x01

Rumble and subcommand.

The OUTPUT 1 report is how all normal subcommands are sent. It also includes rumble data.

Sample C code for sending a subcommand:

```
uint8_t buf[0x40]; bzero(buf, 0x40);
buf[0] = 1; // 0x10 for rumble only
buf[1] = rumbleTimer; // Increment by 1 for each rumble frame.
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

### Rumble data

A timing byte, then 4 bytes of rumble data for left Joy-Con, followed by 4 bytes for right Joy-Con.
[00 01 40 40 00 01 40 40] (320Hz 0.0f 160Hz 0.0f) is neutral.
The rumble data structure contains 2 bytes High Band data, 2 byte Low Band data.
The values for HF Band frequency and LF amplitude are encoded.

|   Byte #   |        Range            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   0, 4 | `04` - `FC` (81.75Hz - 313.14Hz) | High Band Lower Frequency. Steps `+0x0004`. |
|   0-1, 4-5 | `00 01` - `FC 01` (320.00Hz - 1252.57Hz) | Byte `1`,`5` LSB enables High Band Higher Frequency. Steps `+0x0400`. |
|   1, 5 | `00 00` - `C8 00` (0.0f - 1.0f) | High Band Amplitude. Steps `+0x0200`. Real max: `FE`. |
|   2, 6 | `01` - `7F` (40.87Hz - 626.28Hz) | Low Band Frequency. |
|   3, 7  | `40` - `72` (0.0f - 1.0f) | Low Band Amplitude. Safe max: `00 72`. |
|   2-3, 6-7 | `80 40` - `80 71` (0.01f - 0.98f) | Byte `2`,`6` +0x80 enables intermediate LF amplitude. Real max: `80 FF`. |

For a rumble values table, example and the algorithm for frequency, check rumble_data_table.md.

The byte values for frequency raise the frequency in Hz exponentially and not linearly.

Don't use real maximum values for Amplitude. Otherwise, they can damage the linear actuators.
These safe amplitude ranges are defined by Switch HID library.


## INPUT 0x3F

This input packet is pushed to the host when a button is pressed or released, and provides the "normal controller" interface for the OS.

|    Byte #    |        Sample value            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   0          | `3F` | Header, same as report ID |
|   1-2        | `28 CA` | Button status |
|   3          | `08` | Stick hat data |
|   4-11       | `00 80 00 80 00 80 00 80` | Filler data |

### Stick hat data

Hold your controller sideways so that SL, SYNC, and SR line up with the screen. Pushing the stick towards a direction in this table will cause that value to be sent.

| SL | SYNC | SR |
| --:|:----:|:-- |
| 7  |  0   |  1 |
| 6  |  8   |  2 |
| 5  |  4   |  3 |

### Button status format

| Byte | Bit `01` | `02` |`04`|`08`|`10`|`20`|`40`|`80`|
|:--:|:---:|:---:|:------|:--:|:--:|:--:|:--:|:--:|:--:|
| 1 |     Down | Right | Left | Up | SL | SR | -- | -- |
| 2 | Minus | Plus | Left Stick | Right Stick | Home | Capture | L / R | ZL / ZR |

## INPUT 0x21

Standard input reports used for subcommand replies when the current mode is set to 0x30.

## INPUT 0x23

MCU update state report?

## INPUT 0x30

Standard full mode - input reports with IMU data instead of subcommand replies. Pushes current state @60Hz, or @120Hz if Pro Controller.

## INPUT 0x31

NFC Mode. Pushes large packets with standard input reports and subcommand replies.

## INPUT 0x32

Unknown. Sends standard input reports.

## INPUT 0x33

Unknown. Sends standard input reports.

### Standard input report format

The 2nd byte belongs entirely to the Right Joy-Con, while the 4th byte belongs entirely to the Left Joy-Con.
The middle byte is shared between the controllers.

(Note: in the following table, the byte with the packet ID is cut off, located at byte "-1".)

|   Byte #   |        Sample            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   0        | `00` - `FF`      | Timer. Increments very fast. Can be used to estimate excess Bluetooth latency. |
|   1 high nibble | `1` - `9`   | Battery level. 1 is low, 8 is full, and 9 is charging. |
| 1 low nibble | `xE` | Unknown. |
| 2, 3, 4 | `41 00 82` | Button status (see below table) |
| 5, 6, 7 | -- | Left analog stick data |
| 8, 9, 10 | -- | Right analog stick data |
| 11 | `00`, `80` | ACK acknowledge subcommand reply |
| 12 | `90`, `82`, `B3`, etc | Reply-to subcommand ID. For packet 0x21, `x80` is added to the subcommand ID. For packet `x31` through `x33`, the subcommand ID is used as-is. |
| 13-39 (`x30` only) | -- | Gyroscope data. 3 frames of 6 Int16LE each. |
| 13-EOF (Other) | -- | Subcommand reply data. |


### Standard input report - buttons
| Byte | Bit `01` | `02` | `04` | `08` | `10` | `20` | `40` | `80` |
|:--:|:---:|:---:|:------|:--:|:--:|:--:|:--:|:--:|:--:|
| 2 (Right) | Y | X | B | A | SR | SL | R | ZR |
| 3 (Shared) | Minus | Plus | R Stick | L Stick | Home | Capture | -- | -- |
| 4 (Left) | Down | Up | Right | Left | SR | SL | L | ZL |

Note that the button status of the L and R Joy-Cons can be ORed together to get a complete button status.

### Standard input report - Stick data

The closest estimate I have right now is:

```
uint8_t *data = packet + (left ? 5 : 8);
stick_horizontal = ((data[0] & 0xF0) >> 4) | ((data[1] & 0x0F) << 4);
stick_vertical = data[2];
```

Not quite sure what the other 2 nibbles are for.

Also, these are **uncalibrated** stick data. The location of the calibration data in SPI flash is known, but not the formula for converting that to calibrated axes.


## Subcommands

### Subcommand 0x##: All unused subcommands

All subcommands that do nothing, reply back with ACK `x80` `x##` and `x03`

### Subcommand 0x00: Get Only Controller State

Replies with `x03`

Can be used to get Controller state only, like any subcommand that does nothing

### Subcommand 0x01 (With cmd 0x01 or 0x11): Bluetooth Pairing or Get MCU State

One argument with valid values of `x00` to `x04`

This command handles some of the BT pairing. It sends `x04` or `x01` for pairing and then `x02` or `x03` to handle pairing.

It happens once every BT host change. 

If the command is `x11`, it polls the MCU State. Used with IR Camera or NFC?

### Subcommand 0x02: Request device info

Response data after 02 command byte:

|   Byte #   |        Sample            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   0-1  | `03 48` | Firmware Version. Latest is 3.48 |
|   2  | `01` | 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller |
|   3  | `02` | Unknown. Seems to be always 02 |
|   4-9  | `57 30 EA 8A BB 7C` | Joy-Con MAC address 7C:BB:8A:EA:30:57 |
|   10-1  | `01 01` | Unknown. Seems to be always 01 01 |

### Subcommand 0x03: Set input report mode

One argument:

|   Argument #   | Remarks |
|:------------:|:-----:|
|   `00`  | Used with cmd `x11`. Active polling mode for IR camera data. Answers with more than 300 bytes ID 31 packet |
|   `01`  | Active polling mode |
|   `02`  | Active polling mode for IR camera data. Special IR mode or before configuring it? |
|   `23`  | MCU update state report? |
|   `30`  | Standard full mode. Pushes current state @60Hz |
|   `31`  | NFC mode. Pushes large packets @60Hz |
|   `33`  | Unknown mode, WIP |
|   `35`  | Unknown mode, WIP |
|   `3F`  | Simple HID mode. Pushes updates with every button press |

Starts pushing input data at 60Hz.

### Subcommand 0x04: Trigger buttons elapsed time

Replies with 7 little-endian uint16. The values are in 10ms. They reset by turning off the controller.

```
Left_trigger_ms = ((byte[1] << 8) | byte[0]) * 10;
```

|   Bytes #   | Remarks |
|:------------:|:-----:|
|   1-0  | L |
|   3-2  | R |
|   5-4  | ZL |
|   7-6  | ZR |
|   9-8  | SL |
|   10-9  | SR |
|   12-11  | HOME |

### Subcommand 0x05: Get page

Replies a uint8 with a value of `x01`.

### Subcommand 0x06: Reset connection (Disconnect)

Causes the controller to disconnect the Bluetooth connection.

Takes as argument `x00` or `x01`.

### Subcommand 0x07: Unknown

Replies with ACK `x80` `x07`.

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
[01 .. .. .. .. .. .. .. .. 10 80 60 00 00 18]
                            ^ subcommand
                               ^~~~~~~~~~~ address x6080
                                           ^ length = 0x18 bytes
Response: INPUT 21
[xx .E .. .. .. .. .. .. .. .. .. 0. 90 80 60 00 00 18 .. .. .. ....]
                                     ^ subcommand reply
                                        ^~~~~~~~~~~ address
                                                    ^ length = 0x18 bytes
                                                       ^~~~~ data
```

### Subcommand 0x11: SPI flash Write

Little-endian int32 address, int8 size. Max size `x1D` data to write.
Subcommand reply echos the request info.

### Subcommand 0x12: SPI sector erase

Erases a 4KB sector.

### Subcommand 0x20: MCU (Microcontroller for Sensors and Peripherals) reset

### Subcommand 0x21: Write configuration to MCU

Write configuration data to MCU. This data can be IR configuration, NFC configuration or data for the 512KB MCU firmware update.

### Subcommand 0x22: MCU Resume mode

Takes one argument:


|   Argument #   | Remarks |
|:------------:|:-----:|
|   `00`  | Suspends |
|   `01`  | Resume |
|   `02`  | Resume for update |

### Subcommand 0x28: Set unknown data?

Replies with ACK `x80` `x28`.

### Subcommand 0x29: Get `x28` data and Set unknown data?

Replies with ACK `xA8` `x29`. Normally these take an address and a size as an argument and give out data.

### Subcommand 0x2A: Unknown

Replies with ACK `x00` `x2A`.
The only subcommand that does not have the ACK MSB (`x80`).

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

### Subcommand 0x38: Set HOME Light

25 bytes argument that control 49 elements.

|   Byte #, Nibble #    | Remarks |
|:------------:|:-----:|
|   Byte `x00`, High nibble  | Number of Mini Cycles. 1-15. If number of cycles is > 0 then `x0` = `x1` |
|   Byte `x00`, Low nibble   | Global Mini Cycle Duration. 8ms - 175ms. Value `x0` = 0ms/OFF  |
|   Byte `x01`, High nibble  | LED Start Intensity. Value `x0`=0% - `xF`=100% |
|   Byte `x01`, Low nibble   | Number of Full Cycles. 1-15. Value `x0` is repeat forever, but if also Byte `x00` High nibble is set to `x0`, it does the 1st Mini Cycle and then the LED stays on with LED Start Intensity. |

When all selected Mini Cycles play and then end, this is a full cycle.

The Mini Cycle configurations are grouped in two (except the 15th):

|   Byte #, Nibble #    | Remarks |
|:------------:|:-----:|
|   Byte `x02`, High nibble  | Mini Cycle 1 LED Intensity |
|   Byte `x02`, Low nibble   | Mini Cycle 2 LED Intensity  |
|   Byte `x03`, High nibble  | Fading Transition Duration to Mini Cycle 1 (Uses PWM). Value is a Multiplier of Global Mini Cycle Duration |
|   Byte `x03`, Low nibble   | LED Duration Multiplier of Mini Cycle 1. `x0` = `x1` = x1|
|   Byte `x04`, High nibble  | Fading Transition Duration to Mini Cycle 2 (Uses PWM). Value is a Multiplier of Global Mini Cycle Duration |
|   Byte `x04`, Low nibble   | LED Duration Multiplier of Mini Cycle 1. `x0` = `x1` = x1|

The Fading Transition uses a PWM to increment the transition to the Mini Cycle. 

The LED Duration Multiplier and the Fading Multiplier use the same algorithm: Global Mini Cycle Duration ms * Multiplier value. 

Example: GMCD is set to `xF` = 175ms and LED Duration Multiplier is set to `x4`. The Duration that the LED will stay on it's configured intensity is then  175 * 4 = 700ms.

Unused Mini Cycles can be skipped from the output packet.

Table of Mini Cycle configuration:

|   Byte #, Nibble #    | Remarks |
|:------------:|:-----:|
|   Byte `x02`, High nibble  | Mini Cycle 1 LED Intensity |
|   Byte `x02`, Low nibble   | Mini Cycle 2 LED Intensity  |
|   Byte `x03`  | Fading/LED Duration Multipliers for MC 1 |
|   Byte `x04`  | Fading/LED Duration Multipliers for MC 2 |
|   Byte `x05`, High nibble  | Mini Cycle 3 LED Intensity |
|   Byte `x05`, Low nibble   | Mini Cycle 4 LED Intensity  |
|   Byte `x06`  | Fading/LED Duration Multipliers for MC 3 |
|   Byte `x06`  | Fading/LED Duration Multipliers for MC 4 |
|   ...  | ... |
|   Byte `x20`, High nibble  | Mini Cycle 13 LED Intensity |
|   Byte `x20`, Low nibble   | Mini Cycle 14 LED Intensity  |
|   Byte `x21`  | Fading/LED Duration Multipliers for MC 13 |
|   Byte `x22`  | Fading/LED Duration Multipliers for MC 14 |
|   Byte `x23`, High nibble  | Mini Cycle 15 LED Intensity |
|   Byte `x23`, Low nibble   | Unused  |
|   Byte `x24`  | Fading/LED Duration Multipliers for MC 15 |

### Subcommand 0x40: Enable 6-Axis sensor

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x41: 6-Axis sensor configuration

Two arguments of one byte. LO byte takes `x00` to `x03`, `x00` is error in config data and also sets HI byte to `x00`. HI byte takes `x00` to `x02`, `x00` is error.

### Subcommand 0x42: 6-Axis sensor write

### Subcommand 0x43: Get 6-Axis sensor data?

Replies with ACK `xC0` `x43`. Normally these take an address and a size as an argument and give out data.

### Subcommand 0x48: Enable vibration

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x50: Set/get unknown data?

Replies with ACK `x80` `x50` and 2bytes `[4E 06]` or `[B5 05]` and maybe other.

### Subcommand 0x51: Unknown

Replies with ACK `x80` `x51`.

### Subcommand 0x52: Unknown

Replies with ACK `xD1` `x52` and one byte. Probably sets the data that you can get with `x51` subcmd.
