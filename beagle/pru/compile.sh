#!/usr/bin/env sh

# export PRU_SDK_DIR=/segfs/linux/pru_sdk
export PRU_CGT_DIR=/home/user/workspace/bbbrout2/host/usr/share/ti-cgt-pru


# compile support library without optimization
# enabled to keep argument passing convention

$PRU_CGT_DIR/bin/clpru \
--silicon_version=2 \
--hardware_mac=on \
-i$PRU_CGT_DIR/include \
-i$PRU_CGT_DIR/usr/include \
-i$PRU_CGT_DIR/usr/include/am335x \
-i$PRU_CGT_DIR/lib \
-icommon \
-c \
hello.pru0.c


# compile and link
# optimizations allowed

$PRU_CGT_DIR/bin/clpru \
--silicon_version=2 \
--hardware_mac=on \
-i$PRU_CGT_DIR/include \
-i$PRU_CGT_DIR/lib \
-z \
hello.pru0.obj -llibc.a \
-m pru_main.map \
-o pru_main.elf \
common/am335x_pru.cmd

#$PRU_CGT_DIR/example/AM3359_PRU.cmd


# convert ELF to binary file pru_main.bin

$PRU_CGT_DIR/bin/hexpru \
$PRU_CGT_DIR/bin.cmd \
./pru_main.elf


/home/user/workspace/bbbrout2/host/bin/arm-none-linux-gnueabihf-gcc \
-o loader loader.c -lprussdrv
