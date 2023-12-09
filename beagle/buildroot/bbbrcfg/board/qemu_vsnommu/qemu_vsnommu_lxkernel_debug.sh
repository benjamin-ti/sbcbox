#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel images/zImage \
	-dtb images/versatile-pb.dtb \
	-append "console=ttyAMA0,115200" \
	-m 256M \
	-serial stdio \
	-nic user,hostfwd=tcp::8888-:22 \
	-name Versatile_ARM \
	-s -S

# ssh root@127.0.0.1 -p 8888
