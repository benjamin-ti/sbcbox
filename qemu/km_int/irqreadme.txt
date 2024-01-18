ARM Linux-Start und Interrupt-Vectors:
    arch/arm/kernel/entry-armv.S

Enabel/Disable Interrupts:
    local_irq_enable -> raw_local_irq_enable -> arch_local_irq_enable -> native_irq_enable
    enable_irq
    disable_irq