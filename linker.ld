ENTRY(kernel_main)

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.text.entry)
        *(.text)
    }

    .data : {
        *(.data)
        *(.rdata)
    }

    .bss : {
        *(.bss)
    }

    . = 0x50000;
    .bss.safe : {
        *(.bss.safe)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.reloc)
    }
}