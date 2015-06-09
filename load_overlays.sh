#!/bin/bash

SLOTS=/sys/devices/bone_capemgr.9/slots

echo "ADAFRUIT-UART1" > $SLOTS
echo "PyBBIO-epwmss0" > $SLOTS
echo "PyBBIO-eqep0" > $SLOTS
echo "PyBBIO-epwmss1" > $SLOTS
echo "PyBBIO-eqep1" > $SLOTS

cat $SLOTS
