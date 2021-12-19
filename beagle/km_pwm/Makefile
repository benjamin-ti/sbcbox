# export ARCH:=arm
# export CROSS_COMPILE:=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-

-include myconfig

KDIR ?= /lib/modules/$(shell uname -r)/build
# KDIR ?= /home/user/workspaceCpp/vcoctavobr_outStd/build/linux-4.19.36
# KDIR ?= /home/user/workspaceCppSys/kdir-arm-4.19.94-ti-r68/build/
BUILD_DIR ?= $(PWD)/O
BUILD_DIR_MAKEFILE ?= $(PWD)/O/Makefile

obj-m += km_pwm.o

all: $(BUILD_DIR_MAKEFILE)
#	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=arm-linux-gnueabihf- modules
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean

compile-on-target:
	sshpass -p temppwd scp km_pwm.c debian@$(BOARDIP):~
	sshpass -p temppwd scp Makefile debian@$(BOARDIP):~
	sshpass -p temppwd ssh debian@$(BOARDIP) "make"