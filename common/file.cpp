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

FILE *File::fopenNoCase(const char *filename, const char * directory, const char *mode) {
	FILE *file;
	char buf[256];
	char *ptr;

	strcpy(buf, directory);
	if (directory[0] != 0) {
#ifdef __MORPHOS__
		if (buf[strlen(buf)-1] != ':' && buf[strlen(buf)-1] != '/')
#endif
		strcat(buf, "/");
	}
	strcat(buf, filename);

	file = fopen(buf, mode);
	if (file)
		return file;

	const char *dirs[] = {
		"",
		"video/",
		"VIDEO/",
		"data/",
		"DATA/",
		"resource/",
		"RESOURCE/"
	};

	for (uint8 l = 0; l < 7; l++) {
		strcpy(buf, directory);
		if (directory[0] != 0) {
#ifdef __MORPHOS__
			if (buf[strlen(buf)-1] != ':' && buf[strlen(buf)-1] != '/')
#endif
			strcat(buf, "/");
		}
		strcat(buf, dirs[l]);
		int8 len = strlen(buf);
		strcat(buf, filename);

		ptr = buf + len;
		do
			*ptr++ = toupper(*ptr);
		while (*ptr);
		file = fopen(buf, mode);
		if (file)
			return file;

		ptr = buf + len;
		do
			*ptr++ = tolower(*ptr);
		while (*ptr);
		file = fopen(buf, mode);
		if (file)
			return file;
	}

	return NULL;
}

File::File() {
	_handle = NULL;
	_ioFailed = false;
	_encbyte = 0;
}

File::~File() {
	close();
}

bool File::open(const char *filename, const char *directory, int mode, byte encbyte) {

	if (_handle) {
		debug(2, "File %s already opened", filename);
		return false;
	}

	clearIOFailed();

	if (mode == kFileReadMode) {
		_handle = fopenNoCase(filename, directory, "rb");
		if (_handle == NULL) {
			debug(2, "File %s not found", filename);
			return false;
		}
	}
	else if (mode == kFileWriteMode) {
		_handle = fopenNoCase(filename, directory, "wb");
		if (_handle == NULL) {
			debug(2, "File %s not opened", filename);
			return false;
		}
	}	else {
		warning("Only read/write mode supported!");
		return false;
	}

	_encbyte = encbyte;

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

uint32 File::size() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	uint32 oldPos = ftell(_handle);
	fseek(_handle, 0, SEEK_END);
	uint32 length = ftell(_handle);
	fseek(_handle, oldPos, SEEK_SET);
	
	return length;
}

void File::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File is not open!");
		return;
	}

	if (fseek(_handle, offs, whence) != 0)
		clearerr(_handle);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if ((uint32)fread(ptr2, 1, len, _handle) != len) {
		clearerr(_handle);
		_ioFailed = true;
	}

	if (_encbyte != 0) {
		uint32 t_size = len;
		do {
			*ptr2++ ^= _encbyte;
		} while (--t_size);
	}

	return len;
}

byte File::readByte() {
	byte b;

	if (fread(&b, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
	return b ^ _encbyte;
}

uint16 File::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 File::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 File::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 File::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}

uint32 File::write(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if (_encbyte != 0) {
		uint32 t_size = len;
		do {
			*ptr2++ ^= _encbyte;
		} while (--t_size);
	}

	if ((uint32)fwrite(ptr2, 1, len, _handle) != len) {
		clearerr(_handle);
		_ioFailed = true;
	}

	return len;
}

void File::writeByte(byte value) {
	value ^= _encbyte;

	if (fwrite(&value, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
}

void File::writeUint16LE(uint16 value) {
	writeByte((byte)(value & 0xff));
	writeByte((byte)(value >> 8));
}

void File::writeUint32LE(uint32 value) {
	writeUint16LE((uint16)(value & 0xffff));
	writeUint16LE((uint16)(value >> 16));
}

void File::writeUint16BE(uint16 value) {
	writeByte((byte)(value >> 8));
	writeByte((byte)(value & 0xff));
}

void File::writeUint32BE(uint32 value) {
	writeUint16BE((uint16)(value >> 16));
	writeUint16BE((uint16)(value & 0xffff));
}
