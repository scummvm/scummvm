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

#ifndef IMAGE_CODECS_CDTOONS_H
#define IMAGE_CODECS_CDTOONS_H

#include "graphics/palette.h"

#include "image/codecs/codec.h"

#include "common/hashmap.h"

namespace Image {

struct CDToonsBlock {
	uint16 flags;
	uint32 size;
	uint16 startFrame;
	uint16 endFrame;
	uint16 unknown12;
	byte *data;
};

/**
 * Broderbund CDToons decoder.
 *
 * Used by PICT/QuickTime.
 */
class CDToonsDecoder : public Codec {
public:
	CDToonsDecoder(uint16 width, uint16 height);
	~CDToonsDecoder() override;

	Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override { return Graphics::PixelFormat::createFormatCLUT8(); }
	bool containsPalette() const override { return true; }
	const byte *getPalette() override { _dirtyPalette = false; return _palette.data; }
	bool hasDirtyPalette() const override { return _dirtyPalette; }

private:
	Graphics::Surface *_surface;
	Graphics::Palette _palette;
	bool _dirtyPalette;
	uint16 _currentPaletteId;

	Common::HashMap<uint16, CDToonsBlock> _blocks;

	void renderBlock(byte *data, uint size, int x, int y, uint width, uint height);
	void setPalette(byte *data);
};

} // End of namespace Image

#endif
