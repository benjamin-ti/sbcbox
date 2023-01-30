#include <stdio.h>

// -----------------------------------------------------------------------------

volatile unsigned int * const UART0DR = (unsigned int *)0x10009000;

// -----------------------------------------------------------------------------

void print_uart0(const char *s);

// -----------------------------------------------------------------------------

void __attribute__((interrupt)) irq_handler() { for(;;); }

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
void main()
{
    print_uart0("Hello World\r\n");
}
