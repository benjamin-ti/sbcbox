#include <linux/kernel.h>

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>

#include <linux/async_tx.h>
#include <linux/async.h>

#include <linux/fs.h>
#include <asm/io.h>

#include <linux/platform_device.h>
#include <linux/of_device.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

/*
dma_test {
	compatible = "cet,am335x-dma-test";
	status = "okay";
	dmas = <&edma_xbar 32 0 32>;
	dma-names = "dma_test";
};
*/

// Async API
// async_memcpy

static struct dma_chan *dma_ch;

static struct completion dma_m2m_ok;

static dma_addr_t  busSrc;

static int kmmemcpy(void)
{
    u16* src;
    u16* dest;
    size_t len = 0x20000;
    int i;

    struct dma_async_tx_descriptor *tx;
    struct async_submit_ctl submit;
    struct completion cmp;

    printk("init_module\n");

    src = kmalloc(len, GFP_KERNEL);
    dest = kmalloc(len, GFP_KERNEL);

    for (i=0; i<len/2; i++)
    {
    	src[i] = 0xDEAD;
        dest[i] = 0xAA55;
    }

    printk("src: %hx\n", src[0]);
    printk("dest: %hx\n", dest[0]);

    init_async_submit(&submit, 0, NULL, NULL, NULL,
                    NULL);

//    submit->depend_tx = tx;
    tx = async_memcpy((struct page *)dest, (struct page *)src, 0, 0, len, &submit);
    printk("tx: %hx\n", (unsigned int)tx);

    init_completion(&cmp);

    async_tx_issue_pending_all();

    wait_for_completion(&cmp);

    printk("src: %hx\n", src[0]);
    printk("dest: %hx\n", dest[0]);

    return 0;
}

static void dev_release(struct device *dev)
{
	printk("releasing dma capable device\n");
}

// static u64 _dma_mask = DMA_BIT_MASK(64);
static struct device dev = {
	.release = dev_release,
	.coherent_dma_mask = ~0, 	/* dma_alloc_coherent(): allow any address */
	.dma_mask = &dev.coherent_dma_mask,	/* other APIs: use the same mask as coherent */
};

// GPIO Name = Z = 32*X + Y where X is the gpio register and Y is the position within that register
// i.e. GPIO2_24 is 32*2+24, making it GPIO_88
// AM335x-Manual Memory Map GPIO-Register
// GPIO0: 0x44E0 7000
// GPIO1: 0x4804 C000
// GPIO2: 0x481A C000
// GPIO3: 0x481A E000
#define AM335X_GPIO0_BASE 0x44E07000
#define AM335X_GPIO2_BASE 0x481AC000

volatile unsigned* gprev;
volatile unsigned* gpoe;
volatile unsigned* gpset;
volatile unsigned* gpclr;

dma_addr_t  busDestSet = (AM335X_GPIO0_BASE+0x194);
dma_addr_t  busDestClr = (AM335X_GPIO0_BASE+0x190);

unsigned int uiDMACounter = 0;

unsigned int gpioIrqNumber;
void __iomem *gpio_map;

static int gpioHardcore(void)
{
    void __iomem *gpio_map2;
    volatile unsigned* gpoe2;
    volatile unsigned* gpin;

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
//    usleep_range(1000*1000, 1000*1000);
//    *gpclr = 0x00000080;
//    printk("Del\n");

    gpio_map2 = ioremap(AM335X_GPIO2_BASE, 4096);
    if (!gpio_map2) {
        printk("Failed to ioremap GPIO2\n");
        return -1;
    }

    gpoe2  = (volatile unsigned*)(gpio_map2+0x134);
    gpin   = (volatile unsigned*)(gpio_map2+0x138);
    printk("GPIO2_OE: %x\n", *gpoe2);
    printk("GPIO2_IN: %x\n", *gpin);

    iounmap(gpio_map2);

    return 0;
}

static void dma_clrgpio_callback(void *data);

static void dma_memcpy_callback(void *data)
{
	printk("dma_memcpy_callback\n");
	complete(&dma_m2m_ok);
}

static void dma_setgpio_callback(void *data)
{
	struct dma_async_tx_descriptor *dma_m2m_desc;
	dma_cookie_t cookie;

	printk("dma_setgpio_callback\n");

	uiDMACounter++;

    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
    		busDestClr, busSrc, 0x4, 0);
    if (!dma_m2m_desc) {
    	printk("device_prep_dma_memcpy failed\n");
    	return;
    }
    dma_m2m_desc->callback = dma_clrgpio_callback;

    cookie = dmaengine_submit(dma_m2m_desc);
    if (dma_submit_error(cookie)){
        printk(KERN_INFO "Failed to do DMA tx_submit");
        return;
    }

    dma_async_issue_pending(dma_ch);
}

