#!/bin/sh

qemu-system-arm \
	-M vexpress-a9 \
	-kernel images/u-boot \
    -drive file=images/sdcard.img,if=sd,format=raw, \
    -drive file=images/flash.bin,if=pflash,format=raw \
	-cpu cortex-a9 \
	-m 256M \
	-serial stdio \
	-net nic,model=lan9118 \
	-net user \
	-name Vexpress_ARM \
	-s -S
