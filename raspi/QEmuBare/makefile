# CC=arm-linux-gnueabihf-gcc -g
CC=arm-none-eabi-gcc -g

all: mkO O/qemu_test.bin

O/qemu_test.bin: O/qemu_test.elf 
	arm-none-eabi-objcopy -O binary $< $@

O/qemu_test.elf: O/startup.o O/main.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -nostartfiles -T test.ld -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=arm926ej-s -c $< -o $@

O/%.o : %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=arm926ej-s -c $< -o $@

clean: 
	rm O/*.o
