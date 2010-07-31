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

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_MEMLEAKS_H
#define SWORD25_MEMLEAKS_H

#ifdef BS_MEMLOG

#ifdef _MSC_VER
#pragma warning(disable : 4291)
#endif

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

#endif
