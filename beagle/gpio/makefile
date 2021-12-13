# CC=arm-linux-gnueabihf-gcc -g 
#CC=/home/user/workspaceCpp/btoctavobr_out2/host/bin/arm-linux-gcc -g
CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

include myconfig

all: mkO O/gpioHardcore O/mygpio O/mygpiod O/mygpiodinp O/motor

upload:
	sshpass -p temppwd scp O/motor debian@$(BOARDIP):~
	
upload-tools:
	sshpass -p temppwd scp stress.sh debian@$(BOARDIP):~
	sshpass -p temppwd scp cyclictest.sh debian@$(BOARDIP):~  

O/gpioHardcore: O/gpioHardcore.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

O/mygpio: O/mygpio.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
	
O/mygpiod: O/mygpiod.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -o $@
	
O/motor: O/motor.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -lpthread -o $@

O/mygpiodinp: O/mygpiodinp.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -lgpiod -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
