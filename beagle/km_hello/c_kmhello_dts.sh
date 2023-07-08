#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

# source myconfig
BOARDIP=10.102.12.42

KDIR=/home/user/opt/linux-headers-4.19.94-ti-rt-r73

KM=kmhello

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

cp $KM.dts $KDIR/arch/arm/boot/dts/overlays/

cd $KDIR

make overlays/$KM.dtbo

cd $SCRIPTPATH

cp $KDIR/arch/arm/boot/dts/overlays/$KM.dtbo O/$KM.dtbo

sshpass -p temppwd scp O/$KM.dtbo debian@$BOARDIP:~
