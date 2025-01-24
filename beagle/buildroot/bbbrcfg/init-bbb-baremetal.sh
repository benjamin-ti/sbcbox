#!/bin/sh

defcfg=bbb_baremetal_defconfig
br_path=../buildroot-2023.02.8/

brcfg_dir=$(realpath $(dirname "$0"))

help()
{
    echo "To build a linux image, create a directory, go into that directory, and call this script:"
    echo "i.e. your outputdir dir is in /home/user/bbbrout"
    echo "    mkdir /home/user/bbbrout"
    echo "    cd /home/user/bbbrout"
    echo "    ../bbbrcfg/"$(basename "$0")
    echo "    make"
    echo "-h: help"
    exit
}

while getopts h flag
do
    case "${flag}" in
        h) help;;
    esac
done

echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig"

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
