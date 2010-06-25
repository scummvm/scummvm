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

// e_type values
#define ET_NONE		0	/* no file type */
#define ET_REL		1	/* relocatable */
#define ET_EXEC		2	/* executable */
#define ET_DYN		3	/* shared object */
#define ET_CORE		4	/* core file */

// e_machine values
#define EM_ARM		40

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

// p_type values
#define PT_NULL 		0	/* ignored */
#define PT_LOAD			1	/* loadable segment */
#define PT_DYNAMIC		2	/* dynamic linking info */
#define PT_INTERP		3	/* info about interpreter */
#define PT_NOTE			4	/* note segment */
#define PT_SHLIB		5	/* reserved */
#define PT_PHDR			6	/* Program header table */
#define PT_ARM_ARCHEXT 	0x70000000 /* Platform architecture compatibility information */
#define PT_ARM_EXIDX 	0x70000001 /* Exception unwind tables */

// p_flags value
#define PF_X	1	/* execute */
#define PF_W	2	/* write */
#define PF_R	4	/* read */

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

// sh_type values
#define SHT_NULL			0	/* Inactive section */
#define SHT_PROGBITS		1	/* Proprietary */
#define SHT_SYMTAB			2	/* Symbol table */
#define SHT_STRTAB			3	/* String table */
#define SHT_RELA			4	/* Relocation entries with addend */
#define SHT_HASH			5	/* Symbol hash table */
#define SHT_DYNAMIC			6	/* Info for dynamic linking */
#define SHT_NOTE			7	/* Note section */
#define SHT_NOBITS			8	/* Occupies no space */
#define SHT_REL				9	/* Relocation entries without addend */
#define SHT_SHLIB			10	/* Reserved */
#define SHT_DYNSYM			11	/* Minimal set of dynamic linking symbols */
#define SHT_ARM_EXIDX 		0x70000001	/* Exception Index table */
#define SHT_ARM_PREEMPTMAP 	0x70000002	/* BPABI DLL dynamic linking pre-emption map */
#define SHT_ARM_ATTRIBUTES 	0x70000003	/* Object file compatibility attributes */

// sh_flags values
#define SHF_WRITE		0	/* writable section */
#define SHF_ALLOC		2	/* section occupies memory */
#define SHF_EXECINSTR	4	/* machine instructions */

// Symbol entry (contain info about a symbol)
typedef struct {
	Elf32_Word    st_name;                /* Symbol name (string tbl index) */
	Elf32_Addr    st_value;               /* Symbol value */
	Elf32_Word    st_size;                /* Symbol size */
	unsigned char st_info;                /* Symbol type and binding */
	unsigned char st_other;               /* Symbol visibility */
	Elf32_Section st_shndx;               /* Section index */
} Elf32_Sym;

// Extract from the st_info
#define SYM_TYPE(x)		((x)&0xF)
#define SYM_BIND(x)		((x)>>4)

// Symbol binding values from st_info
#define STB_LOCAL 	0	/* Symbol not visible outside object */
#define STB_GLOBAL 	1	/* Symbol visible to all object files */
#define STB_WEAK	2	/* Similar to STB_GLOBAL */

// Symbol type values from st_info
#define STT_NOTYPE	0	/* Not specified */
#define STT_OBJECT	1	/* Data object e.g. variable */
#define STT_FUNC	2	/* Function */
#define STT_SECTION	3	/* Section */
#define STT_FILE	4	/* Source file associated with object file */

// Special section header index values from st_shndex
#define SHN_UNDEF  		0
#define SHN_LOPROC 		0xFF00	/* Extended values */
#define SHN_ABS	   		0xFFF1	/* Absolute value: don't relocate */
#define SHN_COMMON 		0xFFF2	/* Common block. Not allocated yet */
#define SHN_HIPROC 		0xFF1F
#define SHN_HIRESERVE 	0xFFFF

// Relocation entry (info about how to relocate)
typedef struct {
	Elf32_Addr    r_offset;               /* Address */
	Elf32_Word    r_info;                 /* Relocation type and symbol index */
	Elf32_Sword   r_addend;               /* Addend */
} Elf32_Rela;

// Access macros for the relocation info
#define REL_TYPE(x)		((unsigned char) (x))	/* Extract relocation type */
#define REL_INDEX(x)	((x)>>8)				/* Extract relocation index into symbol table */

