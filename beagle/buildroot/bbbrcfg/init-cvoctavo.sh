#!/bin/sh

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8 (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_cvoctavo_defconfig -C ../buildroot-2020.02.8/ O=$(pwd) defconfig

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_cvoctavo_defconfig -C ../buildroot-2020.02.8/ O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh
