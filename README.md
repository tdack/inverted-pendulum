# README

This code is for my 4th Year Engineering Project.

It is to control a rotary inverted pendulum that is connected to a BeagleBone Black.

The BeagleBone Black is interfaced to the following peripherals:

 + Pololu 18v15 Simple Motor Controller (serial connection via UART)
 + 400ppr quadrature encoder (1600ppr in x4 mode)
 + 600ppr quadrature encoder (2400ppr in x4 mode)
 + SSD1306 128x64 pixel OLED display (I2C connection)
  
The encoders are read via the built in eQEP (Enhanced Quadrature Encoder Pulse) modules on the BeagleBone Black

See [header_pinout](doc/header_pinout.md) for connections used.

## Device Tree Overlays

The `dts` directory includes device tree overlays for the eQEP modules to enable
direct memory map use of them using the bbb-eqep class.

The PyBBIO-epwmss and PyBBIO-eqep device tree overlays installed by default in 
`/lib/firmware/` are equivalent and do not require the device tree compiler.

Access to eqep1 requires disabling of the HDMI interface.  See [uEnv.txt](uEnv.txt)
for an example of disabling the HDMI interface.

[`load_overlays.sh`](scripts/load_overlays.sh) is a simple shell script to load the relevant overlays for the
peripherals that are being used on the BBB.

## Third Party Libraries

This project makes use of third party libraries to access various parts of the BeagleBone 
Black hardware. They are used in accordance with their respective licenses.

 + [BlackLib](https://github.com/yigityuce/BlackLib) - GPIO, PWM and threading library
 + [BBB-eQEP](https://github.com/jadedanemone/BBB-eQEP) - Memory mapped library for fast access to eQEPs.
 + [Bonelib](http://sourceforge.net/p/bonelib/wiki/SSD1306.hpp/) - SSD1306 driver, modified to use BlackLib
