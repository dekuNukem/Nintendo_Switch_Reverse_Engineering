# My Nintendo Switch reverse engineering attempts

I'm just going to dump all my discoveries here, and hopefully they would be useful to the Nintendo Switch community.

## Questions?

Please make a post in the [issue section](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/issues), contributors have been immensely helpful.

## Contributors

Please remember that the information in this repo is the work of a lot more people than just me. Please take a look at the [contributors list](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/graphs/contributors) and appreciate their work. Be sure to credit them as well.

## Joy-Con PCB Layout and test points

![Alt text](http://i.imgur.com/7Ui8lFv.jpg)

[Full-size PDF](./joycon_pcb.pdf) of Joy-Con pinouts, both left and right.

The "JC" is for the 10-pin Joycon connector, see below for details.

### Remarks

* Joy-Con runs at 1.8V

* There are no silkscreen marking component and test point numbers. Nintendo maybe is trying to discourage people from doing funky things to the Joy-Con?

* Also, in a bizarre move, Nintendo didn't use the traditional "one side pulled-up other side to ground" way of reading buttons, instead they used a keypad configuration where buttons are arranged in rows and columns. They used the keypad scanner built-in inside the BCM20734 with 128KHz clock for reading the buttons. That means it would be extremely hard to spoof button presses for TAS and twitch-plays. Maybe the Pro controller is different, need to buy one though.

* The only button that's not part of the keypad is the joystick button, which is still activated by pulling it down to ground.

## SPI Peripherals

![Alt text](https://i.imgur.com/2c3tmyd.png)

There are 2 SPI devices on the bus, one 4Mb MX25U4033E flash memory and one LSM6DS3 6-axis MEMS accelerometer and gyroscope.

[Here is a capture](./logic_captures/left_grey_joycon_spi_poweron_then_dock.logicdata) of SPI lines when the Joy-Con battery is connected, and then attached to the console.

It looks like SCK runs at 12.5MHz when accessing the flash memory, but switches to 6.25MHz when accessing the MEMS chip.

### Accelerometer and gyroscope

Upon connection the microcontroller initializes a software reset of the MEMS chip, then set up the accelerometer and gyroscope as follows:

| Accelerometer               | Gyroscope                      |
|-----------------------------|--------------------------------|
| ODR 1.66KHz, full-scale ±8g | ODR 208Hz, full-scale 2000dps  |

The accelerometer also has AA filter at 100Hz bandwidth, low-pass filter enabled, slope filter enabled with cut-off frequency at 416Hz.

The Joy-Con then polls LSM6DS3 every 1.35ms(740Hz) for both accelerometer and gyroscope data in all axises, totaling 12 bytes(6 axises, each axis 2 bytes).

Since the Joy-Con polls MEMS data every 1.35ms but only send out controller update every 15ms, there might be some internal averaging to smooth out the data, needs to go through the numbers to find out.

### Flash Memory

Well there's a capture of the SPI lines when the Joy-Con is powered up (battery connected), which contains all the address and data Joy-Con reads from the flash memory. I don't have time to go through it right now but of course you can if you want.

The SPI flash can be dumped post-pairing over UART using `19 01 03 38 00 92 00 31 00 00 d4 e6 01 0c 00 01 40 40 00 01 40 40 10 XX XX XX XX YY 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` where XX XX XX XX is a 32-bit little-endian address and YY is the amount to dump up to 0x1c bytes. The resulting data will be at +0x20 in the Joy-Con response.

Joy-Con color, serial and calibration settings are all stored on the SPI flash and are accessed through the above query.

## Joy-Con to Console Communication

When attached to the console, the Joy-Con talks to it through a physical connection instead of Bluetooth. There are 10 pins on the connector, I'm just going to arbitrarily name it like this:

![Alt text](https://i.imgur.com/52xjlRb.jpg)

![Alt text](https://i.imgur.com/LIFiq8X.jpg)

Looking at the pins on both Joy-Con facing towards you, the left most one is Pin 1, and the right most one is Pin 10. I simply removed the rumble motor, burned a hole on the back cover, and routed all the wires out through that.

Capture of the docking of the [left](./logic_captures/left_grey_joycon_docking_controllers_screen.logicdata) and [right](./logic_captures/right_grey_joycon_docking_controllers_screen.logicdata) Joycon.

![Alt text](https://i.imgur.com/iUq5RNG.png)

### Joy-Con Connector Pinout


| Logic analyzer channel | Joycon Connector Pin |            Function           |                                                       Remarks                                                                       |
|:----------------------:|:--------------------:|:-----------------------------:| ----------------------------------------------------------------------------------------------------------------------------------- |
|            -           |           1          |              GND              |                                                          -                                                                          |
|            -           |           2          |              GND              |                                                          -                                                                          |
|            0           |           3          |              Jdet             | HIGH when connected to console via Bluetooth. Joy-Con will not send serial data post-handshake unless pin is pulled LOW by console. |
|            1           |           4          |               5V              |                                              Joy-Con power and charging                                                             |
|            2           |           5          | Serial data console to Joycon |                                             Inverted level (idle at GND)                                                            |
|            3           |           6          |              JRST             |                                      Joy-Con reset signal , high level is reset                                                     |
|            -           |           7          |              GND              |                                                          -                                                                          |
|            4           |           8          | Serial data Joycon to console |                                            Standard level (idle at 1.8V)                                                            |
|            5           |           9          |           Power output        |                             Joy can output power to Ring Fit Adventure or starlink toy                                              |
|            6           |          10          |          Flow control         |                             Joy-Con will only send data to console when this line is HIGH                                           |

* When first connected the baud rate is at 1000000bps(!), after the initial handshake the speed is then switched to 3125000bps(!!)

* Thanks to [yujc1986's contribution](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/commit/bfeadd4f3f7de46a9bd9e06a190f76e4a1887d31), the pin names has been updated according to the circuit board labels on the [Starlink: Battle for Atlas](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/tree/master/peripheral%20device/startlink%20toy) Joycon adapter.

### Handshake procedure

I took apart 2 left Joy-Con, one grey one red. Below you can see the difference in the response between two Joy-Con.

**Console to Joy-Con**                                       |**GREY Joy-Con response**                                    |**RED Joy-Con response**                                     |**Different?**|**Remarks**
:-----------------------------------------------------------:|:-----------------------------------------------------------:|:-----------------------------------------------------------:|:-------:|:-----:
`A1 A2 A3 A4 19 01 03 07 00 A5 02 01 7E 00 00 00`            |`19 81 03 07 00 A5 02 02 7D 00 00 64`                        |`19 81 03 07 00 A5 02 02 7D 00 00 64`                        |Same     |Handshake start; 1000000bps
`19 01 03 07 00 91 01 00 00 00 00 24`                        |`19 81 03 0F 00 94 01 08 00 00 FA E8 01 31 67 9C 8A BB 7C 00`|`19 81 03 0F 00 94 01 08 00 00 8F 87 01 E6 4C 5F B9 E6 98 00`|Different|Joycon MAC
`19 01 03 0F 00 91 20 08 00 00 BD B1 C0 C6 2D 00 00 00 00 00`|`19 81 03 07 00 94 20 00 00 00 00 A8`                        |`19 81 03 07 00 94 20 00 00 00 00 A8`                        |Same     |Command to switch to 3125000bps
`19 01 03 07 00 91 11 00 00 00 00 0E`                        |`19 81 03 07 00 94 11 00 00 0F 00 33`                        |`19 81 03 07 00 94 11 00 00 0F 00 33`                        |Same     |?; 3125000 bps from now on
`19 01 03 07 00 91 10 00 00 00 00 3D`                        |`19 81 03 07 00 94 10 00 00 00 00 D6`                        |`19 81 03 07 00 94 10 00 00 00 00 D6`                        |Same     |?
`19 01 03 0B 00 91 12 04 00 00 12 A6 0F 00 00 00`            |`19 81 03 07 00 94 12 00 00 00 00 B0`                        |`19 81 03 07 00 94 12 00 00 00 00 B0`                        |Same     |?
`19 01 03 08 00 92 00 01 00 00 69 2D 1F`                     |61B Controller status                                        |61B Controller status                                        |Different|Handshake done. Console sends this controller status request command every 15ms from now on.

* Pin 5 (Serial data, console to Joy-Con) is normally pulled high on the console side when nothing is connected. Since this line is inverted on the Joy-Con side, it will be pulled down when a Joy-Con is attached to the console, thus initializing a handshake.

* It seems Pin 5 needs to be pulled down for a while for the handshake to take place, 500ms works for me.

* Handshake starts at 1000000bps, and the console will send a 4-byte start sequence of `A1 A2 A3 A4`, followed by 12 byte command of `19 01 03 07 00 A5 02 01 7E 00 00 00`. It will send those commands repeatedly every 100ms (10Hz) for 3 seconds. Joy-Con respond with `19 81 03 07 00 A5 02 02 7D 00 00 64`. If no response is received it gives up and wait for another event on the line.

* The console then sends `19 01 03 07 00 91 01 00 00 00 00 24`, to which Joy-Con respond with a 20-byte MAC response used to pair the Joy-Con to the console. After the response is received the little Joy-Con insertion animation starts on the screen. The color for this animation is cached on the console after a Joy-Con has been connected for the first time and the color has been retrieved from SPI flash.

* They console sends `19 01 03 0F 00 91 20 08 00 00 BD B1 C0 C6 2D 00 00 00 00 00`, a command that switches baud rate from 1000000 to 3125000. Joy-Con respond with `19 81 03 07 00 94 20 00 00 00 00 A8`. Note that the faster baud rate takes effect from the next command. This command is not required for pairing to complete.

* Now serial comm is at 3125000bps. Console sends `19 01 03 07 00 91 11 00 00 00 00 0E`, Joy-Con responds with `19 81 03 07 00 94 11 00 00 0F 00 33`.

* Console sends `19 01 03 07 00 91 10 00 00 00 00 3D`, Joy-Con responds with `19 81 03 07 00 94 10 00 00 00 00 D6`.

* Now the pairing is seemingly done, the console will now send `19 01 03 08 00 92 00 01 00 00 69 2D 1F` every 15ms to ask for a controller status update. See "Protocol" section below for details.

### Pesky checksums

It turns out the last byte of each command sent over serial seems to be a checksum of some sort, and without figuring it out it would be rather difficult testing what each command does because the console will not accept commands with the wrong checksum. 

**Thanks to [ewalt1's](https://github.com/ewalt1) effort and contribution, we seems to have a solution to the checksum problem:** 

The first 4 bytes are a header, with the 4th byte being the length of the remaining packet (not counting the checksum). The next 7 bytes are some type of data, with the 8th byte being the CRC of that data. The CRC used is CRC-8 with a polynomial of 0x8D and an initial value of 0x00.
 
There's some example code for calculating this CRC using a lookup table in [packet_parse/joycon_crc.py](./packet_parse/joycon_crc.py).

*Note: these checksums are only sent over serial, not over the Bluetooth or USB HID mode.*

### Joy-Con status data packet

In normal operation the console asks Joy-Con for an update every 15ms (66.6fps), the command for requesting update is:

```
19 01 03 08 00 92 00 01 00 00 69 2d 1f
```

Around 4ms later, Joy-Con respond with a 61 bytes long answer.

One sample:

```
19 81 03 38 
00 92 00 31 
00 00 e9 2e 
30 7f 40 00 
00 00 65 f7 
81 00 00 00 
c0 23 01 e2 
ff 3e 10 0a 
00 d6 ff d0 
ff 23 01 e1 
ff 37 10 0a 
00 d6 ff cf 
ff 29 01 dd 
ff 34 10 0a 
00 d7 ff ce 
ff 
```

Here is what I figured out:


|   Byte #  |        Sample value       |               Remarks              |
|:---------:|:-------------------------:|:----------------------------------:|
|   0 to 8  | `19 81 03 38 00 92 00 31` |            Header, fixed           |
| 16 and 17 |          `00 02`          |  Button status, see section below  |
|     19    |            `f7`           | Joystick X value, reversed nibble? |
|     20    |            `81`           |          Joystick Y value          |
| 31 and 32 |          `4e 05`          |          Gyroscope X value         |
| 33 and 34 |          `cc fb`          |          Gyroscope Y value         |
| 35 and 36 |          `eb ff`          |          Gyroscope Z value         |
| 37 and 38 |          `41 00`          |        Accelerometer X Value       |
| 39 and 40 |          `1b 03`          |        Accelerometer Y Value       |
| 41 and 42 |          `82 f0`          |        Accelerometer Z Value       |

Each accelerometer and gyroscope axis data is 2 bytes long and forms a int16_t, last byte is the higher byte.

### Button status

The 16th and 17th byte (on line 5, before `65 f7`) are the button status, when a button is pressed the corresponding bit is set to 1.

![Alt text](http://i.imgur.com/QXBaCIe.png)

### Joystick value

Byte 19 and 20 (`f7 81` between 5th and 6th line) are the Joystick values, most likely the raw 8-bit ADC data. Byte 19 is X while byte 20 is Y. Again, bizarrely, the X nibble is reversed, as in the `f7` should actually be `7f` (127 at neutral position). The Y value is correct though (`0x81` is 129).

### Joy-Con status data packet

See the [bluetooth_hid_subcommands_notes.md](./bluetooth_hid_subcommands_notes.md) file for details about the data transferred during normal operations (button status, joysticks, etc).

### Rumble commands

Details on rumble data are in the [rumble_data_table.md](./rumble_data_table.md) file.

## Touchscreen controller

![Alt text](http://i.imgur.com/ZZWBv5d.jpg)

The console itself uses a FT9CJ capacitive touchscreen controller. And [according to techinsights](http://www.techinsights.com/about-techinsights/overview/blog/nintendo-switch-teardown/) it's a custom part by STMicroelectronics for the Nintendo Switch. After looking at the communication it appears to use I2C, which is in line with other touchscreen controller chips. [Here is a capture](./logic_captures/touchscreen_controller_poweron_i2c.logicdata) of the I2C bus on power-up.

The 7-bit I2C address of the chip is 0x49 (0x92 write, 0x93 read), and it's polled around every 4ms for update.

## Docking station firmware dump

The docking station uses a STM32F048 microcontroller. It's actually labeled as STM32P048 because it uses the FASTROM option where ST pre-programs the flash memory inside the factory. It has 32KB flash memory and 6KB RAM, runs at 48MHz. 

It uses SWD debugging and programming interface, and interestingly the programming testpoints are on the PCB and clearly labeled. After connecting a ST-Link programmer to it reveals that the chip is not read-protected at all, so a firmware dump was easily made. I'm not going to post it in the repo, but if you want it just ask.


## Ending remarks

I'll update this from time to time when I have new discoveries. Please share if you find this useful.

