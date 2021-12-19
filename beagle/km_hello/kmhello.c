#include <linux/module.h>

int kmhello_init(void)
{
    printk("init_module\n");
    return 0;
}

void kmhello_cleanup(void)
{
    printk("cleanup_module\n");
}

module_init(kmhello_init);
module_exit(kmhello_cleanup);
