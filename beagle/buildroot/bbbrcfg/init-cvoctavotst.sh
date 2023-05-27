#!/bin/sh

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8 (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

defcfg=bbb_cvoctavotst_defconfig
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
    echo "-d: [bbb,bbb-bt,bbb-4DCap-bt,osdred,osdred-4DCap-bt]"
    exit
}

while getopts hd: flag
do
    case "${flag}" in
        h) help;;
        d) dtb=${OPTARG};;
    esac
done

if [ "$dtb" = "bbb-bt" ]; then
	echo "Using am335x-boneblack-bt.dtb"
	echo "export BENSBBB_DTB=am335x-boneblack-bt.dtb" > script_config.sh
fi
if [ "$dtb" = "bbb-4DCap-bt" ]; then
	echo "Using am335x-boneblack-4DCap-bt.dtb"
	echo "export BENSBBB_DTB=am335x-boneblack-4DCap-bt.dtb" > script_config.sh
fi
if [ "$dtb" = "osdred" ]; then
	echo "Using am335x-osd3358-sm-red.dtb"
	echo "export BENSBBB_DTB=am335x-osd3358-sm-red.dtb" > script_config.sh
fi
if [ "$dtb" = "osdred-4DCap-bt" ]; then
	echo "Using am335x-boneblack-4DCap-bt-osdred.dtb"
	echo "export BENSBBB_DTB=am335x-boneblack-4DCap-bt-osdred.dtb" > script_config.sh
fi

echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig"

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

make BR2_EXTERNAL=$brcfg_dir BR2_DEFCONFIG=$brcfg_dir/config/$defcfg -C $br_path O=$(pwd) defconfig

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_cvoctavo_defconfig -C ../buildroot-2020.02.8/ O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
