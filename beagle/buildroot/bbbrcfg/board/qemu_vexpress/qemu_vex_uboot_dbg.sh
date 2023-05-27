#!/bin/sh

qemu-system-arm \
	-M vexpress-a9 \
	-kernel images/u-boot \
	-drive file=images/rootfs.ext2,if=sd,format=raw \
	-cpu cortex-a9 \
	-m 512M \
	-serial stdio \
	-net nic,model=lan9118 \
	-net user \
	-name Vexpress_ARM \
	-s -S
