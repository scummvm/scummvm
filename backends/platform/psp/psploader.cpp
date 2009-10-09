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

#if defined(DYNAMIC_MODULES) && defined(__PSP__)

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/_default_fcntl.h>

#include <psputils.h>

#include "backends/platform/psp/psploader.h"
#include "backends/platform/psp/powerman.h"

//#define __PSP_DEBUG_PLUGINS__

#ifdef __PSP_DEBUG_PLUGINS__
#define DBG(x,...) fprintf(stderr,x, ## __VA_ARGS__)
#else
#define DBG(x,...)
#endif

#define seterror(x,...) fprintf(stderr,x, ## __VA_ARGS__)

extern char __plugin_hole_start;	// Indicates start of hole in program file for shorts
extern char __plugin_hole_end;		// Indicates end of hole in program file
extern char _gp[];			// Value of gp register

DECLARE_SINGLETON(ShortSegmentManager);	// For singleton

// Get rid of symbol table in memory
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

	if (_shortsSegment) {
		ShortsMan.deleteSegment(_shortsSegment);
		_shortsSegment = NULL;
	}
}

/**
 * Follow the instruction of a relocation section.
 *
 * @param fd 		 File Descriptor
 * @param offset 	 Offset into the File
 * @param size   	 Size of relocation section
 * @param relSegment Base address of relocated segment in memory (memory offset)
 *
 */
