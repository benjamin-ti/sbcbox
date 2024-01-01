# CC=arm-linux-gnueabihf-gcc -g
CC=arm-none-eabi-gcc -ggdb

all: mkO O/qbraspi.bin

O/qbraspi.bin: O/qbraspi.elf 
	arm-none-eabi-objcopy -O binary $< $@

O/qbraspi.elf: O/boot.o O/kernel.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -nostartfiles -T link.ld -o $@

mkO:
	mkdir -p O

CPU=cortex-a7
#CPU=arm926ej-s
#CPU=cortex-a8

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

clean: 
	rm -f O/*.o
