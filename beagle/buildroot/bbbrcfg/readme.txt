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
make BR2_DEFCONFIG=../bbbrcfg/bbb_defconfig -C ../<buildroot>/ O=$(pwd) defconfig
make

make gconfig
make savedefconfig
