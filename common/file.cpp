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
#include "engine.h"	// For debug/warning/error

File::File() {
	_handle = NULL;
	_ioFailed = false;
	_encbyte = 0;
}

File::~File() {
	close();
}

bool File::open(const char *filename, int mode, byte encbyte) {
	char buf[256], *ptr;
	if (_handle) {
		debug(2, "File %s already opened", filename);
		return false;
	}

	clearIOFailed();

	int32 i = 0, pos = 0;

	strcpy(buf, filename);
	while (buf[i] != 0) {
		if ((buf[i] == '/') || (buf[i] == '\\')) {
			pos = i + 1;
		}
		i++;
	}
	
	if (mode == 1) {
		_handle = fopen(buf, "rb");
		if (_handle == NULL) {
			ptr = buf + pos;
			do
				*ptr++ = toupper(*ptr);
			while (*ptr);
			_handle = fopen(buf, "rb");
		}
		if (_handle == NULL) {
			ptr = buf + pos;
			do
				*ptr++ = tolower(*ptr);
			while (*ptr);
			_handle = fopen(buf, "rb");
		}
		if (_handle == NULL) {
			debug(2, "File %s not found", filename);
			return false;
		}
	}
	else if (mode == 2) {
		_handle = fopen(buf, "wb");
		if (_handle == NULL) {
			ptr = buf + pos;
			do
				*ptr++ = toupper(*ptr);
			while (*ptr);
			_handle = fopen(buf, "wb");
		}
		if (_handle == NULL) {
			ptr = buf + pos;
			do
				*ptr++ = tolower(*ptr);
			while (*ptr);
			_handle = fopen(buf, "wb");
		}
		if (_handle == NULL) {
			debug(2, "File %s not opened", filename);
			return false;
		}
	}	else {
		warning("Only read/write mode supported!");
		return false;
	}

	return true;
}

void File::close() {
	if (_handle)
		fclose(_handle);
	_handle = NULL;
}

bool File::isOpen() {
	return _handle != NULL;
}

bool File::ioFailed() {
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	_ioFailed = false;
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

uint32 File::read(void *ptr, uint32 size) {
	byte *ptr2 = (byte *)ptr;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (size == 0)
		return 0;

	if ((uint32)fread(ptr2, 1, size, _handle) != size) {
		clearerr(_handle);
		_ioFailed = true;
	}

	if (_encbyte != 0) {
		uint32 t_size = size;
		do {
			*ptr2++ ^= _encbyte;
		} while (--t_size);
	}

	return size;
}

byte File::readByte() {
	byte b;

	if (fread(&b, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
	return b ^ _encbyte;
}

uint16 File::readWordLE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 File::readDwordLE() {
	uint32 a = readWordLE();
	uint32 b = readWordLE();
	return (b << 16) | a;
}

uint16 File::readWordBE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 File::readDwordBE() {
	uint32 b = readWordBE();
	uint32 a = readWordBE();
	return (b << 16) | a;
}

uint32 File::write(void *ptr, uint32 size) {
	byte *ptr2 = (byte *)ptr;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (size == 0)
		return 0;

	if (_encbyte != 0) {
		uint32 t_size = size;
		do {
			*ptr2++ ^= _encbyte;
		} while (--t_size);
	}

	if ((uint32)fwrite(ptr2, 1, size, _handle) != size) {
		clearerr(_handle);
		_ioFailed = true;
	}

	return size;
}

void File::writeByte(byte value) {
	value ^= _encbyte;

	if (fwrite(&value, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
}

void File::writeWordLE(uint16 value) {
	writeByte((byte)(value & 0xff));
	writeByte((byte)(value >> 8));
}

void File::writeDwordLE(uint32 value) {
	writeWordLE((uint16)(value & 0xffff));
	writeWordLE((uint16)(value >> 16));
}

void File::writeWordBE(uint16 value) {
	writeByte((byte)(value >> 8));
	writeByte((byte)(value & 0xff));
}

void File::writeDwordBE(uint32 value) {
	writeWordBE((uint16)(value >> 16));
	writeWordBE((uint16)(value & 0xffff));
}
