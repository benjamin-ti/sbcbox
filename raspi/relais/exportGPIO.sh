#!/bin/sh

sudo echo "18" > /sys/class/gpio/export
sudo echo "1" > /sys/class/gpio/gpio18/active_low
sudo echo "out" > /sys/class/gpio/gpio18/direction
echo "0" > /sys/class/gpio/gpio18/value

sudo echo "15" > /sys/class/gpio/export
sudo echo "1" > /sys/class/gpio/gpio15/active_low
sudo echo "out" > /sys/class/gpio/gpio15/direction
echo "0" > /sys/class/gpio/gpio15/value
