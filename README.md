# project 4e03

## state: p3 of hardware version E4

this project is an attempt to develop an alternate firmware for the Epilot E4 powerbank. Epilot E4 is a smart powerbank that is capable of replacing its 4 18650 battery cells. It utilizes an atmega168pa to control and monitor the charging of each individual cell, so that salvaged cells can be mixed in use. currently there are two hardware versions observed, E4 and S4, but the firmwares from both versions seems identical.

## project plan:

* p1, research the schematics
* p2, research the functions the original firmware
* p3, develop a preliminary arduino firmware to fullfill the functions
* p4, port the arduino code to avr c code
* p5, extra: make improvements

## project details:

p1: epilot_v1.0.pdf is the schematic of hardware version E4, here are some main components
* main controller: atmega168pa
* 4 charger IC: TP4055
* boost converter IC: MT5033
* 2.5v LDO U2: unknow (marking "A521"), replaced with 2.8v LDO TPS79928DDCR (marking "AWX")
* temperature sensor U8: unknow (marking "BLJW")
* display: controller ST7565 with 128x32 (the display has no visible marking outside, the controller model is determined experimentally by running U8g2 library examples)

<img src="https://raw.githubusercontent.com/cosailer/4e03/master/front_s.jpg" width="400" height="500">

p2: arduino code v1.0 is complete

kown functions that derived from the schematic
1. detect usb charger input **[done]**
2. detect usb output load **[almost]**
3. measure the voltage of each cell **[almost]**
4. charge each cell to full (4.2v) **[done]**
5. disable usb output when cell drops to 2.7v
6. enable/disable boost converter **[done]**
7. measure the voltage of usb output port **[done]**
8. measure the current of two usb output port **[done]**
9. display information on the lcd screen **[done]**
10. turn off the display back light after 10 seconds of inactivity **[done]**
11. in none charging state, go to sleep after 20 seconds of inactivity **[done]**
12. long press the button to sleep **[done]**
13. in sleep state, short press button to wake **[done]**
13. in wake state, short press button to turn on the LED light **[done]**
14. in LED light on state, short press button to turn on the SOS light
15. in SOS light on state, short press button to turn off LED light

p3: the preliminary test arduino code is complete, the file is eploit_arduino.ino, compiled binary is arduino_v1.0.bin. most basic functions are implemented and are usable, but the software structure should be improved.
    the display is shown as the following example,
```
     334|U3537|CHG:1
    3801|R2760|USB:1
    3530|T 266|I   0
    3900|t  24|I   0
```
there are 3 columns,
* first column shows the voltage of 4 batteries in mV.
* second column, U3537 is the voltage of the USB output in mV, it is low because boost converter is not enabled. R2760 is the VCC for the atmega168pa in mV. T266 is the measured ADC value from onboard temperature sensor U8, no conversion is done. t24 is the reading from the internal temperature sensor of atmega168pa.
* thrid column, CHG:1 shows whether the usb input charger is inserted. USB:1 shows whether the USB load is present. the last two I0 is the current of two USB output port in mA.

the picture below shows the development setup. to the left is a hand made usbasp, to the right is a hand made power logger(from great scott) displaying the current information of cell in socket 4.

<img src="https://raw.githubusercontent.com/cosailer/4e03/master/E4_0_s.jpg" width="500" height="400">

some notes:
1. the voltage/current measurement is accurate enough for casual indication, however, when cell is charging, voltage measurement cannot show the actual cell voltage, only the voltage connected to charger
2. usb load detection is a little tricky. when the boost converter is enabled, it is not possible to detect usb load, only by measuring usb output current at the moment. however, if the current is small, it will not be detectable, which is a problem
3. both display back light led and the front led light are connected to hardware pwm pins, the brightness of both can be adjusted easily
4. the onboard 2.5v ldo has no reverse voltage protection, however the TPS799 seemed to be OK for 3.3v at Vout when program the MCU

some data:

single cell in socket 4, current measured on battery side:
```
                                     [original]  [mod1]
1. idle, lcd back light on, led on:     30mA      
2. idle, lcd back light on :          4.72mA      4.60mA
3. idle, lcd back light off:          3.50mA      3.42mA
4. system sleep:                      0.65mA      0.59mA
```

p4: in development:

the arduino replacement firmware is mostly finished. after some minor adjustments, the onboard atmega168pa will be replaced with atmega328pb and focus will be porting arduino code to avr code. thus the original unmodified hardware version of E4 and S4 will likely only have the arduino firmware.

p5: some random thoughts

originally I plan to add some I2C sensors to the powerbank, however as can be seen from the schematics, we have almost used all pins of the atmega168pa, the charger enable for battery 1 and 2 even shares the same pin. also the external crystal pins, I2C port and UART port are occupied. therefore the function upgradeability is limited. however the onboard temperature sensor U8 is a bad idea since it only mesures PCB instead of cell temperature. maybe PC0 could be freed, plus the unused PD7, we can have 2 pins to implement extra functions.

if we upgrade the atmega168pa to atmega328pb, then 2 extra pins( pin 3 and pin 6) can also be used, also, they seem to be the second I2C channel, so adding a I2C sensor is possible again!

I have another interesting way to add the I2C sensor boards, since the signal lines of USB port 1 is not connected, they can be connected directly to the I2C port of atmega328pb, then the sensor boards can be inserted like a USB dongle, and the charging ability should not be affected. A LDO to convert 4v-5v to 3.3v may be needed.

some ideas:
1. upgrade atmega168pa to atmega328pb, more pin, more flash/ram/eeprom
2. remove sensor U8, move the connection from PB4 to PC0 to add the ability to measure usb input voltage
3. change the backlight led to change color of the display
4. change the button to a softer one or a touch button
5. add invidual enable pin for each charger IC instead of two ICs per pin on PB6 and PB7
6. increase the voltage divider resisters to decrease the cell self-discharging current, maybe also add a capacitor
7. move pin PD0 and PD1, free up the UART for sensors or even external microcontrollers
8. implement the floppy bird game or the chrome jumping dinosaur game, since there is only one button
9. another idea is to record charging history of each cell in the eeprom to improve charging, not sure if its possible
10. add I2C sensor boards on the USB output port
11. lead the ICSP header outside the case for programming
12. onboard MT5033 has a upgraded version MT5035

