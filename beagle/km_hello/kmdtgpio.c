#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

/*=========================================================================*/

static struct gpio_desc *test_gpio = NULL;
static bool gpio_set;
unsigned int gpioIrqNumber;

/*=========================================================================*/
// sudo sh -c "echo Y > /sys/module/kmdtgpio/parameters/gpio_set" to set to High
// sudo sh -c "echo N > /sys/module/kmdtgpio/parameters/gpio_set" to set to Low
int notify_gpiofs(const char *val, const struct kernel_param *kp)
{
    int res = param_set_bool(val, kp); // Use helper for write variable
    if (res==0) {
        if (gpio_set) {
            gpiod_set_value(test_gpio, 1);
        }
        else {
            gpiod_set_value(test_gpio, 0);
        }
    }
    return -1;
}

const struct kernel_param_ops param_gpio =
{
    .set = &notify_gpiofs, // Use our setter ...
    .get = &param_get_bool, // .. and standard getter
};

module_param_cb(gpio_set, &param_gpio, &gpio_set, S_IRUSR | S_IWUSR | S_IRGRP);

/*=========================================================================*/
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
    pr_info("gpio int occured in rising edge \n");
    return(irq_handler_t) IRQ_HANDLED;
}

/*=========================================================================*/
static int kmdtgpio_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;
    const char *label;
    int irq_gpio;

    struct pinctrl *p;
    struct pinctrl_state *s;

    printk("probe\n");

    // --- Get some example variables from device tree ----------------------
    // Check for device properties
    if(!device_property_present(dev, "label")) {
        printk("probe - Error! Device property 'label' not found!\n");
        return -1;
    }
    if(!device_property_present(dev, "irq-gpio")) {
        printk("probe - Error! Device property 'irq-gpioe' not found!\n");
        return -1;
    }
    if(!device_property_present(dev, "test-gpio")) {
        printk("probe - Error! Device property 'test-gpio' not found!\n");
        return -1;
    }

    // Read device properties
    ret = device_property_read_string(dev, "label", &label);
    if(ret) {
        printk("probe - Error! Could not read 'label'\n");
        return -1;
    }
    printk("probe - label: %s\n", label);
    ret = device_property_read_u32(dev, "irq-gpio", &irq_gpio);
    if(ret) {
        printk("probe - Error! Could not read 'my_value'\n");
        return -1;
    }
    printk("probe - irq-gpio: %d\n", irq_gpio);

    // --- Get gpio output from device tree ---------------------------------
    test_gpio = gpiod_get(dev, "test", GPIOD_OUT_LOW);
    if(IS_ERR(test_gpio)) {
        printk("dt_gpio - Error! Could not setup the GPIO\n");
        return -1 * IS_ERR(test_gpio);
    }

    // --- Set gpio input interrupt -----------------------------------------
    gpioIrqNumber = gpio_to_irq(irq_gpio);
    pr_info("gpioIrqNumber = %d\n", gpioIrqNumber);
    if ( request_irq(gpioIrqNumber, (irq_handler_t)gpio_irq_handler,
                     IRQF_TRIGGER_RISING,
                     "kmmotor_gpio_handler", // Used in /proc/interrupts to identify the owner
                     NULL) )
    {
        pr_err("cannot register IRQ for GPIO");
        return -1;
    }

    // --- Toggle gpio for test interrupt (should not work now) -------------
    // < 10us: use udelay
    // 10us-20ms: use usleep_range
    // > 20ms: use msleep
    // msleep_interruptible(1000);
    gpiod_set_value(test_gpio, 0);
    msleep(50);
    gpiod_set_value(test_gpio, 1);

    // --- Pin-Ctrl ---------------------------------------------------------
    p = devm_pinctrl_get(dev);
    if (IS_ERR(p)) {
        pr_err("devm_pinctrl_get failed");
        return -1;
    }
    s = pinctrl_lookup_state(p, "gpio"); // or: PINCTRL_STATE_DEFAULT
    if (IS_ERR(s)) {
        pr_err("pinctrl_lookup_state failed");
        return -1;
    }
    ret = pinctrl_select_state(p, s);
    if (ret < 0) {
        pr_err("pinctrl_select_state failed");
        return -1;
    }

    // --- Rest -------------------------------------------------------------
    gpiod_set_value(test_gpio, 1);
    printk("test_gpio set\n");

    printk("probe done\n");

    return 0;
}

/*=========================================================================*/
static int kmdtgpio_remove(struct platform_device *pdev)
{
    gpiod_set_value(test_gpio, 0);
    free_irq(gpioIrqNumber, NULL);
    return 0;
}

/*=========================================================================*/
static struct of_device_id kmdtgpio_driver_ids[] = {
    {
        .compatible = "kmhello",
    }, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, kmdtgpio_driver_ids);

static struct platform_driver kmdtgpio_driver = {
    .probe = kmdtgpio_probe,
    .remove = kmdtgpio_remove,
    .driver = {
        .name = "kmdtgpio_driver",
        .of_match_table = kmdtgpio_driver_ids,
    },
};

/*=========================================================================*/
int __init kmdtgpio_init(void)
{
    if (platform_driver_register(&kmdtgpio_driver)) {
        printk("dt_probe - Error! Could not load driver\n");
        return -1;
    }

    return 0;
}

/*=========================================================================*/
void __exit kmdtgpio_exit(void)
{
    platform_driver_unregister(&kmdtgpio_driver);

    printk("kmdtgpio_exit done\n");
}

/*=========================================================================*/
module_init(kmdtgpio_init);
module_exit(kmdtgpio_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");

