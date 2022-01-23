#include <linux/module.h>

int __init simple_rtdm_init(void)
{
    rtdm_printk("RTDM Init\n");
}

void __exit simple_rtdm_exit(void)
{
    rtdm_printk("RTDM Exit\n");
}

module_init(simple_rtdm_init);
module_exit(simple_rtdm_exit);
