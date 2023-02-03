CC=arm-none-eabi-gcc -g -O0 -fno-stack-protector
#CC=arm-linux-gnueabihf-gcc -g -O0

CPU=cortex-a8

all: mkO O/qemubare.bin

mkO:
	mkdir -p O

O/qemubare.bin: O/qemubare.elf
	arm-none-eabi-objcopy -O binary $< $@

O/qemubare.elf: O/vector.o O/sbrk.o O/main.o
	$(CC) -mcpu=$(CPU) -Xlinker -Map -Xlinker O/qemubare.map $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -nostartfiles -T link-arm.ld -o $@

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.s
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

O/%.o : %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -mcpu=$(CPU) -c $< -o $@

clean: 
	rm O/*.o

run: all
	qemu-system-arm -M realview-pb-a8 -m 512 -kernel O/qemubare.bin -nographic -no-reboot -monitor telnet:127.0.0.1:2048,server,nowait,ipv4
