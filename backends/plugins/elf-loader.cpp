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

#if defined(DYNAMIC_MODULES) && defined(ELF_LOADER_TARGET)

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>	// for memalign() (Linux specific)
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/_default_fcntl.h>	// FIXME: Why do we need this DevKitPro specific header?

#include "common/file.h"
#include "common/fs.h"
#include "elf-loader.h"

#ifdef __DS__
#include <nds.h>
#endif

#define __DEBUG_PLUGINS__

#ifdef __DEBUG_PLUGINS__
#define DBG(x,...) printf(x, ## __VA_ARGS__)
#else
#define DBG(x,...)
#endif

#define seterror(x,...) printf(x, ## __VA_ARGS__)

/**
 * Flushes the data cache (Platform Specific).
 */
void flushDataCache() {
#ifdef __DS__
  DC_FlushAll();
#endif
#ifdef __PLAYSTATION2__
  FlushCache(0);
  FlushCache(2);
#endif
}

// Expel the symbol table from memory
void DLObject::discard_symtab() {
	free(_symtab);
	free(_strtab);
	_symtab = NULL;
	_strtab = NULL;
	_symbol_cnt = 0;
}

// Unload all objects from memory
void DLObject::unload() {
	discard_symtab();
	free(_segment);
	_segment = NULL;

#ifdef MIPS_TARGET
	if (_shortsSegment) {
		ShortsMan.deleteSegment(_shortsSegment);
		_shortsSegment = NULL;
	}
#endif
}

bool DLObject::readElfHeader(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr) {

	// Start reading the elf header. Check for errors
	if (DLFile->read(ehdr, sizeof(*ehdr)) != sizeof(*ehdr) ||
	        memcmp(ehdr->e_ident, ELFMAG, SELFMAG) ||			// Check MAGIC
	        ehdr->e_type != ET_EXEC ||							// Check for executable
#ifdef ARM_TARGET
	        ehdr->e_machine != EM_ARM ||						// Check for ARM machine type
#endif
#ifdef MIPS_TARGET
	        ehdr->e_machine != EM_MIPS ||
#endif
	        ehdr->e_phentsize < sizeof(Elf32_Phdr)	 ||			// Check for size of program header
	        ehdr->e_shentsize != sizeof(Elf32_Shdr)) {			// Check for size of section header
		seterror("Invalid file type.");
		return false;
	}

	DBG("phoff = %d, phentsz = %d, phnum = %d\n",
	    ehdr->e_phoff, ehdr->e_phentsize, ehdr->e_phnum);

	return true;
}

bool DLObject::readProgramHeaders(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, int num) {

	// Read program header
	if (DLFile->seek(ehdr->e_phoff + sizeof(*phdr)*num, SEEK_SET) < 0 ||
	    DLFile->read(phdr, sizeof(*phdr)) != sizeof(*phdr)) {
		seterror("Program header load failed.");
		return false;
	}

	// Check program header values
	if (phdr->p_type != PT_LOAD  || phdr->p_filesz > phdr->p_memsz) {
		seterror("Invalid program header.");
		return false;
	}

	DBG("offs = %x, filesz = %x, memsz = %x, align = %x\n",
	    phdr->p_offset, phdr->p_filesz, phdr->p_memsz, phdr->p_align);

	return true;

}

