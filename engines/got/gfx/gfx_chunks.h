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

#ifndef GOT_GFX_GFX_CHUNKS_H
#define GOT_GFX_GFX_CHUNKS_H

#include "common/stream.h"
#include "got/gfx/palette.h"
#include "graphics/managed_surface.h"

namespace Got {
namespace Gfx {

class GfxChunks;

enum CompressMode { UNCOMPRESSED = 0,
					LZSS = 1,
					RLE = 2 };

struct GraphicChunk {
private:
	Common::Array<byte> _decompressedData;

public:
	int _compressMode = UNCOMPRESSED;
	uint32 _offset = 0;
	uint16 _uncompressedSize = 0;
	uint16 _compressedSize = 0;
	uint16 _width = 0;
	uint16 _height = 0;

	const byte *_data = nullptr;

	/**
     * Load the overall info for a chunk
     */
	void load(Common::SeekableReadStream *src, const byte *data);

	/**
     * Handles any decompression necessary for the entry
     */
	void enable();

	/**
     * Provides a managed surface wrapper for raw data
     */
	operator const Graphics::ManagedSurface() const;

	/**
     * Provides a data pointer, used for getting palette chunks
     */
	operator const Gfx::Palette63() const;
};

/**
 * Interface for accessing the graphics.got file.
 * In the release, this is embedded in the executable starting
 * at offset 18af2h onwards. The preceding two bytes should be E2 4A.
 * The collection is mostly images, but there are some palettes and
 * sounds included as well.
 */
class GfxChunks {
private:
	Common::Array<GraphicChunk> _chunks;
	byte *_data = nullptr;

	/**
     * Opens the graphics for access
     */
	Common::SeekableReadStream *getStream() const;

public:
	~GfxChunks() {
		delete[] _data;
	}

	/**
     * Loads the graphic data
     */
	void load();

	/**
     * Access a chunk
     */
	GraphicChunk &operator[](uint idx);
};

} // namespace Gfx
} // namespace Got

#endif
