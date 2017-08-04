

# Bluetooth HID Information

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

| Byte | Bit | Value |
|:--:|:---:|:---:|
| 1 | `01` | Down |
| 1 | `02` | Right |
| 1 | `04` | Left |
| 1 | `08` | Up |
| 1 | `10` | SL |
| 1 | `20` | SR |
| 1 | `40` |  |
| 1 | `80` |  |
| 2 | `01` | Minus |
| 2 | `02` | Plus |
| 2 | `04` | Left Stick |
| 2 | `08` | Right Stick |
| 2 | `10` | Home |
| 2 | `20` | Capture |
| 2 | `40` | L / R |
| 2 | `80` | ZL / ZR |

## OUTPUT 0x01

The OUTPUT 1 report requests the current controller status. It takes no data, but you need to include 1 (zeroed) byte of data with the report to comply with the spec.

```
uint8_t buf[2];
buf[0] = 1;
buf[1] = 0;
hid_write(handle, buf, 2);
```

### Button status format

The 2nd byte belongs entirely to the Right Joy-Con, while the 4th byte belongs entirely to the Left Joy-Con.
The middle byte is shared between the controllers.

| Byte |    Bit     |        Value |
|:--:|:---:|:---:|
| R | `01` | Y |
| R | `02` | X |
| R | `04` | B |
| R | `08` | A |
| R | `10` | SR |
| R | `20` | SL |
| R | `40` | R |
| R | `80` | ZR |
| M | `01` | Minus |
| M | `02` | Plus |
| M | `04` | Right Stick |
| M | `08` | Left Stick |
| M | `10` | Home |
| M | `20` | Capture |
| M | `40` | -- |
| M | `80` | -- |
| L | `01` | Down |
| L | `02` | Up |
| L | `04` | Right |
| L | `08` | Left |
| L | `10` | SR |
| L | `20` | SL |
| L | `40` | L |
| L | `80` | ZL |

### Stick data

The closest estimate I have right now is:

```
uint8_t *data = packet + (left ? 5 : 8);
stick_horizontal = ((data[0] & 0xF0) >> 4) | ((data[1] & 0x0F) << 4);
stick_vertical = data[2];
```

Not quite sure what the other 2 nibbles are for.

Also, these are **uncalibrated** stick data. In an experiment, the console was able to grab the calibration data from a controller it'd never seen before, so that's probably available through a command using one of the 0x10, 0x11, or 0x12.

## OUTPUT 0x03

MCU FW Update packet

## OUTPUT 0x10

Unknown.

## OUTPUT 0x11

Command to MCU.

## OUTPUT 0x12

Unknown.

## INPUT 0x23

MCU update state report?

## INPUT 0x30

Standard full mode. Pushes current state @60Hz or @120Hz if Pro Controller.

## INPUT 0x31

NFC Mode. Pushes large Packets.

## INPUT 0x32

Unknown.

## INPUT 0x33

Unknown.


### Command 0x01 or 0x10: Rumble data

You can send rumble data and subcommand with `x01` command, otherwise only rumble with `x10` command.

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


## Subcommands

### Subcommand 0x00: Get Only Controller State

Replies with 2 bytes.

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

Replies with 7 little-endian uint16. The values are in 10ms.

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

### Subcommand 0x18

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

### Subcommand 0x30: Set player lights

First argument byte is a bitfield:

```
aaaa bbbb
     3210 - keep player light on
3210 - flash player light
```

On overrides flashing. When on USB, flashing bits work like always on bits.

### Subcommand 0x38: HOME Light

|    Nibble #    |        Sample value            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   High nibble  | `5` | Time OFF - Based on Time ON value. 0=always on |
|   Low nibble   | `E` | Time ON  - 0=OFF - F=200ms |

Time ON value starts has a base 16ms and increments by 12ms. 0 value disables the light

Time OFF value is a multiplier of the Time ON time (ms). 0 value keeps the light always on.

|    Time OFF value #    |        Multiplier             | Remarks |
|:------------:|:------------------------------:|:-----:|
|   `0` | 0 | Always ON |
|   `1` | 1 | Same as Time ON time |
|   `8`   | 21 ||
|   `F`   | 30 ||

The above behavior is for USB. In bluetooth it uses a PWM driver and acts as a breathing light, with slight different timing.

### Subcommand 0x40: Enable 6-Axis sensor

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x41: 6-Axis sensor configuration

Two arguments of one byte. LO byte takes `x00` to `x03`, `x00` is error in config data and also sets HI byte to `x00`. HI byte takes `x00` to `x02`, `x00` is error.

### Subcommand 0x42: 6-Axis sensor write

### Subcommand 0x48: Enable vibration

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x50

Just replies with `[4E 06]` ?

### Subcommand 0x70: BT OTA FW update?
