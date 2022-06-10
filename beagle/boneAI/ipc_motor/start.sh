#!/bin/sh

sudo cp ipcmotor.xem4 /lib/firmware/dra7-ipu1-fw.xem4
echo stop > /sys/class/remoteproc/remoteproc0/state
echo start > /sys/class/remoteproc/remoteproc0/state
