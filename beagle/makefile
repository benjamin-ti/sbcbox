CC=arm-linux-gnueabihf-gcc -g 

all: mkO O/gpioHardcore

O/gpioHardcore: O/gpioHardcore.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

mkO:
	mkdir -p O

O/%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	rm O/*.o
