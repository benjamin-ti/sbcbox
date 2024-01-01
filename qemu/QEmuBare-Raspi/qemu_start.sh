#!/bin/sh

qemu-system-arm -M raspi2 -serial mon:stdio -kernel O/qbraspi.elf -nographic
