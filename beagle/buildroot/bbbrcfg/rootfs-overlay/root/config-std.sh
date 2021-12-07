#!/bin/sh

echo "default" > /sys/devices/platform/ocp/ocp:P9_18_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:P9_21_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:P9_22_pinmux/state

echo "default" > /sys/devices/platform/ocp/ocp:P9_29_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:P9_30_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:P9_31_pinmux/state

echo "default" > /sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state
echo "default" > /sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state
