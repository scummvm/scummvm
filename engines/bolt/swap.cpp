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

#include "bolt/bolt.h"

namespace Bolt {

void BoltEngine::swapPicHeader() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	WRITE_UINT16(data + 0x02, READ_BE_INT16(data + 0x02));
	WRITE_UINT16(data + 0x04, READ_BE_INT16(data + 0x04));
}

void BoltEngine::swapAndResolvePicDesc() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	for (int16 i = 0; i < 4; i++) {
		resolveIt((uint32 *)(data + 8 + i * 4));
		WRITE_UINT16(data + i * 2, READ_BE_INT16(data + i * 2));
	}
}

void BoltEngine::swapFirstWord() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	WRITE_UINT16(data, READ_BE_INT16(data));
}

void BoltEngine::swapFirstTwoWords() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	WRITE_UINT16(data, READ_BE_INT16(data));
	WRITE_UINT16(data + 2, READ_BE_INT16(data + 2));
}

void BoltEngine::swapFirstFourWords() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	WRITE_UINT16(data + 0x00, READ_BE_INT16(data + 0x00));
	WRITE_UINT16(data + 0x02, READ_BE_INT16(data + 0x02));
	WRITE_UINT16(data + 0x04, READ_BE_INT16(data + 0x04));
	WRITE_UINT16(data + 0x06, READ_BE_INT16(data + 0x06));
}

void BoltEngine::swapSpriteHeader() {
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	if (!data)
		return;

	WRITE_UINT16(data + 0x00, READ_BE_INT16(data + 0x00));
	WRITE_UINT16(data + 0x06, READ_BE_INT16(data + 0x06));
	WRITE_UINT16(data + 0x08, READ_BE_INT16(data + 0x08));
	WRITE_UINT16(data + 0x0A, READ_BE_INT16(data + 0x0A));
	WRITE_UINT16(data + 0x0C, READ_BE_INT16(data + 0x0C));
	WRITE_UINT16(data + 0x16, READ_BE_INT16(data + 0x16));

	if (!(data[0] & 0x10)) {
		uint32 idx = g_resolvedPtrs.size();
		g_resolvedPtrs.push_back(data + 0x18);
		WRITE_UINT32(data + 0x12, idx | 0x80000000);
	}
}

void BoltEngine::freeSpriteCleanUp() {
	// No-op
}

} // End of namespace Bolt
