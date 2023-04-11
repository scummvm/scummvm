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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/system/graphics.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

GraphicResource::GraphicResource(AsylumEngine *engine) : _vm(engine), _resourceId(kResourceNone) {
}

GraphicResource::GraphicResource(AsylumEngine *engine, ResourceId id) : _vm(engine), _resourceId(kResourceNone) {
	if (!load(id))
		error("[GraphicResource::GraphicResource] Error loading resource (0x%X)", id);
}

GraphicResource::~GraphicResource() {
	clear();
}

bool GraphicResource::load(ResourceId id) {
	if (id == kResourceNone)
		error("[GraphicResource::load] Trying to load an invalid resource!");

	// Clear previously loaded data
	clear();

	ResourceEntry *resEntry = getResource()->get(id);
	if (!resEntry)
		return false;

	_resourceId = id;
	init(resEntry->data, resEntry->size);

	return true;
}

void GraphicResource::clear() {
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].surface.free();
	}

	_frames.clear();
}

GraphicFrame *GraphicResource::getFrame(uint32 frame) {
	if (frame >= _frames.size())
		error("[GraphicResource::getFrame] Invalid frame index (was: %d, max:%d)", frame, _frames.size() - 1);

	return &_frames[frame];
}

void GraphicResource::init(byte *data, int32 size) {
	byte   *dataPtr      = data;

	// Read tag
	for (uint i = 0; i < sizeof(_data.tag); i++) {
		_data.tag[i] = *dataPtr;
		++dataPtr;
	}

	_data.flags  = READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	int32 contentOffset = (int32)READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	_data.field_C  = READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	_data.field_10  = READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	_data.field_14  = READ_LE_UINT32(dataPtr);
	dataPtr += 4;

	uint16 frameCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	_data.maxWidth = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	_frames.resize(frameCount);

	// Read frame offsets
	int32 prevOffset = (int32)READ_LE_UINT32(dataPtr) + contentOffset;
	dataPtr += 4;
	int32 nextOffset = 0;

	for (int32 i = 0; i < frameCount; i++) {
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
	for (uint32 i = 0; i < frameCount; i++) {
		dataPtr = data + _frames[i].offset;

		dataPtr += 4; // size
		dataPtr += 4; // flag

		_frames[i].x  = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;
		_frames[i].y  = (int16)READ_LE_UINT16(dataPtr);
		dataPtr += 2;

		uint16 height = READ_LE_UINT16(dataPtr);
		dataPtr += 2;
		uint16 width  = READ_LE_UINT16(dataPtr);
		dataPtr += 2;

		if (width > 0 && height > 0) {
			_frames[i].surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			_frames[i].surface.copyRectToSurface(dataPtr, width, 0, 0, width, height);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////

uint32 GraphicResource::getFrameCount(AsylumEngine *engine, ResourceId id) {
	GraphicResource *resource = new GraphicResource(engine, id);
	uint32 frameCount = resource->count();
	delete resource;

	return frameCount;
}

Common::Rect GraphicResource::getFrameRect(AsylumEngine *engine, ResourceId id, uint32 index) {
	GraphicResource *resource = new GraphicResource(engine, id);
	GraphicFrame *frame = resource->getFrame(index);

	Common::Rect rect = frame->getRect();

	delete resource;

	return rect;
}

} // end of namespace Asylum
