
#include <stdio.h>
#include <stdlib.h>

#define CM_DPLL_BASE          0x44E00500
#define CM_DPLL_CLKSEL_TIMER2 (volatile unsigned*)(CM_DPLL_BASE+0x8)
#define CM_DPLL_CLKSEL_TIMER3 (volatile unsigned*)(CM_DPLL_BASE+0xC)
#define TCLKIN    0x0
#define CLK_M_OSC 0x1
#define CLK_32KHZ 0x2

#define CM_PER_BASE		0x44e00000
#define CM_PER_L3S      (volatile unsigned*)(CM_PER_BASE+0x04)
#define CM_PER_TIMER2   (volatile unsigned*)(CM_PER_BASE+0x80)
#define CM_PER_TIMER3   (volatile unsigned*)(CM_PER_BASE+0x84)
#define CM_PER_GPIO1    (volatile unsigned*)(CM_PER_BASE+0xAC)

#define CM_WKUP_BASE    0x44E00400
#define CM_WKUP_UART0_CLKCTRL (volatile unsigned*)(CM_WKUP_BASE+0xB4)


#define INTCPS_BASE       0x48200000
#define INTC_SIR_IRQ      (volatile unsigned*)(INTCPS_BASE+0x40)
#define INTC_CONTROL      (volatile unsigned*)(INTCPS_BASE+0x48)
#define INTC_IRQ_PRIORITY (volatile unsigned*)(INTCPS_BASE+0x60)
#define INTC_THRESHOLD    (volatile unsigned*)(INTCPS_BASE+0x68)
#define INTC_MIR2         (volatile unsigned*)(INTCPS_BASE+0xC4)
#define INTC_MIR_CLEAR2   (volatile unsigned*)(INTCPS_BASE+0xC8)
#define INTC_MIR_SET2     (volatile unsigned*)(INTCPS_BASE+0xCC)
#define INTC_ILR68        (volatile unsigned*)(INTCPS_BASE+0x210)
#define INTC_ILR69        (volatile unsigned*)(INTCPS_BASE+0x214)

#define NEWIRQAGR       0x1;

#define DMTIMER2_BASE          0x48040000
#define DMTIMER2_EOI           (volatile unsigned*)(DMTIMER2_BASE+0x20)
#define DMTIMER2_IRQSTATUS     (volatile unsigned*)(DMTIMER2_BASE+0x28)
#define DMTIMER2_IRQENABLE_SET (volatile unsigned*)(DMTIMER2_BASE+0x2C)
#define DMTIMER2_IRQENABLE_CLR (volatile unsigned*)(DMTIMER2_BASE+0x30)
#define DMTIMER2_TCLR          (volatile unsigned*)(DMTIMER2_BASE+0x38)
#define DMTIMER2_TCRR          (volatile unsigned*)(DMTIMER2_BASE+0x3C)

#define DMTIMER3_BASE          0x48042000
#define DMTIMER3_EOI           (volatile unsigned*)(DMTIMER3_BASE+0x20)
#define DMTIMER3_IRQSTATUS     (volatile unsigned*)(DMTIMER3_BASE+0x28)
#define DMTIMER3_IRQENABLE_SET (volatile unsigned*)(DMTIMER3_BASE+0x2C)
#define DMTIMER3_IRQENABLE_CLR (volatile unsigned*)(DMTIMER3_BASE+0x30)
#define DMTIMER3_TCLR          (volatile unsigned*)(DMTIMER3_BASE+0x38)
#define DMTIMER3_TCRR          (volatile unsigned*)(DMTIMER3_BASE+0x3C)

#define GPIO1_BASE		   0x4804C000
#define GPIO1_OE           (volatile unsigned*)(GPIO1_BASE+0x134)
#define GPIO1_CLEARDATAOUT (volatile unsigned*)(GPIO1_BASE+0x190)
#define GPIO1_SETDATAOUT   (volatile unsigned*)(GPIO1_BASE+0x194)

// -----------------------------------------------------------------------------
void handle_DMTimer2(void)
{
#define TIME2 1000000
    static int b;
    unsigned int ui;

    *DMTIMER2_IRQSTATUS = 0x2; // Ack DMTimer2 Int

    if (b) {
        *GPIO1_SETDATAOUT = (0x8<<21);
    }
    else {
        *GPIO1_CLEARDATAOUT = (0x8<<21);
    }
    b = !b;

    *DMTIMER2_TCRR = 0xFF000000;
    *DMTIMER2_TCLR = 0x1;

    for(ui=0; ui<TIME2; ui++);
}

// -----------------------------------------------------------------------------
void handle_DMTimer3(void)
{
    static int b;

    *DMTIMER3_IRQSTATUS = 0x2; // Ack DMTimer3 Int

    if (b) {
        *GPIO1_SETDATAOUT = (0x4<<21);
    }
    else {
        *GPIO1_CLEARDATAOUT = (0x4<<21);
    }
    b = !b;

    *DMTIMER3_TCRR = 0xFFF00000;
    *DMTIMER3_TCLR = 0x1;
}

