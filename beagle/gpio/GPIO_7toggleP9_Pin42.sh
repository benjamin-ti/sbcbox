#!/bin/bash

# echo "7" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio7/direction

while true
do
    echo 1 > /sys/class/gpio/gpio7/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio7/value
    sleep 1
done
