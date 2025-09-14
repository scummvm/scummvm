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

#ifndef ULTIMA8_GFX_FONTS_TTFONT_H
#define ULTIMA8_GFX_FONTS_TTFONT_H

#include "ultima/ultima8/gfx/fonts/font.h"
#include "graphics/font.h"
#include "graphics/pixelformat.h"

namespace Ultima {
namespace Ultima8 {

class TTFont : public Font {
public:
	TTFont(Graphics::Font *font, uint32 rgb, int bordersize,
	       bool antiAliased, bool SJIS);
	~TTFont() override;

	int getHeight() override;
	int getBaseline() override;
	int getBaselineSkip() override;

	bool isAntialiased() {
		return _antiAliased;
	}

	void getStringSize(const Std::string &text,
		int32 &width, int32 &height) override;

	void getTextSize(const Std::string &text,
		int32 &resultwidth, int32 &resultheight, unsigned int &remaining,
		int32 width = 0, int32 height = 0, TextAlign align = TEXT_LEFT,
		bool u8specials = false, bool pagebreaks = false) override;

	RenderedText *renderText(const Std::string &text,
		unsigned int &remaining, int32 width = 0, int32 height = 0,
		TextAlign align = TEXT_LEFT, bool u8specials = false,
		bool pagebreaks = false,
		Std::string::size_type cursor = Std::string::npos) override;

protected:
	Graphics::Font *_ttfFont;
	uint32 _color;
	int _borderSize;
	bool _antiAliased;
	bool _SJIS;
	Graphics::PixelFormat _PF_RGBA;

	uint16 _bullet;

	void addTextBorder(Graphics::ManagedSurface &textSurf, uint32 *texBuf, const Common::Rect32 &dims, int32 resultWidth, int32 resultHeight, uint32 borderColor);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
