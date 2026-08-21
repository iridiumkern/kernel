/* AArch64 ELF64 */
OUTPUT_FORMAT(elf64-littleaarch64)

ENTRY(kmain)

PHDRS
{
    text   PT_LOAD FLAGS(5); /* R_X */
    rodata PT_LOAD FLAGS(4); /* R__ */
    data   PT_LOAD FLAGS(6); /* RW_ */
}

SECTIONS
{
    /*
     * Limine requires AArch64 kernels to be loaded at or above
     * 0xffffffff80000000.
     */
    . = 0xffffffff80000000;

    /*
     * Limine requests must remain in the kernel image and between
     * the request start/end markers.
     */
    .limine_requests : ALIGN(8)
    {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text :
    {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata :
    {
        *(.rodata .rodata.*)
    } :rodata

    .note.gnu.build-id :
    {
        *(.note.gnu.build-id)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data :
    {
        *(.data .data.*)
    } :data

    .bss :
    {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ :
    {
        *(.eh_frame*)
        *(.note .note.*)
    }
}