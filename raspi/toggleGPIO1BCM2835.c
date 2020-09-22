#include <bcm2835.h>
#include <sched.h>

#define PIN RPI_GPIO_P1_12 // Physical Pin Numbering

int main(int argc, char *argv[]) {
	const struct sched_param priority = {1};
	sched_setscheduler(0, SCHED_FIFO, &priority);
	
    if(!bcm2835_init())
        return 1;

    // Set the pin to be an output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

    while(1) {
        bcm2835_gpio_write(PIN, HIGH);
        bcm2835_delayMicroseconds(500);
        bcm2835_gpio_write(PIN, LOW);
        bcm2835_delayMicroseconds(500);
    }

    return 0;
}



