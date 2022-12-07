# git clone --depth 1 --branch 5.10.145-ti-rt-r54 https://github.com/beagleboard/linux.git linux-headers-5.10.145-ti-rt-r54
git clone --depth 1 --branch 4.19.94-ti-rt-r73 https://github.com/beagleboard/linux.git linux-headers-4.19.94-ti-rt-r73

cd linux-headers-4.19.94-ti-rt-r73
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- scripts prepare modules_prepare
