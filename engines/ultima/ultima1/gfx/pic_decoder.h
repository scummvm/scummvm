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

#ifndef ULTIMA1_GFX_PIC_DECODER_H
#define ULTIMA1_GFX_PIC_DECODER_H

#include "graphics/palette.h"
#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

/**
 * Decodes Ultima I's raw EGA/Tandy full-screen pictures (e.g. "castle.16"):
 * 320x200, 4 bits/pixel, 2 pixels packed per byte (high nibble = left
 * pixel, low nibble = right pixel), stored linearly top-to-bottom with no
 * interlacing or compression. See
 * https://moddingwiki.shikadi.net/wiki/Ultima_I_Full_Screen_Graphic_Format
 *
 * The pixel data assumes a custom "cross-over" palette rather than the
 * engine's standard EGA one, which loadStream() decodes into getPalette()
 * for the caller to apply.
 */
class PicDecoder : public Image::ImageDecoder {
private:
	Graphics::Palette _palette;
	Graphics::Surface _surface;
public:
	~PicDecoder() {
		destroy();
	}

	/**
	 * Load an image from the specified stream.
	 *
	 * loadStream() should implicitly call destroy() to free the memory
	 * of the last loadStream() call.
	 *
	 * @param stream  Input stream.
	 *
	 * @return Whether loading the file succeeded.
	 *
	 * @see getSurface
	 * @see getPalette
	 */
	bool loadStream(Common::SeekableReadStream &stream) override;

	/**
	 * Destroy this decoder's surface and palette.
	 *
	 * This should be called by a loadStream() implementation as well
	 * as by the destructor.
	 */
	void destroy() override {
		_surface.free();
	}

	/**
	 * Get the decoded surface.
	 *
	 * This surface is owned by this ImageDecoder and remains valid
	 * until destroy() or loadStream() is called, or until the destructor of
	 * this ImageDecoder is called.
	 *
	 * @return The decoded surface, or 0 if no surface is present.
	 */
	const Graphics::Surface *getSurface() const override {
		return &_surface;
	}

	/**
	 * Get the decoded palette.
	 *
	 * This palette is owned by this ImageDecoder and remains valid
	 * until destroy() or loadStream() is called, or until the destructor of
	 * this ImageDecoder is called.
	 *
	 * @return The decoded palette, or empty if no palette is present.
	 */
	const Graphics::Palette &getPalette() const override {
		return _palette;
	}
};

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima

#endif
