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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/surface.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/text/truetype_font.h"

namespace ZVision {

StyledTTFont::StyledTTFont(ZVision *engine) {
	_engine = engine;
	_style = 0;
	_font = NULL;
	_lineHeight = 0;
}

StyledTTFont::~StyledTTFont() {
	if (_font)
		delete _font;
}

bool StyledTTFont::loadFont(const Common::String &fontName, int32 point, uint style) {
	_style = style;
	return loadFont(fontName, point);
}

bool StyledTTFont::loadFont(const Common::String &fontName, int32 point) {
	struct FontStyle {
		const char *zorkFont;
		const char *fontBase;
		const char *freeFontBase;
		const char *freeFontItalicName;
	};

	const FontStyle systemFonts[] = {
		{ "*times new roman*",    "times",   "FreeSerif", "Italic"  },
		{ "*times*",              "times",   "FreeSerif", "Italic"  },
		{ "*century schoolbook*", "censcbk", "FreeSerif", "Italic"  },
		{ "*garamond*",           "gara",    "FreeSerif", "Italic"  },
		{ "*courier new*",        "cour",    "FreeMono",  "Oblique" },
		{ "*courier*",            "cour",    "FreeMono",  "Oblique" },
		{ "*ZorkDeath*",          "cour",    "FreeMono",  "Oblique" },
		{ "*arial*",              "arial",   "FreeSans",  "Oblique" },
		{ "*ZorkNormal*",         "arial",   "FreeSans",  "Oblique" },
	};

	Common::String newFontName;
	Common::String freeFontName;

	for (int i = 0; i < ARRAYSIZE(systemFonts); i++) {
		if (fontName.matchString(systemFonts[i].zorkFont, true)) {
			newFontName = systemFonts[i].fontBase;
			freeFontName = systemFonts[i].freeFontBase;

			if ((_style & STTF_BOLD) && (_style & STTF_ITALIC)) {
				newFontName += "bi";
				freeFontName += "Bold";
				freeFontName += systemFonts[i].freeFontItalicName;
			} else if (_style & STTF_BOLD) {
				newFontName += "bd";
				freeFontName += "Bold";
			} else if (_style & STTF_ITALIC) {
				newFontName += "i";
				freeFontName += systemFonts[i].freeFontItalicName;
			}

			newFontName += ".ttf";
			freeFontName += ".ttf";
			break;
		}
	}

	if (newFontName.empty()) {
		debug("Could not identify font: %s. Reverting to Arial", fontName.c_str());
		newFontName = "arial.ttf";
		freeFontName = "FreeSans.ttf";
	}

	bool sharp = (_style & STTF_SHARP) == STTF_SHARP;

	Common::File file;
	if (!file.open(newFontName) && !file.open(freeFontName) && !_engine->getSearchManager()->openFile(file, newFontName) && !_engine->getSearchManager()->openFile(file, freeFontName))
		error("Unable to open font file %s (free alternative: %s)", newFontName.c_str(), freeFontName.c_str());

	Graphics::Font *_newFont = Graphics::loadTTFFont(file, point, 60, (sharp ? Graphics::kTTFRenderModeMonochrome : Graphics::kTTFRenderModeNormal)); // 66 dpi for 640 x 480 on 14" display
	if (_newFont) {
		if (!_font)
			delete _font;
		_font = _newFont;
	}

	_fntName = fontName;
	_lineHeight = point;

	if (_font)
		return true;
	return false;
}

void StyledTTFont::setStyle(uint newStyle) {
	if ((_style & (STTF_BOLD | STTF_ITALIC | STTF_SHARP)) != (newStyle & (STTF_BOLD | STTF_ITALIC | STTF_SHARP))) {
		_style = newStyle;
		loadFont(_fntName, _lineHeight);
	} else {
		_style = newStyle;
	}
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
		if (_style & STTF_UNDERLINE) {
			int16 pos = floor(_font->getFontHeight() * 0.87);
			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);
			dst->fillRect(Common::Rect(x, y + pos, x + _font->getCharWidth(chr), y + pos + thk), color);
		}
		if (_style & STTF_STRIKEOUT) {
			int16 pos = floor(_font->getFontHeight() * 0.60);
			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);
			dst->fillRect(Common::Rect(x, y + pos, x + _font->getCharWidth(chr), y + pos + thk), color);
		}
	}
}

void StyledTTFont::drawString(Graphics::Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, Graphics::TextAlign align) {
	if (_font) {
		_font->drawString(dst, str, x, y, w, color, align);
		if (_style & STTF_UNDERLINE) {
			int16 pos = floor(_font->getFontHeight() * 0.87);
			int16 wd = MIN(_font->getStringWidth(str), w);
			int16 stX = x;
			if (align == Graphics::kTextAlignCenter)
				stX += (w - wd) / 2;
			else if (align == Graphics::kTextAlignRight)
				stX += (w - wd);

			int thk = MAX((int)(_font->getFontHeight() * 0.05), 1);

			dst->fillRect(Common::Rect(stX, y + pos, stX + wd, y + pos + thk), color);
		}
		if (_style & STTF_STRIKEOUT) {
			int16 pos = floor(_font->getFontHeight() * 0.60);
			int16 wd = MIN(_font->getStringWidth(str), w);
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
