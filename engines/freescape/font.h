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
 */

#ifndef FREESCAPE_FONT_H
#define FREESCAPE_FONT_H

#include "common/array.h"
#include "common/stream.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Freescape {

class Font : public Graphics::Font {
public:
	Font();
	Font(Common::Array<Graphics::ManagedSurface *> &chars);
	~Font() override;

	void setBackground(uint32 color);
	void setSecondaryColor(uint32 color);
	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override { return _kerningOffset; }
	void setKernelingOffset(int offset) { _kerningOffset = offset; }
	void setCharWidth(int width) { _charWidth = width; }

	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	//const Graphics::ManagedSurface &getImageSurface() const { return _image; }
private:
    Common::Array<Graphics::ManagedSurface *> _chars;
	uint32 _backgroundColor;
	uint32 _secondaryColor;
	int _kerningOffset;
	int _charWidth;
};

} // End of namespace Freescape

#endif // FREESCAPE_FONT_H
