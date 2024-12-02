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

#ifndef GOT_GFX_IMAGES_H
#define GOT_GFX_IMAGES_H

#include "common/stream.h"
#include "graphics/managed_surface.h"

namespace Got {
namespace Gfx {

class GfxChunks;

struct GraphicChunk {
private:
	GfxChunks *_owner = nullptr;
	int _index = 0;

public:
	Graphics::ManagedSurface _image;
	uint16 _field0 = 0;
	uint16 _field2 = 0;
	uint16 _field4 = 0;
	uint16 _uncompressedSize = 0;
	uint16 _compressedSize = 0;
	uint16 _width = 0;
	uint16 _height = 0;

	/**
	 * Load the overall info for a chunk
	 */
	void loadInfo(GfxChunks *owner, int index, Common::SeekableReadStream *src);

	void load();
	void unload();
};

/**
 * Interface for accessing the graphics.got file.
 * In the release, this is embedded in the executable starting
 * at offset 18af2h onwards. The preceding two bytes should be E2 4A.
 */
class GfxChunks {
private:
	Common::Array<GraphicChunk> _images;
	byte *_data = nullptr;

	/**
	 * Opens the graphics for access
	 */
	Common::SeekableReadStream *getStream() const;

public:
	~GfxChunks();

	/**
	 * Loads the graphic data
	 */
	void load();

	void load(int index);
	void unload(int index);
	void loadRange(int start, int count);
};

} // namespace Gfx
} // namespace Got

#endif
