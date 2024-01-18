#include <stdio.h>

// -----------------------------------------------------------------------------

volatile unsigned int * const UART0DR = (unsigned int *)0x101f1000;

// -----------------------------------------------------------------------------

#define TIMER0_BASE 0x101E2000
#define TIMER0_LOAD  ((volatile unsigned int *)(TIMER0_BASE + 0x0))
#define TIMER0_VALUE ((volatile unsigned int *)(TIMER0_BASE + 0x4))
#define TIMER0_CONTROL ((volatile unsigned int *)(TIMER0_BASE + 0x8))
#define TIMER0_INTCLR ((volatile unsigned int *)(TIMER0_BASE + 0xC))

#define TIMER2_BASE 0x101E3000
#define TIMER2_LOAD  ((volatile unsigned int *)(TIMER2_BASE + 0x0))
#define TIMER2_VALUE ((volatile unsigned int *)(TIMER2_BASE + 0x4))
#define TIMER2_CONTROL ((volatile unsigned int *)(TIMER2_BASE + 0x8))
#define TIMER2_INTCLR ((volatile unsigned int *)(TIMER2_BASE + 0xC))

#define TIMER_EN 0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN 0x20
#define TIMER_32BIT 0x02
#define TIMER_ONESHOT 0x01

// -----------------------------------------------------------------------------

#define PIC_BASE 0x10140000
#define PIC_IRQStatus  ((volatile unsigned int *)(PIC_BASE + 0x000))
#define PIC_IntEnable  ((volatile unsigned int *)(PIC_BASE + 0x010))
#define PIC_IntEnClear ((volatile unsigned int *)(PIC_BASE + 0x014))
#define PIC_VectAddr   ((volatile unsigned int *)(PIC_BASE + 0x030))
#define PIC_VectAddr0  ((volatile unsigned int *)(PIC_BASE + 0x100))
#define PIC_VectAddr1  ((volatile unsigned int *)(PIC_BASE + 0x104))
#define PIC_VectCntl0  ((volatile unsigned int *)(PIC_BASE + 0x200))
#define PIC_VectCntl1  ((volatile unsigned int *)(PIC_BASE + 0x204))

#define PIC_TIMER0_1 (1 << 4)
#define PIC_TIMER2_3 (1 << 5)

int dummy;

// -----------------------------------------------------------------------------

void print_uart0(const char *s);

// -----------------------------------------------------------------------------
void timer0_1_irq(void)
{
    int i;
    print_uart0("timer0_1_irq start\n");
    for (i=0; i<0x3FFFFFFF; i++) {
        dummy++;
    }
    print_uart0("timer0_1_irq end\n");
    *TIMER0_INTCLR = PIC_TIMER0_1;
}

void timer2_3_irq(void)
{
    print_uart0("timer2_3_irq\n");
    *TIMER2_INTCLR = PIC_TIMER2_3;
}

// -----------------------------------------------------------------------------
void __attribute__((interrupt)) irq_handler()
{
    void* pIsrAddr;
    unsigned int irqstat;

    pIsrAddr = (void*)(*PIC_VectAddr);
    irqstat = *PIC_IRQStatus;

    // Enable IRQ/FIQ at ARM side: activate IRQs in CPSR-Register
    asm volatile (
            "MRS R0, CPSR\n"        // Read the status register
            "BIC R0, R0, #0x80\n"    // Clear the I bit (bit 7)
            "MSR CPSR, R0\n"         // Write it back to enable IRQs
        );

/*
    if (pIsrAddr ==  &timer0_1_irq)
        timer0_1_irq();

    if (pIsrAddr ==  &timer2_3_irq)
        timer2_3_irq();
*/

    if (irqstat & PIC_TIMER0_1)
        timer0_1_irq();

    if (irqstat & PIC_TIMER2_3)
        timer2_3_irq();

    // Disable IRQ/FIQ at ARM side: disable IRQs in CPSR-Register
    asm volatile (
            "MRS R0, CPSR\n"        // Read the status register
            "ORR R0, R0, #0x80\n"    // Set the I bit (bit 7)
            "MSR CPSR, R0\n"         // Write it back to enable IRQs
        );

    *PIC_VectAddr = (int)pIsrAddr;
}

/* all other handlers are infinite loops */
void __attribute__((interrupt)) undef_handler(void) { for(;;); }
void __attribute__((interrupt)) swi_handler(void) { for(;;); }
void __attribute__((interrupt)) prefetch_abort_handler(void) { for(;;); }
void __attribute__((interrupt)) data_abort_handler(void) { for(;;); }
void __attribute__((interrupt)) fiq_handler(void) { for(;;); }

// -----------------------------------------------------------------------------
void copy_vectors(void)
{
    extern uint32_t vectors_start;
    extern uint32_t vectors_end;
    uint32_t *vectors_src = &vectors_start;
    uint32_t *vectors_dst = (uint32_t *)0;

    while (vectors_src < &vectors_end)
        *vectors_dst++ = *vectors_src++;
}

// -----------------------------------------------------------------------------
void print_uart0(const char *s)
{
    while(*s != '\0') { // Loop until end of string
        *UART0DR = (unsigned int)(*s); /* Transmit char */
        s++; /* Next char */
    }
}

// -----------------------------------------------------------------------------
#define BUF_SIZE 128

void main()
{
    char pcBuf[BUF_SIZE];
    int i;
    unsigned int uiTimerValue;

    *TIMER0_LOAD = 0x4FFFF;
    *TIMER0_CONTROL = TIMER_EN | TIMER_PERIODIC | 0x4 | TIMER_INTEN;
    *TIMER2_LOAD = 0xAFFF;
    *TIMER2_CONTROL = TIMER_EN | TIMER_PERIODIC | 0x4 | TIMER_INTEN;

//    *PIC_VectCntl1 = 0x00000020 | 4;
//    *PIC_VectCntl0 = 0x00000020 | 5;
//    *PIC_VectAddr1 = (int)&timer0_1_irq;
//    *PIC_VectAddr0 = (int)&timer2_3_irq;
    *PIC_IntEnable = PIC_TIMER0_1 | PIC_TIMER2_3;

    i = 0;
    for (;;) {
        i++;
        if (i==0x3FFFFFFF) {
            snprintf(pcBuf, BUF_SIZE, "Hello world: %x\n", uiTimerValue);
            print_uart0(pcBuf);
            i = 0;
        }
    }
}
