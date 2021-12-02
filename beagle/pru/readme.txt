On a Debian GNU/Linux:
copy pru_main.elf to /lib/firmware/am335x-pru0-fw
Start with
echo start > /sys/class/remoteproc/remoteproc1/state
How it loads:
/sys/class/remoteproc/remoteproc1/frimware points to /lib/firmware/am335x-pru0-fw:
cat /sys/class/remoteproc/remoteproc1/frimware

