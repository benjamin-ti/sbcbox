
MEMORY
{
    sram : ORIGIN = 0x40300000, LENGTH = 0x10000
    dram : ORIGIN = 0x80000000, LENGTH = 0x10000000
}

SECTIONS
{
    .text :
    {
        *(.text*);
        *(.rodata*);
    } > sram

    .data :
    {
        *(.data)
    } > sram

    .bss :
    {
        *(.bss*)
    } > sram

    . = . + 0x1000;
    stack_top = .;
    . = . + 0x1000;
    irq_stack_top = .;
}
