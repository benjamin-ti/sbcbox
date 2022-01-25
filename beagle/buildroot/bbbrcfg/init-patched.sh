#!/bin/sh

# make dir struct:
# bbbrcfg (with this file)
# bbbrout (created)
# buildroot-2020.02.8_patched (downloaded)
#
# cd bbbrout
# ../bbbrcfg/init.sh

rm -rf ../buildroot-2020.02.8_patched/package/ti-sgx-km
rm -rf ../buildroot-2020.02.8_patched/package/ti-sgx-um
cp -R ../bbbrcfg/br-patches/ti-sgx-km ../buildroot-2020.02.8_patched/package/ti-sgx-km
cp -R ../bbbrcfg/br-patches/ti-sgx-um ../buildroot-2020.02.8_patched/package/ti-sgx-um

make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_patched_defconfig -C ../buildroot-2020.02.8_patched/ O=$(pwd) defconfig

echo "# $0" > current_config.sh
echo "make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_patched_defconfig -C ../buildroot-2020.02.8_patched/ O=$(pwd) defconfig" >> current_config.sh
chmod +x current_config.sh

echo "sudo dd if=images/sdcard.img of=/dev/sdb status=progress bs=1MB" > mkcard_sdb.sh
chmod +x mkcard_sdb.sh
