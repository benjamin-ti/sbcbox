#! /bin/sh

mkdir -p /opt
mkdir -p /opt/pi
cd /opt/pi
mkdir -p sysroot
cd sysroot

PI_IP_ADDR=192.168.0.81

rsync -avz pi@$PI_IP_ADDR:/lib .
rsync -avz pi@$PI_IP_ADDR:/usr/include usr
rsync -avz pi@$PI_IP_ADDR:/usr/lib usr
# evtl. /usr/local/include
# evtl. /usr/local/lib