SYSROOT := /opt/bbb_xenomai_sdk/arm-buildroot-linux-gnueabihf/sysroot
XENO_CONFIG := $(SYSROOT)/usr/bin/xeno-config --skin=alchemy
CFLAGS := $(shell DESTDIR=$(SYSROOT) $(XENO_CONFIG) --cflags)
LDFLAGS := -lgpiod $(shell DESTDIR=$(SYSROOT) $(XENO_CONFIG) --ldflags)
CC := $(shell DESTDIR=$(SYSROOT) $(XENO_CONFIG) --cc)

EXECUTABLE := xenohello

all: $(EXECUTABLE)

%: %.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
	
clean:
	rm $(EXECUTABLE)

upload:
	scp $(EXECUTABLE) root@10.102.1.214:~