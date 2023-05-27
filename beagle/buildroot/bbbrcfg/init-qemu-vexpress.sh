#!/bin/sh

defcfg=qemu_arm_vexpress_defconfig
br_path=../buildroot-2020.02.8/

brcfg_dir=$(realpath $(dirname "$0"))

cp $brcfg_dir/board/qemu_vexpress/*.sh .

echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig"

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig
