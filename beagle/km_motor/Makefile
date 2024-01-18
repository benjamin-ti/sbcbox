BOARDIP=10.102.13.14
USER=root
PASSWD=root

export ARCH:=arm
export CROSS_COMPILE:=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-
# export CROSS_COMPILE:=arm-linux-gnueabihf-

export KM=kmmotor2

# KDIR ?= /lib/modules/$(shell uname -r)/build
# KDIR ?= /home/user/wsCppSys/vcoctavobr_outStdKiTi/build/linux-custom
# KDIR ?= /home/user/wsSbcBox/bbbrout_cvoctavo/build/linux-custom/
KDIR ?= /home/user/temp/xenomai/linux-beagle
# KDIR ?= /opt/bbb_debian10_lx4_19rt_sysroot/lib/modules/4.19.94-ti-rt-r73/build
# KDIR ?= /home/user/opt/linux-headers-4.19.94-ti-rt-r73
BUILD_DIR ?= $(PWD)/O
BUILD_DIR_MAKEFILE ?= $(PWD)/O/Makefile

obj-m += $(KM).o

all: $(BUILD_DIR_MAKEFILE)
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean

upload: all
	sshpass -p $(PASSWD) scp O/$(KM).ko $(USER)@$(BOARDIP):~

run: all upload
	sshpass -p $(PASSWD) ssh $(USER)@$(BOARDIP) "insmod $(KM).ko"
	
stop:
	sshpass -p $(PASSWD) ssh $(USER)@$(BOARDIP) "sudo rmmod $(KM)"