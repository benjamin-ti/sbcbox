BOARDIP=10.102.1.114

export ARCH:=arm
export CROSS_COMPILE:=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-
# export CROSS_COMPILE:=arm-linux-gnueabihf-

# KDIR ?= /lib/modules/$(shell uname -r)/build
KDIR ?= /home/user/wsCppSys/vcoctavobr_outStd/build/linux-4.19.36
# KDIR ?= /opt/bbb_debian10_lx4_19rt_sysroot/lib/modules/4.19.94-ti-rt-r73/build
# KDIR ?= /home/user/opt/linux-headers-4.19.94-ti-rt-r73
BUILD_DIR ?= $(PWD)/O
BUILD_DIR_MAKEFILE ?= $(PWD)/O/Makefile

obj-m += kmfpga.o

all: $(BUILD_DIR_MAKEFILE)
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean
	
upload:
	scp O/kmfpga.ko root@$(BOARDIP):~

run: all upload
	ssh root@$(BOARDIP) "insmod kmfpga.ko"
	
stop:
	ssh root@$(BOARDIP) "rmmod kmfpga"
	