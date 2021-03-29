#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#define PrintfErrAndExit(fmt, ...)  PrintfErrorAndExitReal(__LINE__, fmt, ##__VA_ARGS__)
static void PrintfErrorAndExitReal(unsigned int uiLine, const char* fmt, ...);

static void CleanAndExit(int ret)
{
    static int inExit = 0;

    if (inExit)
        return;

    inExit = 1;

    exit(ret);
}

static int GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

#define VALUE_MAX 30
    char path[VALUE_MAX];
    int fd;

    if ((value != 0) && (value != 1))
        return -1;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        PrintfErrAndExit("Failed to open gpio%d value for writing!\n", pin);
        return(-1);
    }

    if (1 != write(fd, &s_values_str[value], 1)) {
        PrintfErrAndExit("Failed to write value %c on gpio%d!\n", s_values_str[value], pin);
        return(-1);
    }

    close(fd);

    return(0);
}

/*=========================================================================*/
static void WriteValue2File(char* pcFile, char* pcValue)
{
    int fd;
    unsigned int uiLen;

    if (NULL==pcFile || NULL==pcValue) {
        PrintfErrAndExit("NULL Pointer error!\n");
    }

    fd = open(pcFile, O_WRONLY);
    if (-1 == fd) {
        PrintfErrAndExit("Failed to open %s!\n", pcFile);
    }
    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        PrintfErrAndExit("Failed to write value %s in %s!\n", pcValue, pcFile);
    }
    close(fd);
}

static void PrintfErrorAndExitReal(unsigned int uiLine, const char* fmt, ...)
{
    va_list arglist;

    va_start(arglist,fmt);
    fprintf(stderr, "%s, %u: ", __FILE__, uiLine);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    CleanAndExit(-1);
}

int main(int argc, char *argv[])
{
    while(1) {
        GPIOWrite(43, 0);
        usleep(500*1000);
        GPIOWrite(43, 1);
        usleep(500*1000);
    }

    CleanAndExit(0);
}
