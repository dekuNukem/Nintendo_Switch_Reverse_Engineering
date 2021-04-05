# Subcommands

### Subcommand 0x##: All unused subcommands

All subcommands that do nothing, reply back with ACK `x80##` and `x03`

### Subcommand 0x00: Get Only Controller State

Replies with `x8000` `x03`

Does nothing actually, but can be used to get Controller state only (w/o 6-Axis sensor data), like any subcommand that does nothing.

### Subcommand 0x01: Bluetooth manual pairing

Takes max 2 arguments. A Pair request type uint8 and Host BD_ADDR 6 bytes in LE.

When used with cmd x01 it handles pairing. It is especially useful to change on the fly the pairing info for the next session, or to bluetooth pair through wired (rail/usb) connection.

The procedure must be done sequentially:

- 1: x01 x01 [{BD_ADDR_LE}] (Send host MAC and acquire Joy-Con MAC)
- 2: x01 x02 (Acquire the XORed LTK hash)
- 3: x01 x03 (saves pairing info in Joy-Con)

Host Pair request x01 (send HOST BT MAC and request Joy-Con BT MAC):

| Byte # | Sample               | Remarks                                 |
|:------:|:--------------------:| --------------------------------------- |
|  0     | `x01`                | subcmd                                  |
|  1     | `x01`                | Pair request type                       |
|  2-7   | `x16 30 AA 82 BB 98` | Host Bluetooth address in Little-Endian |

Joy-Con Pair request x01 reply:

| Byte # | Sample               | Remarks                         |
|:------:|:--------------------:| ------------------------------- |
|  0     | `x01`                | Pair request type               |
|  1-6   | `x57 30 EA 8A BB 7C` | Joy-Con BT MAC in Little-Endian |
|  7-31  |                      | Descriptor?                     |

Host Pair request x03 (request LTK):

Joy-Con Pair request x02 reply:

Long Term Key (LTK) in Little-Endian. Each byte is XORed with 0xAA.

Host Pair request x03:

Joy-Con saves pairing info in x2000 SPI region.

If the command is `x11`, it polls the MCU State? Used with IR Camera or NFC?

### Subcommand 0x02: Request device info

Response data after 02 command byte:

| Byte # | Sample               | Remarks                                                  |
|:------:|:--------------------:| -------------------------------------------------------- |
|  0-1   | `x03 48`             | Firmware Version. Latest is 4.07 (from 5.0.0 and up).    |
|  2     | `x01`                | 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller.       |
|  3     | `x02`                | Unknown. Seems to be always `02`                         |
|  4-9   | `x7C BB 8A EA 30 57` | Joy-Con MAC address in Big Endian                        |
|  10    | `x01`                | Unknown. Seems to be always `01`                         |
|  11    | `x01`                | If `01`, colors in SPI are used. Otherwise default ones. |

### Subcommand 0x03: Set input report mode

One argument:

| Arg #  | Remarks                                                                                          |
|:------:| ------------------------------------------------------------------------------------------------ |
|  `x00` | Used with cmd `x11`. Active polling for NFC/IR camera data. 0x31 data format must be set first.  |
|  `x01` | Same as `00`. Active polling mode for NFC/IR MCU configuration data.                             |
|  `x02` | Same as `00`. Active polling mode for NFC/IR data and configuration. For specific NFC/IR modes   |
|  `x03` | Same as `00`. Active polling mode for IR camera data. For specific IR modes                      |
|  `x23` | MCU update state report?                                                                         |
|  `x30` | Standard full mode. Pushes current state @60Hz                                                   |
|  `x31` | NFC/IR mode. Pushes large packets @60Hz                                                          |
|  `x33` | Unknown mode.                                                                                    |
|  `x35` | Unknown mode.                                                                                    |
|  `x3F` | Simple HID mode. Pushes updates with every button press                                          |

`x31` input report has all zeroes for IR/NFC data if a `11` ouput report with subcmd `03 00/01/02/03` was not sent before.

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

