#!/bin/sh

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8 (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

export BENSBBB_DTB=am335x-osd3358-sm-red-cvprtcape1.dtb

make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_cvprtcape1_defconfig -C ../buildroot-2021.05.1/ O=$(pwd) defconfig

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_cvprtcape1_defconfig -C ../buildroot-2021.05.1/ O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
