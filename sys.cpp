/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * Change Log:
 * $Log$
 * Revision 1.3  2001/11/05 19:21:49  strigeus
 * bug fixes,
 * speech in dott
 *
 * Revision 1.2  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void *Scumm::fileOpen(const char *filename, int mode) {
	_fileMode = mode;
	_whereInResToRead = 0;	
	clearFileReadFailed(_fileHandle);

	if (mode==1)
		return fopen(filename, "rb");
	
	if (mode==2) {
		error("fileOpen: write not supported");
	}
	
	return NULL;
}

void Scumm::fileClose(void *file) {
	if (_fileMode==1 || _fileMode==2)
		fclose((FILE*)file);
}

bool Scumm::fileReadFailed(void *file) {
	return _fileReadFailed != 0;
}

void Scumm::clearFileReadFailed(void *file) {
	_fileReadFailed = false;
}

bool Scumm::fileEof(void *file) {
	FILE *a = (FILE*)file;
	return feof((FILE*)file) != 0;
}

void Scumm::fileSeek(void *file, long offs, int whence) {
	switch(_fileMode) {
	case 1: case 2:
		fseek((FILE*)file, offs, whence);
		return;
	case 3:
		_whereInResToRead = offs;
		return;
	}
}

void Scumm::fileRead(void *file, void *ptr, uint32 size) {
	byte *ptr2 = (byte*)ptr, *src;

	switch(_fileMode) {
	case 1:
		if (size==0)
			return;

		if ((uint32)fread(ptr2, size, 1, (FILE*)file) != 1)
			_fileReadFailed = true;

		do {
			*ptr2++	^= _encbyte;
		} while(--size);

		return;

	case 3:
		if (size==0)
			return;

		src = getResourceAddress(rtTemp, 3) + _whereInResToRead;
		_whereInResToRead += size;
		do {
			*ptr2++ = *src++ ^ _encbyte;
		} while (--size);
		return;
	}
}

int Scumm::fileReadByte() {
	byte b;
	byte *src;

	switch(_fileMode) {
	case 1:
		if (fread(&b,1,1,(FILE*)_fileHandle) != 1)
			_fileReadFailed = true;
		return b ^ _encbyte;

	case 3:
		src = getResourceAddress(rtTemp, 3) + _whereInResToRead;
		_whereInResToRead++;
		return *src ^ _encbyte;
	}
	return 0;
}

uint Scumm::fileReadWordLE() {
	uint a = fileReadByte();
	uint b = fileReadByte();
	return a|(b<<8);
}

uint32 Scumm::fileReadDwordLE() {
	uint a = fileReadWordLE();
	uint b = fileReadWordLE();
	return (b<<16)|a;
}

uint Scumm::fileReadWordBE() {
	uint b = fileReadByte();
	uint a = fileReadByte();
	return a|(b<<8);
}

uint32 Scumm::fileReadDwordBE() {
	uint b = fileReadWordBE();
	uint a = fileReadWordBE();
	return (b<<16)|a;
}

byte *Scumm::alloc(int size) {
	byte *me = (byte*)::calloc(size+4,1);
	if (me==NULL)
		return NULL;

	*((uint32*)me) = 0xDEADBEEF;
	return me + 4;
}

void Scumm::free(void *mem) {
	if (mem) {
		byte *me = (byte*)mem - 4;
		if ( *((uint32*)me) != 0xDEADBEEF) {
			error("Freeing invalid block.");
		}

		*((uint32*)me) = 0xC007CAFE;
		::free(me);
	}
}

bool Scumm::checkFixedDisk() {
	return true;
}

