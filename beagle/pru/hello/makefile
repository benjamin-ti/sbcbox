# PRU_SDK_DIR=/home/user/workspace/bbbrout2/host
PRU_SDK_DIR=/opt/bbb_pru_sdk
PRU_CGT_DIR=$(PRU_SDK_DIR)/usr/share/ti-cgt-pru

all: mkO O/hello.elf O/gpio.elf

O/hello.elf: O/hello.pru0.obj
	$(PRU_CGT_DIR)/bin/clpru --silicon_version=2 --hardware_mac=on \
        -i$(PRU_CGT_DIR)/include -i$(PRU_CGT_DIR)/lib \
        --obj_directory O -z $^ -llibc.a -m O/hello.map \
        -o $@ common/am335x_pru.cmd
#	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

O/gpio.elf: O/PRU_gpioToggle.obj
	$(PRU_CGT_DIR)/bin/clpru --silicon_version=2 --hardware_mac=on \
        -i$(PRU_CGT_DIR)/include -i$(PRU_CGT_DIR)/lib \
        --obj_directory O -z $^ -llibc.a -m O/gpio.map \
        -o $@ common/am335x_pru.cmd

O/%.obj : %.c
	$(PRU_CGT_DIR)/bin/clpru --silicon_version=2 --hardware_mac=on \
        -i$(PRU_CGT_DIR)/include -i$(PRU_CGT_DIR)/usr/include \
        -i$(PRU_CGT_DIR)/usr/include/am335x -i$(PRU_CGT_DIR)/lib \
        -icommon -c --obj_directory O $<
#	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

mkO:
	mkdir -p O