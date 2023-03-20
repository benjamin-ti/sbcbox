#include <linux/module.h>
#include <linux/dmaengine.h>
#include <linux/slab.h>
#include <linux/types.h>

struct dma_chan *chan;

/*
void foo (void)
{
    int index = 0;
    dma_cookie_t cookie;
    size_t len = 0x20000;

    ktime_t start, end, end1, end2, end3;
    s64 actual_time;

    u16* dest;
    u16* src;

    dest = kmalloc(len, GFP_KERNEL);
    src = kmalloc(len, GFP_KERNEL);

    for (index = 0; index < len/2; index++)
    {
        dest[index] = 0xAA55;
        src[index] = 0xDEAD;
    }

    start = ktime_get();
    cookie = dma_async_memcpy_buf_to_buf(chan, dest, src, len);

    while (dma_async_is_tx_complete(chan, cookie, NULL, NULL) == DMA_IN_PROGRESS)
    {
       dma_sync_wait(chan, cookie);
    }
    end = ktime_get();
    actual_time = ktime_to_ns(ktime_sub(end, start));
    printk("Time taken for function() execution     dma: %lld\n",(long long)actual_time);

    memset(dest, 0 , len);

    start = ktime_get();
    memcpy(dest, src, len);

    end = ktime_get();
    actual_time = ktime_to_ns(ktime_sub(end, start));
    printk("Time taken for function() execution non-dma: %lld\n",(long long)actual_time);
}
*/

int kmhello_init(void)
{
    u16* dest;
    u16* src;
    size_t len = 0x20000;
    int i;
    int ret;

    dma_cap_mask_t mask;
    struct dma_chan *dma_ch;
    struct dma_slave_config dma_cfg;
    struct dma_async_tx_descriptor *dma_m2m_desc;


    printk("init_module\n");

    dest = kmalloc(len, GFP_KERNEL);
    src = kmalloc(len, GFP_KERNEL);

    for (i=0; i<len/2; i++)
    {
        dest[i] = 0xAA55;
        src[i] = 0xDEAD;
    }

    printk("src: %hu\n", src[0]);
    printk("dest: %hu\n", dest[0]);

    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    dma_ch = dma_request_channel(mask, 0, NULL);

    if (dma_ch == NULL)
    {
        printk("request_channel failed.\n");
        goto err;
    }
    printk("got dma_ch = 0x%x\n", dma_ch);


    memset(&dma_cfg, 0, sizeof(struct dma_slave_config));
    dma_cfg.direction = DMA_MEM_TO_MEM;
    dma_cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    ret = dmaengine_slave_config(dma_ch, &dma_cfg);
    if (ret)
       goto err;
    printk("dmaengine_slave_config pass.\n");

    dma_m2m_desc = dma_ch->device->device_prep_dma_memcpy(dma_ch,
                                        dest, src, len, 0);
//    dma_m2m_desc->callback = dma_memcpy_callback_from_fpga;
    dmaengine_submit(dma_m2m_desc);
    dma_async_issue_pending (dma_ch);

    printk("src: %hu\n", src[0]);
    printk("dest: %hu\n", dest[0]);

err:
    return 0;
}

void kmhello_cleanup(void)
{
    printk("cleanup_module\n");
}

module_init(kmhello_init);
module_exit(kmhello_cleanup);
