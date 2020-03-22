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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/filesys/io.h"

namespace Ultima {
namespace Ultima4 {

int writeInt(unsigned int i, Common::WriteStream *f) {
	f->writeUint32LE(i);
	return 1;
}

int writeShort(unsigned short s, Common::WriteStream *f) {
	f->writeUint16LE(s);
	return 1;
}

int writeChar(unsigned char c, Common::WriteStream *f) {
	f->writeByte(c);
	return 1;
	return 1;
}

int readInt(unsigned int *i, Common::ReadStream *f) {
	*i = f->readUint32LE();
	return 1;
}

int readShort(unsigned short *s, Common::ReadStream *f) {
	*s = f->readUint16LE();
	return 1;
}

int readChar(unsigned char *c, Common::ReadStream *f) {
	*c = f->readByte();
	return 1;
}

} // End of namespace Ultima4
} // End of namespace Ultima
