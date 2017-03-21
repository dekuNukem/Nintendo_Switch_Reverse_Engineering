# Joycon reader

A simple program that pretends to be the console and complete a handshake with the Joycon then read its button and MEMS status.

## Connections

| Joycon connector pin |                       Funtion                      |  Connect to STM32 |
|:--------------------:|:--------------------------------------------------:|:-----------------:|
|           3          |                   Attach detect?                   |        GND        |
|           5          |      Serial data, console to Joycon, inverted      |  PA2 (USART2_TX)  |
|           6          |  Flow Control, console may send to Joycon when LOW |  PA1 (GPIO INPUT) |
|           8          |            Serial data Joycon to console           |  PA3 (USART2_RX)  |
|          10          | Flow Control, Joycon may send to console when HIGH | PA0 (GPIO OUTPUT) |

* The STM32 board I used is my [NintenDAC dev board](https://github.com/dekuNukem/NintenDAC), it has some special features but for this application you can use any dev board you like.

* Joycon runs 1.8V logic, however I ran my STM32 at 3.0V and have no problem transmitting or receiving.

* Pin3 needs to be connected to GND otherwise Joycon will not respond to status update commands.

* The firmware completes the handshake, then polls the Joycon every 100ms. The board appears as an USB serial device and simply prints the 61-byte packet through the serial port.

* Joycon seems to not sent accelerometer and gyroscope data by default, it needs some special commands to enable it. Needs to look into this.

* A simple python parser script is included to print button presses.