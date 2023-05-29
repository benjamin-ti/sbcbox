#!/bin/sh

qemu-system-arm \
	-M vexpress-a9 \
	-kernel images/zImage \
	-dtb images/vexpress-v2p-ca9.dtb \
	-drive file=images/sdcard.img,if=sd,format=raw, \
	-drive file=images/flash.bin,if=pflash,format=raw \
	-append "rw console=ttyAMA0 console=tty root=/dev/mmcblk0p2" \
	-cpu cortex-a9 \
	-m 256M \
	-serial stdio \
	-net nic,model=lan9118 \
	-net user \
	-name Vexpress_ARM

#-device sd-card,drive=sdcard2 -drive id=sdcard2,if=none,format=raw,file=images/sdcard.img \
#-drive file=images/rootfs.ext2,if=sd,format=raw \
