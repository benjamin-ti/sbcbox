#!/bin/sh

# Std BeagleBone Black Project with some additional dtbs

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8 (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

defcfg=bbb_defconfig
br_path=../buildroot-2020.02.8/

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
    echo "-b: load boot-script over tftp"
    exit
}

bootftp=0
while getopts hb flag
do
    case "${flag}" in
        h) help;;
        b) bootftp=1;;
    esac
done

if [ "$bootftp" -eq "1" ]; then
#gawk '{ if (/BR2_TARGET_UBOOT_SPL_NAME/) {printf("%s\nBR2_TARGET_UBOOT_BOOT_SCRIPT=y\nBR2_TARGET_UBOOT_BOOT_SCRIPT_SOURCE=\"$(BR2_EXTERNAL_BENSBBB_PATH)/board/beaglebone/boot.cmd\"\n", $0);} else {print($0);} }' $brcfg_dir/config/$defcfg > $brcfg_dir/config/new_$defcfg
    sed -e 's/genimage.cfg/genimage_bootscr.cfg/' $brcfg_dir/config/$defcfg > $brcfg_dir/config/temp_$defcfg
    sed -e 's/boot.cmd/new_boot.cmd/' $brcfg_dir/config/temp_$defcfg > $brcfg_dir/config/new_$defcfg
    sed -e "s/192.168.0.1/$(hostname -I)/" $brcfg_dir/board/beaglebone/boot.cmd > $brcfg_dir/board/beaglebone/new_boot.cmd
    rm $brcfg_dir/config/temp_$defcfg
    defcfg=new_$defcfg
fi

echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig"

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
