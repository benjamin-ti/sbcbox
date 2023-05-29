setenv fdtaddr  0x60000000
setenv loadaddr 0x60008000

echo "Load zImage from sdcard ..."
load mmc 0:1 $loadaddr /zImage

echo "Load /vexpress-v2p-ca9.dtb from sdcard ..."
load mmc 0:1 $fdtaddr /vexpress-v2p-ca9.dtb

setenv bootargs rw console=ttyAMA0 console=tty root=/dev/mmcblk0p2

echo "Boot kernel ..."
bootz $loadaddr - $fdtaddr
