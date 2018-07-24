#include <linux/module.h> // Needed by all modules
#include <linux/kernel.h> // Needed for KERN_INFO

// Load kernel module: insmod
// Remove kernel module: rmmod
// List kernel modules: lsmod
// See output 
// less var/log/syslog for reading it later or tailf var/log/syslog for "just in time"
// or dmesg

/*
modprobe is an intelligent command for listing, inserting as well as 
removing modules from the kernel. It searches in the module directory 
/lib/modules/$(uname -r) for all the modules and related files, but 
excludes alternative configuration files in the /etc/modprobe.d directory.

modprobe speedstep-lib

To remove a module, use the -r flag like this.
modprobe -r speedstep-lib
*/

int init_module(void)
{
	printk(KERN_ALERT "Hello world 1.\n");

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_ALERT "Goodbye world 1.\n");
}
