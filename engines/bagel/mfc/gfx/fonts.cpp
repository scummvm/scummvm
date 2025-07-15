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

#include "graphics/fonts/winfont.h"
#include "bagel/mfc/gfx/fonts.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

struct FontSizeOverride {
	const char *_faceName;
	int _height;
	int _avgWidth;
	int _charHeight;
};
static const FontSizeOverride FONT_SIZE_OVERRIDES[] = {
	{ "MS Sans Serif", 8, 7, 13 },
	{ "MS Sans Serif", 12, 10, 20 },
	{ nullptr, 0, 0, 0 }
};

Fonts::~Fonts() {
	_fonts.clear();
}

int Fonts::addFontResource(const char *filename) {
	if (!resExists(filename)) {
		_fontResources.push_back(filename);
		return 1;
	}

	return 0;
}

bool Fonts::removeFontResource(const char *filename) {
	int idx = resIndexOf(filename);
	if (idx != -1) {
		_fontResources.remove_at(idx);
		return true;
	} else {
		return false;
	}
}

HFONT Fonts::createFont(int nHeight, int nWidth, int nEscapement,
		int nOrientation, int nWeight, byte bItalic, byte bUnderline,
		byte cStrikeOut, byte nCharSet, byte nOutPrecision,
		byte nClipPrecision, byte nQuality, byte nPitchAndFamily,
		const char *lpszFacename) {
	Gfx::Font *font = nullptr;

	// for nHeight, -ve means char height (which ScummVM uses),
	// whilst +ve means cell height
	int charHeight;
	if (nHeight < 0) {
		charHeight = ABS(nHeight);
	} else {
		// Cell to char height mappings
		// TODO: This needs to be set properly
		static const int8 HEIGHTS[][2] = {
			{ 8, 8 },		// Default font
			{ 10, 10 },		// Boardgame dialog
			{ 12, 8 },
			{ 14, 9 }, { 16, 10 }, { 18, 24 },
			{ 21, 12 },		// The Gesng Gme
			{ 0, 0 }
		};
		charHeight = nHeight * 16 / 24;	// Rough default
		for (int i = 0; HEIGHTS[i][0]; ++i) {
			if (nHeight == HEIGHTS[i][0]) {
				charHeight = HEIGHTS[i][1];
				break;
			}
		}
	}

	// First scan for an existing cached copy of the font
	for (auto &it : _fonts) {
		if (it._faceName == lpszFacename &&
				it._height == nHeight) {
			font = it._font;
			break;
		}
	}

	if (!font) {
		// Create the font
		Graphics::WinFont *winFont = new Graphics::WinFont();

		for (auto &filename : _fontResources) {
			// Note: Windows does some rounding up or down to
			// the closest size for a given face name if the
			// requested size isn't available
			for (int h = charHeight; h >= 2; --h) {
				if (winFont->loadFromFON(filename, Graphics::WinFontDirEntry(
					lpszFacename, (h >= 6) ? h : charHeight + (6 - h)))) {
					// Loaded successfully
					font = new Gfx::Font(winFont, lpszFacename, nHeight);

					// Add to the font cache
					_fonts.push_back(FontEntry());
					_fonts.back().set(lpszFacename, nHeight, font);
					break;
				}
			}

			if (font)
				break;
		}

		if (!font)
			delete winFont;
	}

	// If we found a found, return a font Impl,
	// which can be cast as a HFONT for our MFC classes
	if (font) {
		CFont::Impl *f = new CFont::Impl(font);
		return f;
	}

	error("Could not locate font %s - size %d",
		lpszFacename, nHeight);
	return nullptr;
}

int Fonts::resIndexOf(const char *filename) const {
	for (uint i = 0; i < _fontResources.size(); ++i) {
		if (!strcmp(_fontResources[i], filename))
			return i;
	}

	return -1;
}

HFONT Fonts::getFont(const char *lpszFacename, int nHeight) {
	return createFont(nHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, lpszFacename);
}

/*--------------------------------------------*/

Fonts::FontEntry::~FontEntry() {
	delete _font;
}

/*--------------------------------------------*/

Font::Font(Graphics::Font *font, const Common::String &faceName, int height,
		DisposeAfterUse::Flag disposeAfterUse) :
		_font(font), _faceName(faceName), _height(height),
		_charWidth(0), _charHeight(0),
		_disposeAfterUse(disposeAfterUse) {
	for (const FontSizeOverride *aw = FONT_SIZE_OVERRIDES; aw->_faceName; ++aw) {
		if (faceName == aw->_faceName && height == aw->_height) {
			_charWidth = aw->_avgWidth;
			_charHeight = aw->_charHeight;
			return;
		}
	}
}

Font::~Font() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _font;
}

int Font::getFontHeight() const {
	return _font->getFontHeight();
}

int Font::getFontAscent() const {
	return _font->getFontAscent();
}

int Font::getFontDescent() const {
	return _font->getFontDescent();
}

int Font::getFontLeading() const {
	return _font->getFontLeading();
}

int Font::getMaxCharWidth() const {
	return _font->getMaxCharWidth() + 1;
}

int Font::getCharWidth(uint32 chr) const {
	return _font->getCharWidth(chr) + 1;
}

int Font::getKerningOffset(uint32 left, uint32 right) const {
	return _font->getKerningOffset(left, right);
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	_font->drawChar(dst, chr, x, y, color);
	_font->drawChar(dst, chr, x + 1, y, color);
}

void Font::drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const {
	const int charW = _font->getCharWidth(chr) + 1;
	const int charH = _font->getFontHeight();

	if (x >= 0 && y >= 0 && (x + charW) <= dst->w &&
			(y + charH) <= dst->h) {
		_font->drawChar(dst, chr, x, y, color);
		_font->drawChar(dst, chr, x + 1, y, color);
	}
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
