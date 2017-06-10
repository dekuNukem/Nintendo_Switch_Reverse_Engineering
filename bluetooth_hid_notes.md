

# Bluetooth HID Information

## INPUT 63

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

## OUTPUT 1

The OUTPUT 1 report requests the current controller status. It takes no data, but you need to include 1 (zeroed) byte of data with the report to comply with the spec.

```
uint8_t buf[2];
buf[0] = 1;
buf[1] = 0;
hid_write(handle, buf, 2);
```

## INPUT 33

The "Controller Status Report" packet is sent in reply to each OUTPUT 1 report.

Packet format:

|    Byte #    |        Sample value            | Remarks |
|:------------:|:------------------------------:|:-----:|
|   0          | `8D`, `A6`, `41` | Extremely fast timer |
|   1 high nibble  | `9`, `8`, `5`, `2`    | Battery level - 9=charging 8=full 2=low |
|   1 low nibble   | `E`              | Unknown |
| 2-4          | `25 02 00` | Button status, see below |
| 5-7          | `E3 56 9D` | Left stick data, see below |
| 8-10         | `DF 86 A4` | Right stick data, see below |
| 11           | `03`, `0B` | Unknown |
| 12           | `80`       | Terminator - subcommand reply follows. Removing the 0x80 bit gives the subcommand ID |
| 13-49        | Zero, garbage       | Filler |

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

## OUTPUT 16

Unknown.

## OUTPUT 17

Unknown.

## OUTPUT 18

Unknown.

## INPUT 48

Unknown.

## INPUT 49

Unknown.

## INPUT 50

Unknown.

## INPUT 51

Unknown.


## Subcommands

### Subcommand 0x01: Rumble data.

A timing byte, then 8 bytes of rumble data. [0 1 x40 x40 0 1 x40 x40] is neutral.

### Subcommand 0x03: Request input

Starts pushing input data at 60Hz.

### Subcommand 0x04: Invalid?

```
Request:
[01 .. .. .. .. .. .. .. .. 04]

Response: INPUT 21
[xx .E .. .. .. .. .. .. .. .. .. 0. 83 04]
```

### Subcommand 0x06: Disconnect

Causes the controller to disconnect the Bluetooth connection.

### Subcommand 0x10: SPI flash read.
Little-endian int32 address, int8 size.
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

### Subcommand 0x18

### Subcommand 0x30: Set player lights

First argument byte is a bitfield:

```
aaaa bbbb
     3210 - keep player light on
3210 - flash player light
```

On overrides flashing.

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

### Subcommand 0x50

Just replies with `[4E 06]` ?