// ARM relocation types
#define R_ARM_NONE			0
#define R_ARM_PC24			1
#define R_ARM_ABS32			2
#define R_ARM_REL32			3
#define R_ARM_LDR_PC_G0 		4
#define R_ARM_ABS16			5
#define R_ARM_ABS12			6
#define R_ARM_THM_ABS5 			7
#define R_ARM_ABS8			8
#define R_ARM_SBREL32 			9
#define R_ARM_THM_CALL 			10
#define R_ARM_THM_PC8			11
#define R_ARM_BREL_ADJ			12
#define R_ARM_TLS_DESC			13
#define R_ARM_TLS_DTPMOD32		17
#define R_ARM_TLS_DTPOFF32		18
#define R_ARM_TLS_TPOFF32		19
#define R_ARM_COPY			20
#define R_ARM_GLOB_DAT			21
#define R_ARM_JUMP_SLOT			22
#define R_ARM_RELATIVE			23
#define R_ARM_GOTOFF32			24
#define R_ARM_BASE_PREL			25
#define R_ARM_GOT_BREL			26
#define R_ARM_PLT32			27
#define R_ARM_CALL			28
#define R_ARM_JUMP24			29
#define R_ARM_THM_JUMP24		30
#define R_ARM_BASE_ABS			31
#define R_ARM_ALU_PCREL_7_0		32
#define R_ARM_ALU_PCREL_15_8		33
#define R_ARM_ALU_PCREL_23_15		34
#define R_ARM_LDR_SBREL_11_0_NC		35
#define R_ARM_ALU_SBREL_19_12_NC	36
#define R_ARM_ALU_SBREL_27_20_CK	37
#define R_ARM_TARGET1			38
#define R_ARM_SBREL31			39
#define R_ARM_V4BX			40
#define R_ARM_TARGET2			41
#define R_ARM_PREL31			42
#define R_ARM_MOVW_ABS_NC		43
#define R_ARM_MOVT_ABS			44
#define R_ARM_MOVW_PREL_NC		45
#define R_ARM_MOVT_PREL			46
#define R_ARM_THM_MOVW_ABS_NC		47
#define R_ARM_THM_MOVT_ABS		48
#define R_ARM_THM_MOVW_PREL_NC		49
#define R_ARM_THM_MOVT_PREL		50
#define R_ARM_THM_JUMP19		51
#define R_ARM_THM_JUMP6			52
#define R_ARM_THM_ALU_PREL_11_0		53
#define R_ARM_THM_PC12			54
#define R_ARM_ABS32_NOI			55
#define R_ARM_REL32_NOI			56
#define R_ARM_ALU_PC_G0_NC	 	57
#define R_ARM_ALU_PC_G0			58
#define R_ARM_ALU_PC_G1_NC		59
#define R_ARM_ALU_PC_G1			60
#define R_ARM_ALU_PC_G2			61
#define R_ARM_LDR_PC_G1			62
#define R_ARM_LDR_PC_G2			63
#define R_ARM_LDRS_PC_G0		64
#define R_ARM_LDRS_PC_G1		65
#define R_ARM_LDRS_PC_G2		66
#define R_ARM_LDC_PC_G0			67
#define R_ARM_LDC_PC_G1			68
#define R_ARM_LDC_PC_G2			69
#define R_ARM_ALU_SB_G0_NC		70
#define R_ARM_ALU_SB_G0			71
#define R_ARM_ALU_SB_G1_NC		72
#define R_ARM_ALU_SB_G1 		73
#define R_ARM_ALU_SB_G2			74
#define R_ARM_LDR_SB_G0			75
#define R_ARM_LDR_SB_G1			76
#define R_ARM_LDR_SB_G2			77
#define R_ARM_LDRS_SB_G0		78
#define R_ARM_LDRS_SB_G1		79
#define R_ARM_LDRS_SB_G2		80
#define R_ARM_LDC_SB_G0			81
#define R_ARM_LDC_SB_G1			82
#define R_ARM_LDC_SB_G2			83
#define R_ARM_MOVW_BREL_NC		84
#define R_ARM_MOVT_BREL			85
#define R_ARM_MOVW_BREL			86
#define R_ARM_THM_MOVW_BREL_NC		87
#define R_ARM_THM_MOVT_BREL		88
#define R_ARM_THM_MOVW_BREL		89
#define R_ARM_TLS_GOTDESC		90
#define R_ARM_TLS_CALL			91
#define R_ARM_TLS_DESCSEQ		92
#define R_ARM_THM_TLS_CALL		93
#define R_ARM_PLT32_ABS			94
#define R_ARM_GOT_ABS			95
#define R_ARM_GOT_PREL			96
#define R_ARM_GOT_BREL12		97
#define R_ARM_GOTOFF12			98
#define R_ARM_GOTRELAX			99
#define R_ARM_THM_JUMP11		102
#define R_ARM_THM_JUMP8			103
#define R_ARM_TLS_GD32			104
#define R_ARM_TLS_LDM32			105
#define R_ARM_TLS_LDO32			106
#define R_ARM_TLS_IE32			107
#define R_ARM_TLS_LE32			108
#define R_ARM_TLS_LDO12			109
#define R_ARM_TLS_LE12			110
#define R_ARM_TLS_IE12GP		111
#define R_ARM_PRIVATE_0			112
#define R_ARM_PRIVATE_1			113
#define R_ARM_PRIVATE_2			114
#define R_ARM_PRIVATE_3			115
#define R_ARM_PRIVATE_4			116
#define R_ARM_PRIVATE_5			117
#define R_ARM_PRIVATE_6			118
#define R_ARM_PRIVATE_7			119
#define R_ARM_PRIVATE_8			120
#define R_ARM_PRIVATE_9			121
#define R_ARM_PRIVATE_10		122
#define R_ARM_PRIVATE_11		123
#define R_ARM_PRIVATE_12		124
#define R_ARM_PRIVATE_13		125
#define R_ARM_PRIVATE_14		126
#define R_ARM_PRIVATE_15		127
#define R_ARM_THM_TLS_DESCSEQ16         129
#define R_ARM_THM_TLS_DESCSEQ32 	130

#endif /* BACKENDS_ELF_H */