static void dma_clrgpio_callback(void *data)
{
	struct dma_async_tx_descriptor *dma_m2m_desc;
	dma_cookie_t cookie;

	printk("dma_clrgpio_callback\n");

	if (uiDMACounter >= 10)
	{
		complete(&dma_m2m_ok);
	}
	else
	{
	    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
	    		busDestSet, busSrc, 0x4, 0);
	    if (!dma_m2m_desc) {
	    	printk("device_prep_dma_memcpy failed\n");
	    	return;
	    }
	    dma_m2m_desc->callback = dma_setgpio_callback;

	    cookie = dmaengine_submit(dma_m2m_desc);
	    if (dma_submit_error(cookie)){
	        printk(KERN_INFO "Failed to do DMA tx_submit");
	        return;
	    }

	    dma_async_issue_pending(dma_ch);
	}
}

/*
static int dma_init(void)
{
	dma_cap_mask_t mask;
	struct dma_slave_config dma_cfg;
	struct dma_async_tx_descriptor *dma_m2m_desc;
	dma_cookie_t cookie;

	int ret;

    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    dma_ch = dma_request_channel(mask, NULL, NULL);
    if (dma_ch == NULL)
    {
        printk("request_channel failed.\n");
        return -1;
    }

    printk("got dma_ch = 0x%x\n", (unsigned int)dma_ch);

    memset(&dma_cfg, 0, sizeof(struct dma_slave_config));
    dma_cfg.direction = DMA_MEM_TO_MEM;
    dma_cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    ret = dmaengine_slave_config(dma_ch, &dma_cfg);
    if (ret) {
    	printk("dmaengine_slave_config error\n");
       goto err;
    }
    printk("dmaengine_slave_config pass\n");

    if (dma_ch->device->device_prep_dma_memcpy == NULL)
    {
        printk("device_prep_dma_memcpy = NULL\n");
        goto err;
    }

    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
    								busDest, busSrc, len, 0);
    if (!dma_m2m_desc) {
    	printk("device_prep_dma_memcpy failed\n");
    	goto err;
    }
    dma_m2m_desc->callback = dma_memcpy_callback;

    cookie = dmaengine_submit(dma_m2m_desc);
//    cookie = dma_m2m_desc->tx_submit(dma_m2m_desc); //submit the desc
    if (dma_submit_error(cookie)){
        printk(KERN_INFO "Failed to do DMA tx_submit");
        goto err;
    }
}
*/

/*=========================================================================*/
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
    pr_info("gpio int occured in rising edge \n");
    volatile unsigned* gpeoi;
    gpeoi = (volatile unsigned*)(gpio_map+0x20);
    *gpeoi = 0;
    return(irq_handler_t) IRQ_HANDLED;
}

/*=========================================================================*/
static int kmfpga_probe(struct platform_device *pdev)
{
    u32* src;
    u32* dest;
    dma_addr_t  busDest;

    size_t len = 0x4;
    int i;
    int ret;

    dma_cap_mask_t mask;

    struct dma_slave_config dma_cfg;
    struct dma_async_tx_descriptor *dma_m2m_desc;
    dma_cookie_t cookie;

    printk("kmfpga_probe\n");

    gpioHardcore();
    usleep_range(2000*1000, 2000*1000);

    // Handle GPIO
    gpioIrqNumber = gpio_to_irq(20);
    pr_info("gpioIrqNumber = %d\n", gpioIrqNumber);
    if ( request_irq(gpioIrqNumber, (irq_handler_t)gpio_irq_handler,
                     IRQF_TRIGGER_RISING,
                     "kmmotor_gpio_handler", // Used in /proc/interrupts to identify the owner
                     NULL) )
    {
        pr_err("cannot register IRQ for GPIO");
        return -EAGAIN;
    }

    init_completion(&dma_m2m_ok);

    //src = kzalloc(len, GFP_KERNEL | GFP_DMA);
    src = dma_alloc_coherent(&pdev->dev, len, &busSrc, GFP_KERNEL);
    //dest = kzalloc(len, GFP_KERNEL | GFP_DMA);
    dest = dma_alloc_coherent(&pdev->dev, len, &busDest, GFP_KERNEL);
//    dest = gpset;
//    busDest = busDestSet;

    for (i=0; i<len/4; i++)
    {
    	src[i] = 0x00000080;
        dest[i] = 0xAA55DEAD;
    }

    printk("src: %x\n", src[0]);
    printk("dest: %x\n", dest[0]);

/*
    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    dma_ch = dma_request_channel(mask, NULL, NULL);
    if (dma_ch == NULL)
    {
        printk("request_channel failed.\n");
        return 0;
    }
*/

    // dma_ch = dma_request_chan(&pdev->dev, "gpioevt");
    dma_ch = dma_request_slave_channel(&pdev->dev, "kmfpga_evt");
    if (!dma_ch) {
        printk("no gpio channel for gpevt\n");
        return -EAGAIN;
    }
    printk("got dma_ch = 0x%x\n", (unsigned int)dma_ch);
//    printk("got dma_ch = 0x%x; %x; %x; %s\n", (unsigned int)dma_ch,
//           dma_ch->chan_id, dma_ch->dev->dev_id, dma_ch->dev->device.init_name);

    memset(&dma_cfg, 0, sizeof(struct dma_slave_config));
    dma_cfg.direction = DMA_MEM_TO_MEM;
//    dma_cfg.src_addr = busSrc;
//    dma_cfg.dst_addr = busDest;
//    dma_cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dma_cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
/*
    dma_cfg.src_maxburst = 1;
    dma_cfg.dst_maxburst = 1;
    dma_cfg.device_fc = 0;
    dma_cfg.slave_id = 0;
*/
    ret = dmaengine_slave_config(dma_ch, &dma_cfg);
    if (ret) {
    	printk("dmaengine_slave_config error\n");
       goto err;
    }
    printk("dmaengine_slave_config pass\n");

/*
    busSrc = dma_map_single(&dev, src, len, DMA_TO_DEVICE);
	if (dma_mapping_error(&dev, busSrc)) {
		printk("Could not map src buffer\n");
		goto err;
	}
	busDest = dma_map_single(&dev, dest, len, DMA_FROM_DEVICE);
	if (dma_mapping_error(&dev, busDest)) {
		printk("Could not map dst buffer\n");
		dma_unmap_single(&dev, busSrc, len, DMA_TO_DEVICE);
		goto err;
	}
*/

    if (dma_ch->device->device_prep_dma_memcpy == NULL)
    {
        printk("device_prep_dma_memcpy = NULL\n");
        goto err;
    }

    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
    								busDest, busSrc, len, DMA_CTRL_REUSE);
    if (!dma_m2m_desc) {
    	printk("device_prep_dma_memcpy failed\n");
    	goto err;
    }
    dma_m2m_desc->callback = dma_memcpy_callback;

    cookie = dmaengine_submit(dma_m2m_desc);
