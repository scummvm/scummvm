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

#ifndef HARVESTER_STARTUP_ART_H
#define HARVESTER_STARTUP_ART_H

#include "common/array.h"

namespace Harvester {

class ResourceManager;

struct IndexedBitmap {
	uint32 width = 0;
	uint32 height = 0;
	Common::Array<byte> pixels;

	bool isValid() const {
		return width != 0 && height != 0 && pixels.size() >= width * height;
	}
};

struct AbmFrame : IndexedBitmap {
	int32 xOffset = 0;
	int32 yOffset = 0;
};

class StartupArt {
public:
	bool load(ResourceManager &resources);
	bool loadQuickTipsResources(ResourceManager &resources);
	void drawWaitFrame() const;
	void drawQuickTipsScreen() const;

	const byte *getWaitPalette() const { return _waitPalette; }
	const Common::Array<AbmFrame> &getWaitFrames() const { return _waitFrames; }
	const Common::Array<AbmFrame> &getPointerFrames() const { return _pointerFrames; }
	const IndexedBitmap &getInventoryBitmap() const { return _inventoryBitmap; }
	const IndexedBitmap &getLogoBitmap() const { return _logoBitmap; }

private:
	bool loadPalette(ResourceManager &resources, const Common::String &path, byte *dest) const;
	bool loadBitmap(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) const;
	bool loadAnimation(ResourceManager &resources, const Common::String &path, Common::Array<AbmFrame> &frames) const;
	bool decodeAnimationFrame(const byte *source, uint32 sourceSize, bool compressed, Common::Array<byte> &dest) const;
	void blitBitmap(const IndexedBitmap &bitmap, int x, int y) const;
	void blitAnimationFrame(const Common::Array<AbmFrame> &frames, uint frameIndex, int x, int y) const;

	byte _waitPalette[256 * 3] = { 0 };
	byte _pcRoomPalette[256 * 3] = { 0 };
	Common::Array<AbmFrame> _waitFrames;
	Common::Array<AbmFrame> _pointerFrames;
	Common::Array<AbmFrame> _pcRoomClockFrames;
	Common::Array<AbmFrame> _pcActorFrames;
	Common::Array<IndexedBitmap> _textboxes;
	Common::Array<IndexedBitmap> _ammoIcons;
	IndexedBitmap _inventoryBitmap;
	IndexedBitmap _logoBitmap;
	IndexedBitmap _pcRoomBitmap;
	IndexedBitmap _pcRoomMaskA;
	IndexedBitmap _pcRoomMaskB;
	IndexedBitmap _tipsBitmap;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_ART_H
