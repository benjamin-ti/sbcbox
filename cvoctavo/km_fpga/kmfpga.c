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

static void dma_memcpy_callback(void *data)
{
	printk("dma_memcpy_callback\n");

	complete(&dma_m2m_ok);
}

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

static int kmhello_init(void)
{
    u16* src;
    dma_addr_t  busSrc;
    u16* dest;
    dma_addr_t  busDest;

    size_t len = 0x20000;
    int i;
    int ret;

    dma_cap_mask_t mask;

    struct dma_slave_config dma_cfg;
    struct dma_async_tx_descriptor *dma_m2m_desc;
    dma_cookie_t cookie;

    printk("init_module\n");

    init_completion(&dma_m2m_ok);

    //src = kzalloc(len, GFP_KERNEL | GFP_DMA);
    src = dma_alloc_coherent(NULL, len, &busSrc, GFP_KERNEL);
    //dest = kzalloc(len, GFP_KERNEL | GFP_DMA);
    dest = dma_alloc_coherent(NULL, len, &busDest, GFP_KERNEL);

    for (i=0; i<len/2; i++)
    {
    	src[i] = 0xDEAD;
        dest[i] = 0xAA55;
    }

    printk("src: %hx\n", src[0]);
    printk("dest: %hx\n", dest[0]);

    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    dma_ch = dma_request_channel(mask, NULL, NULL);
    if (dma_ch == NULL)
    {
        printk("request_channel failed.\n");
        return 0;
    }

    printk("got dma_ch = 0x%x\n", (unsigned int)dma_ch);

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

    printk("wait for dma fin\n");
    dma_async_issue_pending(dma_ch);
    wait_for_completion(&dma_m2m_ok);

    printk("src: %hx\n", src[0]);
    printk("dest: %hx\n", dest[0]);

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

    return 0;
}

static void kmhello_cleanup(void)
{
	if (dma_ch != NULL) {
		dma_release_channel(dma_ch);
	}
    printk("cleanup_module\n");
}

//late_initcall(kmhello_init);
late_initcall(kmhello_init);
//module_init(kmhello_init);
module_exit(kmhello_cleanup);

MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");
