#!/bin/sh

qemu-img resize $BINARIES_DIR/sdcard.img 128M

if [ ! -f $BINARIES_DIR/flash.bin ]; then
    qemu-img create $BINARIES_DIR/flash.bin 128M
fi
