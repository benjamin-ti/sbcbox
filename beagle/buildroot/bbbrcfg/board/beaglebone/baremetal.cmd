setenv loadaddr 0x80000000;
fatload mmc 0 ${loadaddr} prtsoft.bin;
echo *** Booting to BareMetal ***;
go 0x80000058;
