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

#include "zvision/truetype_font.h"

#include "zvision/zvision.h"
#include "zvision/render_manager.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/unzip.h"

#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/surface.h"


namespace ZVision {

TruetypeFont::TruetypeFont(ZVision *engine, int32 fontHeight)
	: _engine(engine),
	  _fontHeight(fontHeight),
	  _font(0),
	  _lineHeight(0),
	  _maxCharWidth(0),
	  _maxCharHeight(0) {
}

TruetypeFont::~TruetypeFont(void) {
	delete _font;
}

bool TruetypeFont::loadFile(const Common::String &filename) {
	Common::File file;

	bool fileOpened = false;
	if (!Common::File::exists(filename)) {
		debug("TTF font file %s was not found. Reverting to arial.ttf", filename.c_str());
		fileOpened = file.open("arial.ttf");
	} else {
		fileOpened = file.open(filename);
	}

	if (!fileOpened) {
		debug("TTF file could not be opened");
		return false;
	}

	_font = Graphics::loadTTFFont(file, _fontHeight);
	_lineHeight = _font->getFontHeight();

	return true;
}

Graphics::Surface *TruetypeFont::drawTextToSurface(const Common::String &text, uint16 textColor, int maxWidth, int maxHeight, Graphics::TextAlign align, bool wrap) {
	if (text.equals("")) {
		return nullptr;
	}

	Graphics::Surface *surface = new Graphics::Surface();

	if (!wrap) {
		int width = MIN(_font->getStringWidth(text), maxWidth);
		surface->create(width, _lineHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		// TODO: Add better alpha support by getting the pixels from the backbuffer.
		// However doing that requires some kind of caching system so future text doesn't try to use this text as it's alpha background.
		surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), 0);

		_font->drawString(surface, text, 0, 0, maxWidth, textColor, align);
		return surface;
	}

	Common::Array<Common::String> lines;
	_font->wordWrapText(text, maxWidth, lines);

	while (maxHeight > 0 && (int)lines.size() * _lineHeight > maxHeight) {
		lines.pop_back();
	}
	if (lines.size() == 0) {
		return nullptr;
	}

	surface->create(maxWidth, lines.size() * _lineHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), 0);

	int heightOffset = 0;
	for (Common::Array<Common::String>::iterator it = lines.begin(); it != lines.end(); it++) {
		_font->drawString(surface, *it, 0, 0 + heightOffset, maxWidth, textColor, align);
		heightOffset += _lineHeight;
	}

	return surface;
}

sTTFont::sTTFont(ZVision *engine) {
	_engine = engine;
	_style = 0;
	_font = NULL;
	_lineHeight = 0;
}

sTTFont::~sTTFont() {
	if (_font)
		delete _font;
}

bool sTTFont::loadFont(const Common::String &fontName, int32 point, uint style) {
	_style = style;
	return loadFont(fontName, point);
}

