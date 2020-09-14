# project 4e03

## state: p3 of hardware version E4

this project is an attempt to develop an alternate firmware for the Epilot E4 powerbank. Epilot E4 is a smart powerbank that is capable of replacing its 4 18650 battery cells. It utilizes an atmega168pa to control and monitor the charging of each individual cell, so that salvaged cells can be mixed in use.

currently there are two hardware versions observed, E4 and S4.

## project plan:

*p1, research the schematics.
*p2, research the functions the original firmware.
*p3, develop a preliminary arduino testing code to fullfill the functions.
*p4, port the arduino code to avr c code.
*p5, additional: make improvements

## project details:

p1: epilot_v1.0.pdf is the schematic of hardware version E4, here are some main components
* main controller: atmega168pa
* 4 charger IC: TP4055
* boost converter IC: MT5033
* 2.5v LDO U2: unknow (marking A521)
* temperature sensor U8: unknow (marking BLJW)
* display: controller ST7565 with 128x32

p2: kown functions that derived from the schematic
1 detect charger input
2 detect usb load
3 measure the voltage of each cell
4 charge each cell until 4.2v
5 enable/disable boost converter
6 measure the voltage of usb output port
7 measure the current of two usb output port
8 display info on the lcd screen
9 turn off the screen after 1 min of inactivity
10 long press the button to wake
11 in wake state, short press button to turn on the LED light
12 in LED light on state, short press button to turn on the SOS light
13 in SOS light on state, short press button to turn off LED light

p3: in development, basic battery charging and usb output is working, display also works
p4: in development

p5: some random thoughts

as can be seen from the schematics, we have almost used all pins of the atmega168pa, the charger enable for battery 1 and 2 even shares the same pin. also the external crystal pins, I2C port and UART port are occupied as well. there fore the function upgradeability is limited. however the onboard temperature sensor U8 is not working as intended since it only mesures PCB instead of cell temperature. therefore maybe PC0 could be freed, plus the unused PD7, we can have 2 pins to implement extra function.

if we upgrade the atmega168pa to atmega328pb, then 2 extra pins( pin 3 and pin 6) can also be used.


