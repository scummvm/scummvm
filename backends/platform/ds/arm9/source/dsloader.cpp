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

#if defined(DYNAMIC_MODULES) && defined(__DS__)

#include <string.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "backends/platform/ds/arm9/source/dsloader.h"

#define __DS_DEBUG_PLUGINS__

#ifdef __DS_DEBUG_PLUGINS__
#define DBG(x,...) printf(x, ## __VA_ARGS__)
#else
#define DBG(x,...)
#endif

#define seterror(x,...) fprintf(stderr,x, ## __VA_ARGS__)

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
}

/**
 * Follow the instruction of a relocation section.
 *
 * @param fd 		 File Descriptor
 * @param offset 	 Offset into the File
 * @param size   	 Size of relocation section
 *
 */
bool DLObject::relocate(int fd, unsigned long offset, unsigned long size, void *relSegment) {
	Elf32_Rela *rela; //relocation entry

	// Allocate memory for relocation table
	if (!(rela = (Elf32_Rela *)malloc(size))) {
		seterror("Out of memory.");
		return false;
	}

	// Read in our relocation table
	if (lseek(fd, offset, SEEK_SET) < 0 ||
	        read(fd, rela, size) != (ssize_t)size) {
		seterror("Relocation table load failed.");
		free(rela);
		return false;
	}

	// Treat each relocation entry. Loop over all of them
	int cnt = size / sizeof(*rela);

	// TODO: Loop over relocation entries
	for (int i = 0; i < cnt; i++) {

		DBG("attempting to relocate!");

	    //Elf32_Sym *sym = ???;

		//void *target = ???;

		/*switch (REL_TYPE()) {*/
		//case ??? :
			//TODO: Cases for each relocation type.
			//break;
	//	default:
			//seterror("Unknown relocation type %d.", ?? ?);
			free(rela);
			return false;
	//	}

	}

	free(rela);
	return true;
}

bool DLObject::readElfHeader(int fd, Elf32_Ehdr *ehdr) {

	// Start reading the elf header. Check for errors
	if (read(fd, ehdr, sizeof(*ehdr)) != sizeof(*ehdr) ||
	        memcmp(ehdr->e_ident, ELFMAG, SELFMAG) ||			// Check MAGIC
	        ehdr->e_type != ET_EXEC ||							// Check for executable
	        ehdr->e_machine != EM_ARM ||						// Check for ARM machine type
	        ehdr->e_phentsize < sizeof(Elf32_Phdr)	 ||			// Check for size of program header
	        ehdr->e_shentsize != sizeof(Elf32_Shdr)) {			// Check for size of section header
		seterror("Invalid file type.");
		return false;
	}

	DBG("phoff = %d, phentsz = %d, phnum = %d\n",
	    ehdr->e_phoff, ehdr->e_phentsize, ehdr->e_phnum);

	return true;
}

