#define GPIO1_BASE		0x4804C000

#define CM_PER_BASE		0x44e00000
#define CM_PER_GPIO1    0xAC

#define TIME 100000
void _main (void)
{
    unsigned int ui;

    volatile unsigned int* cm_per_base = (unsigned int*)((char*)CM_PER_BASE+CM_PER_GPIO1);
    volatile char* gpio_map = (char*)GPIO1_BASE;
    volatile unsigned* gprev;
    volatile unsigned* gpoe;
    volatile unsigned* gpset;
    volatile unsigned* gpclr;

	gprev = (volatile unsigned*)(gpio_map+0x0000);
    gpoe  = (volatile unsigned*)(gpio_map+0x134);
    gpclr = (volatile unsigned*)(gpio_map+0x190);
    gpset = (volatile unsigned*)(gpio_map+0x194);

    *cm_per_base = (1<<18 | 2);

    *gpoe &= ~(0xF<<21);

    while (1)
    {
        *gpset = (0xF<<21);
        for(ui=0; ui<TIME; ui++);
        *gpclr = (0xF<<21);
        for(ui=0; ui<TIME; ui++);
    }
}
