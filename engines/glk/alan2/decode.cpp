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

#include "glk/alan2/decode.h"

namespace Glk {
namespace Alan2 {

int Decode::inputBit() {
	int bit;

	if (!_bitsToGo) {		// More bits available ?
		_decodeBuffer = _txtFile->readByte(); // No, so get more
		if (_txtFile->eos()) {
			_garbageBits++;

			if (_garbageBits > VALUEBITS - 2)
				error("Error in encoded data file.");
		} else
			_bitsToGo = 8;		// Another Char, 8 new bits
	}

	bit = _decodeBuffer & 1;			// Get next bit
	_decodeBuffer = _decodeBuffer >> 1; // and remove it
	_bitsToGo--;

	return bit;
}

void Decode::startDecoding() {
	_bitsToGo = 0;
	_garbageBits = 0;

	_value = 0;
	for (int i = 0; i < VALUEBITS; i++)
		_value = 2 * _value + inputBit();

	_low = 0;
	_high = TOPVALUE;
}

int Decode::decodeChar() {
	const long range = (long)(_high - _low) + 1;
	const int f = (((long)(_value - _low) + 1) * _freq[0] - 1) / range;
	int symbol;

	// Find the symbol
	for (symbol = 1; _freq[symbol] > f; symbol++);

	_high = _low + range * _freq[symbol - 1] / _freq[0] - 1;
	_low = _low + range * _freq[symbol] / _freq[0];

	for (;;) {
		if (_high < HALF) {
			// Do nothing
		} else if (_low >= HALF) {
			_value = _value - HALF;
			_low = _low - HALF;
			_high = _high - HALF;
		} else if (_low >= ONEQUARTER && _high < THREEQUARTER) {
			_value = _value - ONEQUARTER;
			_low = _low - ONEQUARTER;
			_high = _high - ONEQUARTER;
		} else
			break;

		// Scale up the range
		_low = 2 * _low;
		_high = 2 * _high + 1;
		_value = 2 * _value + inputBit();
	}

	return symbol - 1;
}

// Save so much about the decoding process, so it is possible to restore
// and continue later.
DecodeInfo* Decode::pushDecode() {
	DecodeInfo *info = new DecodeInfo();

	info->fpos = _txtFile->pos();
	info->buffer = _decodeBuffer;
	info->bits = _bitsToGo;
	info->value = _value;
	info->high = _high;
	info->low = _low;

	return info;
}

// Restore enough info about the decoding process, so it is possible to
// continue after having decoded something else
void Decode::popDecode (DecodeInfo *info){
	_txtFile->seek(info->fpos, SEEK_CUR);
	_decodeBuffer = info->buffer;
	_bitsToGo = info->bits;
	_value = info->value;
	_high = info->high;
	_low = info->low;

	delete info;
}

} // End of namespace Alan2
} // End of namespace Glk
