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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/unzip.h"
#include "common/ustr.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/surface.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/text/truetype_font.h"

namespace ZVision {

const FontStyle systemFonts[] = {
	{ "*times new roman*",	  "times",   "FreeSerif", "Italic", "LiberationSerif"  },
	{ "*times*",		  "times",   "FreeSerif", "Italic", "LiberationSerif"  },
	{ "*century schoolbook*", "censcbk", "FreeSerif", "Italic", "LiberationSerif"  },
	{ "*garamond*", 	  "gara",    "FreeSerif", "Italic", "LiberationSerif"  },
	{ "*courier new*",	  "cour",    "FreeMono",  "Oblique", "LiberationMono" },
	{ "*courier*",		  "cour",    "FreeMono",  "Oblique", "LiberationMono" },
	{ "*ZorkDeath*",	  "cour",    "FreeMono",  "Oblique", "LiberationMono" },
	{ "*arial*",		  "arial",   "FreeSans",  "Oblique", "LiberationSans" },
	{ "*ZorkNormal*",	  "arial",   "FreeSans",  "Oblique", "LiberationSans" }
};

const FontStyle getSystemFont(int fontIndex) {
	return systemFonts[fontIndex];
}

StyledTTFont::StyledTTFont(ZVision *engine) {
	_engine = engine;
	_style = 0;
	_font = nullptr;
	_lineHeight = 0;
}

StyledTTFont::~StyledTTFont() {
	delete _font;
}

bool StyledTTFont::loadFont(const Common::String &fontName, int32 point, uint style) {
	// Don't re-load the font if we've already loaded it
	// We have to check for empty so we can default to Arial
	if (!fontName.empty() && _fontName.equalsIgnoreCase(fontName) && _lineHeight == point && _style == style) {
		return true;
	}

	_style = style;

	Common::String newFontName;
	Common::String freeFontName;
	Common::String liberationFontName;

	for (int i = 0; i < FONT_COUNT; i++) {
		FontStyle curFont = getSystemFont(i);
		if (fontName.matchString(curFont.zorkFont, true)) {
			newFontName = curFont.fontBase;
			freeFontName = curFont.freeFontBase;
			liberationFontName = curFont.liberationFontBase;

			if ((_style & TTF_STYLE_BOLD) && (_style & TTF_STYLE_ITALIC)) {
				newFontName += "bi";
				freeFontName += "Bold";
				freeFontName += curFont.freeFontItalicName;
				liberationFontName += "-BoldItalic";
			} else if (_style & TTF_STYLE_BOLD) {
				newFontName += "bd";
				freeFontName += "Bold";
				liberationFontName += "-Bold";
			} else if (_style & TTF_STYLE_ITALIC) {
				newFontName += "i";
				freeFontName += curFont.freeFontItalicName;
				liberationFontName += "-Italic";
			} else {
				liberationFontName += "-Regular";
			}

			newFontName += ".ttf";
			freeFontName += ".ttf";
			liberationFontName += ".ttf";
			break;
		}
	}

	if (newFontName.empty()) {
		debug("Could not identify font: %s. Reverting to Arial", fontName.c_str());
		newFontName = "arial.ttf";
		freeFontName = "FreeSans.ttf";
		liberationFontName = "LiberationSans-Regular.ttf";
	}

	bool sharp = (_style & TTF_STYLE_SHARP) == TTF_STYLE_SHARP;

	Common::File file;
	Graphics::Font *newFont;
	if (!file.open(newFontName) && !_engine->getSearchManager()->openFile(file, newFontName) &&
		!file.open(liberationFontName) && !_engine->getSearchManager()->openFile(file, liberationFontName) &&
		!file.open(freeFontName) && !_engine->getSearchManager()->openFile(file, freeFontName)) {
		newFont = Graphics::loadTTFFontFromArchive(liberationFontName, point, Graphics::kTTFSizeModeCell, 0, (sharp ? Graphics::kTTFRenderModeMonochrome : Graphics::kTTFRenderModeNormal));
	} else {
		newFont = Graphics::loadTTFFont(file, point, Graphics::kTTFSizeModeCell, 0, (sharp ? Graphics::kTTFRenderModeMonochrome : Graphics::kTTFRenderModeNormal));
	}

	if (newFont == nullptr) {
		return false;
	}

	delete _font;
	_font = newFont;

	_fontName = fontName;
	_lineHeight = point;

	return true;
}

int StyledTTFont::getFontHeight() {
	if (_font)
		return _font->getFontHeight();

	return 0;
}

int StyledTTFont::getMaxCharWidth() {
	if (_font)
		return _font->getMaxCharWidth();

	return 0;
}

int StyledTTFont::getCharWidth(byte chr) {
	if (_font)
		return _font->getCharWidth(chr);

	return 0;
}

int StyledTTFont::getKerningOffset(byte left, byte right) {
	if (_font)
		return _font->getKerningOffset(left, right);

	return 0;
}

void StyledTTFont::drawChar(Graphics::Surface *dst, byte chr, int x, int y, uint32 color) {
	if (_font) {
		_font->drawChar(dst, chr, x, y, color);
		if (_style & TTF_STYLE_UNDERLINE) {
			int16 pos = (int16)floor(_font->getFontHeight() * 0.87);
			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);
			dst->fillRect(Common::Rect(x, y + pos, x + _font->getCharWidth(chr), y + pos + thk), color);
		}
		if (_style & TTF_STYLE_STRIKETHROUGH) {
			int16 pos = (int16)floor(_font->getFontHeight() * 0.60);
			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);
			dst->fillRect(Common::Rect(x, y + pos, x + _font->getCharWidth(chr), y + pos + thk), color);
		}
	}
}

void StyledTTFont::drawString(Graphics::Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, Graphics::TextAlign align) {
	if (_font) {
		Common::U32String u32str = Common::convertUtf8ToUtf32(str);
		_font->drawString(dst, u32str, x, y, w, color, align);
		if (_style & TTF_STYLE_UNDERLINE) {
			int16 pos = (int16)floor(_font->getFontHeight() * 0.87);
			int16 wd = MIN(_font->getStringWidth(u32str), w);
			int16 stX = x;
			if (align == Graphics::kTextAlignCenter)
				stX += (w - wd) / 2;
			else if (align == Graphics::kTextAlignRight)
				stX += (w - wd);

			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);

			dst->fillRect(Common::Rect(stX, y + pos, stX + wd, y + pos + thk), color);
		}
		if (_style & TTF_STYLE_STRIKETHROUGH) {
			int16 pos = (int16)floor(_font->getFontHeight() * 0.60);
			int16 wd = MIN(_font->getStringWidth(u32str), w);
			int16 stX = x;
			if (align == Graphics::kTextAlignCenter)
				stX += (w - wd) / 2;
			else if (align == Graphics::kTextAlignRight)
				stX += (w - wd);

			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);

			dst->fillRect(Common::Rect(stX, y + pos, stX + wd, y + pos + thk), color);
		}
	}
}

int StyledTTFont::getStringWidth(const Common::String &str) {
	if (_font)
		return _font->getStringWidth(str);
	return 0;
}

Graphics::Surface *StyledTTFont::renderSolidText(const Common::String &str, uint32 color) {
	Graphics::Surface *tmp = new Graphics::Surface;
	if (_font) {
		int16 w = _font->getStringWidth(str);
		if (w && w < 1024) {
			tmp->create(w, _font->getFontHeight(), _engine->_resourcePixelFormat);
			drawString(tmp, str, 0, 0, w, color);
		}
	}
	return tmp;
}

} // End of namespace ZVision
