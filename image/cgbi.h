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

#ifndef IMAGE_CGBI_H
#define IMAGE_CGBI_H

#include "common/scummsys.h"
#include "image/image_decoder.h"
#include "image/png.h"
#include "graphics/surface.h"

namespace Image {

class CgBIDecoder : public ImageDecoder {
public:
	CgBIDecoder();
	~CgBIDecoder();

	bool loadStream(Common::SeekableReadStream &stream) override;
	void destroy() override;
	Graphics::Surface *getSurface() const override {
		return _surface;
	}
	const Graphics::Palette &getPalette() const override {
		return _palette;
	}
	void unfilterScanline(uint8 *scanline, uint8 *prev, int scanlineLen, int bpp);
	void unfilterScanlines(uint8 *filtered, uint32 width, uint32 height, uint32 bpp);
	void convertBGRAtoRGBA(const byte *filtered, byte *out, uint32 width, uint32 height, uint32 bpp, bool hasAlpha);

private:
	Graphics::Surface *_surface;
	Graphics::Palette _palette;
};

} // End of namespace Image

#endif
