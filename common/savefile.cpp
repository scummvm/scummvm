/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#include "common/savefile.h"
#include "common/util.h"

uint32 SaveFile::read(void *ptr, uint32 size)
{
	return fread(ptr, 1, size);
}

byte SaveFile::readByte()
{
	byte b;
	if (fread(&b, 1, 1) != 1)
		return 0;
	return b;
}

uint16 SaveFile::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 SaveFile::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 SaveFile::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 SaveFile::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}

uint32 SaveFile::write(const void *ptr, uint32 size)
{
	return fwrite(ptr, 1, size);
}

void SaveFile::writeByte(byte value)
{
	fwrite(&value, 1, 1);
}

void SaveFile::writeUint16LE(uint16 value) {
	writeByte((byte)(value & 0xff));
	writeByte((byte)(value >> 8));
}

void SaveFile::writeUint32LE(uint32 value) {
	writeUint16LE((uint16)(value & 0xffff));
	writeUint16LE((uint16)(value >> 16));
}

void SaveFile::writeUint16BE(uint16 value) {
	writeByte((byte)(value >> 8));
	writeByte((byte)(value & 0xff));
}

void SaveFile::writeUint32BE(uint32 value) {
	writeUint16BE((uint16)(value >> 16));
	writeUint16BE((uint16)(value & 0xffff));
}

SaveFile *SaveFileManager::open_savefile(const char *filename, const char *directory, bool saveOrLoad)
{
	char buf[256];
	join_paths(filename, directory, buf, sizeof(buf));
	StdioSaveFile *sf = new StdioSaveFile(buf,
					      (saveOrLoad? "wb":"rb"));
	if (!sf->is_open()) {
		delete sf;
		sf = NULL;
	}
	return sf;
}

void SaveFileManager::join_paths(const char *filename, const char *directory,
				 char *buf, int bufsize)
{
	buf[bufsize-1] = '\0';
	strncpy(buf, directory, bufsize-1);

#ifdef WIN32
	// Fix for Win98 issue related with game directory pointing to root drive ex. "c:\"
	if ((buf[0] != 0) && (buf[1] == ':') && (buf[2] == '\\') && (buf[3] == 0)) {
		buf[2] = 0;
	}
#endif

	const int dirLen = strlen(buf);

	if (dirLen > 0) {
#ifdef __MORPHOS__
		if (buf[dirLen-1] != ':' && buf[dirLen-1] != '/')
#endif

#if !defined(__GP32__) && !defined(__PALM_OS__)
		strncat(buf, "/", bufsize-1);	// prevent double /
#endif
	}
	strncat(buf, filename, bufsize-1);
}
