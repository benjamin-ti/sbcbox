export ARCH:=arm
# export CROSS_COMPILE:=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-
export CROSS_COMPILE:=arm-linux-gnueabihf-

-include myconfig

# KDIR ?= /lib/modules/$(shell uname -r)/build
# KDIR ?= /home/user/workspaceCpp/vcoctavobr_outStd/build/linux-4.19.36
# KDIR ?= /home/user/workspaceCppSys/kdir-arm-4.19.94-ti-r68/build/
#KDIR ?= /home/user/opt/linux-headers-4.19.94-ti-rt-r73
KDIR ?= /home/user/opt/linux-headers-5.10.145-ti-rt-r54
BUILD_DIR ?= $(PWD)/O
BUILD_DIR_MAKEFILE ?= $(PWD)/O/Makefile

obj-m += km_pwm_hr.o
obj-m += km_pwm_tsk.o

all: $(BUILD_DIR_MAKEFILE)
#	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=arm-linux-gnueabihf- modules
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean

upload:
	sshpass -p temppwd scp O/km_pwm_tsk.ko debian@$(BOARDIP):~
	sshpass -p temppwd scp O/km_pwm_hr.ko debian@$(BOARDIP):~

compile-on-target:
	sshpass -p temppwd scp km_pwm.c debian@$(BOARDIP):~
	sshpass -p temppwd scp Makefile debian@$(BOARDIP):~
	sshpass -p temppwd ssh debian@$(BOARDIP) "make"
