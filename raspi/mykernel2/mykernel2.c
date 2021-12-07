#include <linux/module.h> // Needed by all modules
#include <linux/kernel.h> // Needed for KERN_INFO
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>

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

#define GPIO_BASE_BUS 0x7E200000 //this is the physical bus address of the GPIO
                                 //module. This is only used when other
                                 //peripherals directly connected to the bus
                                 //(like DMA) need to read/write the GPIOs

// BCM-Manual 0x7Ennnn-Bus-Adresses go to 0x3Fnnnn-Phys-Adresses
#define DMA_BASE                 0x3F007000
#define BCM2835_GPIO_BASE        0x3F200000
#define BCM2835_PWM_BASE         0x3F20C000
#define BCM2835_PWM_BASE_BUS     0x7E20C000
#define BCM2835_CM_BASE          0x3F101000 // Clock-Manager; see https://elinux.org/BCM2835_registers
                                            // Or https://de.scribd.com/doc/127599939/BCM2835-Audio-clocks

//-------- Relative offsets for DMA registers
//DMA Channel register sets (format of these registers is found in DmaChannelHeader struct):
#define DMACH(n) (0x100*(n))
//Each DMA channel has some associated registers, but only CS (control and status), CONBLK_AD (control block address), and DEBUG are writeable
//DMA is started by writing address of the first Control Block to the DMA channel's CONBLK_AD register and then setting the ACTIVE bit inside the CS register (bit 0)
//Note: DMA channels are connected directly to peripherals, so physical addresses should be used (affects control block's SOURCE, DEST and NEXTCONBK addresses).
#define DMAENABLE 0x00000ff0 //bit 0 should be set to 1 to enable channel 0. bit 1 enables channel 1, etc.

//flags used in the DmaControlBlock struct:
#define DMA_CB_TI_DEST_INC (1<<4)
#define DMA_CB_TI_SRC_INC (1<<8)

#define DMA_NO_WIDE_BURSTS  (1<<26)
#define DMA_WAIT_RESP   (1<<3)
#define DMA_D_DREQ      (1<<6)
#define DMA_PER_MAP(x) ((x)<<16)

//flags used in the DmaChannelHeader struct:
#define DMA_CS_RESET (1<<31)
#define DMA_CS_ACTIVE (1<<0)

#define DMA_DEBUG_READ_ERROR (1<<2)
#define DMA_DEBUG_FIFO_ERROR (1<<1)
#define DMA_DEBUG_READ_LAST_NOT_SET_ERROR (1<<0)

struct DmaChannelHeader {
    //Note: dma channels 7-15 are 'LITE' dma engines (or is it 8-15?), with reduced performance & functionality.
    //Note: only CS, CONBLK_AD and DEBUG are directly writeable
    volatile uint32_t CS; //Control and Status
        //31    RESET; set to 1 to reset DMA
        //30    ABORT; set to 1 to abort current DMA control block (next one will be loaded & continue)
        //29    DISDEBUG; set to 1 and DMA won't be paused when debug signal is sent
        //28    WAIT_FOR_OUTSTANDING_WRITES(0x10000000); set to 1 and DMA will wait until peripheral says all writes have gone through before loading next CB
        //24-27 reserved
        //20-23 PANIC_PRIORITY; 0 is lowest priority
        //16-19 PRIORITY; bus scheduling priority. 0 is lowest
        //9-15  reserved
        //8     ERROR; read as 1 when error is encountered. error can be found in DEBUG register.
        //7     reserved
        //6     WAITING_FOR_OUTSTANDING_WRITES; read as 1 when waiting for outstanding writes
        //5     DREQ_STOPS_DMA(0x20); read as 1 if DREQ is currently preventing DMA
        //4     PAUSED(0x10); read as 1 if DMA is paused
        //3     DREQ; copy of the data request signal from the peripheral, if DREQ is enabled. reads as 1 if data is being requested (or PERMAP=0), else 0
        //2     INT; set when current CB ends and its INTEN=1. Write a 1 to this register to clear it
        //1     END; set when the transfer defined by current CB is complete. Write 1 to clear.
        //0     ACTIVE(0x01); write 1 to activate DMA (load the CB before hand)
    volatile uint32_t CONBLK_AD; //Control Block Address
    volatile uint32_t TI; //transfer information; see DmaControlBlock.TI for description
    volatile uint32_t SOURCE_AD; //Source address
    volatile uint32_t DEST_AD; //Destination address
    volatile uint32_t TXFR_LEN; //transfer length. ONLY THE LOWER 16 BITS ARE USED IN LITE DMA ENGINES
    volatile uint32_t STRIDE; //2D Mode Stride. Only used if TI.TDMODE = 1. NOT AVAILABLE IN LITE DMA ENGINES
    volatile uint32_t NEXTCONBK; //Next control block. Must be 256-bit aligned (32 bytes; 8 words)
    volatile uint32_t DEBUG; //controls debug settings
        //29-31 unused
        //28    LITE (0x10000000)
        //25-27 VERSION
        //16-24 DMA_STATE (dma engine state machine)
        //8-15  DMA_ID    (AXI bus id)
        //4-7   OUTSTANDING_WRITES
        //3     unused
        //2     READ_ERROR
        //1     WRITE_ERROR
        //0     READ_LAST_NOT_SET_ERROR
};

