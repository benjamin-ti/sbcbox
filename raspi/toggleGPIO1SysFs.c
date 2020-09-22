#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int
GPIOExport(int pin)
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int
GPIODirection(int pin, int boolDirIsOut)
{
	static const char s_directions_str[]  = "in\0out";
 
#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;
	const char* s;
	int   len;
 
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}
 
    if (!boolDirIsOut) {
		s = &(s_directions_str[0]);
		len = 2;
	}
	else {
		s = &(s_directions_str[1]);
		len = 3;
	}
		
	if (-1 == write(fd, s, len)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}

static int
GPIOWrite(int pin, int value)
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
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
 
	if (1 != write(fd, &s_values_str[value], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}

int main(void)
{
	GPIOExport(18);
	GPIODirection(18, 1);
	
	while(1) {
		GPIOWrite(18, 1);
	    // usleep(500 * 1000);
	    GPIOWrite(18, 0);
	    // usleep(500 * 1000);
    }
	return 0;
 }
 
