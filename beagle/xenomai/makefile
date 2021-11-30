PATH_TO_STAGE := /home/user/workspace/bbbrout_xenomai2/host/arm-buildroot-linux-gnueabihf/sysroot
XENO_CONFIG := $(PATH_TO_STAGE)/usr/bin/xeno-config
CFLAGS := $(shell DESTDIR=$(PATH_TO_STAGE) $(XENO_CONFIG) --posix --cflags)
LDFLAGS := $(shell DESTDIR=$(PATH_TO_STAGE) $(XENO_CONFIG) --posix --ldflags)
CC := $(shell DESTDIR=$(PATH_TO_STAGE) $(XENO_CONFIG) --cc)

EXECUTABLE := xenohello

all: $(EXECUTABLE)

%: %.c
	$(PATH_TO_STAGE)/usr/bin/wrap-link.sh -v $(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
clean:
	rm $(EXECUTABLE)
