Please add your findings to this document!

## x0000

First chunk of data starts at x0000 and ends at x03c0. No further data until the x2000 section.

## x2000

Motion control calibration data seems to be entirely located around x008000 in the SPI flash, while calibrating the control stick changed two locations in the flash.

```
00001ff0: ffff ffff aa55 f00f 68e5 97d2 0080 0200
00002000: 0022 0c96 98b6 e92a b0cb b7ed f1ab b4a6
00002010: 39e2 4876 4615 62f2 0be8 0000 0000 0000
00002020: 0000 0000 6800[9522 634f 3402 8662 b86b  ; Marked section also changed after calibrating control sticks
00002030: 3623 5b57 8ff0 16b4 a05f fbc0 b99d 6227
00002040: 0000 0000 0000 0000 0000 0800]ffff ffff
```

## x6000

Serial number in ASCII is at x6002 to x6010.

x6012 contains `01a0` on a left Joy-Con and `02a0` on a right.

x6050 - x6056 contains color data - 24-bit body and button colors.

Data ends at x60aa.

## x8000

This section seems to hold motion control and stick calibration data.

```
00008000: ffff ffff ffff ffff ffff ffff ffff ffff
00008010: ffff ffff ffff ffff ffff[ffb2 a1c6 f871  ; Control Stick Calibration data
00008020: 6884 42a3 5442|b2a1 7600 a6fe ea02 0040
00008030: 0040 0040 0e00 fcff e0ff 3b34 3b34 3b34] ; Motion Control Calibration data
```

## x010000

There's firmware at x010000 and x028000. The firmware at x010000 looks older.

