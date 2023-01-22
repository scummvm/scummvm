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

#ifndef MM1_GFX_SCREEN_DECODER_H
#define MM1_GFX_SCREEN_DECODER_H

#include "image/image_decoder.h"
#include "graphics/managed_surface.h"

namespace MM {
namespace MM1 {
namespace Gfx {

class ScreenDecoder : public Image::ImageDecoder {
private:
	Graphics::Surface _surface;
public:
	byte _indexes[4] = { 0 };		// EGA palete indexes used
public:
	ScreenDecoder() {}
	~ScreenDecoder() override;

	void destroy() override;
	bool loadFile(const Common::String &fname,
		int16 w = 320, int16 h = 200);
	bool loadStream(Common::SeekableReadStream &stream, int16 w, int16 h);
	bool loadStream(Common::SeekableReadStream &stream) override {
		return loadStream(stream, 320, 200);
	}

	const Graphics::Surface *getSurface() const override {
		return &_surface;
	}
	const byte *getPalette() const override { return nullptr; }
	uint16 getPaletteColorCount() const override { return 0; }
	void clear() { _surface.free(); }
};

} // namespace Gfx
} // namespace MM1
} // namespace MM

#endif
