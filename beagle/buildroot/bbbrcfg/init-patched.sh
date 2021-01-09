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
make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_defconfig -C ../buildroot-2020.02.8_patched/ O=$(pwd) defconfig
