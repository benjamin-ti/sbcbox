#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file opening
#include <unistd.h> //  for sleep
#include <stdio.h>
#include <stdlib.h>

// AM335x-Manual Memory Map GPIO1-Register
#define AM335X_GPIO1_BASE        0x4804C000

int main(void)
{
    volatile char* gpio_map;
    volatile unsigned* gprev;
    volatile unsigned* gpfsel1;
    volatile unsigned* gpset0;
    volatile unsigned* gpclr0;

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        printf("Failed to open /dev/mem (did you remember to run as root?)\n");
        exit(1);
    }

    gpio_map = mmap(
        NULL,             // Any adddress in our space will do
        4*1024,           // Map length (always 4K)
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       // Shared with other processes
        memfd,            // File to map
        (off_t)AM335X_GPIO1_BASE // Adress to GPIO peripheral
    );

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map); // errno also set!
        exit(-1);
    }

    gprev = (volatile unsigned*)(gpio_map+0x0000);
    printf("%x\n", *gprev);

/*
    gpfsel1 = (volatile unsigned*)(gpio_map+0x0004);
    gpset0  = (volatile unsigned*)(gpio_map+0x001C);
    gpclr0  = (volatile unsigned*)(gpio_map+0x0028);

    *gpfsel1 &= ~0x07000000;
    *gpfsel1 |= 0x01000000;

	*gpset0 = 0x00040000;
	usleep(500*1000);
	*gpclr0 = 0x00040000;
	usleep(500*1000);
*/


    return 0;
}
