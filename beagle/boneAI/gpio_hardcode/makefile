CC=arm-linux-gnueabihf-gcc -g 
# CC=/home/user/workspaceCpp/btoctavobr_out2/host/bin/arm-linux-gcc -g
# CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

all: mkO O/gpio_hardcode

upload:
	sshpass -p temppwd scp O/gpio_hardcode debian@10.102.1.181:~ 

O/gpio_hardcode: O/gpioHardcore.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
