/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"
#include "scummsys.h"
#include "system.h"
#include "simon.h"

#include <stdarg.h>

#ifdef WIN32
#include <conio.h>
#endif

uint16 swap16(uint16 a) {
	return (a>>8)|(a<<8);
}

uint32 swap32(uint32 a) {
	return (a>>24)|(a>>8)&0xFF00|(a<<8)&0xFF0000|(a<<24);
}


uint fileReadByte(FILE *in) {
	byte b;
	fread(&b, sizeof(b), 1, in);
	return b;
}

uint fileReadBE16(FILE *in) {
	byte b[2];
	fread(b, sizeof(b), 1, in);
	return (b[0]<<8) | b[1];
}

uint fileReadLE16(FILE *in) {
	byte b[2];
	fread(b, sizeof(b), 1, in);
	return (b[1]<<8) | b[0];
}

uint32 fileReadBE32(FILE *in) {
	byte b[4];
	fread(b, sizeof(b), 1, in);
	return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3];
}

uint32 fileReadLE32(FILE *in) {
	byte b[4];
	fread(b, sizeof(b), 1, in);
	return (b[3]<<24)|(b[2]<<16)|(b[1]<<8)|b[0];
}


void fileWriteBE32(FILE *in, uint32 value) {
	value = swap32(value);	
	fwrite(&value, sizeof(value), 1, in);
}

void fileWriteBE16(FILE *in, uint16 value) {
	value = swap16(value);
	fwrite(&value, sizeof(value), 1, in);
}



#if 0
void NORETURN CDECL error(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);
#ifdef WIN32
	fprintf(stderr, "Error: %s!\nPress a key to quit.\n", buf);
	_getch();
#else
	fprintf(stderr, "Error: %s!\n", buf);
#endif
	exit(1);

}

void CDECL warning(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);
	fprintf(stdout, "Warning: %s!\n", buf);
}
#endif


#ifndef WIN32
/* GetAsyncKeyState for unix */
int GetAsyncKeyState(int key) {
	return 0;
}


#endif