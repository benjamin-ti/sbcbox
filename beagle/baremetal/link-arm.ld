
MEMORY
{
    ram : ORIGIN = 0x40300000, LENGTH = 0x10000
}

SECTIONS
{
    .text :
    {
        *(.text*);
        *(.rodata*);
    } > ram

    .data :
    {
        *(.data)
    } > ram

    .bss :
    {
        *(.bss*)
    } > ram

    . = . + 0x1000;
    stack_top = .;
    . = . + 0x1000;
    irq_stack_top = .;
}
