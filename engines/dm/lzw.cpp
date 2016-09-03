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

#include "common/memstream.h"

#include "dm/lzw.h"

namespace DM {

LZWdecompressor::LZWdecompressor() {
	_repetitionEnabled = false;
	_codeBitCount = 0;
	_currentMaximumCode = 0;
	_absoluteMaximumCode = 4096;
	for (int i = 0; i < 12; ++i)
		_inputBuffer[i] = 0;
	_dictNextAvailableCode = 0;
	_dictFlushed = false;

	byte leastSignificantBitmasks[9] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
	for (uint16 i = 0; i < 9; ++i)
		_leastSignificantBitmasks[i] = leastSignificantBitmasks[i];
	_inputBufferBitIndex = 0;
	_inputBufferBitCount = 0;
	_charToRepeat = 0;

	_tempBuffer = new byte[5004];
	_prefixCode = new int16[5003];
	_appendCharacter = new byte[5226];
}

LZWdecompressor::~LZWdecompressor() {
	delete[] _appendCharacter;
	delete[] _prefixCode;
	delete[] _tempBuffer;
}

int16 LZWdecompressor::getNextInputCode(Common::MemoryReadStream &inputStream, int32 *inputByteCount) {
	byte *inputBuffer = _inputBuffer;
	if (_dictFlushed || (_inputBufferBitIndex >= _inputBufferBitCount) || (_dictNextAvailableCode > _currentMaximumCode)) {
		if (_dictNextAvailableCode > _currentMaximumCode) {
			_codeBitCount++;
			if (_codeBitCount == 12) {
				_currentMaximumCode = _absoluteMaximumCode;
			} else {
				_currentMaximumCode = (1 << _codeBitCount) - 1;
			}
		}
		if (_dictFlushed) {
			_currentMaximumCode = (1 << (_codeBitCount = 9)) - 1;
			_dictFlushed = false;
		}
		if (*inputByteCount > _codeBitCount) {
			_inputBufferBitCount = _codeBitCount;
		} else {
			_inputBufferBitCount = *inputByteCount;
		}
		if (_inputBufferBitCount > 0) {
			inputStream.read(_inputBuffer, _inputBufferBitCount);
			*inputByteCount -= _inputBufferBitCount;
		} else {
			return -1;
		}
		_inputBufferBitIndex = 0;
		_inputBufferBitCount = (_inputBufferBitCount << 3) - (_codeBitCount - 1);
	}
	int16 bitIndex = _inputBufferBitIndex;
	int16 requiredInputBitCount = _codeBitCount;
	inputBuffer += bitIndex >> 3; /* Address of byte in input buffer containing current bit */
	bitIndex &= 0x0007; /* Bit index of the current bit in the byte */
	int16 nextInputCode = *inputBuffer++ >> bitIndex; /* Get the first bits of the next input code from the input buffer byte */
	requiredInputBitCount -= 8 - bitIndex; /* Remaining number of bits to get for a complete input code */
	bitIndex = 8 - bitIndex;
	if (requiredInputBitCount >= 8) {
		nextInputCode |= *inputBuffer++ << bitIndex;
		bitIndex += 8;
		requiredInputBitCount -= 8;
	}
	nextInputCode |= (*inputBuffer & _leastSignificantBitmasks[requiredInputBitCount]) << bitIndex;
	_inputBufferBitIndex += _codeBitCount;
	return nextInputCode;
}

void LZWdecompressor::outputCharacter(byte character, byte **out) {
	byte *output = *out;

	if (false == _repetitionEnabled) {
		if (character == 0x90)
			_repetitionEnabled = true;
		else
			*output++ = _charToRepeat = character;
	} else {
		if (character) { /* If character following 0x90 is not 0x00 then it is the repeat count */
			while (--character)
				*output++ = _charToRepeat;
		} else /* else output a 0x90 character */
			*output++ = 0x90;

		_repetitionEnabled = false;
	}

	*out = output;
	return;
}

int32 LZWdecompressor::decompress(Common::MemoryReadStream &inStream, int32 inputByteCount, byte *out) {
	byte *reversedDecodedStringStart;
	byte *reversedDecodedStringEnd = reversedDecodedStringStart = _tempBuffer;
	byte *originalOut = out;
	_repetitionEnabled = false;
	_codeBitCount = 9;
	_dictFlushed = false;
	_currentMaximumCode = (1 << (_codeBitCount = 9)) - 1;
	for (int16 code = 255; code >= 0; code--) {
		_prefixCode[code] = 0;
		_appendCharacter[code] = code;
	}
	_dictNextAvailableCode = 257;
	int16 oldCode;
	int16 character = oldCode = getNextInputCode(inStream, &inputByteCount);
	if (oldCode == -1) {
		return -1L;
	}
	outputCharacter(character, &out);
	int16 code;
	while ((code = getNextInputCode(inStream, &inputByteCount)) > -1) {
		if (code == 256) { /* This code is used to flush the dictionary */
			for (int i = 0; i < 256; ++i)
				_prefixCode[i] = 0;
			_dictFlushed = true;
			_dictNextAvailableCode = 256;
			if ((code = getNextInputCode(inStream, &inputByteCount)) == -1) {
				break;
			}
		}
		/* This code checks for the special STRING+CHARACTER+STRING+CHARACTER+STRING case which generates an undefined code.
		It handles it by decoding the last code, adding a single character to the end of the decoded string */
		int16 newCode = code;
		if (code >= _dictNextAvailableCode) { /* If code is not defined yet */
			*reversedDecodedStringEnd++ = character;
			code = oldCode;
		}
		/* Use the string table to decode the string corresponding to the code and store the string in the temporary buffer */
		while (code >= 256) {
			*reversedDecodedStringEnd++ = _appendCharacter[code];
			code = _prefixCode[code];
		}
		*reversedDecodedStringEnd++ = (character = _appendCharacter[code]);
		/* Output the decoded string in reverse order */
		do {
			outputCharacter(*(--reversedDecodedStringEnd), &out);
		} while (reversedDecodedStringEnd > reversedDecodedStringStart);
		/* If possible, add a new code to the string table */
		if ((code = _dictNextAvailableCode) < _absoluteMaximumCode) {
			_prefixCode[code] = oldCode;
			_appendCharacter[code] = character;
			_dictNextAvailableCode = code + 1;
		}
		oldCode = newCode;
	}
	return out - originalOut; /* Byte count of decompressed data */
}
}
