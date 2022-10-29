#CC=arm-linux-gnueabihf-gcc -g 
#CC=/home/user/workspaceCpp/btoctavobr_out2/host/bin/arm-linux-gcc -g
CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

all: mkO O/filewrite

upload:
	sshpass -p root scp O/filewrite root@10.102.3.33:~ 

O/filewrite: O/filewrite.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -pthread -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
