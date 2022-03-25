#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file opening
#include <unistd.h> //  for sleep
#include <stdio.h>
#include <stdlib.h>

#define AM572X_TP_CTRL_MODULE_CORE_TARG 0x4A002000 // 8KB
#define AM572X_TA_CTRL_MODULE_CORE_TARG 0x4A004000 // 4KB
#define AM572X_TP_GPIO1_TARG            0x4AE10000 // 4KB
#define AM572X_TP_GPIO5_TARG            0x4805B000 // 4KB

int main(void)
{
    volatile char* ctrl_module_core;
    volatile char* gpio5_targ;
    volatile unsigned* ctrl_mcasp1_aclkr;
    volatile unsigned* gpio5_oe;
    volatile unsigned* gpio5_set;
    volatile unsigned* gpio5_clr;

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        printf("Failed to open /dev/mem (did you remember to run as root?)\n");
        exit(1);
    }

    ctrl_module_core = mmap(
        NULL,             // Any adddress in our space will do
        8*1024,           // Map length (8K)
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       // Shared with other processes
        memfd,            // File to map
        (off_t)AM572X_TP_CTRL_MODULE_CORE_TARG // Adress to GPIO peripheral
    );
    if (ctrl_module_core == MAP_FAILED) {
        printf("mmap error %d\n", (int)ctrl_module_core); // errno also set!
        exit(-1);
    }

    gpio5_targ = mmap(
        NULL,             // Any adddress in our space will do
        4*1024,           // Map length (4K)
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       // Shared with other processes
        memfd,            // File to map
        (off_t)AM572X_TP_GPIO5_TARG // Adress to GPIO peripheral
    );
    if (gpio5_targ == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio5_targ); // errno also set!
        exit(-1);
    }


    ctrl_mcasp1_aclkr = (volatile unsigned*)(ctrl_module_core+0x16AC);
    // *ctrl_mcasp1_aclkr = 0x0000000E;
    printf("CTRL_MCASP1_ACLKR: %x\n", *ctrl_mcasp1_aclkr);

    gpio5_oe = (volatile unsigned*)(gpio5_targ+0x0134);
    *gpio5_oe &= 0xFFFFFFFE;
    printf("GPIO5_OE: %x\n", *gpio5_oe);

    gpio5_clr = (volatile unsigned*)(gpio5_targ+0x0190);
    gpio5_set = (volatile unsigned*)(gpio5_targ+0x0194);

    *gpio5_set = 0x01;
    usleep(2000*1000);
    *gpio5_clr = 0x01;

    return 0;
}
