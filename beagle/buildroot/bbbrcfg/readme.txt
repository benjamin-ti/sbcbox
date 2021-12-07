Directory-Structure should be like this:

BeagleBone (root, name doesn't matter))
 |
 +--- bbbrcfg (name does matter)
 |    +--- readme.txt
 |    +--- rootfs-overlay
 |    +--- ...
 |
 +--- buildroot-2020.02.8 (name doesn't matter, in the following called <buildroot>)
 |
 +--- output1 (name doesn't matter)
 +--- output2
 +--- ...

Go to your output directory and type:
make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_defconfig -C ../<buildroot>/ O=$(pwd) defconfig
or
make BR2_EXTERNAL=../bbbrcfg BR2_DEFCONFIG=../bbbrcfg/config/bbb_defconfig -C ../buildroot-2020.02.8/ O=$(pwd) defconfig

make

make xconfig
make savedefconfig

make linux-menuconfig

"make linux-rebuild" doesn't rebuild the entire kernel, it simply
triggers a new "make" in the kernel tree. So if only your Device Tree
changed, then only your Device Tree will be regenerated. Maybe the
kernel will be linked again, but it certainly will not be rebuilt again
from scratch.

"make linux-dirclean": Lets now clean up completely the linux package
so that its sources will be re-extracted and our patches applied the
next time we do a build.

Device-Tree:
/proc/device-tree
/sys/frimware/devicetree

Recompile Device-Tree:
dtc -I dts -O dtb -o am335x-boneblack.dtb am335x-boneblack.dts
