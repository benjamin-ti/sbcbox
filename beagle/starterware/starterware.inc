#changed files: common.mk (Line 73)
#ifdef APP_DIR
#  OBJDIR = obj/$(DEPENDENCY_SUB_PATH)/$(PROFILE_$(CORE))/gcc
#  BINDIR = bin/$(PLATFORM)/gcc
#  ifdef MODULE_NAME
#   LIBDIR = $(DEST_ROOT)/$($(MODULE_NAME)_RELPATH)/lib/$(DEPENDENCY_SUB_PATH)/$(PROFILE_$(CORE))/gcc
#  endif
#  PACKAGEDIR = $(DEST_ROOT)/package/$(PACKAGE_SELECT)/gcc
#else
#ifeq ($(DEST_ROOT),)

# Pinmux in board/am335x/am335x_beagleboneblack_pinmux_data.c: gGpio0PinCfg

export starterware_PATH=/home/user/ti/processor-sdk-rtos-am335x-evm-06.03.00.106/pdk_am335x_1_0_17/packages/ti/starterware/
export STARTERWARE_ROOTDIR=/home/user/ti/processor-sdk-rtos-am335x-evm-06.03.00.106/pdk_am335x_1_0_17/packages/ti/starterware/
export PLATFORM=am335x-evm
export CORE=a8host
export CONSOLE=UART
export TOOLCHAIN=gcc
export PROFILE=debug
export PROFILE_a8host=debug
export SOC=am335x
export BUILDCFG=all