bool DLObject::relocate(int fd, unsigned long offset, unsigned long size, void *relSegment) {
	Elf32_Rel *rel = NULL;	// relocation entry

	// Allocate memory for relocation table
	if (!(rel = (Elf32_Rel *)malloc(size))) {
		seterror("Out of memory.");
		return false;
	}

	// Read in our relocation table
	if (lseek(fd, offset, SEEK_SET) < 0 ||
	        read(fd, rel, size) != (ssize_t)size) {
		seterror("Relocation table load failed.");
		free(rel);
		return false;
	}

	// Treat each relocation entry. Loop over all of them
	int cnt = size / sizeof(*rel);

	DBG("Loaded relocation table. %d entries. base address=%p\n", cnt, relSegment);

	bool seenHi16 = false;	// For treating HI/LO16 commands
	int firstHi16 = -1;		// Mark the point of the first hi16 seen
	Elf32_Addr ahl = 0;		// Calculated addend
	int a = 0;				// Addend: taken from the target

	unsigned int *lastTarget = 0;	// For processing hi16 when lo16 arrives
	unsigned int relocation = 0;
	int debugRelocs[10] = {0};	// For debugging
	int extendedHi16 = 0;			// Count extended hi16 treatments

#define DEBUG_NUM 2

	// Loop over relocation entries
	for (int i = 0; i < cnt; i++) {
		// Get the symbol this relocation entry is referring to
		Elf32_Sym *sym = (Elf32_Sym *)(_symtab) + (REL_INDEX(rel[i].r_info));

		// Get the target instruction in the code
		unsigned int *target = (unsigned int *)((char *)relSegment + rel[i].r_offset);

		unsigned int origTarget = *target;	// Save for debugging

		// Act differently based on the type of relocation
		switch (REL_TYPE(rel[i].r_info)) {

		case R_MIPS_HI16:						// Absolute addressing.
			if (sym->st_shndx < SHN_LOPROC &&		// Only shift for plugin section (ie. has a real section index)
			        firstHi16 < 0) {			// Only process first in block of HI16s
				firstHi16 = i;						// Keep the first Hi16 we saw
				seenHi16 = true;
				ahl = (*target & 0xffff) << 16;		// Take lower 16 bits shifted up

				if (debugRelocs[0]++ < DEBUG_NUM)	// Print only a set number
					DBG("R_MIPS_HI16: i=%d, offset=%x, ahl = %x, target = %x\n",
					    i, rel[i].r_offset, ahl, *target);
			}
			break;

		case R_MIPS_LO16:						// Absolute addressing. Needs a HI16 to come before it
			if (sym->st_shndx < SHN_LOPROC) {		// Only shift for plugin section. (ie. has a real section index)
				if (!seenHi16) {				// We MUST have seen HI16 first
					seterror("R_MIPS_LO16 w/o preceding R_MIPS_HI16 at relocation %d!\n", i);
					free(rel);
					return false;
				}

				ahl &= 0xffff0000;				// Clean lower 16 bits for repeated LO16s
				a = *target & 0xffff;			// Take lower 16 bits of the target
				a = (a << 16) >> 16;				// Sign extend them
				ahl += a;						// Add lower 16 bits. AHL is now complete
				relocation = ahl + (Elf32_Addr)_segment;	// Add in the new offset for the segment

				if (firstHi16 >= 0) {					// We haven't treated the HI16s yet so do it now
					for (int j = firstHi16; j < i; j++) {
						if (REL_TYPE(rel[j].r_info) != R_MIPS_HI16) continue;	// Skip over non-Hi16s

						lastTarget = (unsigned int *)((char *)relSegment + rel[j].r_offset);	// get hi16 target
						*lastTarget &= 0xffff0000;		// Clear the lower 16 bits of the last target
						*lastTarget |= (relocation >> 16) & 0xffff;	// Take the upper 16 bits of the relocation
						if (relocation & 0x8000)(*lastTarget)++;	// Subtle: we need to add 1 to the HI16 in this case
					}
					firstHi16 = -1;						// Reset so we'll know we treated it
				} else {
					extendedHi16++;
				}

				*target &= 0xffff0000;						// Clear the lower 16 bits of current target
				*target |= relocation & 0xffff;				// Take the lower 16 bits of the relocation

				if (debugRelocs[1]++ < DEBUG_NUM)
					DBG("R_MIPS_LO16: i=%d, offset=%x, a=%x, ahl = %x, lastTarget = %x, origt = %x, target = %x\n",
					    i, rel[i].r_offset, a, ahl, *lastTarget, origTarget, *target);
				if (ahl & 0x8000 && debugRelocs[2]++ < DEBUG_NUM)
					DBG("R_MIPS_LO16: i=%d, offset=%x, a=%x, ahl = %x, lastTarget = %x, origt = %x, target = %x\n",
					    i, rel[i].r_offset, a, ahl, *lastTarget, origTarget, *target);
			}
			break;

		case R_MIPS_26:									// Absolute addressing
			if (sym->st_shndx < SHN_LOPROC) {			// Only relocate for main segment
				a = *target & 0x03ffffff;				// Get 26 bits' worth of the addend
				a = (a << 6) >> 6; 							// Sign extend a
				relocation = ((a << 2) + (Elf32_Addr)_segment) >> 2;	// a already points to the target. Subtract our offset
				*target &= 0xfc000000;					// Clean lower 26 target bits
				*target |= (relocation & 0x03ffffff);

				if (debugRelocs[3]++ < DEBUG_NUM)
					DBG("R_MIPS_26: i=%d, offset=%x, symbol=%d, stinfo=%x, a=%x, origTarget=%x, target=%x\n",
					    i, rel[i].r_offset, REL_INDEX(rel[i].r_info), sym->st_info, a, origTarget, *target);
			} else {
				if (debugRelocs[4]++ < DEBUG_NUM)
					DBG("R_MIPS_26: i=%d, offset=%x, symbol=%d, stinfo=%x, a=%x, origTarget=%x, target=%x\n",
					    i, rel[i].r_offset, REL_INDEX(rel[i].r_info), sym->st_info, a, origTarget, *target);
			}
			break;

		case R_MIPS_GPREL16:							// GP Relative addressing
			if (_shortsSegment->getOffset() != 0 && 	// Only relocate if we shift the shorts section
			        ShortsMan.inGeneralSegment((char *)sym->st_value)) {	// Only relocate things in the plugin hole
				a = *target & 0xffff;				    // Get 16 bits' worth of the addend
				a = (a << 16) >> 16;						// Sign extend it

				relocation = a + _shortsSegment->getOffset();

				*target &= 0xffff0000;					// Clear the lower 16 bits of the target
				*target |= relocation & 0xffff;

				if (debugRelocs[5]++ < DEBUG_NUM)
					DBG("R_MIPS_GPREL16: i=%d, a=%x, gpVal=%x, origTarget=%x, target=%x, offset=%x\n",
					    i, a, _gpVal, origTarget, *target, _shortsSegment->getOffset());
			}

			break;

		case R_MIPS_32:									// Absolute addressing
			if (sym->st_shndx < SHN_LOPROC) {			// Only shift for plugin section.
				a = *target;							// Get full 32 bits of addend
				relocation = a + (Elf32_Addr)_segment;	// Shift
				*target = relocation;

				if (debugRelocs[6]++ < DEBUG_NUM)
					DBG("R_MIPS_32: i=%d, a=%x, origTarget=%x, target=%x\n", i, a, origTarget, *target);
			}
			break;

		default:
			seterror("Unknown relocation type %x at relocation %d.\n", REL_TYPE(rel[i].r_info), i);
			free(rel);
			return false;
		}
	}

	DBG("Done with relocation. extendedHi16=%d\n\n", extendedHi16);

	free(rel);
	return true;
}

