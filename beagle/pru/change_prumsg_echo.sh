echo stop > /sys/class/remoteproc/remoteproc2/state
cp PRU_RPMsgEcho.elf /lib/firmware/am335x-pru1-fw
echo start > /sys/class/remoteproc/remoteproc2/state