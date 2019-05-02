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

#ifndef GLK_ALAN2_DECODE
#define GLK_ALAN2_DECODE

#include "glk/alan2/alan2.h"
#include "glk/alan2/acode.h"
#include "common/file.h"

namespace Glk {
namespace Alan2 {

// Structure for saved decode info
struct DecodeInfo {
	long fpos;
	int buffer;
	int bits;
	CodeValue value;
	CodeValue high;
	CodeValue low;
};

class Decode {
public:
	Decode(Common::File *txtFile, Aword *freq): _txtFile(txtFile), _freq(freq) {}
	void startDecoding();
	int decodeChar();
	
	/**
	 * Save so much about the decoding process, so it is possible to restore
	 * and continue later.
	 */
	DecodeInfo *pushDecode();
	
	/**
	 * Restore enough info about the decoding process, so it is possible to
	 * continue after having decoded something else.
	 */
	void popDecode(DecodeInfo *info);
	
	int inputBit();

private:
	// Bit output
	int _decodeBuffer;	// Bits to be input
	int _bitsToGo;		// Bits still in buffer
	int _garbageBits;	// Bits past EOF

	Aword *_freq;
	Common::File *_txtFile;

	// Current state of decoding
	CodeValue _value;			// Currently seen code value
	CodeValue _low, _high;		// Current code region
};

} // End of namespace Alan2
} // End of namespace Glk

#endif
