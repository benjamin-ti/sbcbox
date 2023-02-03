#include <stdio.h>

// -----------------------------------------------------------------------------

volatile unsigned int * const UART0DR = (unsigned int *)0x10009000;

// -----------------------------------------------------------------------------

#define TIMER0_BASE 0x10011000

#define TIMER0_LOAD  ((volatile unsigned int *)(TIMER0_BASE + 0x0))
#define TIMER0_VALUE ((volatile unsigned int *)(TIMER0_BASE + 0x4))
#define TIMER0_CONTROL ((volatile unsigned int *)(TIMER0_BASE + 0x8))
#define TIMER0_INTCLR ((volatile unsigned int *)(TIMER0_BASE + 0xC))

#define TIMER_EN 0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN 0x20
#define TIMER_32BIT 0x02
#define TIMER_ONESHOT 0x01

// -----------------------------------------------------------------------------

// GIC - Generic Interrupt Controller

// Timer 0: INT ID 36

#define GIC0_CPU_ITF_BASE 0x1E000000
#define GIC0_DISTRIB_BASE 0x1E001000

#define GIC0_CPU_CONTROL  ((volatile unsigned int *)(GIC0_CPU_ITF_BASE + 0x0))
#define GIC0_PRIORITY_MASK  ((volatile unsigned int *)(GIC0_CPU_ITF_BASE + 0x4))

#define GIC0_DISTRIB_CONTROL  ((volatile unsigned int *)(GIC0_DISTRIB_BASE + 0x0))
#define GIC0_SET_ENABLE1      ((volatile unsigned int *)(GIC0_DISTRIB_BASE + 0x104))

// -----------------------------------------------------------------------------

void print_uart0(const char *s);

// -----------------------------------------------------------------------------

void __attribute__((interrupt)) irq_handler()
{
    print_uart0("irq_handler\n");
    //  *TIMER0_LOAD = 0xFFFF;
    //  *PIC_IntEnClear = PIC_TIMER0_1;
    //  *PIC_IntEnable = PIC_TIMER0_1;
    // *TIMER0_INTCLR = PIC_TIMER0_1;
    for(;;);
}

/* all other handlers are infinite loops */
void __attribute__((interrupt)) undef_handler(void) { for(;;); }
void __attribute__((interrupt)) swi_handler(void) { for(;;); }
void __attribute__((interrupt)) prefetch_abort_handler(void) { for(;;); }
void __attribute__((interrupt)) data_abort_handler(void) { for(;;); }
void __attribute__((interrupt)) fiq_handler(void) { for(;;); }

// -----------------------------------------------------------------------------
/*
void copy_vectors(void)
{
    extern uint32_t vectors_start;
    extern uint32_t vectors_end;
    uint32_t *vectors_src = &vectors_start;
    uint32_t *vectors_dst = (uint32_t *)0;

    while (vectors_src < &vectors_end)
        *vectors_dst++ = *vectors_src++;
}
*/

// -----------------------------------------------------------------------------
void print_uart0(const char *s)
{
    while(*s != '\0') { // Loop until end of string
        *UART0DR = (unsigned int)(*s); // Transmit char
        s++; // Next char
    }
}

// -----------------------------------------------------------------------------
#define BUF_SIZE 128

void main()
{
    char pcBuf[BUF_SIZE];
    unsigned int uiCPSR;
    unsigned int uiTimerValue;

    asm volatile (" mrs  %0, cpsr" : "=r" (uiCPSR) : /* no inputs */  );
    snprintf(pcBuf, BUF_SIZE, "CPSR: %x\n", uiCPSR);
    print_uart0(pcBuf);

    // Enable interrupts
    asm("mrs     r0,CPSR");
    asm("bic     r0,#0x80");
    asm("msr     CPSR,r0");

    *GIC0_CPU_CONTROL = 0x00000001;
    *GIC0_PRIORITY_MASK = 0x000000F0;

    *GIC0_DISTRIB_CONTROL = 0x00000001;
    *GIC0_SET_ENABLE1 = 0x00000010;

    *TIMER0_LOAD = 0xFFFF;
    *TIMER0_CONTROL = TIMER_EN | TIMER_PERIODIC | 0x4 | TIMER_INTEN;

    for (;;) {
        uiTimerValue = *TIMER0_VALUE;
        if (uiTimerValue>0x9999 && uiTimerValue<0xAAAA) {
            snprintf(pcBuf, BUF_SIZE, "Hello world: %x\n", uiTimerValue);
//            print_uart0(pcBuf);
        }
    }
}