bool DLObject::loadSegment(Common::SeekableReadStream* DLFile, Elf32_Phdr *phdr) {

	char *baseAddress = 0;

#ifdef MIPS_TARGET
	// We need to take account of non-allocated segment for shorts
	if (phdr->p_flags & PF_X) {	// This is a relocated segment

		// Attempt to allocate memory for segment
		int extra = phdr->p_vaddr % phdr->p_align;	// Get extra length TODO: check logic here
		DBG("extra mem is %x\n", extra);

		if (phdr->p_align < 0x10000) phdr->p_align = 0x10000;	// Fix for wrong alignment on e.g. AGI

		if (!(_segment = (char *)memalign(phdr->p_align, phdr->p_memsz + extra))) {
			seterror("Out of memory.\n");
			return false;
		}
		DBG("allocated segment @ %p\n", _segment);

		// Get offset to load segment into
		baseAddress = (char *)_segment + phdr->p_vaddr;
		_segmentSize = phdr->p_memsz + extra;
	} else {						// This is a shorts section.
		_shortsSegment = ShortsMan.newSegment(phdr->p_memsz, (char *)phdr->p_vaddr);

		baseAddress = _shortsSegment->getStart();
		DBG("shorts segment @ %p to %p. Segment wants to be at %x. Offset=%x\n",
		    _shortsSegment->getStart(), _shortsSegment->getEnd(), phdr->p_vaddr, _shortsSegment->getOffset());
	}
#else
	// Attempt to allocate memory for segment
	int extra = phdr->p_vaddr % phdr->p_align;	// Get extra length TODO: check logic here
	DBG("extra mem is %x\n", extra);

	if (!(_segment = (char *)memalign(phdr->p_align, phdr->p_memsz + extra))) {
		seterror("Out of memory.\n");
		return false;
	}

	DBG("allocated segment @ %p\n", _segment);

	// Get offset to load segment into
	baseAddress = (char *)_segment + phdr->p_vaddr;
	_segmentSize = phdr->p_memsz + extra;
#endif

	// Set bss segment to 0 if necessary (assumes bss is at the end)
	if (phdr->p_memsz > phdr->p_filesz) {
		DBG("Setting %p to %p to 0 for bss\n", baseAddress + phdr->p_filesz, baseAddress + phdr->p_memsz);
		memset(baseAddress + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
	}

	DBG("Reading the segment into memory\n");

	// Read the segment into memory
	if (DLFile->seek(phdr->p_offset, SEEK_SET) < 0 ||
	        DLFile->read(baseAddress, phdr->p_filesz) != (ssize_t)phdr->p_filesz) {
		seterror("Segment load failed.");
		return false;
	}

	DBG("Segment has been read into memory\n");

	return true;
}

Elf32_Shdr * DLObject::loadSectionHeaders(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr) {

	Elf32_Shdr *shdr = NULL;

	// Allocate memory for section headers
	if (!(shdr = (Elf32_Shdr *)malloc(ehdr->e_shnum * sizeof(*shdr)))) {
		seterror("Out of memory.");
		return NULL;
	}

	// Read from file into section headers
	if (DLFile->seek(ehdr->e_shoff, SEEK_SET) < 0 ||
	        DLFile->read(shdr, ehdr->e_shnum * sizeof(*shdr)) !=
	        (ssize_t)(ehdr->e_shnum * sizeof(*shdr))) {
		seterror("Section headers load failed.");
		return NULL;
	}

	return shdr;
}

int DLObject::loadSymbolTable(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {

	// Loop over sections, looking for symbol table linked to a string table
	for (int i = 0; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_SYMTAB &&
		        shdr[i].sh_entsize == sizeof(Elf32_Sym) &&
		        shdr[i].sh_link < ehdr->e_shnum &&
		        shdr[shdr[i].sh_link].sh_type == SHT_STRTAB &&
		        _symtab_sect < 0) {
			_symtab_sect = i;
		}
	}

	// Check for no symbol table
	if (_symtab_sect < 0) {
		seterror("No symbol table.");
		return -1;
	}

	DBG("Symbol section at section %d, size %x\n", _symtab_sect, shdr[_symtab_sect].sh_size);

	// Allocate memory for symbol table
	if (!(_symtab = malloc(shdr[_symtab_sect].sh_size))) {
		seterror("Out of memory.");
		return -1;
	}

	// Read symbol table into memory
	if (DLFile->seek(shdr[_symtab_sect].sh_offset, SEEK_SET) < 0 ||
	        DLFile->read(_symtab, shdr[_symtab_sect].sh_size) !=
	        (ssize_t)shdr[_symtab_sect].sh_size) {
		seterror("Symbol table load failed.");
		return -1;
	}

	// Set number of symbols
	_symbol_cnt = shdr[_symtab_sect].sh_size / sizeof(Elf32_Sym);
	DBG("Loaded %d symbols.\n", _symbol_cnt);

	return _symtab_sect;

}

bool DLObject::loadStringTable(Common::SeekableReadStream* DLFile, Elf32_Shdr *shdr) {

	int string_sect = shdr[_symtab_sect].sh_link;

	// Allocate memory for string table
	if (!(_strtab = (char *)malloc(shdr[string_sect].sh_size))) {
		seterror("Out of memory.");
		return false;
	}

	// Read string table into memory
	if (DLFile->seek(shdr[string_sect].sh_offset, SEEK_SET) < 0 ||
	        DLFile->read(_strtab, shdr[string_sect].sh_size) !=
	        (ssize_t)shdr[string_sect].sh_size) {
		seterror("Symbol table strings load failed.");
		return false;
	}

	return true;
}

void DLObject::relocateSymbols(Elf32_Addr offset) {

	int mainCount = 0;
	int shortsCount= 0;

	// Loop over symbols, add relocation offset
	Elf32_Sym *s = (Elf32_Sym *)_symtab;
	for (int c = _symbol_cnt; c--; s++) {

#ifdef MIPS_TARGET
		// Make sure we don't relocate special valued symbols
		if (s->st_shndx < SHN_LOPROC) {
			if (!ShortsMan.inGeneralSegment((char *)s->st_value)) {
				mainCount++;
				s->st_value += offset;
				if (s->st_value < (Elf32_Addr)_segment || s->st_value > (Elf32_Addr)_segment + _segmentSize)
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);
			} else {	// shorts section
				shortsCount++;
				s->st_value += _shortsSegment->getOffset();
				if (!_shortsSegment->inSegment((char *)s->st_value))
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);
			}

		}
#else
		// Make sure we don't relocate special valued symbols
		if (s->st_shndx < SHN_LOPROC) {
				mainCount++;
				s->st_value += offset;
				if (s->st_value < (Elf32_Addr)_segment || s->st_value > (Elf32_Addr)_segment + _segmentSize)
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);

		}
