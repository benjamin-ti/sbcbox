#!/bin/bash

# Get Pin Informations:
# Look at System Reference Manual -> Connector P9.12
# GPIO 128 -> sysfs GPIO-Number
# Mode 14: gpio5_0 -> gpiochip4, line 0

echo "128" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio128/direction

while true
do
    echo 1 > /sys/class/gpio/gpio128/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio128/value
    sleep 1
done
