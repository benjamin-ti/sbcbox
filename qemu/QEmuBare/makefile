# CC=arm-linux-gnueabihf-gcc -g
CC=arm-none-eabi-gcc -ggdb

all: mkO O/qemubare.bin

O/qemubare.bin: O/qemubare.elf 
	arm-none-eabi-objcopy -O binary $< $@

O/qemubare.elf: O/vector.o O/sbrk.o O/main.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -nostartfiles -T link-arm.ld -o $@

mkO:
	mkdir -p O

CPU=arm926ej-s
#CPU=cortex-a8

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

clean: 
	rm -f O/*.o