#endif

	}
}

bool DLObject::load(Common::SeekableReadStream* DLFile) {
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	Elf32_Shdr *shdr;
	bool ret = true;

	if (readElfHeader(DLFile, &ehdr) == false) {
		return false;
	}

	for (int i = 0; i < ehdr.e_phnum; i++) {	//Load our segments

		DBG("Loading segment %d\n", i);

		if (readProgramHeaders(DLFile, &ehdr, &phdr, i) == false)
			return false;

		if (!loadSegment(DLFile, &phdr))
			return false;
	}

	if ((shdr = loadSectionHeaders(DLFile, &ehdr)) == NULL)
		ret = false;

	if (ret && ((_symtab_sect = loadSymbolTable(DLFile, &ehdr, shdr)) < 0))
		ret = false;

	if (ret && (loadStringTable(DLFile, shdr) == false))
		ret = false;

	if (ret)
		relocateSymbols((Elf32_Addr)_segment);	// Offset by our segment allocated address

	if (ret && (relocateRels(DLFile, &ehdr, shdr) == false))
		ret = false;

	free(shdr);

	return ret;

}

bool DLObject::open(const char *path) {

	Common::SeekableReadStream* DLFile;
	void *ctors_start, *ctors_end;

	DBG("open(\"%s\")\n", path);

	Common::FSNode file(path);

	if (!(DLFile = file.createReadStream())) {
		seterror("%s not found.", path);
		return false;
	}

	DBG("%s found!\n", path);

	/*Try to load and relocate*/
	if (!load(DLFile)) {
		unload();
		return false;
	}

	DBG("loaded!/n");

	flushDataCache();

	ctors_start = symbol("___plugin_ctors");
	ctors_end = symbol("___plugin_ctors_end");
	_dtors_start = symbol("___plugin_dtors");
	_dtors_end = symbol("___plugin_dtors_end");

	if (ctors_start == NULL || ctors_end == NULL || _dtors_start == NULL ||
	        _dtors_end == NULL) {
		seterror("Missing ctors/dtors.");
		_dtors_start = _dtors_end = NULL;
		unload();
		return false;
	}

	DBG(("Calling constructors.\n"));
	for (void (**f)(void) = (void (**)(void))ctors_start; f != ctors_end; f++)
		(**f)();

	DBG(("%s opened ok.\n", path));

	return true;
}

bool DLObject::close() {
	if (_dtors_start != NULL && _dtors_end != NULL)
		for (void (**f)(void) = (void (**)(void))_dtors_start; f != _dtors_end; f++)
			(**f)();
	_dtors_start = _dtors_end = NULL;
	unload();
	return true;
}

void *DLObject::symbol(const char *name) {
	DBG(("symbol(\"%s\")\n", name));

	if (_symtab == NULL || _strtab == NULL || _symbol_cnt < 1) {
		seterror("No symbol table loaded.");
		return NULL;
	}

	Elf32_Sym *s = (Elf32_Sym *)_symtab;
	for (int c = _symbol_cnt; c--; s++)

		// We can only import symbols that are global or weak in the plugin
		if ((SYM_BIND(s->st_info) == STB_GLOBAL || SYM_BIND(s->st_info) == STB_WEAK) &&
		       !strcmp(name, _strtab + s->st_name)) {

			// We found the symbol
			DBG("=> %p\n", (void*)s->st_value);
			return (void*)s->st_value;
		}

	// We didn't find the symbol
	seterror("Symbol \"%s\" not found.", name);
	return NULL;
}

#endif /* defined(DYNAMIC_MODULES) && defined(ELF_LOADER_TARGET) */

