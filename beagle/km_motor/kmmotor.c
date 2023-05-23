#include <linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()

#include <linux/delay.h>
#include <asm/io.h>

/*=========================================================================*/

// GPIO Name = Z = 32*X + Y where X is the gpio register and Y is the position within that register
// i.e. GPIO2_24 is 32*2+24, making it GPIO_88
// AM335x-Manual Memory Map GPIO-Register
// GPIO0: 0x44E0 7000
// GPIO1: 0x4804 C000
// GPIO2: 0x481A C000
// GPIO3: 0x481A E000
#define GPIO0_BASE 0x44E07000
#define GPIO0_OE           (volatile unsigned*)(gpio0_base+0x134)
#define GPIO0_CLEARDATAOUT (volatile unsigned*)(gpio0_base+0x190)
#define GPIO0_SETDATAOUT   (volatile unsigned*)(gpio0_base+0x194)

void __iomem *gpio0_base;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev kmmotor_cdev;

#define KERNEL_BUF_SIZE 1024
uint8_t *kernel_buffer;

static int      kmmotor_open(struct inode *inode, struct file *file);
static int      kmmotor_release(struct inode *inode, struct file *file);
static ssize_t  kmmotor_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  kmmotor_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = kmmotor_read,
    .write      = kmmotor_write,
    .open       = kmmotor_open,
    .release    = kmmotor_release,
};

bool gpio7;

/*=========================================================================*/
static int kmmotor_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        return 0;
}

/*=========================================================================*/
static int kmmotor_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}

/*=========================================================================*/
static ssize_t kmmotor_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
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
static ssize_t kmmotor_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
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
// sudo sh -c "echo Y > /sys/module/kmmotor/parameters/gpio7" to set to High
// sudo sh -c "echo N > /sys/module/kmmotor/parameters/gpio7" to set to Low
int notify_gpio7(const char *val, const struct kernel_param *kp)
{
	int res = param_set_bool(val, kp); // Use helper for write variable
	if (res==0) {
		if (gpio7) {
			*GPIO0_SETDATAOUT = (1<<7);
		}
		else {
			*GPIO0_CLEARDATAOUT = (1<<7);
		}
	}
	return -1;
}

const struct kernel_param_ops param_gpio7 =
{
	.set = &notify_gpio7, // Use our setter ...
	.get = &param_get_bool, // .. and standard getter
};

module_param_cb(gpio7, &param_gpio7, &gpio7, S_IRUSR|S_IWUSR);

/*=========================================================================*/
int __init kmmotor_init(void)
{
    pr_info("kmmotor_init\n");

    // Allocating Major number
    // 0: firstminor should be the requested first minor number to use; it is usually 0.
    // 1: count is the total number of contiguous device numbers you are requesting.
    if ((alloc_chrdev_region(&dev, 0, 1, "kmmotor")) <0) {
            pr_err("Cannot allocate major number for device\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    // Creating cdev structure
    cdev_init(&kmmotor_cdev, &fops);
    // Adding character device to the system
    if((cdev_add(&kmmotor_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    // Creating struct class
    dev_class = class_create(THIS_MODULE, "kmmotor_class");
    if (IS_ERR(dev_class)) {
        pr_err("Cannot create the struct class for device\n");
        goto r_class;
    }

    // Creating device
    // 1. NULL: parent - pointer to the parent struct device of this new device, if any
    // 2. NULL: drvdata - the data to be added to the device for callbacks
    // fmt - string for the devices name
    // ... - variable arguments
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "kmmotor"))) {
        pr_err("Cannot create the Device\n");
        goto r_device;
    }
    pr_info("kmmotor module inserted successfully...\n");

    // Creating Physical memory
    if ((kernel_buffer = kmalloc(KERNEL_BUF_SIZE , GFP_KERNEL)) == 0) {
        pr_info("Cannot allocate memory in kernel\n");
        goto r_device;
    }
    strcpy(kernel_buffer, "Hello_KMMotor");

    // Prepare GPIO use
	gpio0_base = ioremap(GPIO0_BASE, 4096);
    if (!gpio0_base) {
    	pr_err("Failed to ioremap GPIO\n");
    	goto r_device;
    }

    *GPIO0_OE &= ~(1<<7); // set GPIO_7 as output

    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

/*=========================================================================*/
void __exit kmmotor_exit(void)
{
	iounmap(gpio0_base);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    unregister_chrdev_region(dev, 1);
    pr_info("kmmotor_exit\n");
}

/*=========================================================================*/
module_init(kmmotor_init);
module_exit(kmmotor_exit);

/*=========================================================================*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("B.Tisler <btisler@carl-valentin.de>");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("0.1");
