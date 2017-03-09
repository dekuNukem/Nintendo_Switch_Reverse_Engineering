# My Nintendo Switch reverse engineering attempts

I'm just going to dump all my discoveries here, and hopefully they would be useful to the Nintendo Switch community.

## Left Joycon PCB Layout

![Alt text](http://i.imgur.com/7Ui8lFv.jpg)

For the full-sized PDF [click here](./joycon_left_pcb.pdf).

## Remarks about the left Joycon PCB

* Joycon runs at 1.8V

* In a bizarre move Nintendo didn't use the traditional "one side pulled-up and other side to ground" way of reading buttons, instead they used a keypad configuration where buttons are arranged to rows and columns. They used the keypad scanner built-in inside the BCM20734 for reading the buttons. That means it would be extremely hard to spoof button presses for TAS and twitch-plays. Maybe the Pro controller is different, need to buy one though.

* The only button that's not part of the keypad is the joystick button. That is still activated by pulling down to ground.


