# CC=arm-linux-gnueabihf-gcc -g 
CC=/home/user/workspaceCpp/btoctavobr_out/host/bin/arm-linux-gcc -g
#CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

all: mkO O/gpioHardcore O/mygpio O/cvdispprog

O/gpioHardcore: O/gpioHardcore.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
	
O/mygpio: O/mygpio.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -o $@

O/cvdispprog: O/cvdispprog.o O/atmega.o O/mirrortable.o O/byteoperation.o O/VCEndian.o O/zoomtable.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
