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

#ifndef GOT_GFX_FONT_H
#define GOT_GFX_FONT_H

#include "common/array.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Got {
namespace Gfx {

extern const byte DIALOG_COLOR[];

class Font : public Graphics::Font {
private:
	Common::Array<Graphics::ManagedSurface> _font;

public:
	void load();

	int getFontHeight() const override {
		return 9;
	}
	int getMaxCharWidth() const override {
		return 8;
	}
	int getCharWidth(uint32 chr) const override {
		return 8;
	}
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	void drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override {
		Graphics::Font::drawChar(dst, chr, x, y, color);
	}
	void rawDrawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;

	void drawString(Graphics::ManagedSurface *src, const Common::Point &pos,
					const Common::String &text, int color);
};

} // namespace Gfx
} // namespace Got

#endif
