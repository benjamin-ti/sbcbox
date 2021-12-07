#!/bin/sh

echo "spi" > /sys/devices/platform/ocp/ocp:P9_29_pinmux/state
echo "spi" > /sys/devices/platform/ocp/ocp:P9_30_pinmux/state
echo "spi" > /sys/devices/platform/ocp/ocp:P9_31_pinmux/state
