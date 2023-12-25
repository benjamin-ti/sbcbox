#!/bin/sh

qemu-system-arm \
    -M raspi2 \
    -kernel images/zImage \
    -dtb images/bcm2709-rpi-2-b.dtb \
    -drive file=images/sdcard.img,if=sd,format=raw, \
    -append "rw console=ttyAMA0,115200 rootwait root=/dev/mmcblk0p2 bcm2708_fb.fbwidth=1280 bcm2708_fb.fbheight=1024 dwc_otg.fiq_fsm_enable=0" \
    -cpu arm1176 \
    -m 1G \
    -smp 4 \
    -serial stdio \
    -netdev user,id=net0,hostfwd=tcp::8022-:22 \
    -device usb-net,netdev=net0
    
   
# bcm2708_fb.fbwidth=1280 bcm2708_fb.fbheight=1024 dwc_otg.fiq_fsm_enable=0
#-append "rw earlyprintk loglevel=8 console=ttyAMA0,115200 dwc_otg.lpm_enable=0 rootwait root=/dev/mmcblk0p2"