struct DmaControlBlock {
    volatile uint32_t TI; //transfer information
        //31:27 unused
        //26    NO_WIDE_BURSTS
        //21:25 WAITS; number of cycles to wait between each DMA read/write operation
        //16:20 PERMAP(0x000Y0000); peripheral number to be used for DREQ signal (pacing). set to 0 for unpaced DMA.
        //12:15 BURST_LENGTH
        //11    SRC_IGNORE; set to 1 to not perform reads. Used to manually fill caches
        //10    SRC_DREQ; set to 1 to have the DREQ from PERMAP gate requests.
        //9     SRC_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        //8     SRC_INC;   set to 1 to automatically increment the source address after each read (you'll want this if you're copying a range of memory)
        //7     DEST_IGNORE; set to 1 to not perform writes.
        //6     DEST_DREQ; set to 1 to have the DREQ from PERMAP gate *writes*
        //5     DEST_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        //4     DEST_INC;   set to 1 to automatically increment the destination address after each read (Tyou'll want this if you're copying a range of memory)
        //3     WAIT_RESP; make DMA wait for a response from the peripheral during each write. Ensures multiple writes don't get stacked in the pipeline
        //2     unused (0)
        //1     TDMODE; set to 1 to enable 2D mode
        //0     INTEN;  set to 1 to generate an interrupt upon completion
    volatile uint32_t SOURCE_AD; //Source address
    volatile uint32_t DEST_AD; //Destination address
    volatile uint32_t TXFR_LEN; //transfer length.
        //in 2D mode, TXFR_LEN is separated into two half-words to indicate Y transfers of length X, and STRIDE is added to the src/dest address after each transfer of length X.
        //30:31 unused
        //16-29 YLENGTH
        //0-15  XLENGTH
    volatile uint32_t STRIDE; //2D Mode Stride (amount to increment/decrement src/dest after each 1d copy when in 2d mode). Only used if TI.TDMODE = 1
        //16-31 D_STRIDE; signed (2's complement) byte increment/decrement to apply to destination addr after each XLENGTH transfer
        //0-15  S_STRIDE; signed (2's complement) byte increment/decrement to apply to source addr after each XLENGTH transfer
    volatile uint32_t NEXTCONBK; //Next control block. Must be 256-bit aligned (32 bytes; 8 words)
    uint32_t _reserved[2];
};

//set bits designated by (mask) at the address (dest) to (value), without affecting the other bits
//eg if x = 0b11001100
//  writeBitmasked(&x, 0b00000110, 0b11110011),
//  then x now = 0b11001110
void writeBitmasked(volatile uint32_t *dest, uint32_t mask, uint32_t value) {
    uint32_t cur = *dest;
    uint32_t new = (cur & (~mask)) | (value & mask);
    *dest = new;
    *dest = new; //added safety for when crossing memory barriers.
}

