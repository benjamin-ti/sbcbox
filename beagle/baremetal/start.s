.global _start
_start:
    ldr r0, cpsr_svc
    msr cpsr, r0
    ldr sp,=stack_top

    @ Set IVT base address in CP15 VBAR register
    ldr r0, =0x4030CE00
    mcr p15, 0, r0, c12, c0, 0  @Set VBAR

    @ Assign the IRQ interrupt method
    ldr r0, base_irq_addr
    ldr r1, basic_handler
    str r1,[r0]

    @ Assign the FIQ interrupt method
    ldr r0, base_fiq_addr
    ldr r1, basic_handler
    str r1,[r0]

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
base_irq_addr: .word 0x4030CE38
base_fiq_addr: .word 0x4030CE3C
basic_handler: .word irq_handler
bss_start:     .word __bss_start__
bss_end:       .word __bss_end__

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
