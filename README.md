# My Nintendo Switch reverse engineering attempts

I'm just going to dump all my discoveries here, and hopefully they would be useful to the Nintendo Switch community.

If you think something is wrong, have some observations that I might have missed, stuff you want to contribute, or just general questions please feel free to post here, or email me at dekunukem gmail com.

If you want to use the information here somewhere else, feel free to do so, but do please credit me (dekuNukem).

## Left Joycon PCB Layout and test points

![Alt text](http://i.imgur.com/7Ui8lFv.jpg)

[Full-size PDF](./joycon_left_pcb.pdf).

The "JC" is for the 10-pin Joycon connector, see below for details.

### Remarks

* Joycon runs at 1.8V

* There is no silkscreen marking component and test point numbers, maybe Nintendo is trying to discourage people from doing funky things to the Joycon?

* Also, in a bizarre move, Nintendo didn't use the traditional "one side pulled-up and other side to ground" way of reading buttons, instead they used a keypad configuration where buttons are arranged to rows and columns. They used the keypad scanner built-in inside the BCM20734 with 128KHz clock for reading the buttons. That means it would be extremely hard to spoof button presses for TAS and twitch-plays. Maybe the Pro controller is different, need to buy one though.

* The only button that's not part of the keypad is the joystick button, which is still activated by pulling it down to ground.

## SPI peripherals

![Alt text](https://i.imgur.com/2c3tmyd.png)

There are 2 SPI devices on the bus, one 4Mb MX25U4033E flash memory and one LSM6DS3 6-axis MEMS accelerometer and gyroscope.

[Here is a capture](./logic_captures/left_grey_joycon_spi_poweron_then_dock.logicdata) of SPI lines when the Joycon battery is connected, and then attached to the console.

It looks like SCK runs at 12.5MHz when accessing the flash memory, but switches to 6.25MHz when accessing the MEMS chip.

### Accelerometer and gyroscope

Upon connection the microcontroller initializes a software reset of the MEMS chip, then set up the accelerometer and gyroscope as follows:

| Accelerometer                                                                                                                          | Gyroscope                      |
|----------------------------------------------------------------------------------------------------------------------------------------|--------------------------------|
| ODR 1.66KHz, full-scale ±8g | ODR 208Hz, full-scale 2000dps  |

The accelerometer also has AA filter at 100Hz bandwidth, low-pass filter enabled, slope filter enabled with cut-off frequency at 416Hz.

The Joycon then polls LSM6DS3 every 1.35ms(740Hz) for both accelerometer and gyroscope data in all axises, totaling 12 bytes(6 axises, each axis 2 bytes).

Since the Joycon polls MEMS data every 1.35ms but only send out controller update every 15ms, there might be some internal averaging to smooth out the data, needs to go through the numbers to find out.

### Flash Memory

Well there's a capture of the SPI lines when the Joycon is powered up (battery connected), which contains all the address and data Joycon reads from the flash memory. I don't have time to go through it right now but of course you can if you want.

## Joycon to Console Communication

When attached to the console, the Joycon talks to it through a physical connection instead of Bluetooth. There are 10 pins on the connector, I'm just going to arbitrarily name it like this:

![Alt text](https://i.imgur.com/52xjlRb.jpg)

Looking at the pins on the left Joycon, the left most one is Pin 1, and the right most one is Pin 10. I simply removed the rumble motor, burned a hole on the back cover, and routed all the wires out through that.

[And here](./logic_captures/left_grey_joycon_docking_controllers_screen.logicdata) is a capture of the docking of the left Joycon.

![Alt text](https://i.imgur.com/iUq5RNG.png)

### Joycon Connector Pinout


| Logic analyzer channel | Joycon Connector Pin |            Function           |                                                       Remarks                                                       |
|:----------------------:|:--------------------:|:-----------------------------:|:-------------------------------------------------------------------------------------------------------------------:|
|            -           |           1          |              GND              |                                                          -                                                          |
|            -           |           2          |              GND              |                                                          -                                                          |
|            0           |           3          |           BT status?          | Only high when connected to console via bluetooth, low when unpaired, sleeping, or attached to the console directly |
|            1           |           4          |               5V              |                                              Joycon power and charging                                              |
|            2           |           5          | Serial data console to Joycon |                                             Inverted level (idle at GND)                                            |
|            3           |           6          |          Flow control         |                             Console will only send data to Joycon when this line is LOW                             |
|            -           |           7          |              GND              |                                                          -                                                          |
|            4           |           8          | Serial data Joycon to console |                                            Standard level (idle at 1.8V)                                            |
|            5           |           9          |               ?               |                                                    Always at GND                                                    |
|            6           |          10          |          Flow control         |                             Joycon will only send data to console when this line is HIGH                            |                  |

* When first connected the baud rate is at 1000000bps(!), after the initial handshake the speed is then switched to 3125000bps(!!). The handshake probably exchanges information about the side of the Joycon, the color, and bluetooth address etc.

### Handshake procedure

I took apart 2 left Joycons, one grey one red. Below you can see the difference in the response between two Joycons.

**Console to Joycon**|**GREY Joycon response**|**RED Joycon response**|**Different?**|**Remarks**
:-----:|:-----:|:-----:|:-----:|:-----:
`A1 A2 A3 A4 19 01 03 07 00 A5 02 01 7E 00 00 00`|`19 81 03 07 00 A5 02 02 7D 00 00 64`|`19 81 03 07 00 A5 02 02 7D 00 00 64`|Same|Handshake start; 1000000bps
`19 01 03 07 00 91 01 00 00 00 00 24`|`19 81 03 0F 00 94 01 08 00 00 FA E8 01 31 67 9C 8A BB 7C 00`|`19 81 03 0F 00 94 01 08 00 00 8F 87 01 E6 4C 5F B9 E6 98 00`|Different|Joycon info; possibly color; side; battery level; BT info; etc
`19 01 03 0F 00 91 20 08 00 00 BD B1 C0 C6 2D 00 00 00 00 00`|`19 81 03 07 00 94 20 00 00 00 00 A8`|`19 81 03 07 00 94 20 00 00 00 00 A8`|Same|Command to switch to 3125000bps
`19 01 03 07 00 91 11 00 00 00 00 0E`|`19 81 03 07 00 94 11 00 00 0F 00 33`|`19 81 03 07 00 94 11 00 00 0F 00 33`|Same|?; 3125000 bps from now on
`19 01 03 07 00 91 10 00 00 00 00 3D`|`19 81 03 07 00 94 10 00 00 00 00 D6`|`19 81 03 07 00 94 10 00 00 00 00 D6`|Same|?
`19 01 03 0B 00 91 12 04 00 00 12 A6 0F 00 00 00`|`19 81 03 07 00 94 12 00 00 00 00 B0`|`19 81 03 07 00 94 12 00 00 00 00 B0`|Same|?
`19 01 03 08 00 92 00 01 00 00 69 2D 1F`|61B Controller status|61B Controller status|Different|Handshake done. Console sends this controller status request command every 15ms from now on.

* Pin 5 (Serial data, console to Joycon) is normally pulled high on the console side when nothing is connected. Since this line is inverted on the Joycon side, it will be pulled down when a Joycon is attached to the console, thus initializing a handshake.

* It seems Pin 5 needs to be pulled down for a while for the handshake to take place, 500ms works for me.

* Handshake starts at 1000000bps, and the console will send a 4-byte start sequence of `A1 A2 A3 A4`, around 46us later followed by 12 byte command of `19 01 03 07 00 A5 02 01 7E 00 00 00`. It will send those commands repeatedly every 100ms (10Hz) for 3 seconds. Joycon respond with `19 81 03 07 00 A5 02 02 7D 00 00 64`. If no response is received it gives up and wait for another event on the line.

* The console then sends `19 01 03 07 00 91 01 00 00 00 00 24`, to which Joycon respond with a 20-byte response that's different on each Joycon. That response definitely contains the color information of the Joycon, and also possibly contains the serial number, BT info, battery level, etc. After the response is received the little Joycon insertion animation starts on the screen.

* They console sends `19 01 03 0F 00 91 20 08 00 00 BD B1 C0 C6 2D 00 00 00 00 00`, a command that switches baud rate from 1000000 to 3125000. Joycon respond with `19 81 03 07 00 94 20 00 00 00 00 A8`. Note that the faster baud rate takes effect from the next command.

* Now serial comm is at 3125000bps. Console sends `19 01 03 07 00 91 11 00 00 00 00 0E`, Joycon responds with `19 81 03 07 00 94 11 00 00 0F 00 33`.

* Console sends `19 01 03 07 00 91 10 00 00 00 00 3D`, Joycon responds with `19 81 03 07 00 94 10 00 00 00 00 D6`.

* Now the pairing is seemingly done, the console will now send `19 01 03 08 00 92 00 01 00 00 69 2D 1F` every 15ms to ask for a controller status update. See "Protocol" section below for details.

### Pesky checksums

It turns out the last byte of each command seems to be a checksum of some sort, and without figuring it out it would be rather difficult testing what each command does because the console will not accept commands with the wrong checksum. 

Luckily here are some examples of the checksum, seeing it changes drastically with the difference of a single bit it's probably not some simple xor or modular checksum. If you can figure it out it would be really helpful.

```
19 01 03 07 00 91 10 00 00 00 00 3D
19 01 03 07 00 91 01 00 00 00 00 24
19 01 03 07 00 91 11 00 00 00 00 0E

19 81 03 07 00 94 10 00 00 00 00 D6
19 81 03 07 00 94 11 00 00 0F 00 33
```

**Thanks to [ewalt1's](https://github.com/ewalt1) effort and contribution, we seems to have a solution to the checksum problem:** 

The first 4 bytes are a header, with the 4th byte being the length of the remaining packet (not counting the checksum). The next 7 bytes are some type of data, with the 8th byte being the CRC of that data. The CRC used is CRC-8 with a polynomial of 0x8D and an initial value of 0x00.
 
There's some example code for calculating this CRC using a lookup table in [packet_parse/joycon_crc.py](./packet_parse/joycon_crc.py).

### Joycon status data packet

In normal operation the console asks Joycon for an update every 15ms (66.6fps), the command for requesting update is:

```
19 01 03 08 00 92 00 01 00 00 69 2d 1f
```

Around 4ms later, Joycon respond with a 61 bytes long answer.

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

![Alt text](http://i.imgur.com/H7DUmCx.png)

### Joystick value

Byte 19 and 20 (`f7 81` between 5th and 6th line) are the Joystick values, most likely the raw 8-bit ADC data. Byte 19 is X while byte 20 is Y. Again, bizarrely, the X nibble is reversed, as in the `f7` should actually be `7f` (127 at neutral position). The Y value is correct though (`0x81` is 129).

### The rest of them

Still working on decoding those... It has to contain battery level, button status, joystick position, accelerometer and gyroscope data, and maybe more.

### Rumble commands

I did a capture of the command sent from console to initiate a rumble on the Joycon. It was captured by pressing L to set off a bomb in BotW, which results in a fairly short rumble. [Here is the capture](./logic_captures/left_grey_joycon_botw_rumble.txt).

You can see the console sends longer commands (17 bytes vs 8 bytes) during the rumble period. I'm yet to look into this, but of course you can.

## Touchscreen controller

![Alt text](http://i.imgur.com/ZZWBv5d.jpg)

The console itself uses a FT9CJ capacitive touchscreen controller. And [according to techinsights](http://www.techinsights.com/about-techinsights/overview/blog/nintendo-switch-teardown/) it's a custom part by STMicroelectronics for the Nintendo Switch. After looking at the communication it appears to use I2C, which is in line with other touchscreen controller chips. [Here is a capture](./logic_captures/touchscreen_controller_poweron_i2c.logicdata) of the I2C bus on power-up.

The 7-bit I2C address of the chip is 0x49 (0x92 write, 0x93 read), and it's polled around every 4ms for update.

## Docking station firmware dump

The docking station uses a STM32F048 microcontroller. It's actually labeled as STM32P048 because it uses the FASTROM option where ST pre-programs the flash memory inside the factory. It has 32KB flash memory and 6KB RAM, runs at 48MHz. 

It uses SWD debugging and programming interface, and interestingly the programming testpoints are on the PCB and clearly labeled. After connecting a ST-Link programmer to it reveals that the chip is not read-protected at all, so a firmware dump was easily made. I'm not going to post it in the repo, but if you want it just ask.


## Ending remarks

I'll update this from time to time when I have new discoveries. Please share if you find this useful.

