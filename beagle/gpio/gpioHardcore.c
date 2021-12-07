#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file opening
#include <unistd.h> //  for sleep
#include <stdio.h>
#include <stdlib.h>

// GPIO Name = Z = 32*X + Y where X is the gpio register and Y is the position within that register
// i.e. GPIO2_24 is 32*2+24, making it GPIO_88
// AM335x-Manual Memory Map GPIO-Register
// GPIO0: 0x44E0 7000
// GPIO1: 0x4804 C000
// GPIO2: 0x481A C000
// GPIO3: 0x481A E000
#define AM335X_GPIO_BASE 0x44E07000

int main(void)
{
    volatile char* gpio_map;
    volatile unsigned* gprev;
    volatile unsigned* gpoe;
    volatile unsigned* gpset;
    volatile unsigned* gpclr;

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
        (off_t)AM335X_GPIO_BASE // Adress to GPIO peripheral
    );

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map); // errno also set!
        exit(-1);
    }

    gprev = (volatile unsigned*)(gpio_map+0x0000);
    printf("GPIO_REVISION: %x\n", *gprev);

    gpoe  = (volatile unsigned*)(gpio_map+0x134);
    gpclr = (volatile unsigned*)(gpio_map+0x190);
    gpset = (volatile unsigned*)(gpio_map+0x194);

    *gpoe &= ~0x00000080; // set GPIO_7 as output
    printf("GPIO_OE: %x\n", *gpoe);

    *gpset = 0x00000080;
    usleep(500*1000);
    *gpclr = 0x00000080;

    return 0;
}