// -----------------------------------------------------------------------------
void __attribute__((interrupt)) irq_handler()
{
    unsigned int uiIRQNum;
    unsigned int uiThresholdSave;
    unsigned int uiIntcIrqPriority;

    uiThresholdSave = *INTC_THRESHOLD;
    uiIntcIrqPriority = *INTC_IRQ_PRIORITY;
    *INTC_THRESHOLD = (uiIntcIrqPriority & 0x000000FF);

    uiIRQNum = *INTC_SIR_IRQ & ~0xFFFFFF80;

    *INTC_CONTROL = NEWIRQAGR; // Ack Global Int

    // Enable IRQ/FIQ at ARM side: activate IRQs in CPSR-Register
    asm volatile (
            "MRS R0, CPSR\n"        // Read the status register
            "BIC R0, R0, #0x80\n"    // Clear the I bit (bit 7)
            "MSR CPSR, R0\n"         // Write it back to enable IRQs
        );

    switch (uiIRQNum)
    {
        case 68:
            handle_DMTimer2();
            break;
        case 69:
            handle_DMTimer3();
            break;
        default:
            break;
    }

    // Disable IRQ/FIQ at ARM side: disable IRQs in CPSR-Register
    asm volatile (
            "MRS R0, CPSR\n"        // Read the status register
            "ORR R0, R0, #0x80\n"    // Set the I bit (bit 7)
            "MSR CPSR, R0\n"         // Write it back to enable IRQs
        );

    *INTC_THRESHOLD = uiThresholdSave;
}

#define HWREG(x) (*((volatile unsigned int*)(x)))
#define SOC_CONTROL_REGS (0x44E10000)
#define CONTROL_CONF_UART_RXD (0x970)
#define CONTROL_CONF_UART_TXD (0x974)
void megos_UART0_send_string(char* msg);
void megos_UART0_init(void);
int megos_UART0_test(void);
void serial_send_newline(void);
void serial_flush(void);

// -----------------------------------------------------------------------------
#define TIME 1000000
#define BUF_SIZE 128
void main (void)
{
    char pcBuf[BUF_SIZE];
    unsigned int ui;
    char* pc1;
    char* pc2;

    *CM_PER_L3S = 0x2;

    *CM_PER_GPIO1 = (1<<18 | 0x2);
    *GPIO1_OE &= ~(0xF<<21);

    *CM_DPLL_CLKSEL_TIMER2 = CLK_M_OSC;
    *CM_PER_TIMER2 = 0x2;
//    *INTC_MIR2 &= 0xFFFFFFEF;
    *INTC_MIR_CLEAR2 = 0x00000010;
    // *INTC_ILR68 = 0x11; // Enable FIQ, but is not available on General Purpose Am335x, only on Security Devices
    *INTC_ILR68 = 0xF0; // Low Priority
    *DMTIMER2_TCRR = 0xFF000000;
    *DMTIMER2_TCLR = 0x1;
    *DMTIMER2_IRQENABLE_SET = 0x2;

    *CM_DPLL_CLKSEL_TIMER3 = CLK_M_OSC;
    *CM_PER_TIMER3 = 0x2;
//    *INTC_MIR2 &= 0xFFFFFFDF;
    *INTC_MIR_CLEAR2 = 0x00000020;
    *INTC_ILR69 = 0x10; // Low Priority
    *DMTIMER3_TCRR = 0xFF900000;
    *DMTIMER3_TCLR = 0x1;
    *DMTIMER3_IRQENABLE_SET = 0x2;

    //    *CM_WKUP_UART0_CLKCTRL = 0x2;
    //	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD) = 0x30;
    //	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD) = 0x10;

    megos_UART0_init();
    //	megos_UART0_test();
    pc1 = malloc(1024);
    snprintf(pcBuf, BUF_SIZE, "hello1: %x", pc1);
    megos_UART0_send_string(pcBuf);
    serial_send_newline();
    serial_flush();
    free(pc1);

    pc2 = malloc(1024);
    snprintf(pcBuf, BUF_SIZE, "hello2: %x", pc2);
    megos_UART0_send_string(pcBuf);
    serial_send_newline();
    serial_flush();

    pc1 = malloc(1024);
    snprintf(pcBuf, BUF_SIZE, "hello1: %x", pc1);
    megos_UART0_send_string(pcBuf);
    serial_send_newline();
    serial_flush();

    free(pc1);
    free(pc2);

    while (1)
    {
        *GPIO1_SETDATAOUT = (0x2<<21);
        for(ui=0; ui<TIME; ui++);
        *GPIO1_CLEARDATAOUT = (0x2<<21);
        for(ui=0; ui<TIME; ui++);
    }
}
