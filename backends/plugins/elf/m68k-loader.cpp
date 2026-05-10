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

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(M68K_TARGET)

#include "backends/plugins/elf/elf-loader.h"
#include "backends/plugins/elf/m68k-loader.h"

#include "common/debug.h"

bool M68KDLObject::relocate(Elf32_Off offset, Elf32_Word size, byte *relSegment) {
	Elf32_Rela *rel = (Elf32_Rela *)malloc(size);

	if (!rel) {
		warning("elfloader: Could not allocate %d bytes for the relocation table", size);
		return false;
	}

	if (!_file->seek(offset, SEEK_SET) || _file->read(rel, size) != size) {
		warning("elfloader: Relocation table load failed.");
		free(rel);
		return false;
	}

	uint32 cnt = size / sizeof(*rel);

	debug(2, "elfloader: Loaded relocation table. %d entries. base address=%p", cnt, relSegment);

	for (uint32 i = 0; i < cnt; i++) {
		Elf32_Sym *sym = _symtab + REL_INDEX(rel[i].r_info);
		byte *src = relSegment + rel[i].r_offset - _segmentVMA;
		uint32 value = sym->st_value + rel[i].r_addend;

		switch (REL_TYPE(rel[i].r_info)) {
		case R_68K_NONE:
			break;
		case R_68K_32:
			*(uint32 *)src = value;
			debug(8, "elfloader: R_68K_32 -> 0x%08x", *(uint32 *)src);
			break;
		case R_68K_16:
			*(uint16 *)src = (uint16)value;
			debug(8, "elfloader: R_68K_16 -> 0x%04x", *(uint16 *)src);
			break;
		case R_68K_8:
			*src = (uint8)value;
			debug(8, "elfloader: R_68K_8 -> 0x%02x", *src);
			break;
		case R_68K_PC32:
			*(uint32 *)src = value - (uint32)src;
			debug(8, "elfloader: R_68K_PC32 -> 0x%08x", *(uint32 *)src);
			break;
		case R_68K_PC16:
			*(uint16 *)src = (uint16)(value - (uint32)src);
			debug(8, "elfloader: R_68K_PC16 -> 0x%04x", *(uint16 *)src);
			break;
		case R_68K_PC8:
			*src = (uint8)(value - (uint32)src);
			debug(8, "elfloader: R_68K_PC8 -> 0x%02x", *src);
			break;
		default:
			warning("elfloader: Unknown relocation type %d", REL_TYPE(rel[i].r_info));
			free(rel);
			return false;
		}
	}

	free(rel);
	return true;
}

bool M68KDLObject::relocateRels(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {
	for (uint32 i = 0; i < ehdr->e_shnum; i++) {
		Elf32_Shdr *curShdr = &(shdr[i]);

		if ((curShdr->sh_type == SHT_REL) &&
				curShdr->sh_entsize == sizeof(Elf32_Rel) &&
				int32(curShdr->sh_link) == _symtab_sect &&
				curShdr->sh_info < ehdr->e_shnum &&
				(shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {
			warning("elfloader: REL entries not supported on m68k!");
			return false;
		}

		if ((curShdr->sh_type == SHT_RELA) &&
				curShdr->sh_entsize == sizeof(Elf32_Rela) &&
				int32(curShdr->sh_link) == _symtab_sect &&
				curShdr->sh_info < ehdr->e_shnum &&
				(shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {
			if (!relocate(curShdr->sh_offset, curShdr->sh_size, _segment))
				return false;
		}
	}

	return true;
}

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(M68K_TARGET) */
