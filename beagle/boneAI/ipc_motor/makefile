-include myconfig

all:
	$(MAKE) -C boneai_ipu1 release
	$(MAKE) -C host release

clean:
	$(MAKE) -C boneai_ipu1 clean
	$(MAKE) -C host clean

host_make:
	$(MAKE) -C host release
	
host_clean:
	$(MAKE) -C host clean
	
ipu1_make:
	$(MAKE) -C boneai_ipu1 release
	
ipu1_clean:
	$(MAKE) -C boneai_ipu1 clean
	
upload:
	$(MAKE) -C boneai_ipu1 upload
	$(MAKE) -C host upload

	