bool sTTFont::loadFont(const Common::String &fontName, int32 point) {
	Common::String newFontName;
	if (fontName.matchString("*times new roman*", true) || fontName.matchString("*times*", true)) {
		if ((_style & (STTF_BOLD | STTF_ITALIC)) == (STTF_BOLD | STTF_ITALIC))
			newFontName = "timesbi.ttf";
		else if (_style & STTF_BOLD)
			newFontName = "timesbd.ttf";
		else if (_style & STTF_ITALIC)
			newFontName = "timesi.ttf";
		else
			newFontName = "times.ttf";

	} else if (fontName.matchString("*courier new*", true) || fontName.matchString("*courier*", true) || fontName.matchString("*ZorkDeath*", true)) {
		if ((_style & (STTF_BOLD | STTF_ITALIC)) == (STTF_BOLD | STTF_ITALIC))
			newFontName = "courbi.ttf";
		else if (_style & STTF_BOLD)
			newFontName = "courbd.ttf";
		else if (_style & STTF_ITALIC)
			newFontName = "couri.ttf";
		else
			newFontName = "cour.ttf";

	} else if (fontName.matchString("*century schoolbook*", true)) {
		if ((_style & (STTF_BOLD | STTF_ITALIC)) == (STTF_BOLD | STTF_ITALIC))
			newFontName = "censcbkbi.ttf";
		else if (_style & STTF_BOLD)
			newFontName = "censcbkbd.ttf";
		else if (_style & STTF_ITALIC)
			newFontName = "censcbki.ttf";
		else
			newFontName = "censcbk.ttf";

	} else if (fontName.matchString("*garamond*", true)) {
		if ((_style & (STTF_BOLD | STTF_ITALIC)) == (STTF_BOLD | STTF_ITALIC))
			newFontName = "garabi.ttf";
		else if (_style & STTF_BOLD)
			newFontName = "garabd.ttf";
		else if (_style & STTF_ITALIC)
			newFontName = "garai.ttf";
		else
			newFontName = "gara.ttf";

	} else if (fontName.matchString("*arial*", true) || fontName.matchString("*ZorkNormal*", true)) {
		if ((_style & (STTF_BOLD | STTF_ITALIC)) == (STTF_BOLD | STTF_ITALIC))
			newFontName = "arialbi.ttf";
		else if (_style & STTF_BOLD)
			newFontName = "arialbd.ttf";
		else if (_style & STTF_ITALIC)
			newFontName = "ariali.ttf";
		else
			newFontName = "arial.ttf";

	} else {
		debug("Could not identify font: %s. Reverting to Arial", fontName.c_str());
		newFontName = "arial.ttf";
	}

	bool sharp = (_style & STTF_SHARP) == STTF_SHARP;

	Common::File *file = _engine->getSearchManager()->openFile(newFontName);

	if (!file) {
		Common::SeekableReadStream *themeFile = nullptr;
		if (ConfMan.hasKey("themepath")) {
			Common::FSNode themePath(ConfMan.get("themepath"));
			if (themePath.exists()) {
				Common::FSNode scummModern = themePath.getChild("scummmodern.zip");
				if (scummModern.exists()) {
					themeFile = scummModern.createReadStream();
				}
			}
		}
		if (!themeFile) { // Fallback 2.5: Search for ScummModern.zip in SearchMan.
			themeFile = SearchMan.createReadStreamForMember("scummmodern.zip");
		}
		if (themeFile) {
			Common::Archive *themeArchive = Common::makeZipArchive(themeFile);
			if (themeArchive->hasFile("FreeSans.ttf")) {
				Common::SeekableReadStream *stream = nullptr;
				stream = themeArchive->createReadStreamForMember("FreeSans.ttf");
				Graphics::Font *_newFont = Graphics::loadTTFFont(*stream, point, 60, sharp); // 66 dpi for 640 x 480 on 14" display
				if (_newFont) {
					if (!_font)
						delete _font;
					_font = _newFont;
				}
				if (stream)
					delete stream;
			}
			delete themeArchive;
			themeArchive = nullptr;
		}
	} else {
		Graphics::Font *_newFont = Graphics::loadTTFFont(*file, point, 60, sharp); // 66 dpi for 640 x 480 on 14" display
		if (_newFont) {
			if (!_font)
				delete _font;
			_font = _newFont;
		}
		delete file;
	}

	_fntName = fontName;
	_lineHeight = point;

	if (_font)
		return true;
	return false;
}

void sTTFont::setStyle(uint newStyle) {
	if ((_style & (STTF_BOLD | STTF_ITALIC | STTF_SHARP)) != (newStyle & (STTF_BOLD | STTF_ITALIC | STTF_SHARP))) {
		_style = newStyle;
		loadFont(_fntName, _lineHeight);
	} else {
		_style = newStyle;
	}
}

int sTTFont::getFontHeight() {
	if (_font)
		return _font->getFontHeight();
	return 0;
}

int sTTFont::getMaxCharWidth() {
	if (_font)
		return _font->getMaxCharWidth();
	return 0;
}

int sTTFont::getCharWidth(byte chr) {
	if (_font)
		return _font->getCharWidth(chr);
	return 0;
}

int sTTFont::getKerningOffset(byte left, byte right) {
	if (_font)
		return _font->getKerningOffset(left, right);
	return 0;
}

void sTTFont::drawChar(Graphics::Surface *dst, byte chr, int x, int y, uint32 color) {
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

void sTTFont::drawString(Graphics::Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, Graphics::TextAlign align) {
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

int sTTFont::getStringWidth(const Common::String &str) {
	if (_font)
		return _font->getStringWidth(str);
	return 0;
}

Graphics::Surface *sTTFont::renderSolidText(const Common::String &str, uint32 color) {
	Graphics::Surface *tmp = new Graphics::Surface;
	if (_font) {
		int16 w = _font->getStringWidth(str);
		if (w && w < 1024) {
			tmp->create(w, _font->getFontHeight(), _engine->_pixelFormat);
			drawString(tmp, str, 0, 0, w, color);
		}
	}
	return tmp;
}

} // End of namespace ZVision
