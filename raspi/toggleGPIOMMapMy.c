#include <stdio.h>
#include <bcm2835.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
	volatile unsigned* gpio;
	void* gpio_map;
	char* gpio_addr;
	volatile unsigned* gpio_fsel1;
	volatile unsigned* gpio_set0;
	volatile unsigned* gpio_clr0;
	int  mem_fd;
	
	if(!bcm2835_init())
		return 1;
		
	printf("%x\n", (unsigned int)bcm2835_peripherals_base +
                                 BCM2835_GPIO_BASE);
	
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem \n");
		exit(-1);
	}
 
	gpio_map = mmap(
      NULL,             // Any adddress in our space will do
      4*1024,           // Map length (always 4K)
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       // Shared with other processes
      mem_fd,           // File to map
      (off_t)bcm2835_peripherals_base +
          BCM2835_GPIO_BASE // Adress to GPIO peripheral
	);
 
	close(mem_fd); // No need to keep mem_fd open after mmap
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map); // errno also set!
      exit(-1);
	}
 
	// Always use volatile pointer!
	gpio = (volatile unsigned*)gpio_map;
	gpio_addr = (char*)gpio_map;
	
	gpio_fsel1 = (volatile unsigned*)(gpio_addr+0x0004);
	gpio_set0  = (volatile unsigned*)(gpio_addr+0x001C);
	gpio_clr0  = (volatile unsigned*)(gpio_addr+0x0028);
	
	printf("%x\n", (unsigned int)gpio_fsel1);
	printf("%x\n", (unsigned int)gpio_set0);
	printf("%x\n", (unsigned int)gpio_clr0);
	
	              // register numbering: Bit 31 ... Bit 0
	*gpio_fsel1 = 0x01000000; // Set GPIO18 to Out
	
	while(1) {
		*gpio_set0 = 0x00040000;
		usleep(500*1000);
		*gpio_clr0 = 0x00040000;
		usleep(500*1000);
	}
	
	return 0;
}
