# PRU_CGT environment variable must point to the TI PRU code gen tools directory. E.g.:
#(Desktop Linux) export PRU_CGT=/path/to/pru/code/gen/tools/ti-cgt-pru_2.1.2
#(Windows) set PRU_CGT=C:/path/to/pru/code/gen/tools/ti-cgt-pru_2.1.2
#(ARM Linux*) export PRU_CGT=/usr/share/ti/cgt-pru
#
# *ARM Linux also needs to create a symbolic link to the /usr/bin/ directory in
# order to use the same Makefile
#(ARM Linux) ln -s /usr/bin/ /usr/share/ti/cgt-pru/bin

PRU_SDK_DIR=/home/user/ti
PRU_CGT=$(PRU_SDK_DIR)/ti-cgt-pru_2.3.3
export SYSROOT=/home/user/opt/bbb_debian10_lx4_19rt_sysroot

CC_ARM=arm-linux-gnueabihf-gcc -g --sysroot $$SYSROOT
#CC_ARM=/home/user/workspaceCpp/btoctavobr_out2/host/bin/arm-linux-gcc -g
##CC_ARM=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

include myconfig

ifndef PRU_CGT
define ERROR_BODY

*******************************************************************************
PRU_CGT environment variable is not set. Examples given:
(Desktop Linux) export PRU_CGT=/path/to/pru/code/gen/tools/ti-cgt-pru_2.1.2
(Windows) set PRU_CGT=C:/path/to/pru/code/gen/tools/ti-cgt-pru_2.1.2
(ARM Linux*) export PRU_CGT=/usr/share/ti/cgt-pru

*ARM Linux also needs to create a symbolic link to the /usr/bin/ directory in
order to use the same Makefile
(ARM Linux) ln -s /usr/bin/ /usr/share/ti/cgt-pru/bin
*******************************************************************************

endef
$(error $(ERROR_BODY))
endif

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=lib/rpmsg_lib.lib

INCLUDE=--include_path=include --include_path=include/am335x
STACK_SIZE=0x100
HEAP_SIZE=0x100
GEN_DIR=O

#Common compiler and linker flags (Defined in 'PRU Optimizing C/C++ Compiler User's Guide)
CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
#Linker flags (Defined in 'PRU Optimizing C/C++ Compiler User's Guide)
LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

PROG_NAME1=PRU_gpioToggle
TARGET1=$(GEN_DIR)/$(PROG_NAME1).elf
MAP1=$(GEN_DIR)/$(PROG_NAME1).map
SOURCES1=PRU_gpioToggle.c
OBJECTS1=$(patsubst %,$(GEN_DIR)/%,$(SOURCES1:.c=.obj))

PROG_NAME2=PRU_RPMsgEcho
TARGET2=$(GEN_DIR)/$(PROG_NAME2).elf
MAP2=$(GEN_DIR)/$(PROG_NAME2).map
SOURCES2=PRU_RPMsgEcho.c
OBJECTS2=$(patsubst %,$(GEN_DIR)/%,$(SOURCES2:.c=.obj))

PROG_NAME3=PRU_Motor
TARGET3=$(GEN_DIR)/$(PROG_NAME3).elf
MAP3=$(GEN_DIR)/$(PROG_NAME3).map
SOURCES3=PRU_Motor.c
OBJECTS3=$(patsubst %,$(GEN_DIR)/%,$(SOURCES3:.c=.obj))

all: printStart $(TARGET1) $(TARGET2) $(TARGET3) O/host_motor O/host_echo printEnd

upload_all:
	sshpass -p temppwd scp O/host_echo debian@$(BOARDIP):~
	sshpass -p temppwd scp O/host_motor debian@$(BOARDIP):~
	sshpass -p temppwd scp O/PRU_gpioToggle.elf debian@$(BOARDIP):~
	sshpass -p temppwd scp O/PRU_RPMsgEcho.elf debian@$(BOARDIP):~
	sshpass -p temppwd scp O/PRU_Motor.elf debian@$(BOARDIP):~
	sshpass -p temppwd scp change_prumsg_echo.sh debian@$(BOARDIP):~

updnstart_motor: all
	sshpass -p temppwd ssh debian@$(BOARDIP) "echo stop > /sys/class/remoteproc/remoteproc2/state; exit 0"
	sshpass -p temppwd scp O/host_motor debian@$(BOARDIP):~
	sshpass -p temppwd scp O/PRU_Motor.elf debian@$(BOARDIP):~
	sshpass -p temppwd scp O/PRU_Motor.elf debian@$(BOARDIP):/lib/firmware/am335x-pru1-fw
	sshpass -p temppwd ssh debian@$(BOARDIP) "echo start > /sys/class/remoteproc/remoteproc2/state; exit 0"	

printStart:
	@echo ''
	@echo '************************************************************'
	@echo 'Building project'

printEnd:
	@echo ''
	@echo 'Output files can be found in the "$(GEN_DIR)" directory'
	@echo ''
	@echo 'Finished building project'
	@echo '************************************************************'
	@echo ''

O/host_echo: O/host_echo.o
	$(CC_ARM) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -lpthread -o $@

O/host_motor: O/host_motor.o
	$(CC_ARM) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -lpthread -o $@	

O/%.o : %.c
	$(CC_ARM) -c $< -o $@

# Invokes the linker (-z flag) to make the .out file
$(TARGET1): $(OBJECTS1) $(LINKER_COMMAND_FILE)
	@echo ''
	@echo 'Building target: $@'
	@echo 'Invoking: PRU Linker'
	$(PRU_CGT)/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $@ $(OBJECTS1) -m$(MAP1) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo 'Finished building target: $@'
	
$(TARGET2): $(OBJECTS2) $(LINKER_COMMAND_FILE)
	@echo ''
	@echo 'Building target: $@'
	@echo 'Invoking: PRU Linker'
	$(PRU_CGT)/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $@ $(OBJECTS2) -m$(MAP2) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo 'Finished building target: $@'

$(TARGET3): $(OBJECTS3) $(LINKER_COMMAND_FILE)
	@echo ''
	@echo 'Building target: $@'
	@echo 'Invoking: PRU Linker'
	$(PRU_CGT)/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $@ $(OBJECTS3) -m$(MAP3) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo 'Finished building target: $@'

# Invokes the compiler on all c files in the directory to create the object files
$(GEN_DIR)/%.obj: %.c
	@mkdir -p $(GEN_DIR)
	@echo ''
	@echo 'Building file: $<'
	@echo 'Invoking: PRU Compiler'
	$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

.PHONY: all clean

# Remove the $(GEN_DIR) directory
clean:
	@echo ''
	@echo '************************************************************'
	@echo 'Cleaning project'
	@echo ''
	@echo 'Removing files in the "$(GEN_DIR)" directory'
	@rm -rf $(GEN_DIR)
	@echo ''
	@echo 'Finished cleaning project'
	@echo '************************************************************'
	@echo ''

# Includes the dependencies that the compiler creates (-ppd and -ppa flags)
-include $(OBJECTS1:%.obj=%.pp)

