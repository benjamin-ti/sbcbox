CHAIN=arm-none-eabi

CC=$(CHAIN)-gcc -g

all: mkO O/baremetal.bin

O/baremetal.bin: O/baremetal.elf 
	$(CHAIN)-objcopy -O binary $< $@

O/baremetal.elf: O/start.o O/main.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -nostartfiles -T link-arm.ld -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

O/%.o : %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

O/%.o : %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm -f O/*.o