### Subcommand 0x05: Get page list state

Replies a uint8 with a value of `x01` if there's a Host list with BD addresses/link keys in memory.

### Subcommand 0x06: Set HCI state (disconnect/page/pair/turn off)

Causes the controller to change power state.

Takes as argument a uint8_t:

| Arg value # | Remarks                                       |
|:-----------:| --------------------------------------------- |
|   `x00`     | Disconnect (sleep mode / page scan mode)      |
|   `x01`     | Reboot and Reconnect (page mode)              |
|   `x02`     | Reboot and enter Pair mode (discoverable)     |
|   `x04`     | Reboot and Reconnect (page mode / HOME mode?) |

Option `x01`: It does a reboot and tries to reconnect. If no host is found, it goes into sleep.

Option `x02`: If some time passes without a pairing, or pressing a button, the controller connects to the last active BD_ADDR.

Option `x04`: It does a reboot and tries to reconnect. If no host is found, it goes into sleep. It probably does more, as this mode is calle HOME mode.

Page mode (x01, x04) is not to be confused with page scan mode. It's like pressing a button to reconnect, but it does it automatically.

All extra modes default to sleep mode if nothing happens. This is a R1 page scan mode which can accept a request from host (as seen with the "Search for Controllers" option).

### Subcommand 0x07: Reset pairing info

Initializes the 0x2000 SPI section.

### Subcommand 0x08: Set shipment low power state

Takes as argument `x00` or `x01`.

If `x01` it writes `x01` @`x5000` of SPI flash. With `x00`, it resets to `xFF` @`x5000`.

If `x01` is set, the feature Triggered Broadcom Fast Connect scans when in suspened or disconnected state is disabled. Additionally, it sets the low power mode, when disconnected, to HID OFF.

This is useful when the controllers ship, because the controller cannot wake up from button presses. It does not disable all buttons when it has pairing data, only the easy pressable. A long press from the others can wake up the controller, **if it has pairing data**.

Switch always sends `x08 00` subcmd after every connection, and thus enabling Triggered Broadcom Fast Connect and LPM mode to SLEEP.

### Subcommand 0x10: SPI flash read
Little-endian int32 address, int8 size, max size is `x1D`.
Replies with `x9010` ack and echoes the request info, followed by `size` bytes of data.

```
Request:
[01 .. .. .. .. .. .. .. .. .. 10 80 60 00 00 18]
                               ^ subcommand
                                  ^~~~~~~~~~~ address x6080
                                              ^ length = 0x18 bytes
Response: INPUT 21
[21 .. .. .. .. .. .. .. .. .. .. .. .. 90 10 80 60 00 00 18 .. .. .. ....]
                                        ^ subcommand reply
                                              ^~~~~~~~~~~ address
                                                          ^ length = 0x18 bytes
                                                             ^~~~~ data
```

### Subcommand 0x11: SPI flash Write

Little-endian int32 address, int8 size. Max size `x1D` data to write.
Replies with `x8011` ack and a uint8 status. `x00` = success, `x01` = write protected.

### Subcommand 0x12: SPI sector erase

Takes a Little-endian uint32. Erases the whole 4KB in the specified address to 0xFF.
Replies with `x8012` ack and a uint8 status. `x00` = success, `x01` = write protected.

### Subcommand 0x20: Reset NFC/IR MCU

### Subcommand 0x21: Set NFC/IR MCU configuration

Write configuration data to MCU. This data can be IR configuration, NFC configuration or data for the 512KB MCU firmware update.

Takes 38 or 37 bytes long argument data.

Replies with ACK `xA0` `x20` and 34 bytes of data.

### Subcommand 0x22: Set NFC/IR MCU state

Takes one argument:

| Argument # | Remarks           |
|:----------:| ----------------- |
|   `00`     | Suspend           |
|   `01`     | Resume            |
|   `02`     | Resume for update |

