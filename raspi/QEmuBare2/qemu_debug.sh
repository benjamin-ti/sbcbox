#!/bin/sh

qemu-system-arm -M realview-pb-a8 -m 128M -nographic -s -S -kernel O/qemubare.bin
