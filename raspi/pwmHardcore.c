#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file opening
#include <unistd.h> //  for sleep
#include <stdio.h>
#include <stdlib.h>

// BCM-Manual 0x7Ennnn-Bus-Adresses go to 0x3Fnnnn-Phys-Adresses
#define BCM2835_GPIO_BASE        0x3F200000
#define BCM2835_PWM_BASE         0x3F20C000
#define BCM2835_CM_BASE          0x3F101000 // Clock-Manager; see https://elinux.org/BCM2835_registers
                                            // Or https://de.scribd.com/doc/127599939/BCM2835-Audio-clocks

int main(void)
{
    volatile char* gpio_map;
    volatile char* pwm_map;
    volatile char* cm_map;
    volatile unsigned* gpfsel0;
    volatile unsigned* gpfsel1;
    volatile unsigned* gpset0;
    volatile unsigned* gpclr0;
    volatile unsigned* ctl;
    volatile unsigned* sta;
    volatile unsigned* dmac;
    volatile unsigned* rng1;
    volatile unsigned* dat1;
    volatile unsigned* fif1;
    volatile unsigned* cmpwmctl;
    volatile unsigned* cmpwmdiv;

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
		(off_t)BCM2835_GPIO_BASE // Adress to GPIO peripheral
    );
    if (gpio_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)gpio_map); // errno also set!
		exit(-1);
	}

    pwm_map = mmap(
		NULL,             // Any adddress in our space will do
		4*1024,           // Map length (always 4K)
		PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
		MAP_SHARED,       // Shared with other processes
		memfd,            // File to map
		(off_t)BCM2835_PWM_BASE // Adress to GPIO peripheral
    );
    if (pwm_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)pwm_map); // errno also set!
		exit(-1);
	}

    cm_map = mmap(
		NULL,             // Any adddress in our space will do
		4*1024,           // Map length (always 4K)
		PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
		MAP_SHARED,       // Shared with other processes
		memfd,            // File to map
		(off_t)BCM2835_CM_BASE // Adress to GPIO peripheral
    );
    if (pwm_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)pwm_map); // errno also set!
		exit(-1);
	}

    gpfsel0 = (volatile unsigned*)(gpio_map+0x0000);
    gpfsel1 = (volatile unsigned*)(gpio_map+0x0004);
    gpset0  = (volatile unsigned*)(gpio_map+0x001C);
    gpclr0  = (volatile unsigned*)(gpio_map+0x0028);

	*gpfsel0 = 0x00000040; // Set GPIO2 to Out

	*gpset0 = 0x00000004;
	usleep(100 * 1000);
	*gpclr0 = 0x00000004;
	usleep(100 * 1000);
	*gpset0 = 0x00000004;
	usleep(100 * 1000);
	*gpclr0 = 0x00000004;

    *gpfsel1 &= 0x07000000;
    *gpfsel1 |= 0x02000000; // ALT5->PWM0

    ctl = (volatile unsigned*)(pwm_map+0x0000);
    sta = (volatile unsigned*)(pwm_map+0x0004);
    dmac = (volatile unsigned*)(pwm_map+0x0008);
    rng1 = (volatile unsigned*)(pwm_map+0x0010);
    dat1 = (volatile unsigned*)(pwm_map+0x0014);
    fif1 = (volatile unsigned*)(pwm_map+0x0018);

    cmpwmctl = (volatile unsigned*)(cm_map+0x00A0);
    cmpwmdiv = (volatile unsigned*)(cm_map+0x00A4);

    printf("cmpwmctl: %x\n", *cmpwmctl);

    *cmpwmdiv = 0x5A0C0008; // Oscillator: 19,2 MHz : Teiler 192: Pulse 10 us
    usleep(100 * 1000);
    *cmpwmctl = 0x5A000001;
    usleep(100 * 1000);
    *cmpwmctl = 0x5A000011;
    while (!(0x00000080 & *cmpwmctl));

    *rng1 = 4; // Range: x Pulses
    *dat1 = 1; // Count: Number of Pulses inside rng when not M/S mode

    *ctl = 0x000000E1;
    usleep(100 * 1000);

    *fif1 = 1;
    *fif1 = 2;
    *fif1 = 3;
    *fif1 = 1;
    *fif1 = 2;
    *fif1 = 3;
    *fif1 = 2;

    printf("gpfsel1: %x\n", *gpfsel1);

    printf("ctl: %x\n", *ctl);
    printf("sta: %x\n", *sta);
    printf("dmac: %x\n", *dmac);
    printf("rng1: %x\n", *rng1);
    printf("dat1: %x\n", *dat1);

    printf("cmpwmctl: %x\n", *cmpwmctl);
    printf("cmpwmdiv: %x\n", *cmpwmdiv);

    sleep(7);

    *ctl = 0x000000E1;
    usleep(100 * 1000);
    *ctl = 0x000000E0;
    *cmpwmctl = 0x5A000001;
    while (0x00000080 & *cmpwmctl);

    printf("ctl: %x\n", *ctl);
    printf("cmpwmctl: %x\n", *cmpwmctl);

    return 0;
}
