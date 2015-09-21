#!/bin/bash
# Load overlays required for inverted pendulum

SLOTS=/sys/devices/bone_capemgr.9/slots

# /dev/ttyO2 - serial comms to motor controller
echo "ADAFRUIT-UART2" > $SLOTS

# eQEP0 - encoder #1
echo "PyBBIO-epwmss0" > $SLOTS
echo "PyBBIO-eqep0" > $SLOTS

#eQEP1 - encoder #2
echo "PyBBIO-epwmss1" > $SLOTS
echo "PyBBIO-eqep1" > $SLOTS

cat $SLOTS
