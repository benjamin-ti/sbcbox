#!/bin/sh

mkimage -A arm -O linux -a 0 -e 0 -T script -C none -n "Boot Script" -d boot.script boot.scr
