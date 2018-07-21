make menuconfig
make -j4
make -j4 modules
sudo make modules_install
ls /lib/modules
sudo make install
# Press shift at boot and choose extended to choose kernel

