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

#ifndef GRAPHICS_MFC_GFX_FONTS_H
#define GRAPHICS_MFC_GFX_FONTS_H

#include "common/array.h"
#include "common/types.h"
#include "graphics/font.h"
#include "graphics/mfc/minwindef.h"
#include "graphics/mfc/libs/resources.h"

namespace Graphics {
namespace MFC {
namespace Gfx {

/**
 * Font derived class that makes the
 * original face name and height visible.
 * It also bolds the font, which seems to
 * more closely match plain text in the
 * original Hodj n Podj.
 *
 * Note: We cache the underlying Graphics::Font
 * for the lifetime of the application, but
 * Font instances are temporary, and are what
 * we cast as HFONT. And as such, will be destroyed.
 */
class Font : public Graphics::Font {
private:
	Graphics::Font *_font;
	DisposeAfterUse::Flag _disposeAfterUse =
		DisposeAfterUse::YES;
	Common::String _faceName;
	int _height;
	int _charWidth;
	int _charHeight;

public:
	Font(Graphics::Font *font, const Common::String &faceName, int height,
		DisposeAfterUse::Flag disposeAfterUse =
			DisposeAfterUse::YES);
	~Font() override;

	int getFontHeight() const override;
	int getFontAscent() const override;
	int getFontDescent() const override;
	int getFontLeading() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	void drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override;

	const Common::String &getFaceName() const {
		return _faceName;
	}
	int getHeight() const {
		return _height;
	}
	int getCharWidth() const {
		return _charWidth;
	}
	int getCharHeight() const {
		return _charHeight;
	}
};

/**
 * Main fonts cache
 */
class Fonts {
	struct FontEntry {
		Common::String _faceName;
		int _height = 0;
		Font *_font = nullptr;

		void set(const Common::String &faceName,
			int height, Font *font) {
			_faceName = faceName;
			_height = height;
			_font = font;
		}
		~FontEntry();
	};
private:
	//Libs::Resources &_res;
	Common::Array<const char *> _fontResources;
	Common::List<FontEntry> _fonts;

	int resIndexOf(const char *filename) const;
	bool resExists(const char *filename) const {
		return resIndexOf(filename) != -1;
	}

public:
	Fonts(Libs::Resources &res) /*: _res(res) */ {}
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

} // namespace Gfx
} // namespace MFC
} // namespace Graphics

#endif
