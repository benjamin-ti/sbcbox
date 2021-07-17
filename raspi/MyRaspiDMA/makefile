CC=arm-linux-gnueabihf-gcc -g --sysroot /opt/pi/sysroot

all: mkO O/dmaSimpleCopy

O/dmaSimpleCopy: O/dmaSimpleCopy.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lrt -o $@
	scp $@ pi@192.168.0.66:~

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
