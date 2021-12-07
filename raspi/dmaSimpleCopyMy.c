#include <sys/mman.h> //for mmap
#include <unistd.h> //for NULL
#include <stdio.h> //for printf
#include <stdlib.h> //for exit
#include <fcntl.h> //for file opening
#include <stdint.h> //for uint32_t
#include <string.h> //for memset
#include <errno.h> //for errno

#define PAGE_SIZE 4096 //mmap maps pages of memory, so we must give it multiples of this size

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

size_t ceilToPage(size_t size) {
    //round up to nearest page-size multiple
    if (size & (PAGE_SIZE-1)) {
        size += PAGE_SIZE - (size & (PAGE_SIZE-1));
    }
    return size;
}

//allocate some memory and lock it so that its physical address will never change
void* makeLockedMem(size_t size) {
    //void* mem = valloc(size); //memory returned by valloc is not zero'd
    size = ceilToPage(size);
    void *mem = mmap(
        NULL,   //let kernel place memory where it wants
        size,   //length
        PROT_WRITE | PROT_READ, //ask for read and write permissions to memory
        MAP_SHARED | 
        MAP_ANONYMOUS | //no underlying file; initialize to 0
        MAP_NORESERVE | //don't reserve swap space
        MAP_LOCKED, //lock into *virtual* ram. Physical ram may still change!
        -1,	// File descriptor
    0); //no offset into file (file doesn't exist).
    if (mem == MAP_FAILED) {
        printf("makeLockedMem failed\n");
        exit(1);
    }
    memset(mem, 0, size); //simultaneously zero the pages and force them into memory
    mlock(mem, size);
    return mem;
}

//free memory allocated with makeLockedMem
void freeLockedMem(void* mem, size_t size) {
    size = ceilToPage(size);
    munlock(mem, size);
    munmap(mem, size);
}

uintptr_t virtToPhys(void* virt, int pagemapfd) {
    uintptr_t pgNum = (uintptr_t)(virt)/PAGE_SIZE;
    int byteOffsetFromPage = (uintptr_t)(virt)%PAGE_SIZE;
    uint64_t physPage;
    // /proc/self/pagemap is a uint64_t array where the index represents 
    //the virtual page number and the value at that index represents the 
    //physical page number.
    //So if virtual address is 0x1000000, read the value at *array* 
    //index 0x1000000/PAGE_SIZE and multiply that by PAGE_SIZE to get the 
    //physical address.
    //because files are bytestreams, one must explicitly multiply each 
    //byte index by 8 to treat it as a uint64_t array.
    int err = lseek(pagemapfd, pgNum*8, SEEK_SET);
    if (err != pgNum*8) {
        printf("WARNING: virtToPhys %p failed to seek (expected %i got %i. errno: %i)\n", virt, pgNum*8, err, errno);
    }
    read(pagemapfd, &physPage, 8);
    if (!physPage & (1ull<<63)) { //bit 63 is set to 1 if the page is present in ram
        printf("WARNING: virtToPhys %p has no physical address\n", virt);
    }
    physPage = physPage & ~(0x1ffull << 55); //bits 55-63 are flags.
    uintptr_t mapped = (uintptr_t)(physPage*PAGE_SIZE + byteOffsetFromPage);
    return mapped;
}

uintptr_t virtToUncachedPhys(void *virt, int pagemapfd) {
	//bus address of the ram is 0x40000000. With this binary-or, writes 
	//to the returned address will bypass the CPU (L1) cache, but not 
	//the L2 cache. 0xc0000000 should be the base address if L2 must 
	//also be bypassed. However, the DMA engine is aware of L2 cache 
	//- just not the L1 cache (source: 
	//http://en.wikibooks.org/wiki/Aros/Platforms/Arm_Raspberry_Pi_support#Framebuffer )
    return virtToPhys(virt, pagemapfd) | 0x40000000; 
}

