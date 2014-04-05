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

// This file is based on engines/wintermute/base/fonts/base_font_truetype.h/.cpp

#ifndef ZVISION_TRUETYPE_FONT_H
#define ZVISION_TRUETYPE_FONT_H

#include "graphics/font.h"
#include "graphics/pixelformat.h"


namespace Graphics {
struct Surface;
}

namespace ZVision {

class ZVision;

class TruetypeFont {
public:
	TruetypeFont(ZVision *engine, int32 fontHeight);
	~TruetypeFont();

private:
//	ZVision *_engine;
	Graphics::Font *_font;
	int _lineHeight;

//	size_t _maxCharWidth;
//	size_t _maxCharHeight;

public:
	int32 _fontHeight;

public:
	/**
	 * Loads a .ttf file into memory. This must be called
	 * before any calls to drawTextToSurface
	 *
	 * @param filename    The file name of the .ttf file to load
	 */
	bool loadFile(const Common::String &filename);
	/**
	 * Renders the supplied text to a Surface using 0x0 as the
	 * background color.
	 *
	 * @param text         The to render
	 * @param textColor    The color to render the text with
	 * @param maxWidth     The max width the text should take up.
	 * @param maxHeight    The max height the text should take up.
	 * @param align        The alignment of the text within the bounds of maxWidth
	 * @param wrap         If true, any words extending past maxWidth will wrap to a new line. If false, ellipses will be rendered to show that the text didn't fit
	 * @return             A Surface containing the rendered text
	 */
	Graphics::Surface *drawTextToSurface(const Common::String &text, uint16 textColor, int maxWidth, int maxHeight, Graphics::TextAlign align, bool wrap);
};

} // End of namespace ZVision

#endif
