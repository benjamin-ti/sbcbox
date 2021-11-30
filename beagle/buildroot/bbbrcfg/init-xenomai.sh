#!/bin/sh

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8 (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_xenomai_defconfig -C ../buildroot-2021.05.1/ O=$(pwd) defconfig