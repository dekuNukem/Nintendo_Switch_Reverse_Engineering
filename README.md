# My Nintendo Switch reverse engineering attempts

I'm just going to dump all my discoveries here, and hopefully they would be useful to the Nintendo Switch community.

## Left Joycon PCB Layout and test points

![Alt text](http://i.imgur.com/7Ui8lFv.jpg)

For the full-sized PDF [click here](./joycon_left_pcb.pdf).

### Remarks

* Joycon runs at 1.8V

* There is no silkscreen marking component and test point numbers, maybe Nintendo is trying to discourage people from doing funky things to the Joycon?

* Also, in a bizarre move, Nintendo didn't use the traditional "one side pulled-up and other side to ground" way of reading buttons, instead they used a keypad configuration where buttons are arranged to rows and columns. They used the keypad scanner built-in inside the BCM20734 for reading the buttons. That means it would be extremely hard to spoof button presses for TAS and twitch-plays. Maybe the Pro controller is different, need to buy one though.

* The only button that's not part of the keypad is the joystick button. That is still activated by pulling it down to ground.

## Left Joycon SPI flash dump

![Alt text](https://i.imgur.com/2c3tmyd.png)

Well it's not actually a dump, just a capture of the SPI lines when the Joycon is powered up (battery connected). I don't have time to go through it right now of course you can if you want.

The SPI clock runs at 3MHz.

[Click here](./logic_captures/leftjoyconspiflashpoweron.logicdata) for the capture data.

## Joycon Communication Protocol