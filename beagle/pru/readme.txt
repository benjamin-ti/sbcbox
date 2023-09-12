Don't forget to activate
uboot_overlay_pru=/lib/firmware/AM335X-PRU-RPROC-4-19-TI-00A0.dtbo
in uEnv.txt

On a Debian GNU/Linux:
copy pru_main.elf to /lib/firmware/am335x-pru0-fw
Start with
echo start > /sys/class/remoteproc/remoteproc1/state
How it loads:
/sys/class/remoteproc/remoteproc1/firmware points to /lib/firmware/am335x-pru0-fw:
cat /sys/class/remoteproc/remoteproc1/firmware
