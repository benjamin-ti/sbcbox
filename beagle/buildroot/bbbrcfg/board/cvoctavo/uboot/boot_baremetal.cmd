setenv loadaddr 0x80000000
fatload mmc 0 ${loadaddr} baremetal.bin
echo *** Booting to BareMetal ***
go ${loadaddr}
