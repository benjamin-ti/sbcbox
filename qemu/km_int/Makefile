BOARDIP=127.0.0.1
USER=root
PASSWD=root

export ARCH:=arm
# export CROSS_COMPILE:=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-
export CROSS_COMPILE:=arm-linux-gnueabihf-

export KM=kmint

KDIR ?= /home/user/wsSbcBox/qemu_versatile_out/build/linux-4.19.91

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
	sshpass -p $(PASSWD) scp -P 8888 O/$(KM).ko $(USER)@$(BOARDIP):~
