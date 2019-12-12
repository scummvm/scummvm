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

#ifndef ULTIMA8_GRAPHICS_FONTS_TTFONT_H
#define ULTIMA8_GRAPHICS_FONTS_TTFONT_H

#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "graphics/font.h"

namespace Ultima8 {

class TTFont : public Pentagram::Font {
public:
	TTFont(Graphics::Font *font, uint32 rgb, int bordersize,
	       bool antiAliased, bool SJIS);
	virtual ~TTFont();

	virtual int getHeight();
	virtual int getBaseline();
	virtual int getBaselineSkip();

	bool isAntialiased() {
		return antiAliased;
	}

	virtual void getStringSize(const std::string &text,
	                           int &width, int &height);

	virtual void getTextSize(const std::string &text,
	                         int &resultwidth, int &resultheight,
	                         unsigned int &remaining,
	                         int width = 0, int height = 0,
	                         TextAlign align = TEXT_LEFT, bool u8specials = false);

	virtual RenderedText *renderText(const std::string &text,
	                                 unsigned int &remaining,
	                                 int width = 0, int height = 0,
	                                 TextAlign align = TEXT_LEFT,
	                                 bool u8specials = false,
	                                 std::string::size_type cursor
	                                 = std::string::npos);

	ENABLE_RUNTIME_CLASSTYPE()
protected:
	Graphics::Font *ttf_font;
	uint32 rgb;
	int bordersize;
	bool antiAliased;
	bool SJIS;

	uint16 bullet;
};

} // End of namespace Ultima8

#endif
