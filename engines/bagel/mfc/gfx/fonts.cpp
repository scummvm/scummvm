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

struct AvgWidth {
	const char *_faceName;
	int _height;
	int _avgWidth;
	int _charHeight;
};
static const AvgWidth AVG_WIDTH[] = {
	{ "MS Sans Serif", 12, 10, 0 },
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
	// TODO: We don't really handle +/- heights properly
	nHeight = ABS(nHeight);

	// First scan for an existing cached copy of the font
	for (auto &it : _fonts) {
		if (it._faceName == lpszFacename &&
				it._height == nHeight)
			return (HFONT)it._font;
	}

	// Create the font
	Graphics::WinFont *font = new Graphics::WinFont();

	for (auto &filename : _fontResources) {
		// FIXME: Windows does some rounding up or down to
		// the closest size for a given face name if the
		// requested size isn't available. For now,
		// for Hodj n Podj, I'll just have a single + 2 fallback
		for (int h = nHeight; h <= (nHeight + 2); h += 2) {
			if (font->loadFromFON(filename, Graphics::WinFontDirEntry(
				lpszFacename, h))) {
				Gfx::Font *gfxFont = new Gfx::Font(font, lpszFacename, nHeight);
				CFont::Impl *f = new CFont::Impl(gfxFont);

				// Add to the font cache
				_fonts.push_back(FontEntry());
				_fonts.back().set(lpszFacename, nHeight, f);
				return f;
			}
		}
	}

	delete font;

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

HFONT Fonts::getDefaultFont() {
	return createFont(8, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
}

/*--------------------------------------------*/

Fonts::FontEntry::~FontEntry() {
	delete (CFont::Impl *)_font;
}

/*--------------------------------------------*/

Font::Font(Graphics::Font *font, const Common::String &faceName, int height,
		DisposeAfterUse::Flag disposeAfterUse) :
		_font(font), _faceName(faceName), _height(height),
		_charWidth(0), _charHeight(0),
		_disposeAfterUse(disposeAfterUse) {
	for (const AvgWidth *aw = AVG_WIDTH; aw->_faceName; ++aw) {
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
	_font->drawChar(dst, chr, x, y, color);
	_font->drawChar(dst, chr, x + 1, y, color);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
