// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_MEMLEAKS_H
#define SWORD25_MEMLEAKS_H

#ifdef BS_MEMLOG

#ifdef _MSC_VER
#pragma warning(disable : 4291)
#endif

#include "sword25/kernel/memlog_off.h"

#include <malloc.h>

void DumpUnfreed(const char * OutputFilename);
void AddTrack(unsigned int addr,  unsigned int asize,  const char *fname, unsigned int lnum);
void RemoveTrack(unsigned int addr);

inline void * __cdecl operator new(unsigned int size, const char *file, int line)
{
	void *ptr = malloc(size);
	if (ptr) AddTrack((unsigned int)ptr, size, file, line);
	return(ptr);
};

inline void __cdecl operator delete(void *p)
{
	RemoveTrack((unsigned int)p);
	free(p);
};

inline void __cdecl operator delete[](void *p)
{
	RemoveTrack((unsigned int)p);
	free(p);
};

#endif

#include "memlog_on.h"

#endif
