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

#include "comfy/comfy.h"

#include "common/endian.h"

namespace Comfy {

bool ComfyEngine::keyBitAllocate(uint16 keyBitCount) {
	keyBitFree();

	_keyBitsSize = uint16(keyBitCount + 1) / 8 + 1;
	_keyBits = new byte[_keyBitsSize]();
	return true;
}

void ComfyEngine::keyBitFree() {
	delete[] _keyBits;
	_keyBits = nullptr;
	_keyBitsSize = 0;
}

bool ComfyEngine::keyBitTest(uint16 bitIndex) {
	uint32 byteIndex = bitIndex / 8;
	if (!_keyBits || byteIndex >= _keyBitsSize)
		return false;

	return (_keyBits[byteIndex] & (1 << (bitIndex & 7))) != 0;
}

void ComfyEngine::keyBitCopyRange(uint16 destination, uint16 count, uint32 source) {
	uint32 destinationByte = destination / 8;
	uint32 byteCount = count / 8;
	if (!_keyBits || byteCount > sizeof(source) || destinationByte + byteCount > _keyBitsSize)
		return;

	byte sourceBytes[sizeof(source)];
	WRITE_LE_UINT32(sourceBytes, source);
	memcpy(_keyBits + destinationByte, sourceBytes, byteCount);
}

void ComfyEngine::keyBitSet(uint16 bitIndex) {
	uint32 byteIndex = bitIndex / 8;
	if (!_keyBits || byteIndex >= _keyBitsSize)
		return;

	_keyBits[byteIndex] |= 1 << (bitIndex & 7);
}

void ComfyEngine::keyBitClear(uint16 bitIndex) {
	uint32 byteIndex = bitIndex / 8;
	if (!_keyBits || byteIndex >= _keyBitsSize)
		return;

	_keyBits[byteIndex] &= ~(1 << (bitIndex & 7));
}


} // End of namespace Comfy
