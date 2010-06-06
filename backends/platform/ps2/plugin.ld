PHDRS
{
	plugin PT_LOAD ;
	shorts PT_LOAD ;
}
SECTIONS {
	/* originally was 0x00100000 but might be 0 */
	.text 0: {
		_ftext = . ;
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
		KEEP(*(.init))
		KEEP(*(.fini))
		QUAD(0)
	}

	PROVIDE(_etext = .);
	PROVIDE(etext = .);

	/* interp may or may not be needed here */
	.interp : { *(.interp) } : plugin
	.reginfo : { *(.reginfo) } : plugin

	/* Global/static constructors and deconstructors. */
	.ctors ALIGN(16): {
	        ___plugin_ctors = .;
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		___plugin_ctors_end = .;
	}
	.dtors ALIGN(16): {
		___plugin_dtors = .;
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		___plugin_dtors_end = .;
	}

	/* Static data.  */
	.rodata ALIGN(128): {
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
	}

	.data ALIGN(128): {
		_fdata = . ;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		SORT(CONSTRUCTORS)
	}

	.rdata ALIGN(128): { *(.rdata) }
	.gcc_except_table ALIGN(128): { *(.gcc_except_table) }

	/* _gp = ALIGN(128) + 0x7ff0; */
	. = __plugin_hole_start;
	/* the .got line wasn't originally there */
	.got	: { *(.got.plt) *(.got) } : shorts

	.lit4 ALIGN(128): { *(.lit4) }
	.lit8 ALIGN(128): { *(.lit8) }

	.sdata ALIGN(128): {
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.s*)
	}

	_edata = .;
	PROVIDE(edata = .);

	/* Uninitialized data.  */
	.sbss ALIGN(128) : {
		_fbss = . ;
		*(.sbss)
		*(.sbss.*)
		*(.gnu.linkonce.sb*)
		*(.scommon)
	}

	.bss ALIGN(128) : {
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b*)
		*(COMMON)
	}
	_end_bss = .;

	_end = . ;
	PROVIDE(end = .);

	/* Symbols needed by crt0.s.  */
	PROVIDE(_heap_size = -1);
	PROVIDE(_stack = -1);
	PROVIDE(_stack_size = 128 * 1024);
}