//    cookie = dma_m2m_desc->tx_submit(dma_m2m_desc); //submit the desc
    if (dma_submit_error(cookie)){
        printk(KERN_INFO "Failed to do DMA tx_submit");
        goto err;
    }

    printk("wait for dma fin\n");
    dma_async_issue_pending(dma_ch);
    wait_for_completion(&dma_m2m_ok);

/*
    usleep_range(1000*1000, 1000*1000);
    *gpclr = 0x00000080;
    usleep_range(2000*1000, 2000*1000);
*/
    // 2. DMA attempt
    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
    		busDestSet, busSrc, len, DMA_CTRL_REUSE);
    if (!dma_m2m_desc) {
    	printk("device_prep_dma_memcpy failed\n");
    	goto err;
    }
    dma_m2m_desc->callback = dma_memcpy_callback;
    // dma_m2m_desc->next = NULL;

    cookie = dmaengine_submit(dma_m2m_desc);
    if (dma_submit_error(cookie)){
        printk(KERN_INFO "Failed to do DMA tx_submit");
        goto err;
    }

    printk("wait for dma2 fin\n");
    dma_async_issue_pending(dma_ch);
    wait_for_completion(&dma_m2m_ok);


    printk("src: %x\n", src[0]);
    printk("dest: %x\n", dest[0]);

    dma_free_coherent(&pdev->dev, len, src, busSrc);
    dma_free_coherent(&pdev->dev, len, dest, busDest);

    return 0;

err:
/*
	dma_unmap_single(&dev, busSrc, len, DMA_TO_DEVICE);
	dma_unmap_single(&dev, busDest, len, DMA_FROM_DEVICE);

	kfree(src);
	kfree(dest);
*/
	if (dma_ch != NULL) {
		dma_release_channel(dma_ch);
		dma_ch = NULL;
	}

	free_irq(gpioIrqNumber, NULL);

    return -1;
}

/*=========================================================================*/
static int kmfpga_remove(struct platform_device *pdev)
{
    printk("kmfpga_remove\n");

    free_irq(gpioIrqNumber, NULL);

	if (dma_ch != NULL) {
		dma_release_channel(dma_ch);
	}

	iounmap(gpio_map);

    return 0;
}

/*=========================================================================*/
static struct of_device_id kmfpga_driver_ids[] = {
    {
        .compatible = "kmfpga",
    }, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, kmfpga_driver_ids);

static struct platform_driver kmfpga_driver = {
    .probe = kmfpga_probe,
    .remove = kmfpga_remove,
    .driver = {
        .name = "kmfpga_driver",
        .of_match_table = kmfpga_driver_ids,
    },
};

/*=========================================================================*/
int __init kmfpga_init(void)
{
    pr_info("kmfpga_init\n");

    if(platform_driver_register(&kmfpga_driver)) {
        printk("dt_probe - Error! Could not load driver\n");
        return -1;
    }

    pr_info("kmfpga module inserted successfully...\n");

    return 0;
}

/*=========================================================================*/
void __exit kmfpga_exit(void)
{
    platform_driver_unregister(&kmfpga_driver);

    pr_info("kmfpga_exit\n");
}

/*=========================================================================*/
//late_initcall(kmhello_init);
late_initcall(kmfpga_init);
//module_init(kmhello_init);
module_exit(kmfpga_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");
