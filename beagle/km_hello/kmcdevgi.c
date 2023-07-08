#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include<linux/uaccess.h>              //copy_to/from_user()

/*=========================================================================*/

static struct gpio_desc *test_gpio = NULL;

#define KERNEL_BUF_SIZE 1024
uint8_t *kernel_buffer;

dev_t chardev = 0;
static struct class *dev_class;
static struct cdev kmcdevgi_cdev;

/*=========================================================================*/
static int kmcdevgi_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        return 0;
}

/*=========================================================================*/
static int kmcdevgi_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}

/*=========================================================================*/
static ssize_t kmcdevgi_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    // Copy the data from the kernel space to the user-space
    if (copy_to_user(buf, kernel_buffer, KERNEL_BUF_SIZE))
    {
        pr_err("Data Read : Err!\n");
    }
    pr_info("Data Read : Done!\n");
    return KERNEL_BUF_SIZE;
}

/*=========================================================================*/
static ssize_t kmcdevgi_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    // Copy the data to kernel space from the user-space
    if (copy_from_user(kernel_buffer, buf, len))
    {
        pr_err("Data Write : Err!\n");
    }
    kernel_buffer[len] = '\0';
    pr_info("Data Write (%u): %s\n", len, kernel_buffer);
    return len;
}

/*=========================================================================*/
static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = kmcdevgi_read,
    .write      = kmcdevgi_write,
    .open       = kmcdevgi_open,
    .release    = kmcdevgi_release,
};

/*=========================================================================*/
static int kmcdevgi_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret = 0;
    const char *label;
    int my_value;

    printk("probe\n");

    // Creating Physical memory
    if ((kernel_buffer = kmalloc(KERNEL_BUF_SIZE , GFP_KERNEL)) == 0) {
        pr_info("Cannot allocate memory in kernel\n");
        ret = -ENOMEM;
        goto p_mem;
    }
    strcpy(kernel_buffer, "Hello_KMCDevGI");

    // --- Create Character Device ------------------------------------------

    // Allocating Major number
    // 0: firstminor should be the requested first minor number to use; it is usually 0.
    // 1: count is the total number of contiguous device numbers you are requesting.
    if ((alloc_chrdev_region(&chardev, 0, 1, "kmcdevgi")) <0) {
            pr_err("Cannot allocate major number for device\n");
            ret = -EAGAIN;
            goto p_mem;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(chardev), MINOR(chardev));

    // Creating cdev structure
    cdev_init(&kmcdevgi_cdev, &fops);
    // Adding character device to the system
    if((cdev_add(&kmcdevgi_cdev, chardev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto p_class;
    }

    // Creating struct class
    dev_class = class_create(THIS_MODULE, "kmcdevgi_class");
    if (IS_ERR(dev_class)) {
        pr_err("Cannot create the struct class for device\n");
        goto p_class;
    }

    // Creating device
    // 1. NULL: parent - pointer to the parent struct device of this new device, if any
    // 2. NULL: drvdata - the data to be added to the device for callbacks
    // fmt - string for the devices name
    // ... - variable arguments
    if (IS_ERR(device_create(dev_class, NULL, chardev, NULL, "kmcdevgi"))) {
        pr_err("Cannot create the Device\n");
        goto p_device;
    }

    // --- Get some example variables from device tree ----------------------

    // Check for device properties
    if(!device_property_present(dev, "label")) {
        printk("probe - Error! Device property 'label' not found!\n");
        ret = -EAGAIN;
        goto p_err;
    }
    if(!device_property_present(dev, "my_value")) {
        printk("probe - Error! Device property 'my_value' not found!\n");
        return -1;
    }
    if(!device_property_present(dev, "test-gpio")) {
        printk("probe - Error! Device property 'my_value' not found!\n");
        ret = -EAGAIN;
        goto p_err;
    }

    // Read device properties
    ret = device_property_read_string(dev, "label", &label);
    if(ret) {
        printk("probe - Error! Could not read 'label'\n");
        ret = -EAGAIN;
        goto p_err;
    }
    printk("probe - label: %s\n", label);
    ret = device_property_read_u32(dev, "my_value", &my_value);
    if(ret) {
        printk("probe - Error! Could not read 'my_value'\n");
        ret = -EAGAIN;
        goto p_err;
    }

    printk("probe - my_value: %d\n", my_value);
    test_gpio = gpiod_get(dev, "test", GPIOD_OUT_LOW);
    if(IS_ERR(test_gpio)) {
        printk("dt_gpio - Error! Could not setup the GPIO\n");
        ret = -1 * IS_ERR(test_gpio);
        goto p_err;
    }

    gpiod_set_value(test_gpio, 1);
    printk("test_gpio set\n");

    printk("probe done\n");

    return 0;

p_err:
    device_destroy(dev_class, chardev);
p_device:
    class_destroy(dev_class);
p_class:
    unregister_chrdev_region(chardev, 1);
p_mem:
    kfree(kernel_buffer);

    return ret;
}

/*=========================================================================*/
static int kmcdevgi_remove(struct platform_device *pdev)
{
    gpiod_set_value(test_gpio, 0);
    device_destroy(dev_class, chardev);
    class_destroy(dev_class);
    unregister_chrdev_region(chardev, 1);
    kfree(kernel_buffer);
    return 0;
}

/*=========================================================================*/
static struct of_device_id kmcdevgi_driver_ids[] = {
    {
        .compatible = "kmhello",
    }, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, kmcdevgi_driver_ids);

static struct platform_driver kmcdevgi_driver = {
    .probe = kmcdevgi_probe,
    .remove = kmcdevgi_remove,
    .driver = {
        .name = "kmcdevgi_driver",
        .of_match_table = kmcdevgi_driver_ids,
    },
};

/*=========================================================================*/
int __init kmcdevgi_init(void)
{
    if (platform_driver_register(&kmcdevgi_driver)) {
        printk("dt_probe - Error! Could not load driver\n");
        return -1;
    }

    return 0;
}

/*=========================================================================*/
void __exit kmcdevgi_exit(void)
{
    platform_driver_unregister(&kmcdevgi_driver);

    printk("kmcdevgi_exit done\n");
}

/*=========================================================================*/
module_init(kmcdevgi_init);
module_exit(kmcdevgi_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");

