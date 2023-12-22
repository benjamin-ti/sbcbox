#!/usr/bin/env bash

BOARD_DIR="$(dirname $0)"

if [ -e "${BASE_DIR}/script_config.sh" ]
then
	source "${BASE_DIR}/script_config.sh"
fi

if [ -n "$BENSBBB_DTB" ]; then
    awk -v dtb=$BENSBBB_DTB '{if (/^fdtfile/) printf("fdtfile=%s\n", dtb); else print $0}' $BOARD_DIR/uEnv.txt > $BINARIES_DIR/uEnv.txt
else
    cp $BOARD_DIR/uEnv.txt $BINARIES_DIR/uEnv.txt
fi
ln -srf /usr/share/fonts/dejavu ${TARGET_DIR}/usr/lib/fonts

mkdir ${TARGET_DIR}/boot
awk '!/^\/dev\/mmcblk0p1/{print $0;}' ${TARGET_DIR}/etc/fstab > ${TARGET_DIR}/etc/fstab.tmp
echo "/dev/mmcblk0p1	/boot		vfat	defaults	0	2" >> ${TARGET_DIR}/etc/fstab.tmp
mv ${TARGET_DIR}/etc/fstab.tmp ${TARGET_DIR}/etc/fstab

echo "VARIABLES:"
echo BR2_CONFIG $BR2_CONFIG
echo HOST_DIR $HOST_DIR
echo STAGING_DIR $STAGING_DIR
echo TARGET_DIR $TARGET_DIR
echo BUILD_DIR $BUILD_DIR
echo BINARIES_DIR $BINARIES_DIR
echo CONFIG_DIR $CONFIG_DIR
echo BASE_DIR $BASE_DIR

# Mit zImage geht's mit dem unkomprimierten Image nicht - das sollten wir noch rausfinden warum
cat $BUILD_DIR/linux-custom/arch/arm/boot/Image $BUILD_DIR/linux-custom/arch/arm/boot/dts/am335x-boneblack-4DCap-bt-osdred.dtb > $BUILD_DIR/linux-custom/arch/arm/boot/Image.dtb
$HOST_DIR/bin/mkimage -A arm -O linux -T kernel -C none -n Linux-4.19.79 -a 82000000 -e 82000000 -d $BUILD_DIR/linux-custom/arch/arm/boot/Image $BUILD_DIR/linux-custom/arch/arm/boot/uImage
$HOST_DIR//bin/mkimage -A arm -O linux -T kernel -C none -n Linux-4.19.79 -a 82000000 -e 82000000 -d $BUILD_DIR/linux-custom/arch/arm/boot/Image.dtb $BUILD_DIR/linux-custom/arch/arm/boot/uImage.dtb
cp $BUILD_DIR/linux-custom//arch/arm/boot/Image $BINARIES_DIR
cp $BUILD_DIR/linux-custom//arch/arm/boot/uImage $BINARIES_DIR
cp $BUILD_DIR/linux-custom//arch/arm/boot/uImage.dtb $BINARIES_DIR
