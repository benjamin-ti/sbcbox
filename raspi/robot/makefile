CC=arm-linux-gnueabihf-gcc -g --sysroot /opt/pi/sysroot

all: mkO O/robot

O/robot: O/main.o O/wrgpio.o O/vcosal.o O/sllist.o O/curve.o O/MotNTOSSync.o O/MotNTLxS.o O/MotNT.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lwiringPi -lcrypt -lgpiod -lm -lpthread -lrt -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -I MotNT -o $@
	
O/%.o : MotNT/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -I MotNT -o $@
	
O/%.o : MotNT/Utilities/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -I MotNT -o $@

O/%.o : MotNT/MotNT/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -I MotNT -o $@

clean: 
	rm O/*.o
