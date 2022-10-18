/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

PHDRS
{
	/* ScummVM's elf loader only allows a single segment, at the moment. */
	plugin PT_LOAD FLAGS(7) /* Read | Write | Execute */;
}

SECTIONS
{
	/* =========== CODE section =========== */

	/* Start the output high in memory so PC-relative jumps from the plugin
	    to the main binary cannot reach, to force the linker to generate
	    veneers converting the relative jumps to absolute jumps */
	. = 0x8000000;

	.text ALIGN(0x1000) :
	{
		/* .text */
		*(.text.unlikely .text.*_unlikely .text.unlikely.*)
		*(.text.exit .text.exit.*)
		*(.text.startup .text.startup.*)
		*(.text.hot .text.hot.*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.glue_7t) *(.glue_7) *(.riscos.libscl.chunkstub.start) *(SORT(.riscos.libscl.chunkstub.id*)) *(.riscos.libscl.chunkstub.end)
	} : plugin

	/* =========== RODATA section =========== */

	. = ALIGN(0x1000);

	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		*(.rodata1)
		. = ALIGN(4);
	} : plugin
	.rodata1 :
	{
		*(.rodata1)
		. = ALIGN(4);
	} : plugin

	.ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } : plugin
	__exidx_start = .;
	ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } : plugin
	__exidx_end = .;

	/* =========== DATA section =========== */

	. = ALIGN(0x1000);

	.tdata ALIGN(4) :
	{
		*(.tdata)
		*(.tdata.*)
		*(.gnu.linkonce.td.*)
		. = ALIGN(4);
	} : plugin

	.tbss ALIGN(4) :
	{
		*(.tbss)
		*(.tbss.*)
		*(.gnu.linkonce.tb.*)
		*(.tcommon)
		. = ALIGN(4);
	} : plugin

	.preinit_array ALIGN(4) :
	{
		PROVIDE (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE (__preinit_array_end = .);
	} : plugin

	.init_array ALIGN(4) :
	{
		PROVIDE (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		PROVIDE (__init_array_end = .);
	} : plugin

	.fini_array ALIGN(4) :
	{
		PROVIDE (__fini_array_start = .);
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		PROVIDE (__fini_array_end = .);
	} : plugin

	.ctors ALIGN(4) :
	{
		___plugin_ctors = .;
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		___plugin_ctors_end = .;
	} : plugin

	.dtors ALIGN(4) :
	{
		___plugin_dtors = .;
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		___plugin_dtors_end = .;
	} : plugin

	.data :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
	} : plugin
	.data1 :
	{
		*(.data1)
		. = ALIGN(4);
	} : plugin

	__bss_start__ = .;
	.bss ALIGN(4) :
	{
		*(.dynbss)
		*(.bss)
		*(SORT(.bss.*))
		*(.gnu.linkonce.b*)
		*(COMMON)
		. = ALIGN(4);
	} : plugin
	__bss_end__ = .;

	__end__ = ABSOLUTE(.) ;

	/* ==================
	   ==== Metadata ====
	   ================== */

	/* Discard sections that difficult post-processing */
	/DISCARD/ : { *(.group .comment .note) }

	/* Stabs debugging sections. */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }

	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0. */

	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }

	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }

	/* DWARF 3 */
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	.debug_ranges   0 : { *(.debug_ranges) }

	/* DWARF Extension.  */
	.debug_macro    0 : { *(.debug_macro) }
	.gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
	.note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }

	/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