int init_module(void)
{
	void*  srcPage;
	void*  busSrcPage;
	void*  test;
	void*  bustest;
	struct DmaControlBlock *cb1Page;
	void*  busCb1Page;
	struct DmaControlBlock *cb2Page;
	void*  busCb2Page;
	struct DmaControlBlock *cb3Page;
	void*  busCb3Page;
	struct DmaControlBlock *cb4Page;
	void*  busCb4Page;
	size_t cbPageBytes = sizeof(struct DmaControlBlock);
	volatile struct DmaChannelHeader *dmaHeader;

	volatile char* gpio_map;
    volatile char* pwm_map;
    volatile char* cm_map;
    volatile char* dma_map;
    volatile unsigned* gpio_fsel0;
	volatile unsigned* gpio_fsel1;
	volatile unsigned* gpio_set0;
	volatile unsigned* gpio_clr0;
	void* gpio_set0_phys;
	void* gpio_clr0_phys;
    volatile unsigned* pwm_ctl;
    volatile unsigned* pwm_sta;
    volatile unsigned* pwm_dmac;
    volatile unsigned* pwm_rng1;
    volatile unsigned* pwm_dat1;
    volatile unsigned* pwm_fif1;
    volatile unsigned* cm_pwmctl;
    volatile unsigned* cm_pwmdiv;
    void* pwm_fif1_phys;

    volatile unsigned* dma_cs5;
    volatile unsigned* dma_conblkad5;
    volatile unsigned* dma_ti5;
    volatile unsigned* dma_sourcead5;
    volatile unsigned* dma_destad5;
    volatile unsigned* dma_txfrlen5;
    volatile unsigned* dma_stride5;
    volatile unsigned* dma_nextconbk5;
    volatile unsigned* dma_debug5;

	//see dma channel in use:
	//cat /sys/class/dma/dma0chan7/in_use
	int dmaChNum = 5;

	//now map /dev/mem into memory, but only map specific peripheral sections:
    volatile uint32_t *dmaBaseMem = ioremap(DMA_BASE, 4*1024);

	srcPage = dma_alloc_coherent(NULL, 4, (dma_addr_t*)&busSrcPage, GFP_KERNEL);
	test    = dma_alloc_coherent(NULL, 4, (dma_addr_t*)&bustest, GFP_KERNEL);
	cb1Page = dma_zalloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb1Page, GFP_KERNEL);
	cb2Page = dma_zalloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb2Page, GFP_KERNEL);
	cb3Page = dma_zalloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb3Page, GFP_KERNEL);
	cb4Page = dma_zalloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb4Page, GFP_KERNEL);

    gpio_map = ioremap(BCM2835_GPIO_BASE, 4*1024);
    pwm_map  = ioremap(BCM2835_PWM_BASE,  4*1024);
    cm_map   = ioremap(BCM2835_CM_BASE,   4*1024);
    dma_map  = ioremap(DMA_BASE,          4*1024);

    gpio_fsel0 = (volatile unsigned*)(gpio_map+0x0000);
	gpio_fsel1 = (volatile unsigned*)(gpio_map+0x0004);
	gpio_set0  = (volatile unsigned*)(gpio_map+0x001C);
	gpio_clr0  = (volatile unsigned*)(gpio_map+0x0028);
	gpio_set0_phys = (unsigned int*)(GPIO_BASE_BUS+0x001C);
	gpio_clr0_phys = (unsigned int*)(GPIO_BASE_BUS+0x0028);

	*gpio_fsel0 = 0x00000040; // Set GPIO2 to Out

	*gpio_set0 = 0x00000004;
	msleep(100);
	*gpio_clr0 = 0x00000004;
	msleep(100);
	*gpio_set0 = 0x00000004;
	msleep(100);
	*gpio_clr0 = 0x00000004;

    *gpio_fsel1 = 0x02000000; // ALT5->PWM0

    pwm_ctl  = (volatile unsigned*)(pwm_map+0x0000);
    pwm_sta  = (volatile unsigned*)(pwm_map+0x0004);
    pwm_dmac = (volatile unsigned*)(pwm_map+0x0008);
    pwm_rng1 = (volatile unsigned*)(pwm_map+0x0010);
    pwm_dat1 = (volatile unsigned*)(pwm_map+0x0014);
    pwm_fif1 = (volatile unsigned*)(pwm_map+0x0018);
    pwm_fif1_phys = (unsigned*)(BCM2835_PWM_BASE_BUS+0x0018);

    cm_pwmctl = (volatile unsigned*)(cm_map+0x00A0);
    cm_pwmdiv = (volatile unsigned*)(cm_map+0x00A4);

    dma_cs5     = (volatile unsigned*)(dma_map+0x0500);
    dma_conblkad5 = (volatile unsigned*)(dma_map+0x0504);
    dma_ti5     = (volatile unsigned*)(dma_map+0x0508);
    dma_sourcead5 = (volatile unsigned*)(dma_map+0x050C);
    dma_destad5 = (volatile unsigned*)(dma_map+0x0510);
    dma_txfrlen5 = (volatile unsigned*)(dma_map+0x0514);
    dma_stride5 = (volatile unsigned*)(dma_map+0x0518);
    dma_nextconbk5 = (volatile unsigned*)(dma_map+0x051C);
    dma_debug5  = (volatile unsigned*)(dma_map+0x0520);

    printk("\n");

    printk("cm_pwmctl: %x\n", *cm_pwmctl);

    *cm_pwmdiv = 0x5A0C0008; // Oscillator: 19,2 MHz : Teiler 192: Pulse 10 us
    msleep(100);
    *cm_pwmctl = 0x5A000001;
    msleep(100);
    *cm_pwmctl = 0x5A000011;
    while (!(0x00000080 & *cm_pwmctl));

    *pwm_rng1 = 4; // Range: x Pulses
    *pwm_dat1 = 1; // Count: Number of Pulses inside rng when not M/S mode

    *pwm_dmac = 0x00000707;

    printk("pwm_ctl: %x\n", *pwm_ctl);
    printk("cm_pwmctl: %x\n", *cm_pwmctl);

    *(volatile uint32_t*)test = 0x11111111;
	*(volatile uint32_t*)srcPage = 0x00000004;
    //*(volatile uint32_t*)srcPage = 0x23456789;
	msleep(100);
	printk("test: %x\n", *(uint32_t*)test);
	printk("srcPage: %x\n", *(uint32_t*)srcPage);

    // init dma control block
    cb1Page->TI = DMA_NO_WIDE_BURSTS | DMA_WAIT_RESP | DMA_D_DREQ | DMA_PER_MAP(5);
    cb1Page->SOURCE_AD = (uint32_t)busSrcPage;
    cb1Page->DEST_AD = (uint32_t)pwm_fif1_phys;
    //cb1Page->DEST_AD = (uint32_t)bustest;
    cb1Page->TXFR_LEN = 4;
    cb1Page->STRIDE = 0; //no 2D stride
    cb1Page->NEXTCONBK = (uint32_t)busCb2Page;

    cb2Page->TI = DMA_NO_WIDE_BURSTS;
    //cb2Page->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    cb2Page->SOURCE_AD = (uint32_t)busSrcPage; //set source and destination DMA address
    cb2Page->DEST_AD = (uint32_t)gpio_set0_phys;
    //cb2Page->DEST_AD = (uint32_t)gpio_clr0_phys;
    cb2Page->TXFR_LEN = 4;
    cb2Page->STRIDE = 0; //no 2D stride
    //cb2Page->NEXTCONBK = (uint32_t)busCb3Page;

    cb3Page->TI = DMA_NO_WIDE_BURSTS;
    //cb3Page->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    cb3Page->SOURCE_AD = (uint32_t)busSrcPage; //set source and destination DMA address
    cb3Page->DEST_AD = (uint32_t)gpio_clr0_phys;
    cb3Page->TXFR_LEN = 4;
    cb3Page->STRIDE = 0; //no 2D stride
    cb3Page->NEXTCONBK = (uint32_t)0; //busCb2Page;

    //enable DMA channel (it's probably already enabled, but we want to be sure):
    writeBitmasked(dmaBaseMem + DMAENABLE/4, 1 << dmaChNum, 1 << dmaChNum);

	//configure the DMA header to point to our control block:
    dmaHeader
		= (volatile struct DmaChannelHeader*)(dmaBaseMem + (DMACH(dmaChNum))/4);
		//dmaBaseMem is a uint32_t ptr, so divide by 4 before adding byte offset
    dmaHeader->CS = DMA_CS_RESET; //make sure to disable dma first.
    msleep(100); //give time for the reset command to be handled.
    dmaHeader->DEBUG = DMA_DEBUG_READ_ERROR | DMA_DEBUG_FIFO_ERROR
        | DMA_DEBUG_READ_LAST_NOT_SET_ERROR; // clear debug error flags
    dmaHeader->CONBLK_AD = (uint32_t)busCb1Page; //we have to point it to the PHYSICAL address of the control block (cb1)
    //*dma_conblkad5 = (uint32_t)busCb3Page;

    dmaHeader->CS = DMA_CS_ACTIVE; //set active bit, but everything else is 0.

    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 0;

    *pwm_dmac = 0x80000005;
    msleep(100);

    *pwm_ctl = 0x000000A1;
    msleep(100);

    *pwm_dmac = 0x00000000;
    *pwm_ctl = 0x000000E1;
    *gpio_clr0 = 0x00000004;
    msleep(100);
    *pwm_ctl = 0x000000E0;
    msleep(100);

    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 1;
    *pwm_fif1 = 2;
    *pwm_fif1 = 3;
    *pwm_fif1 = 0;

    dmaHeader->CONBLK_AD = (uint32_t)busCb1Page;
    dmaHeader->CS = DMA_CS_ACTIVE;
    *pwm_dmac = 0x80000005;
    *pwm_ctl = 0x000000A1;

    printk("gpio_fsel1: %x\n", *gpio_fsel1);

    printk("pwm_ctl: %x\n", *pwm_ctl);
    printk("pwm_sta: %x\n", *pwm_sta);
    printk("pwm_dmac: %x\n", *pwm_dmac);
    printk("pwm_rng1: %x\n", *pwm_rng1);
    printk("pwm_dat1: %x\n", *pwm_dat1);

    printk("cm_pwmctl: %x\n", *cm_pwmctl);
    printk("cm_pwmdiv: %x\n", *cm_pwmdiv);

    msleep(1000);

    *pwm_dmac = 0x00000000;
    dmaHeader->CS = DMA_CS_RESET; //make sure to disable dma first.
    msleep(100); //give time for the reset command to be handled.
    *dma_nextconbk5 = 0;

    printk("dma_cs5: %x\n", *dma_cs5);
    printk("dma_conblkad5: %x\n", *dma_conblkad5);
    printk("dma_ti5: %x\n", *dma_ti5);
    printk("dma_sourcead5: %x\n", *dma_sourcead5);
    printk("dma_destad5: %x\n", *dma_destad5);
    printk("dma_txfrlen5: %x\n",  *dma_txfrlen5);
    printk("dma_stride5: %x\n", *dma_stride5);
    printk("dma_nextconbk5: %x\n", *dma_nextconbk5);
    printk("dma_debug5: %x\n", *dma_debug5);

    *pwm_ctl = 0x000000E1;
    msleep(100);
    *pwm_ctl = 0x000000E0;
    *cm_pwmctl = 0x5A000001;
    while (0x00000080 & *cm_pwmctl);

    printk("test: %x\n", *(uint32_t*)test);
    msleep(100);

	dma_free_coherent(NULL, 4, srcPage, (dma_addr_t)busSrcPage);
	dma_free_coherent(NULL, 4, test,    (dma_addr_t)bustest);
	dma_free_coherent(NULL, cbPageBytes, cb1Page, (dma_addr_t)busCb1Page);
	dma_free_coherent(NULL, cbPageBytes, cb2Page, (dma_addr_t)busCb2Page);
	dma_free_coherent(NULL, cbPageBytes, cb3Page, (dma_addr_t)busCb3Page);
	dma_free_coherent(NULL, cbPageBytes, cb4Page, (dma_addr_t)busCb4Page);

	return 0;
}

void cleanup_module(void)
{

}
