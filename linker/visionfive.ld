OUTPUT_ARCH(riscv)
ENTRY(_start)

BASE_ADDRESS = 0x80200000;  /* 将 文件基地址 设置为 0x80200000 */
KERNEL_ADDRESS = 0X80220000; /* 将 内核基地址设置为 0x80220000 如果直接设置在0x80200000，会运行出bug，目前不知道怎么排查*/

SECTIONS
{
    /* Load the kernel at this address: "." means the current address */
    . = BASE_ADDRESS;
    kernel_start = .;

    text_start = .;
    .text : {
        *(.text .text.*)
        . = ALIGN(0x1000);
        _trampoline = .;
        *(trampsec)
        . = ALIGN(0x1000);
        ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page");
        _signalTrampoline = .;
        *(.signalTrampoline)
        . = ALIGN(4096);
        ASSERT(. - _signalTrampoline == 4096, "error: signal trampoline not one page");
        PROVIDE(etext = .);
    }

    . = ALIGN(4K);
    rodata_start = .;
    .rodata : {
        srodata = .;
        *(.rodata .rodata.*)
        erodata = .;
    }

    . = ALIGN(4K);
    data_start = .;
    .data : {
        sdata = .;
        *(.data .data.*)
        edata = .;
    }

    . = ALIGN(4K);
    bss_start = .;
    .bss : {
        *(.bss.stack)
        sbss_clear = .;
        *(.sbss .bss .bss.*)
        ebss_clear = .;
    }

    . = ALIGN(4K);
    PROVIDE(kernel_end = .);
}