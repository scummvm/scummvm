/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#include "file.h"

#ifdef NEED_STRDUP
char *strdup(const char *s) {
	if (s) {
		int len = strlen(s) + 1;
		char *d = (char *)malloc(len);
		if (d)
			memcpy(d, s, len);
		return d;
	}
	return NULL;
}
#endif /* NEED_STRDUP */

File::File() {
	_handle = NULL;
	_readFailed = false;
	_encbyte = 0;
}

File::~File() {
	close();
}

bool File::open(const char *filename, int mode, byte encbyte) {
	char * buf;
	if (_handle) {
		debug(2, "File %s already opened", filename);
		return false;
	}

	clearReadFailed();
	buf = strdup(filename);
	if (mode == 1) {
		_handle = fopen(buf, "rb");
		if (_handle == NULL) {
			_handle = fopen(strupr(buf), "rb");
			if (_handle == NULL) {
				_handle = fopen(strlwr(buf), "rb");
			}
			else {
				debug(2, "File %s not found", filename);
				return false;
			}
		}
	}
	else {
		warning("Only read mode supported!");
		return false;
	}

	return true;
}

void File::close() {
	if (_handle)
		fclose(_handle);
}

bool File::readFailed() {
	return _readFailed != 0;
}

void File::clearReadFailed() {
	_readFailed = false;
}

bool File::eof() {
	if (_handle == NULL) {
		error("File is not open!");
		return false;
	}

	return feof(_handle) != 0;
}

uint32 File::pos() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	return ftell(_handle);
}

void File::seek(uint32 offs, int whence) {
	if (_handle == NULL) {
		error("File is not open!");
		return;
	}

	if (fseek(_handle, offs, whence) != 0)
		clearerr(_handle);
}

void File::read(void *ptr, uint32 size) {
	byte *ptr2 = (byte *)ptr;

	if (_handle == NULL) {
		error("File is not open!");
		return;
	}

	if (size == 0)
		return;

	if ((uint32)fread(ptr2, size, 1, _handle) != 1) {
		clearerr(_handle);
		_readFailed = true;
	}

	do {
		*ptr2++ ^= _encbyte;
	} while (--size);
}

byte File::fileReadByte() {
	byte b;

	if (fread(&b, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_readFailed = true;
	}
	return b ^ _encbyte;
}

uint16 File::fileReadWordLE() {
	uint a = fileReadByte();
	uint b = fileReadByte();
	return a | (b << 8);
}

uint32 File::fileReadDwordLE() {
	uint a = fileReadWordLE();
	uint b = fileReadWordLE();
	return (b << 16) | a;
}

uint16 File::fileReadWordBE() {
	uint b = fileReadByte();
	uint a = fileReadByte();
	return a | (b << 8);
}

uint32 File::fileReadDwordBE() {
	uint b = fileReadWordBE();
	uint a = fileReadWordBE();
	return (b << 16) | a;
}
