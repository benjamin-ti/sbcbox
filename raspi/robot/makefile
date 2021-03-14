CC=arm-linux-gnueabihf-gcc -g --sysroot /opt/pi/sysroot

all: mkO O/robot

O/robot: O/main.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -lgpiod -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
