#include <bcm2835.h>

#define PIN RPI_GPIO_P1_12 // Physical Pin Numbering

int main(int argc, char *argv[]) {
    if(!bcm2835_init())
        return 1;

    // Set the pin to be an output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

    while(1) {
        bcm2835_gpio_write(PIN, HIGH);
        delay(500);
        bcm2835_gpio_write(PIN, LOW);
        delay(500);
    }

    return 0;
}
