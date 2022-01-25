echo load boot.scr over tftp
setenv serverip 192.168.0.1
setenv bootfile boot.scr
dhcp
source ${loadaddr}
