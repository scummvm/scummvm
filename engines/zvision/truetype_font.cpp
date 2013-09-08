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

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"

#include "graphics/fonts/ttf.h"
#include "graphics/surface.h"

#include "zvision/truetype_font.h"
#include "zvision/zvision.h"
#include "zvision/render_manager.h"


namespace ZVision {

TruetypeFont::TruetypeFont(ZVision *engine, int32 fontHeight, const Graphics::PixelFormat pixelFormat)
		: _engine(engine),
		  _pixelFormat(pixelFormat),
		  _isBold(false),
		  _isItalic(false),
		  _isStriked(false),
		  _isUnderline(false),
		  _fontHeight(fontHeight),
		  _font(0),
		  _lineHeight(0),
		  _maxCharWidth(0),
		  _maxCharHeight(0) {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		_cachedTexts[i] = 0;
	}
}

TruetypeFont::~TruetypeFont(void) {
	clearCache();

	delete _font;
}

void TruetypeFont::clearCache() {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i]) {
			delete _cachedTexts[i];
		}
		_cachedTexts[i] = 0;
	}
}

bool TruetypeFont::loadFile(const Common::String &filename) {
	Common::File file;
	if (file.open(filename)) {
		debug("TTF font file %s could not be opened", filename.c_str());
		return false;
	}

	_font = Graphics::loadTTFFont(file, 96, _fontHeight); // Use the same dpi as WME (96 vs 72).
	_lineHeight = _font->getFontHeight();

	return true;
}

void TruetypeFont::drawText(const Common::String &text, int x, int y, int width, TextAlign align, int maxHeight) {
	if (text.equals("")) {
		return;
	}

	// Find the cached surface, if it exists
	uint32 minUseTime = UINT_MAX;
	int minIndex = -1;
	Graphics::Surface *surface = nullptr;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i] == nullptr) {
			minUseTime = 0;
			minIndex = i;
		} else {
			if (_cachedTexts[i]->_text == text && _cachedTexts[i]->_align == align && _cachedTexts[i]->_width == width && _cachedTexts[i]->_maxHeight == maxHeight) {
				surface = _cachedTexts[i]->_surface;
				_cachedTexts[i]->_marked = true;
				_cachedTexts[i]->_lastUsed = g_system->getMillis();
				break;
			} else {
				if (_cachedTexts[i]->_lastUsed < minUseTime) {
					minUseTime = _cachedTexts[i]->_lastUsed;
					minIndex = i;
				}
			}
		}
	}

	// It's not cached, so create one
	if (!surface) {
		surface = renderTextToTexture(text, width, align, maxHeight);
		if (surface) {
			// Write the new surface to cache
			if (_cachedTexts[minIndex] != nullptr) {
				delete _cachedTexts[minIndex];
			}
			_cachedTexts[minIndex] = new CachedText;

			_cachedTexts[minIndex]->_surface = surface;
			_cachedTexts[minIndex]->_align = align;
			_cachedTexts[minIndex]->_width = width;
			_cachedTexts[minIndex]->_maxHeight = maxHeight;
			_cachedTexts[minIndex]->_text = text;
			_cachedTexts[minIndex]->_marked = true;
			_cachedTexts[minIndex]->_lastUsed = _engine->_system->getMillis();
		}
	}


	// Render it to the working window
	if (surface) {
		_engine->getRenderManager()->copyRectToWorkingWindow((uint16 *)surface->getPixels(), x, y, surface->w, surface->w, surface->h);
	}
}

Graphics::Surface *TruetypeFont::renderTextToTexture(const Common::String &text, int width, TextAlign align, int maxHeight) {
	Common::Array<Common::String> lines;
	_font->wordWrapText(text, width, lines);

	while (maxHeight > 0 && lines.size() * _lineHeight > maxHeight) {
		lines.pop_back();
	}
	if (lines.size() == 0) {
		return nullptr;
	}

	Graphics::TextAlign alignment = Graphics::kTextAlignInvalid;
	if (align == ALIGN_LEFT) {
		alignment = Graphics::kTextAlignLeft;
	} else if (align == ALIGN_CENTER) {
		alignment = Graphics::kTextAlignCenter;
	} else if (align == ALIGN_RIGHT) {
		alignment = Graphics::kTextAlignRight;
	}

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create((uint16)width, (uint16)(_lineHeight * lines.size()), _pixelFormat);

	uint32 alphaColor = 0xffffffff;
	int heightOffset = 0;
	for (Common::Array<Common::String>::iterator it = lines.begin(); it != lines.end(); it++) {
		_font->drawString(surface, *it, 0, heightOffset, width, alphaColor, alignment);
		heightOffset += (int)_lineHeight;
	}

	return surface;
	// TODO: _isUnderline, _isBold, _isItalic, _isStriked
}

void TruetypeFont::measureText(const Common::String &text, int maxWidth, int maxHeight, int &textWidthOut, int &textHeightOut) {
	if (maxWidth >= 0) {
		Common::Array<Common::String> lines;
		_font->wordWrapText(text, maxWidth, lines);
		Common::Array<Common::String>::iterator it;
		textWidthOut = 0;
		for (it = lines.begin(); it != lines.end(); ++it) {
			textWidthOut = MAX(textWidthOut, _font->getStringWidth(*it));
		}

		textHeightOut = (int)(lines.size() * getLineHeight());
	} else {
		textWidthOut = _font->getStringWidth(text);
		textHeightOut = _fontHeight;
	}
}

} // End of namespace ZVision