void* makeUncachedMemView(void* virtaddr, size_t bytes, int memfd, int pagemapfd) {
    //by default, writing to any virtual address will go through the CPU cache.
    //this function will return a pointer that behaves the same as virtaddr, but 
    //bypasses the CPU L1 cache (note that because of this, the returned pointer 
    //and original pointer should not be used in conjunction, else cache-related 
    //inconsistencies will arise)
    //Note: The original memory should not be unmapped during the lifetime of 
    //the uncached version, as then the OS won't know that our process still owns 
    //the physical memory.
    bytes = ceilToPage(bytes);
    //first, just allocate enough *virtual* memory for the operation. This is done 
    //so that we can do the later mapping to a contiguous range of virtual memory:
    void *mem = mmap(
        NULL,   //let kernel place memory where it wants
        bytes,   //length
        PROT_WRITE | PROT_READ, //ask for read and write permissions to memory
        MAP_SHARED | 
        MAP_ANONYMOUS | //no underlying file; initialize to 0
        MAP_NORESERVE | //don't reserve swap space
        MAP_LOCKED, //lock into *virtual* ram. Physical ram may still change!
        -1,	// File descriptor
    0); //no offset into file (file doesn't exist).
    //now, free the virtual memory and immediately remap it to the physical 
    //addresses used in virtaddr
    munmap(mem, bytes); //Might not be necessary; MAP_FIXED indicates it can map an already-used page
    for (int offset=0; offset<bytes; offset += PAGE_SIZE) {
        void *mappedPage = mmap(mem+offset, PAGE_SIZE, PROT_WRITE|PROT_READ, 
                                MAP_SHARED|MAP_FIXED|MAP_NORESERVE|MAP_LOCKED, 
                                memfd, virtToUncachedPhys(virtaddr+offset, pagemapfd));
        if (mappedPage != mem+offset) { //We need these mappings to be contiguous over virtual memory (in order to replicate the virtaddr array), so we must ensure that the address we requested from mmap was actually used.
            printf("Failed to create an uncached view of memory at addr %p+0x%08x\n", virtaddr, offset);
            exit(1);
        }
    }
    memset(mem, 0, bytes); //Although the cached version might have been reset, those writes might not have made it through.
    return mem;
}

//free memory allocated with makeLockedMem
void freeUncachedMemView(void* mem, size_t size) {
    size = ceilToPage(size);
    munmap(mem, size);
}

//map a physical address into our virtual address space. memfd is the 
//file descriptor for /dev/mem
volatile uint32_t* mapPeripheral(int memfd, int addr) {
    ///dev/mem behaves as a file. We need to map that file into memory:
    //NULL = virtual address of mapping is chosen by kernel.
    //PAGE_SIZE = map 1 page.
    //PROT_READ|PROT_WRITE means give us read and write priveliges to the memory
    //MAP_SHARED means updates to the mapped memory should be written back to the file & shared with other processes
    //memfd = /dev/mem file descriptor
    //addr = offset in file to map
    void *mapped = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, 
                        memfd, addr);
    //now, *mapped = memory at physical address of addr.
    if (mapped == MAP_FAILED) {
        printf("failed to map memory (did you remember to run as root?)\n");
        exit(1);
    } else {
        printf("mapped: %p\n", mapped);
    }
    return (volatile uint32_t*)mapped;
}

int main(void)
{
	//see dma channel in use:
	//cat /sys/class/dma/dma0chan7/in_use
	int dmaChNum = 4; // 1st free
	//First, open the linux device, /dev/mem
    //dev/mem provides access to the physical memory of the entire processor+ram
    //This is needed because Linux uses virtual memory, thus the process's 
    //memory at 0x00000000 will NOT have the same contents as the physical 
    //memory at 0x00000000
    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        printf("Failed to open /dev/mem (did you remember to run as root?)\n");
        exit(1);
    }
    int pagemapfd = open("/proc/self/pagemap", O_RDONLY);
  
    //now map /dev/mem into memory, but only map specific peripheral sections:
    volatile uint32_t *dmaBaseMem = mapPeripheral(memfd, DMA_BASE);
	
	//create src and dest memory
	void *virtSrcPageCached = makeLockedMem(PAGE_SIZE/2);
	void *virtSrcPage = virtSrcPageCached;
