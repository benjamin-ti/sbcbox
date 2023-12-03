#include <linux/module.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/delay.h>

/*=========================================================================*/

#define TIMER0_BASE 0x101E2000
#define TIMER2_BASE 0x101E3000

#define TIMER_EN 0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN 0x20

#define PIC_TIMER2_3 (1 << 5)

void __iomem *timer2_map;

volatile unsigned* timer2_load;
volatile unsigned* timer2_value;
volatile unsigned* timer2_control;
volatile unsigned* timer2_intclr;

unsigned int linux_irq;

volatile int dummy;

/*=========================================================================*/
static irqreturn_t sp804_timer_interrupt(int irq, void *dev_id)
{
	int i;
    printk("timer2_3_irq start\n");
    for (i=0; i<0x0AFFFFFF; i++) {
    		dummy++;
    }
    printk("timer2_3_irq end\n");
    *timer2_intclr = PIC_TIMER2_3;
    return IRQ_HANDLED;
}

/*=========================================================================*/
int __init kmint_init(void)
{
	static struct irq_domain *vic_irq_domain;
    unsigned int hwirq_number;
    struct device_node *vic_node = NULL;
    struct device_node *np;
    int ret;
    struct resource res;

    printk("kmint_init\n");

    timer2_map = ioremap(TIMER2_BASE, 4096);
    if (!timer2_map) {
        printk("Failed to ioremap timer0\n");
        return -1;
    }

    for_each_node_by_name(np, "intc") {
        ret = of_address_to_resource(np, 0, &res);
        if (ret) {
            pr_err("Failed to get resource for VIC node\n");
            return -ENODEV;
        }
        if (res.start == 0x10140000) {
        	vic_node = np;
        }
    }

    if (vic_node == NULL) {
    	pr_err("Failed to get resource for VIC node\n");
    	return -ENODEV;
    }

    vic_irq_domain = irq_find_host(vic_node);
    if (!vic_irq_domain) {
        pr_err("Failed to find IRQ domain for VIC\n");
        return -ENODEV;
    }

    hwirq_number = 5;
    linux_irq = irq_find_mapping(vic_irq_domain, hwirq_number);
    if (!linux_irq) {
        pr_err("Failed to find IRQ mapping\n");
    } else {
        pr_info("Hardware IRQ %u to Linux IRQ: %u\n", hwirq_number, linux_irq);
    }

    if (request_irq(linux_irq, sp804_timer_interrupt, 0, "kmint", NULL) != 0) {
        pr_err("Failed register timer interrupt\n");
        return -ENODEV;
    }

    timer2_load = (volatile unsigned*)(timer2_map+0x0);
    timer2_value = (volatile unsigned*)(timer2_map+0x4);
    timer2_control = (volatile unsigned*)(timer2_map+0x8);
    timer2_intclr = (volatile unsigned*)(timer2_map+0xC);
    printk("timer2_control: %x\n", *timer2_control);

    *timer2_load = 0xAFFF;
    *timer2_control = TIMER_EN | TIMER_PERIODIC | 0x4 | TIMER_INTEN;
    printk("timer2_control: %x\n", *timer2_control);

    return 0;
}

/*=========================================================================*/
void __exit kmint_exit(void)
{
    printk("kmint_exit\n");
    *timer2_control = 0;
    free_irq(linux_irq, NULL);
    iounmap(timer2_map);
}

/*=========================================================================*/
module_init(kmint_init);
module_exit(kmint_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");

