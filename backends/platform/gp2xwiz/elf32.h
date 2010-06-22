/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_ELF_H
#define BACKENDS_ELF_H

/* ELF stuff */

typedef unsigned short Elf32_Half, Elf32_Section;
typedef unsigned int Elf32_Word, Elf32_Addr, Elf32_Off;
typedef signed int  Elf32_Sword;
typedef Elf32_Half Elf32_Versym;

#define EI_NIDENT (16)
#define SELFMAG         6

/* ELF File format structures. Look up ELF structure for more details */

// ELF header (contains info about the file)
typedef struct {
	unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
	Elf32_Half    e_type;                 /* Object file type */
	Elf32_Half    e_machine;              /* Architecture */
	Elf32_Word    e_version;              /* Object file version */
	Elf32_Addr    e_entry;                /* Entry point virtual address */
	Elf32_Off     e_phoff;                /* Program header table file offset */
	Elf32_Off     e_shoff;                /* Section header table file offset */
	Elf32_Word    e_flags;                /* Processor-specific flags */
	Elf32_Half    e_ehsize;               /* ELF header size in bytes */
	Elf32_Half    e_phentsize;            /* Program header table entry size */
	Elf32_Half    e_phnum;                /* Program header table entry count */
	Elf32_Half    e_shentsize;            /* Section header table entry size */
	Elf32_Half    e_shnum;                /* Section header table entry count */
	Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;

// Should be in e_ident
#define ELFMAG          "\177ELF\1\1"	/* ELF Magic number */

// Program header (contains info about segment)
typedef struct {
	Elf32_Word    p_type;                 /* Segment type */
	Elf32_Off     p_offset;               /* Segment file offset */
	Elf32_Addr    p_vaddr;                /* Segment virtual address */
	Elf32_Addr    p_paddr;                /* Segment physical address */
	Elf32_Word    p_filesz;               /* Segment size in file */
	Elf32_Word    p_memsz;                /* Segment size in memory */
	Elf32_Word    p_flags;                /* Segment flags */
	Elf32_Word    p_align;                /* Segment alignment */
} Elf32_Phdr;

// Section header (contains info about section)
typedef struct {
	Elf32_Word    sh_name;                /* Section name (string tbl index) */
	Elf32_Word    sh_type;                /* Section type */
	Elf32_Word    sh_flags;               /* Section flags */
	Elf32_Addr    sh_addr;                /* Section virtual addr at execution */
	Elf32_Off     sh_offset;              /* Section file offset */
	Elf32_Word    sh_size;                /* Section size in bytes */
	Elf32_Word    sh_link;                /* Link to another section */
	Elf32_Word    sh_info;                /* Additional section information */
	Elf32_Word    sh_addralign;           /* Section alignment */
	Elf32_Word    sh_entsize;             /* Entry size if section holds table */
} Elf32_Shdr;

// Symbol entry (contain info about a symbol)
typedef struct {
	Elf32_Word    st_name;                /* Symbol name (string tbl index) */
	Elf32_Addr    st_value;               /* Symbol value */
	Elf32_Word    st_size;                /* Symbol size */
	unsigned char st_info;                /* Symbol type and binding */
	unsigned char st_other;               /* Symbol visibility */
	Elf32_Section st_shndx;               /* Section index */
} Elf32_Sym;

// Relocation entry (info about how to relocate)
typedef struct {
	Elf32_Addr    r_offset;               /* Address */
	Elf32_Word    r_info;                 /* Relocation type and symbol index */
	Elf32_Sword   r_addend;               /* Addend */
} Elf32_Rela;

#endif /* BACKENDS_ELF_H */