bool DLObject::readElfHeader(int fd, Elf32_Ehdr *ehdr) {
	// Start reading the elf header. Check for errors
	if (read(fd, ehdr, sizeof(*ehdr)) != sizeof(*ehdr) ||
	        memcmp(ehdr->e_ident, ELFMAG, SELFMAG) ||					// Check MAGIC
	        ehdr->e_type != ET_EXEC ||									// Check for executable
	        ehdr->e_machine != EM_MIPS ||								// Check for MIPS machine type
	        ehdr->e_phentsize < sizeof(Elf32_Phdr)	 ||					// Check for size of program header
	        ehdr->e_shentsize != sizeof(Elf32_Shdr)) {					// Check for size of section header
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
		//DBG("Section %d: type = %x, size = %x, entsize = %x, link = %x\n",
		// i, shdr[i].sh_type, shdr[i].sh_size, shdr[i].sh_entsize, shdr[i].sh_link);

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

void DLObject::relocateSymbols(Elf32_Addr offset, Elf32_Addr shortsOffset) {

	int shortsCount = 0, othersCount = 0;
	DBG("Relocating symbols by %x. Shorts offset=%x\n", offset, shortsOffset);

	// Loop over symbols, add relocation offset
	Elf32_Sym *s = (Elf32_Sym *)_symtab;
	for (int c = _symbol_cnt; c--; s++) {
		// Make sure we don't relocate special valued symbols
		if (s->st_shndx < SHN_LOPROC) {
			if (!ShortsMan.inGeneralSegment((char *)s->st_value)) {
				othersCount++;
				s->st_value += offset;
				if (s->st_value < (Elf32_Addr)_segment || s->st_value > (Elf32_Addr)_segment + _segmentSize)
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);
			} else {	// shorts section
				shortsCount++;
				s->st_value += shortsOffset;
				if (!_shortsSegment->inSegment((char *)s->st_value))
					seterror("Symbol out of bounds! st_value = %x\n", s->st_value);
			}

		}

	}

	DBG("Relocated %d short symbols, %d others.\n", shortsCount, othersCount);
}

bool DLObject::relocateRels(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {

	// Loop over sections, finding relocation sections
	for (int i = 0; i < ehdr->e_shnum; i++) {

		Elf32_Shdr *curShdr = &(shdr[i]);
		//Elf32_Shdr *linkShdr = &(shdr[curShdr->sh_info]);

		if (curShdr->sh_type == SHT_REL && 						// Check for a relocation section
		        curShdr->sh_entsize == sizeof(Elf32_Rel) &&		    // Check for proper relocation size
		        (int)curShdr->sh_link == _symtab_sect &&			// Check that the sh_link connects to our symbol table
		        curShdr->sh_info < ehdr->e_shnum &&					// Check that the relocated section exists
		        (shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {  	// Check if relocated section resides in memory
			if (!ShortsMan.inGeneralSegment((char *)shdr[curShdr->sh_info].sh_addr)) {			// regular segment
				if (!relocate(fd, curShdr->sh_offset, curShdr->sh_size, _segment)) {
					return false;
				}
			} else { 	// In Shorts segment
				if (!relocate(fd, curShdr->sh_offset, curShdr->sh_size, (void *)_shortsSegment->getOffset())) {
					return false;
				}
			}

		}
	}

	return true;
}


bool DLObject::load(int fd) {
	fprintf(stderr, "In DLObject::load\n");

	Elf32_Ehdr ehdr;	// ELF header
	Elf32_Phdr phdr;	// Program header
	Elf32_Shdr *shdr;	// Section header
	bool ret = true;

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
		relocateSymbols((Elf32_Addr)_segment, _shortsSegment->getOffset());	// Offset by our segment allocated address

	if (ret && (relocateRels(fd, &ehdr, shdr) == false))
		ret = false;

	if (shdr)
		free(shdr);

	return ret;
}

bool DLObject::open(const char *path) {
	int fd;
	void *ctors_start, *ctors_end;

	DBG("open(\"%s\")\n", path);

	// Get the address of the global pointer
	_gpVal = (unsigned int) & _gp;
	DBG("_gpVal is %x\n", _gpVal);

	PowerMan.beginCriticalSection();

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

	PowerMan.endCriticalSection();

	// flush data cache
	sceKernelDcacheWritebackAll();

	// Get the symbols for the global constructors and destructors
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

	DBG("Calling constructors.\n");
	for (void (**f)(void) = (void (**)(void))ctors_start; f != ctors_end; f++)
		(**f)();

	DBG("%s opened ok.\n", path);
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
	DBG("symbol(\"%s\")\n", name);

	if (_symtab == NULL || _strtab == NULL || _symbol_cnt < 1) {
		seterror("No symbol table loaded.");
		return NULL;
	}

	Elf32_Sym *s = (Elf32_Sym *)_symtab;
	for (int c = _symbol_cnt; c--; s++) {

		// We can only import symbols that are global or weak in the plugin
		if ((SYM_BIND(s->st_info) == STB_GLOBAL || SYM_BIND(s->st_info) == STB_WEAK) &&
		        /*_strtab[s->st_name] == '_' && */ // Try to make this more efficient
		        !strcmp(name, _strtab + s->st_name)) {

			// We found the symbol
			DBG("=> %p\n", (void*)s->st_value);
			return (void*)s->st_value;
		}
	}

	seterror("Symbol \"%s\" not found.", name);
	return NULL;
}



ShortSegmentManager::ShortSegmentManager() {
	_shortsStart = &__plugin_hole_start ;
	_shortsEnd = &__plugin_hole_end;
}

ShortSegmentManager::Segment *ShortSegmentManager::newSegment(int size, char *origAddr) {
	char *lastAddress = origAddr;
	Common::List<Segment *>::iterator i;

	// Find a block that fits, starting from the beginning
	for (i = _list.begin(); i != _list.end(); i++) {
		char *currAddress = (*i)->getStart();

		if ((int)(currAddress - lastAddress) >= size) break;

		lastAddress = (*i)->getEnd();
	}

	if ((Elf32_Addr)lastAddress & 3)
		lastAddress += 4 - ((Elf32_Addr)lastAddress & 3);	// Round up to multiple of 4

	if (lastAddress + size > _shortsEnd) {
		seterror("Error. No space in shorts segment for %x bytes. Last address is %p, max address is %p.\n",
		         size, lastAddress, _shortsEnd);
		return NULL;
	}

	Segment *seg = new Segment(lastAddress, size, origAddr);	// Create a new segment

	if (lastAddress + size > _highestAddress) _highestAddress = lastAddress + size;	// Keep track of maximum

	_list.insert(i, seg);

	DBG("Shorts segment size %x allocated. End = %p. Remaining space = %x. Highest so far is %p.\n",
	    size, lastAddress + size, _shortsEnd - _list.back()->getEnd(), _highestAddress);

	return seg;
}

void ShortSegmentManager::deleteSegment(ShortSegmentManager::Segment *seg) {
	DBG("Deleting shorts segment from %p to %p.\n\n", seg->getStart(), seg->getEnd());
	_list.remove(seg);
	delete seg;
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


#endif /* DYNAMIC_MODULES && __PSP__ */
