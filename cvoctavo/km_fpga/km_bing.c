#include <linux/module.h>
#include <linux/init.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

static struct dma_chan *chan;
static dma_addr_t dma_addr;
static void *virt_addr;

static int __init dma_memcpy_init(void)
{
    struct device *dev = NULL;
    struct scatterlist sg;
    int ret;

    dev = &pdev->dev;

    chan = dma_request_chan(dev, "dma_memcpy");
    if (IS_ERR(chan)) {
        dev_err(dev, "dma_request_chan failed\n");
        return PTR_ERR(chan);
    }

    virt_addr = dma_alloc_coherent(dev, PAGE_SIZE, &dma_addr, GFP_KERNEL);
    if (!virt_addr) {
        dev_err(dev, "dma_alloc_coherent failed\n");
        ret = -ENOMEM;
        goto err_dma_alloc_coherent;
    }

    sg_init_one(&sg, virt_addr, PAGE_SIZE);
    ret = dma_map_sg(dev, &sg, 1, DMA_TO_DEVICE);
    if (ret != 1) {
        dev_err(dev, "dma_map_sg failed\n");
        goto err_dma_map_sg;
    }

    ret = dma_async_memcpy_to_device(chan, dma_addr, virt_addr, PAGE_SIZE);
    if (ret != 0) {
        dev_err(dev, "dma_async_memcpy_to_device failed\n");
        goto err_dma_async_memcpy_to_device;
    }

    return 0;

err_dma_async_memcpy_to_device:
err_dma_map_sg:
    dma_free_coherent(dev, PAGE_SIZE, virt_addr, dma_addr);
err_dma_alloc_coherent:
    dma_release_channel(chan);

    return ret;
}

static void __exit dma_memcpy_exit(void)
{
    struct device *dev = NULL;

    dev = &pdev->dev;

    dma_unmap_sg(dev, sg_cpu_ptr(&sg), 1, DMA_TO_DEVICE);
    dma_free_coherent(dev, PAGE_SIZE, virt_addr, dma_addr);
    dma_release_channel(chan);
}

module_init(dma_memcpy_init);
module_exit(dma_memcpy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("DMA memcpy example module");
