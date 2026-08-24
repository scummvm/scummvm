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

#ifndef HOLLYWOOD_GAMEPLAY_INVENTORY_MEDIA_H
#define HOLLYWOOD_GAMEPLAY_INVENTORY_MEDIA_H

#include "common/array.h"
#include "common/types.h"

#include "hollywood/graphics.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;

enum InventoryMediaId {
	kInventoryMediaPoster,
	kInventoryMediaPosterWithoutPhoto,
	kInventoryMediaFrankensteinDiary,
	kInventoryMediaTaffyPass,
	kInventoryMediaForgedPass
};

/**
 * Decodes the full-screen inventory media stored in RESOURCE.I04.
 *
 * Stills are complete framebuffers; tape frames are ordered patches over a
 * decoded base. The caller owns blocking input, speech, presentation, and
 * restoration of the current scene.
 */
class InventoryMediaPlayer {
public:
	enum {
		kSueTapeFrameCount = 10
	};

	InventoryMediaPlayer(HollywoodEngine *vm);

	bool loadStill(InventoryMediaId mediaId);
	bool loadSueTape();
	void drawSueTapeFrame(byte frameIndex);

	const Common::Array<byte> &palette() const { return _palette; }
	const Graphics::Surface &framebuffer() const { return _framebuffer.surface(); }

private:
	bool readChunk(uint index, Common::Array<byte> &destination, uint expectedSize = 0);

	HollywoodEngine *_vm;
	Common::Array<byte> _palette;
	Common::Array<byte> _tapeResource;
	uint32 _tapeDescriptorTableOffset;
	IndexedSurfaceBuffer _framebuffer;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_INVENTORY_MEDIA_H
