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

#ifndef ASYLUM_SYSTEM_GRAPHICS_H
#define ASYLUM_SYSTEM_GRAPHICS_H

#include "common/rect.h"
#include "common/array.h"

#include "graphics/surface.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

struct GraphicFrame {
	int32 size;
	int32 offset;

	int16 x;
	int16 y;
	Graphics::Surface surface;

	uint16 getWidth() { return surface.w; }
	uint16 getHeight() { return surface.h; }

	GraphicFrame() {
		size = offset = 0;
		x = y = 0;
	}

	Common::Rect getRect() {
		return Common::Rect(x, y, x + getWidth(), y + getHeight());
	}
};

// Graphic resources can be sprites or images, with multiple frames
class GraphicResource {
public:
	struct ResourceData {
		char tag[4];
		uint32 flags;
		// contentOffset
		uint32 field_C;
		uint32 field_10;
		uint32 field_14;
		// frameCount
		uint16 maxWidth;
		// Offsets

		ResourceData() {
			memset(&tag, 0, sizeof(tag));
			flags = 0;
			field_C = 0;
			field_10 = 0;
			field_14 = 0;
			maxWidth = 0;
		}
	};

	GraphicResource(AsylumEngine *engine);
	GraphicResource(AsylumEngine *engine, ResourceId id);
	~GraphicResource();

	bool load(ResourceId id);

	/**
	 * Copies an animation frame to the target buffer
	 */
	void copyFrameToDest(byte *dest, uint32 frame);

	/**
	 * Copies a sprite to the target buffer, with transparency
	 */
	void copySpriteToDest(byte *dest, uint32 frame);

	GraphicFrame *getFrame(uint32 frame);
	ResourceId    getResourceId() { return _resourceId; }
	uint32        count()         { return _frames.size(); }

	// FIXME: flags are coordinates for the sound origin!
	ResourceData  getData()       { return _data; }

	// Helper functions
	static uint32 getFrameCount(AsylumEngine *engine, ResourceId id);
	static Common::Rect getFrameRect(AsylumEngine *engine, ResourceId id, uint32 index);

private:
	AsylumEngine *_vm;

	ResourceId _resourceId;
	ResourceData _data;
	Common::Array<GraphicFrame> _frames;

	void init(byte *data, int32 size);
	void clear();
};

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_GRAPHICS_H
