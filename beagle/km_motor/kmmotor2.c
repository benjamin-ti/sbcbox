#include <linux/module.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/delay.h>

/*=========================================================================*/

// GPIO Name = Z = 32*X + Y where X is the gpio register and Y is the position within that register
// i.e. GPIO2_24 is 32*2+24, making it GPIO_88
// AM335x-Manual Memory Map GPIO-Register
// GPIO0: 0x44E0 7000
// GPIO1: 0x4804 C000
// GPIO2: 0x481A C000
// GPIO3: 0x481A E000
#define GPIO1_BASE 0x4804C000
#define GPIO1_OE           (volatile unsigned*)(gpio1_map+0x134)
#define GPIO1_CLEARDATAOUT (volatile unsigned*)(gpio1_map+0x190)
#define GPIO1_SETDATAOUT   (volatile unsigned*)(gpio1_map+0x194)
void __iomem *gpio1_map;

#define GPMCA0_BIT 16

#define CM_PER_BASE          0x44e00000
#define CM_PER_TIMER3   (volatile unsigned*)(cm_per_map+0x84)
void __iomem *cm_per_map;

#define DMTIMER3_BASE          0x48042000
#define DMTIMER3_EOI           (volatile unsigned*)(dmtimer3_map+0x20)
#define DMTIMER3_IRQSTATUS     (volatile unsigned*)(dmtimer3_map+0x28)
#define DMTIMER3_IRQENABLE_SET (volatile unsigned*)(dmtimer3_map+0x2C)
#define DMTIMER3_IRQENABLE_CLR (volatile unsigned*)(dmtimer3_map+0x30)
#define DMTIMER3_TCLR          (volatile unsigned*)(dmtimer3_map+0x38)
#define DMTIMER3_TCRR          (volatile unsigned*)(dmtimer3_map+0x3C)
void __iomem *dmtimer3_map;

#define INTCPS_BASE       0x48200000

#define INTC_ILR67        (volatile unsigned*)(intcps_map+0x20C)
#define INTC_ILR68        (volatile unsigned*)(intcps_map+0x210)
#define INTC_ILR69        (volatile unsigned*)(intcps_map+0x214)
void __iomem *intcps_map;

unsigned int linux_irq;

volatile int dummy;

/*=========================================================================*/
static irqreturn_t dmtimer_irq_handler(int irq, void *dev_id)
{
    static int b;
    unsigned int i;

    pr_info("irq 0\n");

    if (b) {
        *GPIO1_SETDATAOUT = (0x1<<GPMCA0_BIT);
    }
    else {
        *GPIO1_CLEARDATAOUT = (0x1<<GPMCA0_BIT);
    }
    b = !b;

    *DMTIMER3_IRQSTATUS = 0x2;
//    *DMTIMER3_TCRR = 0xFFFFF000;
    *DMTIMER3_TCRR = 0xFD000000;
    *DMTIMER3_TCLR = 0x1;
/*
    for (i=0; i<0x0FFFFFFF; i++) {
            dummy++;
    }
*/
    pr_info("irq 1\n");

    return IRQ_HANDLED;
}

/*=========================================================================*/
int __init kmmotor2_init(void)
{
    unsigned int hwirq_number;
    static struct irq_domain *vic_irq_domain;
    struct device_node *np;
    int ret;
    struct resource res;
    struct device_node *vic_node = NULL;

    pr_info("kmmotor2_init\n");

    // PER_MAP to turn on register access of DMTimer3
    cm_per_map = ioremap(CM_PER_BASE, 1024);
    if (!cm_per_map) {
        printk("Failed to ioremap cm_per_map\n");
        return -1;
    }
    *CM_PER_TIMER3 = 0x2;
    pr_info("cm_per_map: %x\n", *CM_PER_TIMER3);

    // DMTimer3 register access
    dmtimer3_map = ioremap(DMTIMER3_BASE, 4096);
    if (!dmtimer3_map) {
        printk("Failed to ioremap dmtimer3_map\n");
        goto d_err;
    }
    pr_info("dmtimer3_tclr: %x\n", *DMTIMER3_TCLR);

    // GPIO register access
    gpio1_map = ioremap(GPIO1_BASE, 4096);
    if (!gpio1_map) {
        printk("Failed to ioremap gpio1_map\n");
        goto g_err;
    }
    *GPIO1_OE &= ~(0x1<<GPMCA0_BIT);

    // INTCPS register access
    intcps_map = ioremap(INTCPS_BASE, 4096);
    if (!intcps_map) {
        printk("Failed to ioremap intcps_map\n");
        goto i_err;
    }

    // Get irq domain struct from search in device tree
    for_each_node_by_name(np, "interrupt-controller") {
        ret = of_address_to_resource(np, 0, &res);
        if (ret) {
            pr_err("Failed to get resource for VIC node\n");
            goto r_err;
        }
        if (res.start == 0x48200000) {
            vic_node = np;
        }
    }

    if (vic_node == NULL) {
        pr_err("Failed to get resource for VIC node\n");
        goto r_err;
    }

    vic_irq_domain = irq_find_host(vic_node);
    if (!vic_irq_domain) {
        pr_err("Failed to find IRQ domain for VIC\n");
        goto r_err;
    }

    // Translate hw irq number to linux irq number
    hwirq_number = 69;
    linux_irq = irq_find_mapping(vic_irq_domain, hwirq_number);
    if (!linux_irq) {
        pr_err("Failed to find IRQ mapping\n");
    } else {
        pr_info("Hardware IRQ %u to Linux IRQ: %u\n", hwirq_number, linux_irq);
    }

    // register interrupt with linux irq number
    if (request_irq(linux_irq, dmtimer_irq_handler, 0, "kmmotor2", NULL) != 0) {
        pr_err("Failed register timer interrupt\n");
        goto r_err;
    }

    pr_info("INTC_ILR67: %x\n", *INTC_ILR67);
    pr_info("INTC_ILR68: %x\n", *INTC_ILR68);
    pr_info("INTC_ILR69: %x\n", *INTC_ILR69);

    // Start DMTimer3
    *DMTIMER3_TCRR = 0xFFFF0000;
    *DMTIMER3_TCLR = 0x1;
    *DMTIMER3_IRQENABLE_SET = 0x2;

    // Success
    pr_info("kmmotor2 module inserted successfully...\n");

    return 0;

    // Error handling
r_err:
    iounmap(intcps_map);
i_err:
    iounmap(gpio1_map);
g_err:
    iounmap(dmtimer3_map);
d_err:
    iounmap(cm_per_map);
    return -ENODEV;
}

/*=========================================================================*/
void __exit kmmotor2_exit(void)
{

    pr_info("kmmotor2_exit\n");

    *DMTIMER3_TCLR = 0x0;
    *DMTIMER3_IRQENABLE_CLR = 0x2;
    free_irq(linux_irq, NULL);
    iounmap(intcps_map);
    iounmap(gpio1_map);
    iounmap(dmtimer3_map);
    iounmap(cm_per_map);
}

/*=========================================================================*/
module_init(kmmotor2_init);
module_exit(kmmotor2_exit);

/*=========================================================================*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("B.Tisler <btisler@carl-valentin.de>");
MODULE_DESCRIPTION("Driver to learn driver programming techniques");
MODULE_VERSION("0.2");
