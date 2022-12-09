#!/bin/bash

# sudo cat /proc/iomem to see memory layout

source myconfig

/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/dtc -I dts -O dtb -o O/reserve-mem-overlay.dtbo reserve-mem-overlay.dts
sshpass -p temppwd scp O/reserve-mem-overlay.dtbo debian@$BOARDIP:~