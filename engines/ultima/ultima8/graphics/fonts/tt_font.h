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

namespace Ultima {
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

	virtual void getStringSize(const Std::string &text,
		int32 &width, int32 &height) override;

	virtual void getTextSize(const Std::string &text,
		int32 &resultwidth, int32 &resultheight, unsigned int &remaining,
		int32 width = 0, int32 height = 0, TextAlign align = TEXT_LEFT,
		bool u8specials = false) override;

	virtual RenderedText *renderText(const Std::string &text,
		unsigned int &remaining, int32 width = 0, int32 height = 0,
		TextAlign align = TEXT_LEFT, bool u8specials = false,
		Std::string::size_type cursor = Std::string::npos) override;

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
} // End of namespace Ultima

#endif
