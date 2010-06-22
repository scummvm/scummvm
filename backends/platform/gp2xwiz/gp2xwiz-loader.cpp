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

#if defined(DYNAMIC_MODULES) && defined(GP2X)

#include <string.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

#include "backends/platform/gp2xwiz/gp2xwiz-loader.h"

#ifdef __WIZ_DEBUG_PLUGINS__
#define DBG(x) printf(x, ## __VA_ARGS__)
#else
#define DBG(x)
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
	segment = NULL;
}

/**
 * Follow the instruction of a relocation section.
 *
 * @param fd 		 File Descriptor
 * @param offset 	 Offset into the File
 * @param size   	 Size of relocation section
 *
 */
bool DLObject::relocate(int fd, unsigned long offset, unsigned long size) {
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

	        //Elf32_Sym *sym = ???;

		//void *target = ???;

		switch (/*relocation type*/) {
		//case ??? :
			//TODO: Cases for each relocation type.
			//break;
		default:
			seterror("Unknown relocation type %d.", ?? ?);
			free(rela);
			return false;
		}

	}

	free(rela);
	return true;
}


bool DLObject::load(int fd) {
	Elf32_Ehdr ehdr; // ELF header
	Elf32_Phdr phdr; // Program header
	Elf32_Shdr *shdr; // Section header

	//TODO: fill this out!

	free(shdr);

	return true;
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
	for (int c = symbol_cnt; c--; s++)

		//TODO: Figure out which symbols should be detected here
		if ((s->st_info >> 4 == 1 || s->st_info >> 4 == 2) &&
		        strtab[s->st_name] == '_' && !strcmp(name, strtab + s->st_name + 1)) {

			// We found the symbol
			DBG(("=> %p\n", (void*)s->st_value));
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

#endif /* DYNAMIC_MODULES && GP2X__WIZ */
