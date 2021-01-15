#!/bin/sh
BOARD_DIR="$(dirname $0)"

if [ -n "$BENSBBB_DTB" ]; then
    awk -v dtb=$BENSBBB_DTB '{if (/fdtfile/) printf("fdtfile=%s\n", dtb); else print $0}' $BOARD_DIR/uEnv.txt > $BINARIES_DIR/uEnv.txt
else
    cp $BOARD_DIR/uEnv.txt $BINARIES_DIR/uEnv.txt
fi
ln -srf /usr/share/fonts/dejavu ${TARGET_DIR}/usr/lib/fonts