bool DLObject::readProgramHeaders(int fd, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, int num) {

	// Read program header
	if (lseek(fd, ehdr->e_phoff + sizeof(*phdr)*num, SEEK_SET) < 0 ||
	    read(fd, phdr, sizeof(*phdr)) != sizeof(*phdr)) {
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

bool DLObject::loadSegment(int fd, Elf32_Phdr *phdr) {

	char *baseAddress = 0;

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

	// Set bss segment to 0 if necessary (assumes bss is at the end)
	if (phdr->p_memsz > phdr->p_filesz) {
		DBG("Setting %p to %p to 0 for bss\n", baseAddress + phdr->p_filesz, baseAddress + phdr->p_memsz);
		memset(baseAddress + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
	}
	// Read the segment into memory
	if (lseek(fd, phdr->p_offset, SEEK_SET) < 0 ||
	        read(fd, baseAddress, phdr->p_filesz) != (ssize_t)phdr->p_filesz) {
		seterror("Segment load failed.");
		return false;
	}

	return true;
}

Elf32_Shdr * DLObject::loadSectionHeaders(int fd, Elf32_Ehdr *ehdr) {

	Elf32_Shdr *shdr = NULL;

	// Allocate memory for section headers
	if (!(shdr = (Elf32_Shdr *)malloc(ehdr->e_shnum * sizeof(*shdr)))) {
		seterror("Out of memory.");
		return NULL;
	}

	// Read from file into section headers
	if (lseek(fd, ehdr->e_shoff, SEEK_SET) < 0 ||
	        read(fd, shdr, ehdr->e_shnum * sizeof(*shdr)) !=
	        (ssize_t)(ehdr->e_shnum * sizeof(*shdr))) {
		seterror("Section headers load failed.");
		return NULL;
	}

	return shdr;
}

int DLObject::loadSymbolTable(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {

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
	if (lseek(fd, shdr[_symtab_sect].sh_offset, SEEK_SET) < 0 ||
	        read(fd, _symtab, shdr[_symtab_sect].sh_size) !=
	        (ssize_t)shdr[_symtab_sect].sh_size) {
		seterror("Symbol table load failed.");
		return -1;
	}

	// Set number of symbols
	_symbol_cnt = shdr[_symtab_sect].sh_size / sizeof(Elf32_Sym);
	DBG("Loaded %d symbols.\n", _symbol_cnt);

	return _symtab_sect;

}

bool DLObject::loadStringTable(int fd, Elf32_Shdr *shdr) {

	int string_sect = shdr[_symtab_sect].sh_link;

	// Allocate memory for string table
	if (!(_strtab = (char *)malloc(shdr[string_sect].sh_size))) {
		seterror("Out of memory.");
		return false;
	}

	// Read string table into memory
	if (lseek(fd, shdr[string_sect].sh_offset, SEEK_SET) < 0 ||
	        read(fd, _strtab, shdr[string_sect].sh_size) !=
	        (ssize_t)shdr[string_sect].sh_size) {
		seterror("Symbol table strings load failed.");
		return false;
	}

	return true;
}

void DLObject::relocateSymbols(Elf32_Addr offset) {

	int relocCount = 0;
	DBG("Relocating symbols by %x\n", offset);

	// Loop over symbols, add relocation offset
	Elf32_Sym *s = (Elf32_Sym *)_symtab;
	for (int c = _symbol_cnt; c--; s++) {
		// Make sure we don't relocate special valued symbols
		if (s->st_shndx < SHN_LOPROC) {
				relocCount++;
				s->st_value += offset;
				if (s->st_value < (Elf32_Addr)_segment || s->st_value > (Elf32_Addr)_segment + _segmentSize)
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);

		}

	}

	DBG("Relocated %d symbols.\n",relocCount);
}

bool DLObject::relocateRels(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {

	// Loop over sections, finding relocation sections
	for (int i = 0; i < ehdr->e_shnum; i++) {

		Elf32_Shdr *curShdr = &(shdr[i]);
		//Elf32_Shdr *linkShdr = &(shdr[curShdr->sh_info]);

		if (curShdr->sh_type == SHT_REL && 						// Check for a relocation section
		        curShdr->sh_entsize == sizeof(Elf32_Rela) &&		    // Check for proper relocation size
		        (int)curShdr->sh_link == _symtab_sect &&			// Check that the sh_link connects to our symbol table
		        curShdr->sh_info < ehdr->e_shnum &&					// Check that the relocated section exists
		        (shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {  	// Check if relocated section resides in memory

			if (!relocate(fd, curShdr->sh_offset, curShdr->sh_size, _segment)) {
				return false;
			}

		}
	}

	return true;
}

bool DLObject::load(int fd) {
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	Elf32_Shdr *shdr;
	bool ret = true;

	//int symtab_sect = -1;

	if (readElfHeader(fd, &ehdr) == false) {
		return false;
	}

	for (int i = 0; i < ehdr.e_phnum; i++) {	//	Load our 2 segments

		fprintf(stderr, "Loading segment %d\n", i);

		if (readProgramHeaders(fd, &ehdr, &phdr, i) == false)
			return false;

		if (!loadSegment(fd, &phdr))
			return false;
	}

	if ((shdr = loadSectionHeaders(fd, &ehdr)) == NULL)
		ret = false;

	if (ret && ((_symtab_sect = loadSymbolTable(fd, &ehdr, shdr)) < 0))
		ret = false;

	if (ret && (loadStringTable(fd, shdr) == false))
		ret = false;

	if (ret)
		relocateSymbols((Elf32_Addr)_segment);	// Offset by our segment allocated address

	if (ret && (relocateRels(fd, &ehdr, shdr) == false))
		ret = false;

	free(shdr);

	return ret;

}

bool DLObject::open(const char *path) {
	int fd;
	void *ctors_start, *ctors_end;

	DBG(("open(\"%s\")\n", path));

	if ((fd = ::open(path, O_RDONLY)) < 0) {
		seterror("%s not found.", path);
		return false;
	}

	// Try to load and relocate
	if (!load(fd)) {
		::close(fd);
		unload();
		return false;
	}

	::close(fd);

	//TODO: flush data cache

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

		//TODO: Figure out which symbols should be detected here
		if ((s->st_info >> 4 == 1 || s->st_info >> 4 == 2) &&
		        _strtab[s->st_name] == '_' && !strcmp(name, _strtab + s->st_name + 1)) {

			// We found the symbol
			DBG("=> %p\n", (void*)s->st_value);
			return (void*)s->st_value;
		}

	// We didn't find the symbol
	seterror("Symbol \"%s\" not found.", name);
	return NULL;
}


static char dlerr[MAXDLERRLEN];

void *dlopen(const char *filename, int flags) {
	DLObject *obj = new DLObject(dlerr);
	if (obj->open(filename))
		return (void *)obj;
	delete obj;
	return NULL;
}

int dlclose(void *handle) {
	DLObject *obj = (DLObject *)handle;
	if (obj == NULL) {
		strcpy(dlerr, "Handle is NULL.");
		return -1;
	}
	if (obj->close()) {
		delete obj;
		return 0;
	}
	return -1;
}

void *dlsym(void *handle, const char *symbol) {
	if (handle == NULL) {
		strcpy(dlerr, "Handle is NULL.");
		return NULL;
	}
	return ((DLObject *)handle)->symbol(symbol);
}

const char *dlerror() {
	return dlerr;
}

void dlforgetsyms(void *handle) {
	if (handle != NULL)
		((DLObject *)handle)->discard_symtab();
}

#endif /* DYNAMIC_MODULES && __DS__ */