//	void *virtSrcPage = makeUncachedMemView(virtSrcPageCached, PAGE_SIZE/2, 
//                                           memfd, pagemapfd);                                           
	void *physSrcPage = (void*)virtToUncachedPhys(virtSrcPageCached, pagemapfd);
	void *virtDestPageCached = makeLockedMem(PAGE_SIZE/2);
	void *virtDestPage = virtDestPageCached;
//	void *virtDestPage = makeUncachedMemView(virtDestPageCached, PAGE_SIZE/2, 
//                                           memfd, pagemapfd);
	void *physDestPage = (void*)virtToUncachedPhys(virtDestPageCached, pagemapfd);
    char *srcArray = (char*)virtSrcPage;
    snprintf(srcArray, PAGE_SIZE/2, "Hello World");
	
	//create and init dma control block
	size_t cbPageBytes = sizeof(struct DmaControlBlock);
    void *virtCbPageCached = makeLockedMem(cbPageBytes);
    void *virtCbPage = virtCbPageCached;
//    void *virtCbPage = makeUncachedMemView(virtCbPageCached, cbPageBytes, 
//                                           memfd, pagemapfd);
    void *physCbPage = (void*)virtToUncachedPhys(virtCbPageCached, pagemapfd);
                                           
    struct DmaControlBlock *cb1 = (struct DmaControlBlock*)virtCbPage;
	
    cb1->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    cb1->SOURCE_AD = (uint32_t)physSrcPage; //set source and destination DMA address
    cb1->DEST_AD = (uint32_t)physDestPage;
    cb1->TXFR_LEN = 12; //transfer 12 bytes
    cb1->STRIDE = 0; //no 2D stride
    cb1->NEXTCONBK = 0; //no next control block
	
	printf("destination was initially: '%s'\n", (char*)virtDestPage);

    //enable DMA channel (it's probably already enabled, but we want to be sure):
    writeBitmasked(dmaBaseMem + DMAENABLE/4, 1 << dmaChNum, 1 << dmaChNum);
	
	//configure the DMA header to point to our control block:
    volatile struct DmaChannelHeader *dmaHeader 
		= (volatile struct DmaChannelHeader*)(dmaBaseMem + (DMACH(dmaChNum))/4); 
		//dmaBaseMem is a uint32_t ptr, so divide by 4 before adding byte offset
    dmaHeader->CS = DMA_CS_RESET; //make sure to disable dma first.
    sleep(1); //give time for the reset command to be handled.
    dmaHeader->DEBUG = DMA_DEBUG_READ_ERROR | DMA_DEBUG_FIFO_ERROR 
        | DMA_DEBUG_READ_LAST_NOT_SET_ERROR; // clear debug error flags
    dmaHeader->CONBLK_AD = (uint32_t)physCbPage; //we have to point it to the PHYSICAL address of the control block (cb1)
    dmaHeader->CS = DMA_CS_ACTIVE; //set active bit, but everything else is 0.
    
    sleep(1); //give time for copy to happen

    printf("destination reads: '%s'\n", (char*)virtDestPage);
	
//	freeUncachedMemView(virtSrcPage, PAGE_SIZE/2);
    freeLockedMem(virtSrcPageCached, PAGE_SIZE/2);
//	freeUncachedMemView(virtDestPage, PAGE_SIZE/2);
    freeLockedMem(virtDestPageCached, PAGE_SIZE/2);
//	freeUncachedMemView(virtCbPage, cbPageBytes);
    freeLockedMem(virtCbPageCached, cbPageBytes);

	close(memfd);
	close(pagemapfd);

	return 0;
}
