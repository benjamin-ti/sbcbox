#include <linux/module.h>
#include <linux/gpio.h>

/*=========================================================================*/

// GPIO Name = Z = 32*X + Y where X is the gpio register and Y is the position within that register
// i.e. GPIO2_24 is 32*2+24, making it GPIO_88
// AM335x-Manual Memory Map GPIO-Register
// GPIO0: 0x44E0 7000
// GPIO1: 0x4804 C000
// GPIO2: 0x481A C000
// GPIO3: 0x481A E000
#define AM335X_GPIO0_BASE 0x44E07000
#define AM335X_GPIO2_BASE 0x481AC000

void __iomem *gpio_map;

volatile unsigned* gprev;
volatile unsigned* gpoe;
volatile unsigned* gpset;
volatile unsigned* gpclr;

/*=========================================================================*/
int __init kmgpio_init(void)
{
    printk("kmgpio_init\n");

    gpio_map = ioremap(AM335X_GPIO0_BASE, 4096);
    if (!gpio_map) {
        printk("Failed to ioremap GPIO0\n");
        return -1;
    }

    gprev = (volatile unsigned*)(gpio_map+0x0000);
    printk("GPIO_REVISION: %x\n", *gprev);

    gpoe  = (volatile unsigned*)(gpio_map+0x134);
    gpclr = (volatile unsigned*)(gpio_map+0x190);
    gpset = (volatile unsigned*)(gpio_map+0x194);

    *gpoe &= ~0x00000080; // set GPIO_7 as output
    printk("GPIO_OE: %x\n", *gpoe);

    *gpset = 0x00000080;
    printk("Set\n");

    return 0;
}

/*=========================================================================*/
void __exit kmgpio_exit(void)
{
    printk("kmgpio_exit\n");
    *gpclr = 0x00000080;
    iounmap(gpio_map);
}

/*=========================================================================*/
module_init(kmgpio_init);
module_exit(kmgpio_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");

