#!/bin/sh

echo "spi" > /sys/devices/platform/ocp/ocp:P9_18_pinmux/state
echo "spi" > /sys/devices/platform/ocp/ocp:P9_21_pinmux/state
echo "spi" > /sys/devices/platform/ocp/ocp:P9_22_pinmux/state
