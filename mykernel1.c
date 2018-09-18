#include <linux/module.h> // Needed by all modules
#include <linux/kernel.h> // Needed for KERN_INFO
#include <linux/slab.h>
#include <linux/dma-mapping.h>

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

#define HELLO_SIZE 40
static char* lk_pcHello;
static dma_addr_t lk_handle;

int init_module(void)
{
	// lk_pcHello = kmalloc(HELLO_SIZE, GFP_KERNEL | __GFP_DMA);
	lk_pcHello = dma_alloc_coherent(NULL, HELLO_SIZE, &lk_handle, GFP_KERNEL);
	snprintf((char*)lk_pcHello, HELLO_SIZE, "Hello World %x; %x!\n", lk_pcHello, lk_handle);

	return 0;
}

void cleanup_module(void)
{
	printk(lk_pcHello);
	// kfree(lk_pcHello);
	dma_free_coherent(NULL, HELLO_SIZE, lk_pcHello, lk_handle);
}
