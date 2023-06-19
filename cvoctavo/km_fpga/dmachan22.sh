#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

# source myconfig
BOARDIP=10.102.12.42

KDIR=/home/user/opt/linux-headers-4.19.94-ti-rt-r73

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

cp dmachan22.dts $KDIR/arch/arm/boot/dts/overlays/

cd $KDIR

make overlays/dmachan22.dtbo

cd $SCRIPTPATH

cp $KDIR/arch/arm/boot/dts/overlays/dmachan22.dtbo O/dmachan22.dtbo

sshpass -p temppwd scp O/dmachan22.dtbo debian@$BOARDIP:~
