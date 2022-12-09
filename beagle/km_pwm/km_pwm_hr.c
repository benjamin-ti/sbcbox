#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");

#define THREAD_NAME "pwmhr"

unsigned int gpioStep = 73;
unsigned int gpioFail = 71;
unsigned int time_us = 500;
unsigned short jitter_gap_border_us = 300;

struct hrtimer hr_timer;
ktime_t exptime;

enum hrtimer_restart soft_pwm_hrtimer_callback(struct hrtimer *timer)
{
    static int b = 0;
    long diff_us;
    ktime_t now;

    now = ktime_get_boottime();
    gpio_set_value(gpioStep, b);
    b = !b;

    diff_us = ktime_to_us(ktime_sub(now, exptime));
    if (diff_us > jitter_gap_border_us) {
        // printk("CurPWMDiff: %lu us\n", diff_us);
        static int a;
        gpio_set_value(gpioFail, a);
        a = !a;
    }

    // hrtimer_forward_now(timer, ktime_set(0, time_us*1000));
    hrtimer_forward(timer, exptime, ktime_set(0, time_us*1000));
    exptime = ktime_add_ns(exptime,time_us*1000);
    // hrtimer_start(&hr_timer, exptime, HRTIMER_MODE_ABS);
    return HRTIMER_RESTART;
}

static int kmpwm_init(void)
{
    ktime_t now;

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

    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
    hr_timer.function = &soft_pwm_hrtimer_callback;

    now = ktime_get_boottime();
    exptime = ktime_add_ns(now, time_us*1000);

    hrtimer_start(&hr_timer, exptime, HRTIMER_MODE_ABS);

    return 0;
}

static void kmpwm_cleanup(void)
{
    printk("cleanup_module\n");
    hrtimer_cancel(&hr_timer);
    gpio_unexport(gpioStep);
    gpio_free(gpioStep);
    gpio_unexport(gpioFail);
    gpio_free(gpioFail);
}

module_init(kmpwm_init);
module_exit(kmpwm_cleanup);
