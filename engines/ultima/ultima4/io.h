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

#ifndef ULTIMA4_IO_H
#define ULTIMA4_IO_H

#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {

/*
 * These are endian-independant routines for reading and writing
 * 4-byte (int), 2-byte (short), and 1-byte (char) values to and from
 * the ultima 4 data files.  If sizeof(int) != 4, all bets are off.
 */

int writeInt(unsigned int i, Common::WriteStream *f);
int writeShort(unsigned short s, Common::WriteStream *f);
int writeChar(unsigned char c, Common::WriteStream *f);
int readInt(unsigned int *i, Common::ReadStream *f);
int readShort(unsigned short *s, Common::ReadStream *f);
int readChar(unsigned char *c, Common::ReadStream *f);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
