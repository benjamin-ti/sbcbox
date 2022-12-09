#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <linux/ktime.h>

MODULE_LICENSE("GPL");

#define THREAD_NAME "pwmtsk"

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

unsigned int gpioStep = 73;
unsigned int gpioFail = 71;
unsigned int time_us = 200;
unsigned short jitter_gap_border_us = 100;

static int kmpwm_thread(void* arg)
{
    int i;
    ktime_t b4;
    int b = 0;
    long tdiff_us, diff_us;

    i = 0;
    while (1) {
        // printk("kmpwm_thread: %i\n", i++);
        gpio_set_value(gpioStep, b);
        b = !b;
        // < 10us: use udelay
        // 10us-20ms: use usleep_range
        // > 20ms: use msleep
        // msleep_interruptible(1000);
        b4 = ktime_get_boottime();
        // usleep_range(1000, 1000);
        udelay(time_us);
        tdiff_us = ktime_to_us(ktime_sub(ktime_get_boottime(), b4));
        diff_us = tdiff_us-time_us;
        if (diff_us > jitter_gap_border_us) {
            // printk("CurPWMDiff: %lu us\n", diff_us);
            static int a;
            gpio_set_value(gpioFail, a);
            a = !a;
        }

        if (kthread_should_stop())
            break;
    }

    printk("kmpwm_thread ends\n");

    return 0;
}

static int kmpwm_init(void)
{
    printk("init_module\n");

    if (!gpio_is_valid(gpioStep)) {
        printk("gpioStep is invalid\n");
    }
    else {
        printk("gpioStep is valid\n");
    }

    if (!gpio_is_valid(gpioFail)) {
        printk("gpioFail is invalid\n");
    }
    else {
        printk("gpioFail is valid\n");
    }

    gpio_request(gpioStep, "step");
    gpio_direction_output(gpioStep, false);
    gpio_export(gpioStep, false);

    gpio_request(gpioFail, "fail");
    gpio_direction_output(gpioFail, true);
    gpio_export(gpioFail, false);

    thread = kthread_run(kmpwm_thread, NULL, THREAD_NAME);

    return 0;
}

static void kmpwm_cleanup(void)
{
    printk("cleanup_module\n");
    kthread_stop(thread);
    gpio_unexport(gpioStep);
    gpio_free(gpioStep);
    gpio_unexport(gpioFail);
    gpio_free(gpioFail);
}

module_init(kmpwm_init);
module_exit(kmpwm_cleanup);
