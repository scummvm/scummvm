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
#include "common/path.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace Hollywood {

class ChunkArchive;

bool readRunawayFile(const Common::Path &path, Common::Array<byte> &data);
// RESOURCE.000 has a ten-byte (demo) or twenty-byte (retail) prefix,
// followed by two 500-entry tables.
bool readRunawayGlobalChunk(uint chunk, Common::Array<byte> &data);
bool loadRunawaySurface(uint chunk, int width, int height, Graphics::ManagedSurface &surface);
bool loadRunawaySurface(const Common::Path &path, int width, int height, Graphics::ManagedSurface &surface);
bool loadRunawaySurface(ChunkArchive &archive, uint chunk, int width, int height, Graphics::ManagedSurface &surface);
bool loadRunawayMask(ChunkArchive &archive, int width, int height, Common::Array<byte> &mask, Common::Array<byte> &maps, uint chunk = 1);
byte findRunawayLight(Common::Point position, int width, const Common::Array<byte> &mask, const Common::Array<byte> &maps);
bool applyRunawayPatch(const Common::Array<byte> &patch, Graphics::ManagedSurface &surface, int yOffset = 0, int xOffset = 0);

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

// Brian's walking and head banks are indexed RGB565 with separate opacity and
// shadow planes. This differs from the room animation spans.
class RunawayActor {
public:
	enum Model {
		kCabin, kMuseumInterior, kMuseumHall,
		kChapters12, kChapters12Alternate, kCabinAlternate, kChapters456, kChapters456Alternate
	};

	struct RenderParams {
		Common::Point position, camera;
		int roomWidth = 0;
		float height = 455;
		// A depth layer hides the actor until his feet pass its Y threshold.
		int16 depthY[10] = {0, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767};
		byte light[3] = {100, 100, 100};
		bool antialias = true;
	};
	bool load(Model model = kCabin);
	bool loadChapter(byte chapter, bool alternate);
	bool setFrame(uint facing, uint frame, int headFrame = -1);
	bool loadAction(uint chunk, uint frames);
	bool setActionFrame(uint frame);
	static float perspectiveHeight(Common::Point position);
	void draw(Graphics::ManagedSurface &surface, Common::Point position, int cameraX,
		int roomTop, const Common::Array<byte> &mask, const Common::Array<byte> &maps,
		bool sunlit = false, bool antialias = true) const;
	void draw(Graphics::ManagedSurface &surface, const RenderParams &params,
		const Common::Array<byte> &mask, const Common::Array<byte> &maps) const;

private:
	enum { kHeadHeight = 148, kFacings = 8, kFrames = 19,
		kHeadFacings = 6, kHeadFrames = 40 };
	uint _width = 345, _height = 518;
	int _anchorX = 168, _anchorY = 434;
	bool _museum = false;
	bool decodeBodyFrame(const Common::Array<byte> &bank, const byte *descriptor, uint end);
	bool decodeFrame(const Common::Array<byte> &bank, const byte *descriptor, uint end, bool head);
	Common::Array<byte> _descriptors, _headDescriptors;
	Common::Array<byte> _actionDescriptors, _actionBank;
	Common::Array<byte> _banks[kFacings];
	Common::Array<byte> _headBanks[kHeadFacings];
	Common::Array<byte> _pixels, _opacity, _shadow;
	uint16 _palette[256];
	Common::Rect _bounds;
	int _loadedModel = -1;
	int _facing = -1;
	int _frame = -1;
	int _headFrame = -1;
};

} // End of namespace Hollywood

#endif
