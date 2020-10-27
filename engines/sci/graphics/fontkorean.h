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
 *
 */

#ifndef SCI_GRAPHICS_FONTKOREAN_H
#define SCI_GRAPHICS_FONTKOREAN_H

#include "sci/graphics/helpers.h"

namespace Graphics {
class FontKorean;
}

namespace Sci {

/**
 * Special Font class, handles Korean inside sci games, uses ScummVM Korean support
 */
class GfxFontKorean : public GfxFont {
public:
	GfxFontKorean(GfxScreen *screen, GuiResourceId resourceId);
	~GfxFontKorean();

	GuiResourceId getResourceId();
	byte getHeight();
	bool isDoubleByte(uint16 chr);
	byte getCharWidth(uint16 chr);
	byte *getCharData(uint16 chr);
	void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput);
#ifdef ENABLE_SCI32
	// SCI2/2.1 equivalent
	void drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 width, int16 height);
#endif

private:
	GfxScreen *_screen;
	GuiResourceId _resourceId;

	Graphics::FontKorean *_commonFont;
};

} // End of namespace Sci
#endif
