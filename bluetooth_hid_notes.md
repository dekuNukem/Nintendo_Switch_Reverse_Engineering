

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
|   0          | `8D`, `A6`, `41` | Unknown, changes rapidly (checksum?)
|   1 high nibble  | `8`, `6`, `5`, `2`    | Battery level |
|   1 low nibble   | `E`              | Unknown |
| 2-4          | `25 02 00` | Button status, see below |
| 5-7          | `E3 56 9D` | Left stick data, see below |
| 8-10         | `DF 86 A4` | Right stick data, see below |
| 11-14        | `03 80 00 03` | Unknown |
| 15-49        | Zero       | Unknown |


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
