#include <linux/module.h>

/*=========================================================================*/
int __init kmhello_init(void)
{
    printk("kmhello_init\n");
    return 0;
}

/*=========================================================================*/
void __exit kmhello_exit(void)
{
    printk("kmhello_exit\n");
}

/*=========================================================================*/
module_init(kmhello_init);
module_exit(kmhello_exit);

/*=========================================================================*/
MODULE_AUTHOR("Benjamin Tisler");
MODULE_LICENSE("GPL v2");
