# USB HID Notes

Both the Pro Controller and the Charging Joy-Con Grip support communication over USB HID. Both use identical STM32s and both, for the most part, have the same firmware for the micro-controller. The only significant difference is that the grip exposes two interfaces for talking with each Joy-Con individually, and the Pro Controller only exposes one. Additionally, the Pro Controller firmware seems to have functionality for ignoring checksums if the Broadcom chip sends a 00 checksum for its first reply.

## HID Protocol

The STM32 handles most UART communication for handshaking. HID commands may start with an 01, 10, or 80. 80 seems to contain all of Nintendo's custom protocol.

#### `01 .. ..`

Sends `19 01 03 07 00 00 92 00 00 00 00 00 01 .. ..` over UART.

#### `10 .. ..`

Sends `19 01 03 07 00 00 92 00 00 00 00 00 10 .. ..` over UART.

#### `80 01`
Sample response: `81 01 00 02 57 30 ea 8a bb 7c`

Sends current connection status, and if the Joy-Con are connected, a MAC address and the type of controller. In the above case, it sent an `02` for a right Joy-Con and a Joy-Con MAC address 7c:bb:8a:ea:30:57. These same packets are also sent during a session if the Joy-Con connection state changes.


#### `80 02`
Sample response: `81 02`

Sends handshaking packets over UART to the Joy-Con or Pro Controller Broadcom chip. This command can only be called once per session.

#### `80 03`
Sample response: `81 03`

Switches baudrate to 3Mbit, needed for improved Joy-Con latency. This command can only be called following `80 02`, but allows another `80 02` packet to be sent following it. A second handshake is required for the baud switch to work.

#### `80 04`

Forces the Joy-Con or Pro Controller to only talk over USB HID without any timeouts. This is required for the Pro Controller to not time out and revert to Bluetooth.

#### `80 05`

Allows the Joy-Con or Pro Controller to time out and talk Bluetooth again. If the controller is not send a USB HID packet within a certain amount of time, it will time out and reset the connection.

#### `80 06`

Possibly resets? Sends `19 01 03 07 00 00 92 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 06 01` over UART.

#### `80 91`

Sends a pre-handshake command, looks up UART post-header lengths for various pre-handshake commands when sending. UART command 01 is 0F large, 18 is 37 large, 40 uses the size specified in the 16-bit word following `80 91`, 03, 05, 06, 07, 13, anything else defaults to 0B large.

#### `80 92`

Sends any UART command. The UART post-header length is specified in the 16-bit word following `80 92`. For example, sending `80 92 00 01 00 00 00 00 1F` will send a UART input packet and retrieves a response for that packet.


