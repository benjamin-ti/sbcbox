CC=arm-linux-gnueabihf-gcc -g --sysroot /opt/pi/sysroot

all: mkO O/dmaSimpleCopy O/dmaSimpleGPIO

O/dmaSimpleCopy: O/dmaSimpleCopy.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lpthread -lrt -o $@
	scp $@ pi@192.168.0.81:~

O/dmaSimpleGPIO: O/dmaSimpleGPIO.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lbcm2835 -lpthread -lrt -o $@
	scp $@ pi@192.168.0.81:~

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