### Subcommand 0x24: Set unknown data (fw 3.86 and up)

Takes a 38 byte long argument.

Sets a byte to `x01` (enable something?) and sets also an unknown data (configuration? for NFC/IR MCU?) to the bt device struct that copies it from given argument.

Replies with `x80 24 00` always.

### Subcommand 0x25: Reset 0x24 unknown data (fw 3.86 and up)

Sets the above byte to `x00` (disable something?) and resets the previous 38 byte data to all zeroes.

Replies with `x80 25 00` always.

### Subcommand 0x28: Set unknown NFC/IR MCU data

Takes a 38 byte long argument and copies it to unknown array_222640[96] at &array_222640[3].

Does the same job with OUTPUT report 0x12.

Replies with ACK `x80` `x28`.

### Subcommand 0x29: Get `x28` NFC/IR MCU data

Replies with ACK `xA8` `x29` and 34 bytes data, from a different buffer than the one the x28 writes.

### Subcommand 0x2A: Set GPIO Pin Output value (2 @Port 2)

Takes a uint8_t and sets unknown GPIO Pin 2 at Port 2 to `0` = GPIO_PIN_OUTPUT_LOW` or `1` = GPIO_PIN_OUTPUT_HIGH`.

This normally enables a function. For example, subcmd `x48` sets GPIO Pin 7 @Port 2 output value, which disables or enables IMU.

Replies always with ACK `x00` `x2A`.

`x00` as an ACK here is a bug. Devs forgot to add an ACK reply.

### Subcommand 0x2B: Get `x29` NFC/IR MCU data

Replies with ACK `xA9` `x2B` and 20 bytes long data (which has also a part from x24 subcmd).

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

### Subcommand 0x40: Enable IMU (6-Axis sensor)

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x41: Set IMU sensitivity

Sets the 6-axis sensor sensitivity for accelerometer and gyroscope. 4 uint8_t.

Sending x40 x01 (IMU enable), if it was previously disabled, resets your configuration to Acc: 1.66 kHz (high perf), ±8G, 100 Hz Anti-aliasing filter bandwidth and Gyro: 208 Hz (high performance), ±2000dps..

Gyroscope sensitivity (Byte 0):

| Arg # | Remarks            |
|:-----:|:------------------:|
| `00`  | ±250dps            |
| `01`  | ±500dps            |
| `02`  | ±1000dps           |
| `03`  | ±2000dps (default) |

Accelerometer sensitivity (Byte 1):

| Arg # | Remarks       |
|:-----:|:-------------:|
| `00`  | ±8G (default) |
| `01`  | ±4G           |
| `02`  | ±2G           |
| `03`  | ±16G          |

Gyroscope performance rate (Byte 2):

| Arg # | Remarks                    |
|:-----:|:--------------------------:|
| `00`  | 833Hz (high perf)          |
| `01`  | 208Hz (high perf, default) |

Accelerometer Anti-aliasing filter bandwidth (Byte 3):

| Arg # | Remarks         |
|:-----:|:---------------:|
| `00`  | 200Hz           |
| `01`  | 100Hz (default) |

### Subcommand 0x42: Write to IMU registers

It takes 3 uint8_t arguments and writes to the selected register. You can write only writable registers (r/w).

Consult LSM6DS3.pdf for all registers and their meaning. The registers addresses are mapped 1:1 in the subcmd.

With this subcmd you can completely control the IMU.

| Byte # | Remarks                  |
|:------:|:------------------------:|
| `00`   | Register address         |
| `01`   | Always `x01` for writing |
| `02`   | Value to write           |

### Subcommand 0x43: Read IMU registers

It takes 2 uint8t_t.

| Byte # | Remarks                    |
|:------:| -------------------------- |
| `00`   | Register start address     |
| `01`   | Registers to show. Max x20 |

It replies with `xC043##$$`, where ## is the first register address to show and $$ is how many registers to show. After these the data follows.

For example, by sending `x0020` you can view registers `x00` - `x1F`, `x2020`: `x20` - `x2F`, etc.

