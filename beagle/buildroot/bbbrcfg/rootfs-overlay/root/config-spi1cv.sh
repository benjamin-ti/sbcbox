#!/bin/sh

echo "spi1_sclk" > /sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state
echo "spi1" > /sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state
echo "spi1" > /sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state
