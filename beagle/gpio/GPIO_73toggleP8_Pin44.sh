#!/bin/bash

echo "73" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio73/direction

while true
do
    echo 1 > /sys/class/gpio/gpio73/value
    sleep 1
    echo 0 > /sys/class/gpio/gpio73/value
    sleep 1
done
