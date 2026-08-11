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

/**
 * @file
 * Image decoder used in engines:
 *  - gob
 */

#ifndef IMAGE_BRC_H
#define IMAGE_BRC_H

#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_brc BRC decoder
 * @ingroup image
 *
 * @brief Decoder for BRC images.
 *
 * @{
 */

class BRCDecoder : public ImageDecoder {
public:
	BRCDecoder();
	~BRCDecoder() override;

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return &_surface; }
	const Graphics::Palette &getPalette() const override { return _palette; }

private:
	Graphics::Surface _surface;
	Graphics::Palette _palette;
	Graphics::PixelFormat _format;
	void loadBRCData(Common::SeekableReadStream &stream, uint32 nbrOfChunks, bool firstChunkIsRLE);
	void loadBRCDataColumnWise(Common::SeekableReadStream &stream, uint32 nbrOfChunks, bool firstChunkIsRLE);

	void readRawChunk(Common::SeekableReadStream &stream, uint16 *&dest);
	void readRLEChunk(Common::SeekableReadStream &stream, uint16 *&dest);
	void readRawChunkColumnWise(Common::SeekableReadStream &stream, uint16 *&dest, uint16 *&destColumnStart, uint32 &remainingHeight);
	void readRLEChunkColumnWise(Common::SeekableReadStream &stream, uint16 *&dest, uint16 *&destColumnStart, uint32 &remainingHeight);
};

} // End of namespace Image

#endif
