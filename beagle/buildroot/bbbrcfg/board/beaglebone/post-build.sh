#!/bin/sh
BOARD_DIR="$(dirname $0)"

cp $BOARD_DIR/uEnv.txt $BINARIES_DIR/uEnv.txt
ln -srf /usr/share/fonts/dejavu ${TARGET_DIR}/usr/lib/fonts

