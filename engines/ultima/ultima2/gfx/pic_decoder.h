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

#ifndef ULTIMA2_GFX_PIC_DECODER_H
#define ULTIMA2_GFX_PIC_DECODER_H

#include "graphics/palette.h"
#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

/**
 * Decodes Ultima II's raw "pic???" full-screen title/demo art: a literal
 * 16384-byte dump of the CGA mode 4 (320x200, 4-color, palette 1)
 * framebuffer segment -- two interleaved 8192-byte banks (even scanlines
 * in the first, odd in the second), 2 bits/pixel, 4 pixels per byte,
 * MSB-first.
 */
class PicDecoder : public Image::ImageDecoder {
private:
	Graphics::Palette _palette;
	Graphics::Surface _surface;
public:
	~PicDecoder() {
		destroy();
	}

	bool loadStream(Common::SeekableReadStream &stream) override;

	void destroy() override {
		_surface.free();
	}

	const Graphics::Surface *getSurface() const override {
		return &_surface;
	}

	const Graphics::Palette &getPalette() const override {
		return _palette;
	}
};

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima

#endif
