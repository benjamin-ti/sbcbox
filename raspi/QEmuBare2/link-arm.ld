ENTRY(entryPoint)
SECTIONS
{
    . = 0x70010000;
    .text : {
        O/vector.o
        *(.text .rodata)
    }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
