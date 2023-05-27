#!/bin/sh

qemu-system-arm \
	-M vexpress-a9 \
	-kernel images/zImage \
	-dtb images/vexpress-v2p-ca9.dtb \
	-drive file=images/rootfs.ext2,if=sd,format=raw \
	-append "rw console=ttyAMA0 console=tty root=/dev/mmcblk0" \
	-cpu cortex-a9 \
	-m 32 \
	-serial stdio \
	-net nic,model=lan9118 \
	-net user \
	-name Vexpress_ARM
