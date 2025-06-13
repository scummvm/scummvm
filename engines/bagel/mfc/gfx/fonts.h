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

#ifndef BAGEL_MFC_GFX_FONTS_H
#define BAGEL_MFC_GFX_FONTS_H

#include "common/array.h"
#include "common/types.h"
#include "graphics/font.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/libs/resources.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

/**
 * Main fonts cache
 */
class Fonts {
	struct FontEntry {
		Common::String _faceName;
		int _height = 0;
		Graphics::Font *_font = nullptr;

		void set(const Common::String &faceName, int height,
			Graphics::Font *font) {
			_faceName = faceName;
			_height = height;
			_font = font;
		}
		~FontEntry() {
			delete _font;
		}
	};
private:
	Libs::Resources &_res;
	Common::Array<const char *> _fontResources;
	Common::List<FontEntry> _fonts;

	int resIndexOf(const char *filename) const;
	bool resExists(const char *filename) const {
		return resIndexOf(filename) != -1;
	}

public:
	Fonts(Libs::Resources &res) : _res(res) {}
	~Fonts();

	int addFontResource(const char *filename);
	bool removeFontResource(const char *filename);
	HFONT createFont(int nHeight, int nWidth, int nEscapement,
		int nOrientation, int nWeight, byte bItalic, byte bUnderline,
		byte cStrikeOut, byte nCharSet, byte nOutPrecision,
		byte nClipPrecision, byte nQuality, byte nPitchAndFamily,
		const char *lpszFacename);
	HFONT getFont(const char *lpszFacename, int nHeight);
};

/**
 * Font wrapper for when a bold font
 * is requested
 */
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
} // namespace MFC
} // namespace Bagel

#endif
