 /* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef IMMORTAL_COMPRESSION_H
#define IMMORTAL_COMPRESSION_H

#include "common/file.h"
#include "common/memstream.h"

namespace Compression {

// Only unCompress() is called from outside Compression, the others are subroutines.
Common::SeekableReadStream *unCompress(Common::File *src, int srcLen);
void setupDictionary(uint16 start[], uint16 ptk[], uint16 &findEmpty);
int getInputCode(bool &carry, Common::File *src, int &srcLen, uint16 &evenOdd);
uint16 getMember(uint16 codeW, uint16 k, uint16 &findEmpty, uint16 start[], uint16 ptk[]);
void appendList(uint16 codeW, uint16 k, uint16 &hash, uint16 &findEmpty, uint16 start[], uint16 ptk[], uint16 &tmp);

} // namespace compression

#endif