#include <linux/module.h> // Needed by all modules
#include <linux/kernel.h> // Needed for KERN_INFO

int init_module(void)
{
	printk(KERN_ALERT "Hello world 1.\n");

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ALERT "Goodbye world 1.\n");
}

