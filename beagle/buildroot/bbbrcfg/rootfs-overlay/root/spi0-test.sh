#!/bin/sh

spidev_test -D /dev/spidev0.0 -s 140000 -p "\xAC\x58\x00\xAA" -v
