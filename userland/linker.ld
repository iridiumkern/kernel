ENTRY(_start)

PHDRS {
	text PT_LOAD FLAGS(5);
	rodata PT_LOAD FLAGS(4);
	data PT_LOAD FLAGS(6);
	dynamic PT_DYNAMIC FLAGS(6);
}

SECTIONS {
	.text : ALIGN(0x1000) {
		*(.text .text.*)
	} :text

	.rodata : ALIGN(0x1000) {
		*(.rodata .rodata.*)
		*(.eh_frame .eh_frame.*)
	} :rodata

	.data : ALIGN(0x1000) {
		*(.data .data.*)
		*(.got .got.*)
		*(.got.plt)
	} :data

	.dynamic : ALIGN(8) {
		*(.dynamic)
	} :data :dynamic

	.bss : ALIGN(0x1000) {
		*(.bss .bss.*)
		*(COMMON)
	} :data
}