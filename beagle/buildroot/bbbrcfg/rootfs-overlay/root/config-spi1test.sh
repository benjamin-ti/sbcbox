#!/bin/sh

echo "spi1" > /sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state
echo "spi" >  /sys/devices/platform/ocp/ocp:P9_30_pinmux/state
echo "spi" >  /sys/devices/platform/ocp/ocp:P9_31_pinmux/state
