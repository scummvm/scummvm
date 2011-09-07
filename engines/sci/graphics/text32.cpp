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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/sci/graphics/text16.cpp $
 * $Id: text16.cpp 55178 2011-01-08 23:16:44Z thebluegr $
 *
 */

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {

GfxText32::GfxText32(SegManager *segMan, GfxCache *fonts, GfxScreen *screen)
	: _segMan(segMan), _cache(fonts), _screen(screen) {
}

GfxText32::~GfxText32() {
	purgeCache();
}

void GfxText32::purgeCache() {
	for (TextCache::iterator cacheIterator = _textCache.begin(); cacheIterator != _textCache.end(); cacheIterator++) {
		delete[] cacheIterator->_value->surface;
		delete cacheIterator->_value;
		cacheIterator->_value = 0;
	}

	_textCache.clear();
}

void GfxText32::createTextBitmap(reg_t textObject) {
	if (_textCache.size() >= MAX_CACHED_TEXTS)
		purgeCache();

	uint32 textId = (textObject.segment << 16) | textObject.offset;

	if (_textCache.contains(textId)) {
		// Delete the old entry
		TextEntry *oldEntry = _textCache[textId];
		delete[] oldEntry->surface;
		delete oldEntry;
		_textCache.erase(textId);
	}

	_textCache[textId] = createTextEntry(textObject);
}

// TODO: Finish this!
void GfxText32::drawTextBitmap(reg_t textObject, uint16 textX, uint16 textY, uint16 w) {
	uint32 textId = (textObject.segment << 16) | textObject.offset;

	if (!_textCache.contains(textId))
		createTextBitmap(textObject);

	TextEntry *entry = _textCache[textId];

	// This draws text the "SCI0-SCI11" way. In SCI2, text is prerendered in kCreateTextBitmap
	// TODO: rewrite this the "SCI2" way (i.e. implement the text buffer to draw inside kCreateTextBitmap)
	GfxFont *font = _cache->getFont(readSelectorValue(_segMan, textObject, SELECTOR(font)));
	bool dimmed = readSelectorValue(_segMan,textObject, SELECTOR(dimmed));
	uint16 foreColor = readSelectorValue(_segMan, textObject, SELECTOR(fore));

	const char *txt = entry->text.c_str();
	int16 charCount;

	while (*txt) {
		charCount = GetLongest(txt, w, font);
		if (charCount == 0)
			break;

		uint16 curX = textX;

		for (int i = 0; i < charCount; i++) {
			unsigned char curChar = txt[i];
			font->draw(curChar, textY, curX, foreColor, dimmed);
			curX += font->getCharWidth(curChar);
		}

		textY += font->getHeight();
		txt += charCount;
		while (*txt == ' ')
			txt++; // skip over breaking spaces
	}

	// TODO: The "SCI2" way of font drawing. Currently buggy
	/*
	for (int x = textX; x < entry->width; x++) {
		for (int y = textY; y < entry->height; y++) {
			byte pixel = entry->surface[y * entry->width + x];
			if (pixel)
				_screen->putPixel(x, y, 1, pixel, 0, 0);
		}
	}
	*/
}

TextEntry *GfxText32::getTextEntry(reg_t textObject) {
	uint32 textId = (textObject.segment << 16) | textObject.offset;

	if (!_textCache.contains(textId))
		createTextBitmap(textObject);

	return _textCache[textId];
}

// TODO: Finish this! Currently buggy.
TextEntry *GfxText32::createTextEntry(reg_t textObject) {
	reg_t stringObject = readSelector(_segMan, textObject, SELECTOR(text));

	// The object in the text selector of the item can be either a raw string
	// or a Str object. In the latter case, we need to access the object's data
	// selector to get the raw string.
	if (_segMan->isHeapObject(stringObject))
		stringObject = readSelector(_segMan, stringObject, SELECTOR(data));

	const char *text = _segMan->getString(stringObject).c_str();
	GfxFont *font = _cache->getFont(readSelectorValue(_segMan, textObject, SELECTOR(font)));
	bool dimmed = readSelectorValue(_segMan, textObject, SELECTOR(dimmed));
	uint16 foreColor = readSelectorValue(_segMan, textObject, SELECTOR(fore));
	uint16 x = readSelectorValue(_segMan, textObject, SELECTOR(x));
	uint16 y = readSelectorValue(_segMan, textObject, SELECTOR(y));

	// Now get the bounding box from the associated plane
	reg_t planeObject = readSelector(_segMan, textObject, SELECTOR(plane));
	Common::Rect planeRect;
	if (!planeObject.isNull()) {
		planeRect.top = readSelectorValue(_segMan, planeObject, SELECTOR(top));
		planeRect.left = readSelectorValue(_segMan, planeObject, SELECTOR(left));
		planeRect.bottom = readSelectorValue(_segMan, planeObject, SELECTOR(bottom)) + 1;
		planeRect.right = readSelectorValue(_segMan, planeObject, SELECTOR(right)) + 1;
	} else {
		planeRect.top = 0;
		planeRect.left = 0;
		planeRect.bottom = _screen->getHeight();
		planeRect.right = _screen->getWidth();
	}

	TextEntry *newEntry = new TextEntry();
	newEntry->object = stringObject;
	newEntry->x = x;
	newEntry->y = y;
	newEntry->width = planeRect.width();
	newEntry->height = planeRect.height();
	newEntry->surface = new byte[newEntry->width * newEntry->height];
	memset(newEntry->surface, 0, newEntry->width * newEntry->height);
	newEntry->text = _segMan->getString(stringObject);

	int16 maxTextWidth, charCount;
	uint16 curX = 0, curY = 0;

	maxTextWidth = 0;
	while (*text) {
		charCount = GetLongest(text, planeRect.width(), font);
		if (charCount == 0)
			break;

		for (int i = 0; i < charCount; i++) {
			unsigned char curChar = text[i];
			font->drawToBuffer(curChar, curY, curX, foreColor, dimmed, newEntry->surface, newEntry->width, newEntry->height);
			curX += font->getCharWidth(curChar);
		}

		curY += font->getHeight();
		text += charCount;
		while (*text == ' ')
			text++; // skip over breaking spaces
	}

	return newEntry;
}

int16 GfxText32::GetLongest(const char *text, int16 maxWidth, GfxFont *font) {
	uint16 curChar = 0;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;

	while (width <= maxWidth) {
		curChar = (*(const byte *)text++);

		switch (curChar) {
		// We need to add 0xD, 0xA and 0xD 0xA to curCharCount and then exit
		//  which means, we split text like
		//  'Mature, experienced software analyst available.' 0xD 0xA
		//  'Bug installation a proven speciality. "No version too clean."' (normal game text, this is from lsl2)
		//   and 0xA '-------' 0xA (which is the official sierra subtitle separator)
		//  Sierra did it the same way.
		case 0xD:
			// Check, if 0xA is following, if so include it as well
			if ((*(const unsigned char *)text) == 0xA)
				curCharCount++;
			// it's meant to pass through here
		case 0xA:
			curCharCount++;
			// and it's also meant to pass through here
		case 0:
			return curCharCount;
		case ' ':
			maxChars = curCharCount; // return count up to (but not including) breaking space
			break;
		}
		if (width + font->getCharWidth(curChar) > maxWidth)
			break;
		width += font->getCharWidth(curChar);
		curCharCount++;
	}

	return maxChars;
}

} // End of namespace Sci
