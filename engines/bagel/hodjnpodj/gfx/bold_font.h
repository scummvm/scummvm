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

#ifndef HODJNPODJ_BOLD_FONT_H
#define HODJNPODJ_BOLD_FONT_H

#include "common/types.h"
#include "graphics/font.h"

namespace Bagel {
namespace HodjNPodj {
namespace Gfx {

class BoldFont : public Graphics::Font {
private:
	Graphics::Font *_font;
	DisposeAfterUse::Flag _disposeAfterUse =
	    DisposeAfterUse::YES;

public:
	BoldFont(Graphics::Font *font, DisposeAfterUse::Flag
	         disposeAfterUse = DisposeAfterUse::YES) :
		_font(font), _disposeAfterUse(disposeAfterUse) {
	}
	~BoldFont() override;

	int getFontHeight() const override;
	int getFontAscent() const override;
	int getFontDescent() const override;
	int getFontLeading() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	void drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

} // namespace Gfx
} // namespace HodjNPodj
} // namespace Bagel

#endif
