#!/bin/sh

make  -C ../buildroot-2023.02.8/ O=$(pwd) beaglebone_defconfig

echo "# $0" > current_config.sh
echo "make  -C ../buildroot-2023.02.8/ O=$(pwd) beaglebone_defconfig" >> current_config.sh
chmod +x current_config.sh

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
