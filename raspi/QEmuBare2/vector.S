#include "makro.h"

#define SDRAM_START 0x70000000
#define SDRAM_END   0x90000000

#define STACK_BASE_FIQ   (SDRAM_START+0x1000)
#define STACK_BASE_ABORT   (SDRAM_START+0x3000)
#define STACK_BASE_UNDEFINED   (SDRAM_START+0x4000)
#define STACK_BASE_SYSTEM   (SDRAM_START+0x5000)

#define CODE_START (SDRAM_START+0x10000)  // this is where the board will jump and where qemu will copy our image

.global entryPoint

.text

entryPoint:
    // copy IDT to begining of ram
    ldr     r0,=IDT_START
    ldr     r1,=IDT_END
    mov     r2,#0
1:  ldr     r3,[r0],#4
    str     r3,[r2],#4
    cmp     r0,r1
    bne     1b


    // initialize stack for all modes, except user mode because we will set it on process creation
    msr     CPSR_c,#0b11010001           // stack for FIQ mode
    ldr     r13,=STACK_BASE_FIQ
    msr     CPSR_c,#0b11010111           // stack for Abort mode
    ldr     r13,=STACK_BASE_ABORT
    msr     CPSR_c,#0b11011011           // stack for Undefined mode
    ldr     r13,=STACK_BASE_UNDEFINED
    msr     CPSR_c,#0b11010011           // stack for Supervisor mode. And we will stay in that mode
    ldr     r13,=STACK_BASE_UNDEFINED   // This is temporary, We will use another stack after MMU is init

    PRINTK("Booting\r\n")
/*
    PRINTK("Init GIC\r\n")
    bl      initGIC

    // Enable interrupts
    mrs     r0,CPSR
    bic     r0,#0b11000000
    msr     CPSR,r0

    // set Supervisor stack
    LDR sp, =stack_top
    // copy vector table to address 0
    BL copy_vectors
    // get Program Status Register
    MRS r0, cpsr
    // go in IRQ mode
    BIC r1, r0, #0x1F
    ORR r1, r1, #0x12
    MSR cpsr, r1
    // set IRQ stack
    LDR sp, =irq_stack_top
    // Enable IRQs
    BIC r0, r0, #0x80
    // go back in Supervisor mode
    MSR cpsr, r0
*/

    // jump to main
    BL main
    B .

debugprint:
    ldr     r3,=SERIAL0_FLAG
1:  ldr     r5,[r3]
    ands    r5,#(1<<5)
    bne     1b
    ldr     r3,=SERIAL0_BASE
    ldrb    r5,[r4],#1
    cmp     r5,#0
    moveq   r15,r14
    str     r5,[r3]
    b       debugprint

.extern entryPoint
.extern undef_handler
.extern data_abort_handler
.extern prefetch_abort_handler
.extern swi_handler
.extern irq_handler
.extern fiq_handler

#define IDT_RESET                      ldr  r15,=entryPoint
#define IDT_UNDEFINED_INSTRUCTION      ldr  r15,=undef_handler
#define IDT_SWI                        ldr  r15,=swi_handler
#define IDT_INSTRUCTION_FETCH_ABORT    ldr  r15,=prefetch_abort_handler
#define IDT_DATA_ABORT                 ldr  r15,=data_abort_handler
#define IDT_RESERVED                   .word 0
#define IDT_IRQ                        ldr  r15,=irq_handler
#define IDT_FIRQ                       ldr  r15,=fiq_handler

// The IDT will be copied to the beginning of the RAM once the kernel is launched
IDT_START:
IDT_RESET
IDT_UNDEFINED_INSTRUCTION
IDT_SWI
IDT_INSTRUCTION_FETCH_ABORT
IDT_DATA_ABORT
IDT_RESERVED
IDT_IRQ
IDT_FIRQ
.ltorg // this will dump the literal pool here. Needed for IDT
IDT_END:
