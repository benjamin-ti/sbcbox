#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");

#define THREAD_NAME "pwm"

static struct task_struct *thread;

/*
Atomic-Kontext (ISR, Tasklet, Soft-IRQ)
ndelay(unsigned long nsecs)     Verzögerungszeit in Nanosekunden
udelay(unsigned long usecs)     Verzögerungszeit in Mikrosekunden
mdelay(unsigned long msecs)     Verzögerungszeit in Millisekunden

Non-atomic-Kontext (Kernelthread, normale Treiberfunktionen)
udelay(unsigned long usecs)     Verzögerungszeit in Mikrosekunden
usleep_range(unsigned long min, unsigned long max)      Schlafenszeit in Mikrosekunden
msleep(unsigned long msecs)     Schlafenszeit in Mikrosekunden
msleep_interruptible(unsigned long msecs)       Schlafenszeit in Mikrosekunden
schedule_timeout(long timeout)  Schlafenszeit in Jiffies
schedule_timeout_interruptible(long timeout)    Schlafenszeit in Jiffies
*/

static int kmpwm_thread(void* arg)
{
    int i;
    unsigned int gpioCurr = 71;
    unsigned int gpioStep = 73;
    unsigned int gpioReset = 76;
    int b = 0;

    if (!gpio_is_valid(gpioCurr)) {
        printk("gpio is invalid\n");
    }
    else {
        printk("gpio is valid\n");
    }

    gpio_request(gpioCurr, "curr");
    gpio_direction_output(gpioCurr, true);
    gpio_set_value(gpioCurr, true);

    gpio_request(gpioStep, "step");
    gpio_direction_output(gpioStep, true);
    gpio_set_value(gpioStep, true);

    gpio_request(gpioReset, "reset");
    gpio_direction_output(gpioReset, true);
    gpio_set_value(gpioReset, true);

    i = 0;
    while (1) {
        // printk("kmpwm_thread: %i\n", i++);
        gpio_set_value(gpioCurr, b);
        gpio_set_value(gpioStep, b);
        b = !b;
        // < 10us: use udelay
        // 10us-20ms: use usleep_range
        // > 20ms: use msleep
        // msleep_interruptible(1000);
        usleep_range(1000, 1000);
        if (kthread_should_stop())
            break;
    }

    printk("kmpwm_thread ends\n");

    return 0;
}

static int kmpwm_init(void)
{
    printk("init_module\n");
    thread = kthread_run(kmpwm_thread, NULL, THREAD_NAME);
    return 0;
}

static void kmpwm_cleanup(void)
{
    printk("cleanup_module\n");
    kthread_stop(thread);
}

module_init(kmpwm_init);
module_exit(kmpwm_cleanup);
