#!/bin/sh

qemu-system-arm -M realview-pb-a8 -m 128M -nographic -kernel O/qemubare.bin
