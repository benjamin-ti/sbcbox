setenv bootpart 0:1
setenv devtype mmc
setenv fdtfile am335x-boneblack-4DCap-bt-osdred.dtb
setenv bootdir
setenv bootfile zImage
setenv bootpartition mmcblk0p2
setenv console ttyS0,115200n8
setenv loadaddr 0x82000000
setenv fdtaddr 0x88000000
setenv bootargs console=${console} root=/dev/${bootpartition} rw rootfstype=ext4 rootwait

echo "load ${devtype} ${bootpart} ${loadaddr} ${bootdir}/${bootfile}"
load ${devtype} ${bootpart} ${loadaddr} ${bootdir}/${bootfile}
echo "load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}"
load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}
echo "Boote kernel..."
bootz ${loadaddr} - ${fdtaddr}
