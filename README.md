# README

This code is for my 4th Year Engineering Project.

It is to control a rotary inverted pendulum that is connected to a BeagleBone Black.

The BeagleBone Black is interfaced to the following peripherals:

 + Pololu 18v7 Simple Motor Controller
 + 2 x 400ppr quadrature encoders (giving 1600ppr in x4 mode)
  
The encoders are read via the built in eQEP modules on the BeagleBone Black

See header_pinout.md for connections used.

## Device Tree Overlays

The `dts` directory includes device tree overlays for the eQEP modules to enable
direct memory map use of them using the bbb-eqep class.

`load_overlays.sh` is a simple shell script to load the relevant overlays for the
peripherals that are being used on the BBB.
  