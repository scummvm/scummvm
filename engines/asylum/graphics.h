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

#ifndef ASYLUM_GRAPHICS_H_
#define ASYLUM_GRAPHICS_H_

#include "common/array.h"
#include "graphics/surface.h"
#include "asylum/respack.h"

namespace Asylum {

struct GraphicFrame {
	uint32 size;
	uint32 offset;

	uint16 x;
	uint16 y;
	Graphics::Surface surface;
};

// Graphic resources can be sprites or images, with multiple frames
class GraphicResource {
public:
	GraphicResource(ResourcePack *resPack, int entry);
	~GraphicResource();

	uint32 getFrameCount() {
		return _frames.size();
	}
	GraphicFrame *getFrame(int frame) {
		return &_frames[frame];
	}
	uint32 getFlags() {
		return _flags;
	}

	/**
	 * Copies an animation frame to the target buffer
	 */
	void copyFrameToDest(byte *dest, int frame);

	/**
	 * Copies a sprite to the target buffer, with transparency
	 */
	void copySpriteToDest(byte *dest, int frame);

	uint32 getEntryNum() {
		return _entryNum;
	}

private:
	Common::Array <GraphicFrame> _frames;
	uint32 _flags;
	uint32 _entryNum;

	void init(byte *data, uint32 size);
};

} // end of namespace Asylum

#endif
