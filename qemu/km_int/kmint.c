#include <linux/module.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>

/*=========================================================================*/

#define TIMER0_BASE 0x101E2000
#define TIMER2_BASE 0x101E3000

#define TIMER_EN 0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN 0x20

void __iomem *timer2_map;

volatile unsigned* timer2_load;
volatile unsigned* timer2_value;
volatile unsigned* timer2_control;

/*=========================================================================*/
static irqreturn_t sp804_timer_interrupt(int irq, void *dev_id)
{
    printk("sp804_timer_interrupt\n");
    return IRQ_HANDLED;
}

/*=========================================================================*/
int __init kmint_init(void)
{
    unsigned int hwirq_number;
    unsigned int linux_irq;

    printk("kmint_init\n");

    timer2_map = ioremap(TIMER2_BASE, 4096);
    if (!timer2_map) {
        printk("Failed to ioremap timer0\n");
        return -1;
    }

    hwirq_number = 5;
    linux_irq = irq_create_mapping(NULL, hwirq_number);
    if (!linux_irq) {
        pr_err("Failed to create IRQ mapping\n");
    } else {
        pr_info("Mapped Hardware IRQ %u to Linux IRQ: %u\n", hwirq_number, linux_irq);
    }

    if (request_irq(linux_irq, sp804_timer_interrupt, 0, "sp804", NULL) != 0) {
        pr_err("Failed register timer interrupt\n");
        return -ENODEV;
    }

    timer2_load = (volatile unsigned*)(timer2_map+0x0);
    timer2_value = (volatile unsigned*)(timer2_map+0x4);
    timer2_control = (volatile unsigned*)(timer2_map+0x8);
    printk("timer0_control: %x\n", *timer2_control);

    *timer2_load = 0xAFFF;
    *timer2_control = TIMER_EN | TIMER_PERIODIC | 0x4 | TIMER_INTEN;
    printk("timer0_control: %x\n", *timer2_control);

    return 0;
}

/*=========================================================================*/
void __exit kmint_exit(void)
{
    printk("kmint_exit\n");
    iounmap(timer2_map);
}

/*=========================================================================*/
module_init(kmint_init);
module_exit(kmint_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");

