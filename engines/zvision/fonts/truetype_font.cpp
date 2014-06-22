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

#include "zvision/fonts/truetype_font.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"

#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/surface.h"


namespace ZVision {

TruetypeFont::TruetypeFont(ZVision *engine, int32 fontHeight)
		: _fontHeight(fontHeight),
		  _font(0),
		  _lineHeight(0) {
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

} // End of namespace ZVision
