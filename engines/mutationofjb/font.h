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

#ifndef MUTATIONOFJB_FONT_H
#define MUTATIONOFJB_FONT_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Common {
class String;
}

namespace MutationOfJB {

class Font : public Graphics::Font {
	friend class FontBlitOperation;
public:
	Font(const Common::String &fileName, int horizSpacing, int lineHeight);

	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

protected:
	virtual uint8 transformColor(uint8 baseColor, uint8 glyphColor) const;

private:
	bool load(const Common::String &fileName);

	int _horizSpacing;
	int _lineHeight;
	int _maxCharWidth;
	typedef Common::HashMap<uint8, Graphics::ManagedSurface> GlyphMap;
	GlyphMap _glyphs;
};

class SystemFont : public Font {
public:
	SystemFont();
};

class SpeechFont : public Font {
public:
	SpeechFont();

protected:
	uint8 transformColor(uint8 baseColor, uint8 glyphColor) const override;
};

}

#endif
