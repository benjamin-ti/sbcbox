CC=arm-linux-gnueabihf-gcc -g --sysroot /opt/pi/sysroot

all: mkO O/rgpio

O/rgpio: O/rgpio.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -lgpiod -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
