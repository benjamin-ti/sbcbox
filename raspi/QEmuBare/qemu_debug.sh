#!/bin/sh

qemu-system-arm -M versatilepb -m 128M -nographic -s -S -kernel O/qemubare.bin
