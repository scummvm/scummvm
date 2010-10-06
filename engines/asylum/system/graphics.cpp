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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/graphics.h"

#include "common/endian.h"

namespace Asylum {

GraphicResource::GraphicResource(ResourcePack *resPack, uint32 entry) {
	ResourceEntry *resEntry = resPack->getResource(entry);
	_entryNum = entry;
	init(resEntry->data, resEntry->size);
}

GraphicResource::~GraphicResource() {
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].surface.free();
	}

	_frames.clear();
}

void GraphicResource::init(byte *data, int32 size) {
	byte   *dataPtr      = data;
	int32 contentOffset = 0;
	uint16 frameCount    = 0;

	int32 i = 0;

	dataPtr += 4; // tag value

	_flags = (int32)READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	contentOffset = (int32)READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	dataPtr += 4; // unknown
	dataPtr += 4; // unknown
	dataPtr += 4; // unknown

	frameCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	dataPtr += 2; // max width

	_frames.resize(frameCount);

	// Read frame offsets
	int32 prevOffset = (int32)READ_LE_UINT32(dataPtr) + contentOffset;
	dataPtr += 4;
	int32 nextOffset = 0;

	for (i = 0; i < frameCount; i++) {
		GraphicFrame frame;
		frame.offset = prevOffset;

		// Read the offset of the next entry to determine the size of this one
		nextOffset = (i < frameCount - 1) ? (int32)READ_LE_UINT32(dataPtr) + contentOffset : size;
		dataPtr += 4; // offset
		frame.size = (nextOffset > 0) ? nextOffset - prevOffset : size - prevOffset;

		_frames[i] = frame;

		prevOffset = nextOffset;
	}

	// Reset pointer
	dataPtr = data;

	// Read frame data
	for (i = 0; i < frameCount; i++) {
		dataPtr = data + _frames[i].offset;

		dataPtr += 4; // size
		dataPtr += 4; // flag

		_frames[i].x  = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;
		_frames[i].y  = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;

		int16 height = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;
		int16 width  = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;

		_frames[i].surface.create(width, height, 1);

		memcpy(_frames[i].surface.pixels, dataPtr, width * height);
	}
}

} // end of namespace Asylum