To quickly get the register you need, send `x##01` (## is the register address) and parse the 1st byte after the subcmd + args reply (`xC043##$$`).

Consult LSM6DS3.pdf for all registers and their meaning. The registers addresses are mapped 1:1 in the subcmd.

### Subcommand 0x48: Enable vibration

One argument of `x00` Disable  or `x01` Enable.

### Subcommand 0x50: Get regulated voltage

Replies with ACK `xD0` `x50` and a little-endian uint16. Raises when charging a Joy-Con.

Internally, the values come from 1000mV - 1800mV regulated voltage samples, that are translated to 1320-1680 values.

These follow a curve between 3.3V and 4.2V (tested with multimeter). So a 2.5x multiplier can get us the real battery voltage in mV.

Based on this info, we have the following table:

|   Range #            |   Range     | Range in mV | Reported battery |
|:--------------------:|:-----------:|:-----------:| ---------------- |
|   `x0528` - `x059F`  | 1320 - 1439 | 3300 - 3599 | 2 - Critical     |
|   `x05A0` - `x05DF`  | 1440 - 1503 | 3600 - 3759 | 4 - Low          |
|   `x05E0` - `x0617`  | 1504 - 1559 | 3760 - 3899 | 6 - Medium       |
|   `x0618` - `x0690`  | 1560 - 1680 | 3900 - 4200 | 8 - Full         |

Tests showed charging stops at 1680 and the controller turns off at 1320.

### Subcommand 0x51: Set GPIO Pin Output value (7 & 15 @Port 1)

This sets the output value for `Pin 7` and `Pin 15` at `Port 1`. It's currently unknown what they do..

It takes a uint8. Valid values are 0x00, 0x04, 0x10, 0x14. Other values result to these bitwise.

The end result values are translated to `GPIO_PIN_OUTPUT_LOW = 0` and `GPIO_PIN_OUTPUT_HIGH = 0`.

| Value # | PIN @Port 1 | GPIO Output Value    |
|:-------:|:-----------:| -------------------- |
| `x00`   | `7`         | GPIO_PIN_OUTPUT_HIGH |
|         | `15`        | GPIO_PIN_OUTPUT_LOW  |
| `x04`   | `7`         | GPIO_PIN_OUTPUT_LOW  |
|         | `15`        | GPIO_PIN_OUTPUT_LOW  |
| `x10`   | `7`         | GPIO_PIN_OUTPUT_HIGH |
|         | `15`        | GPIO_PIN_OUTPUT_HIGH |
| `x14`   | `7`         | GPIO_PIN_OUTPUT_LOW  |
|         | `15`        | GPIO_PIN_OUTPUT_HIGH |

Replies with ACK `x80` `x51`.

### Subcommand 0x52: Get GPIO Pin Input/Output value

Replies with ACK `xD1` `x52` and a uint8. The uint8 value is actually 4bit (b0000WXYZ).

Each bit translates to GPIO_PIN_OUTPUT_LOW or GPIO_PIN_OUTPUT_HIGH. The first 3 bits (ZYX) are **inverted**.

Example with 0x12:

| bit LSB o # | PIN @Port | Example Value           |
|:-----------:|:---------:| ----------------------- |
| `bit 0` (Z) | `4 @0`    | 0: GPIO_PIN_OUTPUT_HIGH |
| `bit 1` (Y) | `2 @3`    | 1: GPIO_PIN_OUTPUT_LOW  |
| `bit 2` (X) | `7 @1`    | 0: GPIO_PIN_OUTPUT_HIGH |
| `bit 3` (W) | `15 @1`   | 1: GPIO_PIN_OUTPUT_HIGH |

If the joy-cons are connected to a charging grip, the reply is `x17` (L, L, L, H). If you remove it from it, changes to `x14` (H, H, L, H).

If you only connect or pair it from sleep mode, the reply is `x04` (H, H, L, L).
