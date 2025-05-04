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

#ifndef IMAGE_CODECS_DITHER_H
#define IMAGE_CODECS_DITHER_H

#include "image/codecs/codec.h"

#include "common/types.h"
#include "graphics/palette.h"

namespace Image {

class DitherCodec : public Codec {
public:
	DitherCodec(Codec *codec, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	virtual ~DitherCodec() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;

	Graphics::PixelFormat getPixelFormat() const override;
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;
	bool containsPalette() const override;
	const byte *getPalette() override;
	bool hasDirtyPalette() const override;
	bool canDither(DitherType type) const override;
	void setDither(DitherType type, const byte *palette) override;
	void setCodecAccuracy(CodecAccuracy accuracy) override;

	/**
	 * Specify the source palette when dithering from CLUT8 to CLUT8.
	 */
	void setPalette(const byte *palette) { _srcPalette = palette; }

	/**
	 * Create a dither table, as used by QuickTime codecs.
	 */
	static byte *createQuickTimeDitherTable(const byte *palette, uint colorCount);

private:
	DisposeAfterUse::Flag _disposeAfterUse;
	Codec *_codec;
	const byte *_srcPalette;

	Graphics::Surface *_ditherFrame;
	Graphics::Palette _forcedDitherPalette;
	byte *_ditherTable;
	bool _dirtyPalette;
};

} // End of namespace Image

#endif
