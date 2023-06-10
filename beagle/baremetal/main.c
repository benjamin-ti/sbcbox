
#define CM_DPLL_BASE          0x44E00500
#define CM_DPLL_CLKSEL_TIMER2 (volatile unsigned*)(CM_DPLL_BASE+0x8)
#define TCLKIN    0x0
#define CLK_M_OSC 0x1
#define CLK_32KHZ 0x2

#define CM_PER_BASE		0x44e00000
#define CM_PER_L3S      (volatile unsigned*)(CM_PER_BASE+0x04)
#define CM_PER_TIMER2   (volatile unsigned*)(CM_PER_BASE+0x80)
#define CM_PER_GPIO1    (volatile unsigned*)(CM_PER_BASE+0xAC)

#define CM_WKUP_BASE    0x44E00400
#define CM_WKUP_UART0_CLKCTRL (volatile unsigned*)(CM_WKUP_BASE+0xB4)


#define INTCPS_BASE     0x48200000
#define INTC_CONTROL    (volatile unsigned*)(INTCPS_BASE+0x48)
#define INTC_MIR2       (volatile unsigned*)(INTCPS_BASE+0xC4)
#define INTC_MIR_CLEAR2 (volatile unsigned*)(INTCPS_BASE+0xC8)

#define DMTIMER2_BASE          0x48040000
#define DMTIMER2_EOI           (volatile unsigned*)(DMTIMER2_BASE+0x20)
#define DMTIMER2_IRQSTATUS     (volatile unsigned*)(DMTIMER2_BASE+0x28)
#define DMTIMER2_IRQENABLE_SET (volatile unsigned*)(DMTIMER2_BASE+0x2C)
#define DMTIMER2_IRQENABLE_CLR (volatile unsigned*)(DMTIMER2_BASE+0x30)
#define DMTIMER2_TCLR          (volatile unsigned*)(DMTIMER2_BASE+0x38)
#define DMTIMER2_TCRR          (volatile unsigned*)(DMTIMER2_BASE+0x3C)

#define GPIO1_BASE		   0x4804C000
#define GPIO1_OE           (volatile unsigned*)(GPIO1_BASE+0x134)
#define GPIO1_CLEARDATAOUT (volatile unsigned*)(GPIO1_BASE+0x190)
#define GPIO1_SETDATAOUT   (volatile unsigned*)(GPIO1_BASE+0x194)

// -----------------------------------------------------------------------------
void __attribute__((interrupt)) irq_handler()
{
	static int b;

    *DMTIMER2_IRQSTATUS = 0x2; // Ack DMTimer2 Int

    if (b) {
        *GPIO1_SETDATAOUT = (0xF<<21);
    }
    else {
    	*GPIO1_CLEARDATAOUT = (0xF<<21);
    }
    b = !b;

    *DMTIMER2_TCRR = 0xFF000000;
    *DMTIMER2_TCLR = 0x1;

    *INTC_CONTROL = 0x1; // Ack Global Int
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
#define TIME 100000
void _main (void)
{
	unsigned int ui;

    *CM_DPLL_CLKSEL_TIMER2 = CLK_M_OSC;

    *CM_PER_L3S = 0x2;
    *CM_PER_TIMER2 = 0x2;

    *INTC_MIR2 &= 0xFFFFFFEF;

    *DMTIMER2_IRQENABLE_SET = 0x2;
    *DMTIMER2_TCRR = 0xFF000000;
    *DMTIMER2_TCLR = 0x1;

    *CM_PER_GPIO1 = (1<<18 | 0x2);

    *GPIO1_OE &= ~(0xF<<21);

//    *CM_WKUP_UART0_CLKCTRL = 0x2;
//	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD) = 0x30;
//	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD) = 0x10;

	megos_UART0_init();
//	megos_UART0_test();
	megos_UART0_send_string("hello\n");
	serial_send_newline();
	serial_flush();

    while (1)
    {
//        *GPIO1_SETDATAOUT = (0xF<<21);
        for(ui=0; ui<TIME; ui++);
//        *GPIO1_CLEARDATAOUT = (0xF<<21);
        for(ui=0; ui<TIME; ui++);
    }
}
