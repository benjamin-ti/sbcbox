CC=arm-linux-gnueabihf-gcc -g 
#CC=/home/user/workspaceCpp/btoctavobr_out2/host/bin/arm-linux-gcc -g
#CC=/opt/buildroot_tc/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc -g

all: mkO O/mthreadt

upload:
	sshpass -p temppwd scp O/mthreadt debian@10.102.1.224:~ 

O/mthreadt: O/mthreadt.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -pthread -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
