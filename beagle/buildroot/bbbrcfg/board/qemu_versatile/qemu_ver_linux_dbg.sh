#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel images/zImage \
	-dtb images/versatile-pb.dtb \
	-drive file=images/rootfs.ext2,if=scsi,format=raw \
	-append "rw console=ttyAMA0 console=tty root=/dev/sda" \
	-m 256M \
	-serial stdio \
	-nic user,hostfwd=tcp::8888-:22 \
	-name Versatile_ARM \
	-s -S
