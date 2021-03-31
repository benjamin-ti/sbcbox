#!/bin/sh

spidev_test -D /dev/spidev1.1 -s 140000 -p "\xAC\x58\x00\xAA" -v
