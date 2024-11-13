.global _start
_start:
    ldr r0, cpsr_svc
    msr cpsr, r0
    ldr sp,=stack_top

    @ Set V=0 in CP15 SCTRL register - for VBAR to point to vector */
    mrc   p15, 0, r0, c1, c0, 0 @ Read CP15 SCTRL Register
    bic   r0, #(1 << 13)        @ V = 0
    mcr   p15, 0, r0, c1, c0, 0 @ Write CP15 SCTRL Register

    @ Set vector address in CP15 VBAR register */
    ldr   r0, =_vector_table
    mcr   p15, 0, r0, c12, c0, 0  @Set VBAR

    @ Setup sp in IRQ mode
    ldr r0, cpsr_irq
    msr cpsr, r0
    ldr sp,=irq_stack_top

    @ Setup sp in FIQ mode
    ldr r0, cpsr_fiq
    msr cpsr, r0
    ldr sp,=fiq_stack_top

    @ Go back to supervisor mode, enable irq
    ldr r0, cpsr_svc_ei
    msr cpsr, r0

    @ Initialize BSS to zero.
    ldr r0, bss_start
    ldr r1, bss_end
    mov r2, #0
bss_clear_loop:
    cmp r0, r1
    beq bss_clear_done
    strb r2, [r0], #1  @ write Null in current byte and inc pointer
    b bss_clear_loop
bss_clear_done:

    bl main
    b .

handle_irq:
   bx lr

cpsr_svc:      .word 0x193
cpsr_fiq:      .word 0x111
cpsr_irq:      .word 0x192
cpsr_svc_ei:   .word 0x153
cpsr_svc_efiq_ei: .word 0x113
bss_start:     .word __bss_start__
bss_end:       .word __bss_end__

.section .startup,"ax"
    .code 32
    .align 0
_vector_table:
    b     _start                        /* reset - _start       */
    ldr   pc, _undf                     /* undefined - _undf    */
    ldr   pc, _swi                      /* SWI - _swi           */
    ldr   pc, _pabt                     /* program abort - _pabt*/
    ldr   pc, _dabt                     /* data abort - _dabt   */
    nop                                 /* reserved             */
    ldr   pc, _irq                      /* IRQ - read the VIC   */
    ldr   pc, _fiq                      /* FIQ - _fiq           */

    _undf:  .word __undf                /* undefined            */
    _swi:   .word 0                     /* SWI                  */
    _pabt:  .word __pabt                /* program abort        */
    _dabt:  .word __pabt                /* data abort           */
    _irq:   .word irq_handler
    _fiq:   .word __swi                 /* FIQ                  */

    __undf: b     .                     /* undefined            */
    __pabt: b     .                     /* program abort        */
    __dabt: b     .                     /* data abort           */
    __fiq:  b     .                     /* FIQ                  */
    __irq:  b     .                     /* data abort           */
    __swi:  b     .                     /* FIQ                  */

.global Register_Write
.global Register_Read

// Arguments are placed in registers based on
// Application Binary Interface (ABI) for the ARM Architecture
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100748_0606_00_en/lmi1470147220260.html
@ Register_Write
@ Arg 1 r0: Register Bank
@ Arg 2 r1: Register Offset
@ Arg 3 r2: Word Value to Write
Register_Write:
    add r0, r1
    mov r1, r2
    str r1, [r0]
    bx  lr

@ Register_Read
@ Arg 1 r0: Register Bank
@ Arg 2 r1: Register Offset
@ Return r0: Register value
Register_Read:
    add r0, r1
    mov r1, r2
    ldr r0, [r0]
    bx  lr
