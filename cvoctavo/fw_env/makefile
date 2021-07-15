CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

all: mkO O/mymtd O/fw_printenv

O/mymtd: O/main.o 
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
	scp $@ root@10.102.1.151:~

O/fw_printenv: O/ctype.o O/linux_string.o O/attr.o O/flags.o O/crc32.o O/fw_env.o O/fw_env_main.o 
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
	scp $@ root@10.102.1.151:~

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DUSE_HOSTCC -I. -c $< -o $@

clean: 
	rm O/*.o
