# EuclideanTrigger
A simple module, based on the ATtiny85 that creates euclidean beats from a clock input. [under development]

The Escanaba Euclidean module has 3 alternate firmware modes:
## Escanaba Euclidean Firmware 
The Eucliean firmware generates euclidean rhythms synchronized to an external eurorack clock source. If no clock source is available it will self clock. Its 3 controls are as follows: 
- Number of hits
- Length of pattern in beats
- Pattern rotation. 
When in self clocking mode, the rotation control becomes a tempo control. 
## Calumet Clock Firmware
The Calument Clock firmware turns the module into a eurorack clock source. Its 3 controls are as follows:
- Swing control
- Clock Division/Multiplication
- Tempo
## Burst Generator
The Burst Generator firmware allows the module to respond to incoming trigger signals by repeating them in a variety of ways. This allows the creation of "bouncing ball" rhythms, ratchets, and other rhythmic tricks. Its controls are as follows:
- Number of repeats
- Initial repeat speed
- A "gravity" control that controls whether the repeats accelerate or decelerate in speed.

# Build instructions

[TODO]


# How To Setup Firmware

1. In the Arduino editor, in the tools menu, set the board to "AtTiny25/45/85"
2. In the Arduino editor, go to Sketch->Include Library->add .zip library and then select the lib folder from the project root.
3. set Processor to "AtTiny85"
4. set Clock to "Internal 16Mhz"
5. plug your programmer into a usb port. We reccomend the (TinyAvr programmer)[https://www.sparkfun.com/products/11801] from sparkfun
6. select your programmer in the tools menu
7. in tools, select burn bootloader. Your AtTiny is now configured to run at 16Mhz.
8. now hit the upload button in the arduino editor to flash the firmware.
9. place the chip into the mount on the back of the module. Take care to make sure the dot on the back of the chip aligns with the indent on the PCB board diagram.
10. plug the module into your rack and have fun!


