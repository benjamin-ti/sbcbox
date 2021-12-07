#!/bin/bash

# M1 Current
echo "12" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio12/direction

# M1 Dir
echo "25" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio25/direction

# M1 Step
echo "16" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio16/direction

echo 1 > /sys/class/gpio/gpio12/value
sleep 1

while true
do
        echo 1 > /sys/class/gpio/gpio16/value
        sleep 1
        echo 0 > /sys/class/gpio/gpio16/value
        sleep 1
done
