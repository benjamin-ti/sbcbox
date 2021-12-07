#!/usr/bin/env sh

# export PRU_SDK_DIR=/home/user/workspace/bbbrout2/host
export PRU_SDK_DIR=/opt/bbb_pru_sdk
export PRU_CGT_DIR=$PRU_SDK_DIR/usr/share/ti-cgt-pru


# compile support library without optimization
# enabled to keep argument passing convention

mkdir -p O

$PRU_CGT_DIR/bin/clpru \
--silicon_version=2 \
--hardware_mac=on \
-i$PRU_CGT_DIR/include \
-i$PRU_CGT_DIR/usr/include \
-i$PRU_CGT_DIR/usr/include/am335x \
-i$PRU_CGT_DIR/lib \
-icommon \
-c \
--obj_directory O \
hello.pru0.c


# compile and link
# optimizations allowed

$PRU_CGT_DIR/bin/clpru \
--silicon_version=2 \
--hardware_mac=on \
-i$PRU_CGT_DIR/include \
-i$PRU_CGT_DIR/lib \
-z \
O/hello.pru0.obj -llibc.a \
-m O/pru_main.map \
--obj_directory O \
-o O/pru_main.elf \
common/am335x_pru.cmd

#$PRU_CGT_DIR/example/AM3359_PRU.cmd


# convert ELF to binary file pru_main.bin

cd O; $PRU_CGT_DIR/bin/hexpru \
$PRU_CGT_DIR/bin.cmd \
pru_main.elf
cd ..

$PRU_SDK_DIR/bin/arm-none-linux-gnueabihf-gcc \
-o O/loader loader.c -lprussdrv
