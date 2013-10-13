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

namespace Graphics {
    class Surface;
}

namespace Common {
    class String;
}

namespace Prince {

class Font : public Graphics::Font {
public:
    Font();
    virtual ~Font();

    bool load(Common::SeekableReadStream &stream);

	virtual int getFontHeight() const override;

	virtual int getMaxCharWidth() const override;

	virtual int getCharWidth(byte chr) const override;

	virtual void drawChar(Graphics::Surface *dst, byte chr, int x, int y, uint32 color) const override;

private:
    struct ChrData {
        byte * _pixels;
        byte _width;
        byte _height;
    };

    ChrData getChrData(byte chr) const;

    byte * _fontData;
};

}

#endif
