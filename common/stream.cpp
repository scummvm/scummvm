/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "common/stream.h"

namespace Common {

/* TODO:
 * - The ReadStream / WriteStream should provide some error handling
 */

byte ReadStream::readByte() {
	byte b = 0;
	read(&b, 1);
	return b;
}

uint16 ReadStream::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 ReadStream::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 ReadStream::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 ReadStream::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}


void WriteStream::writeByte(byte value) {
	write(&value, 1);
}

void WriteStream::writeUint16LE(uint16 value) {
	writeByte((byte)(value & 0xff));
	writeByte((byte)(value >> 8));
}

void WriteStream::writeUint32LE(uint32 value) {
	writeUint16LE((uint16)(value & 0xffff));
	writeUint16LE((uint16)(value >> 16));
}

void WriteStream::writeUint16BE(uint16 value) {
	writeByte((byte)(value >> 8));
	writeByte((byte)(value & 0xff));
}

void WriteStream::writeUint32BE(uint32 value) {
	writeUint16BE((uint16)(value >> 16));
	writeUint16BE((uint16)(value & 0xffff));
}

}	// End of namespace Common
