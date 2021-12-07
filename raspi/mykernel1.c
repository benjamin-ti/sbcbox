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

#define bcm2835_peripherals_base 0x3F000000
#define BCM2835_GPIO_BASE        0x00200000
#define GPIO_BASE_BUS 0x7E200000 //this is the physical bus address of the GPIO 
                                 //module. This is only used when other 
                                 //peripherals directly connected to the bus 
                                 //(like DMA) need to read/write the GPIOs

#define DMA_BASE      0x3F007000

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
	struct DmaControlBlock *cb1Page;
	void*  busCb1Page;
	struct DmaControlBlock *cb2Page;
	void*  busCb2Page;
	size_t cbPageBytes = sizeof(struct DmaControlBlock);
	volatile struct DmaChannelHeader *dmaHeader;
	
	volatile unsigned* gpio;
	void* gpio_map;
	char* gpio_addr;
	volatile unsigned* gpio_fsel1;
	volatile unsigned* gpio_set0;
	volatile unsigned* gpio_clr0;
	void* gpio_set0_phys;
	void* gpio_clr0_phys;
	
	//see dma channel in use:
	//cat /sys/class/dma/dma0chan7/in_use
	int dmaChNum = 4; // 1st free
	
	//now map /dev/mem into memory, but only map specific peripheral sections:
    volatile uint32_t *dmaBaseMem = ioremap(DMA_BASE, 0x24);
	
	srcPage = dma_alloc_coherent(NULL, 4, (dma_addr_t*)&busSrcPage, GFP_KERNEL);
	cb1Page = dma_alloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb1Page, GFP_KERNEL);
	cb2Page = dma_alloc_coherent(NULL, cbPageBytes, (dma_addr_t*)&busCb2Page, GFP_KERNEL);
         
    gpio_map = ioremap(bcm2835_peripherals_base +
		               BCM2835_GPIO_BASE, // Adress to GPIO peripheral
		               4*1024); // Map length (always 4K)
             
	// Always use volatile pointer!
	gpio = (volatile unsigned*)gpio_map;
	gpio_addr = (char*)gpio_map;
	
	gpio_fsel1 = (volatile unsigned*)(gpio_addr+0x0004);
	gpio_set0  = (volatile unsigned*)(gpio_addr+0x001C);
	gpio_clr0  = (volatile unsigned*)(gpio_addr+0x0028);
	gpio_set0_phys = (unsigned int*)(GPIO_BASE_BUS+0x001C);
	gpio_clr0_phys = (unsigned int*)(GPIO_BASE_BUS+0x0028);
	
	printk("%x\n", (unsigned int)bcm2835_peripherals_base +
                                 BCM2835_GPIO_BASE+0x001C);
	printk("%x\n", (unsigned int)gpio_set0_phys);
	printk("%x\n", (unsigned int)gpio_clr0_phys);
	
	              // register numbering: Bit 31 ... Bit 0
	*gpio_fsel1 = 0x01000000; // Set GPIO18 to Out
	
	*gpio_set0 = 0x00040000;
	msleep(500);
	*gpio_clr0 = 0x00040000;
	msleep(500);
	   
	*(uint32_t*)srcPage = 0x00040000;
	   
    // init dma control block
    cb1Page->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    cb1Page->SOURCE_AD = (uint32_t)busSrcPage; //set source and destination DMA address
    cb1Page->DEST_AD = (uint32_t)gpio_set0_phys;
    cb1Page->TXFR_LEN = 4;
    cb1Page->STRIDE = 0; //no 2D stride
    cb1Page->NEXTCONBK = (uint32_t)busCb2Page;
	
    cb2Page->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    cb2Page->SOURCE_AD = (uint32_t)busSrcPage; //set source and destination DMA address
    cb2Page->DEST_AD = (uint32_t)gpio_clr0_phys;
    cb2Page->TXFR_LEN = 4;
    cb2Page->STRIDE = 0; //no 2D stride
    cb2Page->NEXTCONBK = (uint32_t)busCb1Page;
	
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
    dmaHeader->CS = DMA_CS_ACTIVE; //set active bit, but everything else is 0.
    
    msleep(1000); //give time for dma to happen

    dmaHeader->CS = DMA_CS_RESET; //make sure to disable dma first.
    msleep(100); //give time for the reset command to be handled.

	dma_free_coherent(NULL, 4, srcPage, (dma_addr_t)busSrcPage);
	dma_free_coherent(NULL, cbPageBytes, cb1Page, (dma_addr_t)busCb1Page);
	dma_free_coherent(NULL, cbPageBytes, cb2Page, (dma_addr_t)busCb2Page);	

	return 0;
}

void cleanup_module(void)
{
	
}
