-include myconfig

host_all:
	$(MAKE) -C host
	
ipu1_all:
	$(MAKE) -C ipu1
	
upload:
	sshpass -p temppwd scp host/bin/release/app_host debian@$(BOARDIP):~
	sshpass -p temppwd scp ipu1/bin/release/server_ipu1.xem4 debian@$(BOARDIP):~
#	sudo cp server_ipu1.xem4 /lib/firmware/dra7-ipu1-fw.xem4
#   sudo ./app_host IPU1
	