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

#ifndef HOLLYWOOD_RUNAWAY_GRAPHICS_H
#define HOLLYWOOD_RUNAWAY_GRAPHICS_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace Hollywood {

class ChunkArchive;

// RESOURCE.000 has a ten-byte header followed by two 500-entry tables.
bool readRunawayGlobalChunk(uint chunk, Common::Array<byte> &data);
bool loadRunawaySurface(uint chunk, int width, int height, Graphics::ManagedSurface &surface);
bool loadRunawaySurface(ChunkArchive &archive, uint chunk, int width, int height, Graphics::ManagedSurface &surface);
bool applyRunawayPatch(const Common::Array<byte> &patch, Graphics::ManagedSurface &surface, int yOffset = 0);

enum {
	kRunawayScreenWidth = 1024,
	kRunawayScreenHeight = 768,
	kRunawayViewportHeight = 600,
	kRunawayViewportTop = (kRunawayScreenHeight - kRunawayViewportHeight) / 2
};

// Runaway's room animations retain Hollywood's 14-byte descriptors, but use
// RGB565 spans. Coordinates are absolute room coordinates, including shadows.
class RunawayAnimation {
public:
	bool load(ChunkArchive &archive, uint chunk, uint frames);
	bool load(const Common::Array<byte> &data, uint frames);
	void draw(Graphics::ManagedSurface &surface, uint frame, int cameraX, int roomTop) const;
	uint frameCount() const { return _frames; }

private:
	bool validate(uint frames);
	Common::Array<byte> _data;
	uint _frames = 0;
};

// Brian's walking banks are indexed RGB565 with separate opacity and shadow
// planes. This is a different representation from the room animation spans.
class RunawayActor {
public:
	bool load();
	bool setFrame(uint facing, uint frame);
	void draw(Graphics::ManagedSurface &surface, Common::Point position, int cameraX,
		int roomTop, const Common::Array<byte> &mask, const Common::Array<byte> &maps, bool sunlit = false) const;

private:
	enum { kWidth = 345, kHeight = 518, kFacings = 8, kFrames = 19 };
	Common::Array<byte> _descriptors;
	Common::Array<byte> _banks[kFacings];
	Common::Array<byte> _pixels, _opacity, _shadow;
	uint16 _palette[256];
	Common::Rect _bounds;
	int _facing = -1;
	int _frame = -1;
};

} // End of namespace Hollywood

#endif
