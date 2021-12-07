#include <stdio.h>
#include <string.h>
#include <gpiod.h>

static char* _appname;
static struct gpiod_chip* _chip;
static struct gpiod_line* _lines[30];
static int _initDone;

int wrgpio_init(char* appname)
{
    memset(&_lines, 0, 30);
    _chip = gpiod_chip_open_by_name("gpiochip0");
    if (!_chip) {
        perror("access to /sys/class/gpio/gpiochip0 failed\n");
        return 0;
    }
    _appname = appname;
    _initDone = 1;
    return 1;
}

void wrgpio_close(void)
{
    int i;
    if (!_initDone) {
        return;
    }
    for (i=0; i<30; i++) {
        if (_lines[i]) {
            gpiod_line_release(_lines[i]);
        }
    }
    gpiod_chip_close(_chip);
    _initDone = 0;
}

int wrgpio_set_pinmode2outp(unsigned int gpioNum)
{
    int ret;

    if (!_initDone) {
        perror("wrgpio not initialized\n");
        return 0;
    }

    if (gpioNum>=30) {
        fprintf(stderr, "gpio num %u does not exist\n", gpioNum);
        return 0;
    }

    _lines[gpioNum] = gpiod_chip_get_line(_chip, gpioNum);
    if (!_lines[gpioNum]) {
        fprintf(stderr, "init of gpio %u failed\n", gpioNum);
        return 0;
    }

    ret = gpiod_line_request_output(_lines[gpioNum], _appname, 0);
    if (ret < 0) {
        fprintf(stderr, "init of gpio %u as output failed\n", gpioNum);
        return 0;
    }

    return 1;
}

int wrgpio_set(unsigned int gpioNum)
{
    int ret;

    if (!_initDone) {
        perror("wrgpio not initialized\n");
        return 0;
    }

    if (!_lines[gpioNum]) {
        fprintf(stderr, "gpio %u not initialized\n", gpioNum);
        return 0;
    }

    ret = gpiod_line_set_value(_lines[gpioNum], 1);
    if (ret < 0) {
        fprintf(stderr, "failed set gpio %u\n", gpioNum);
        return 0;
    }

    return 1;
}

int wrgpio_del(unsigned int gpioNum)
{
    int ret;

    if (!_initDone) {
        perror("wrgpio not initialized\n");
        return 0;
    }

    if (!_lines[gpioNum]) {
        fprintf(stderr, "gpio %u not initialized\n", gpioNum);
        return 0;
    }

    ret = gpiod_line_set_value(_lines[gpioNum], 0);
    if (ret < 0) {
        fprintf(stderr, "failed set gpio %u\n", gpioNum);
        return 0;
    }

    return 1;
}
