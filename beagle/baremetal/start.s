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

    @ Setup sp in IRQ mode
    ldr r0, cpsr_irq
    msr cpsr, r0
    ldr sp,=irq_stack_top

    @ Go back to supervisor mode, enable irq
    ldr r0, cpsr_svc_ei
    msr cpsr, r0

    bl _main
    b .

handle_irq:
   bx lr

cpsr_svc:      .word 0x193
cpsr_irq:      .word 0x192
cpsr_svc_ei:   .word 0x153
base_irq_addr: .word 0x4030CE38
basic_handler: .word irq_handler
