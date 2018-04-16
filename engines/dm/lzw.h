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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_LZW_H
#define DM_LZW_H

#include "common/file.h"

#include "dm/dm.h"

namespace Common {
	class MemoryReadStream;
}

namespace DM {
class LZWdecompressor {
	bool _repetitionEnabled;
	int16 _codeBitCount;
	int16 _currentMaximumCode;
	int16 _absoluteMaximumCode;
	byte _inputBuffer[12];
	int16 _dictNextAvailableCode;
	bool _dictFlushed;

	byte _leastSignificantBitmasks[9];
	int16 _inputBufferBitIndex;
	int16 _inputBufferBitCount;
	int16 _charToRepeat;

	byte *_tempBuffer;
	int16 *_prefixCode;
	byte *_appendCharacter;

	int16 getNextInputCode(Common::MemoryReadStream &stream, int32 *inputByteCount);
	void outputCharacter(byte character, byte **out);
	void operator=(const LZWdecompressor&); // deleted
public:
	LZWdecompressor();
	~LZWdecompressor();
	int32 decompress(Common::MemoryReadStream &inputStream, int32 inputByteCount, byte *out);
};

}

#endif
