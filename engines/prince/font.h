/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PRINCE_FONT_H
#define PRINCE_FONT_H

#include "graphics/font.h"
#include "graphics/surface.h"

#include "common/str.h"
#include "common/rect.h"

namespace Prince {

class Font : public Graphics::Font {
public:
	Font();
	~Font() override;

	bool loadStream(Common::SeekableReadStream &stream);

	int getFontHeight() const override;

	int getMaxCharWidth() const override;

	int getCharWidth(uint32 chr) const override;

	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	int getKerningOffset(uint32 left, uint32 right) const override { return -2; }

private:
	struct ChrData {
		byte *_pixels;
		byte _width;
		byte _height;
	};

	ChrData getChrData(byte chr) const;

	byte *_fontData;
};

} // End of namespace Prince

#endif
