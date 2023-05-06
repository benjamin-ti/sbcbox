.global _start
_start:
    mrs r0, cpsr
    bic r0, r0, #0x1F @ clear mode bits
    orr r0, r0, #0x13 @ set SVC mode
    orr r0, r0, #0xC0 @ disable FIQ and IRQ
    ldr     sp,=stack_top
    msr cpsr, r0

    bl _main
    b .
