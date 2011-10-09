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

reg_t GfxText32::createTextBitmap(reg_t textObject, uint16 maxWidth, uint16 maxHeight) {
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

	_textCache[textId] = createTextEntry(textObject, maxWidth, maxHeight);

	// TODO: Create a new hunk pointer with the created surface
	return NULL_REG;
}

// TODO: Finish this!
void GfxText32::drawTextBitmap(reg_t textObject, uint16 textX, uint16 textY, uint16 planeWidth) {
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
	uint16 maxWidth = (planeWidth > 0) ? planeWidth : _screen->getWidth() - textX;

	while (*txt) {
		charCount = GetLongest(txt, maxWidth, font);
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
TextEntry *GfxText32::createTextEntry(reg_t textObject, uint16 maxWidth, uint16 maxHeight) {
	reg_t stringObject = readSelector(_segMan, textObject, SELECTOR(text));

	// TODO: maxWidth, maxHeight (if > 0)

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

	int16 /*maxTextWidth = 0,*/ charCount = 0;
	uint16 curX = 0, curY = 0;

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

void GfxText32::kernelTextSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, 0, 0);
	Size(rect, text, font, maxWidth);
	*textWidth = rect.width();
	*textHeight = rect.height();
}

void GfxText32::StringWidth(const char *str, GuiResourceId fontId, int16 &textWidth, int16 &textHeight) {
	Width(str, 0, (int16)strlen(str), fontId, textWidth, textHeight, true);
}

void GfxText32::Width(const char *text, int16 from, int16 len, GuiResourceId fontId, int16 &textWidth, int16 &textHeight, bool restoreFont) {
	uint16 curChar;
	textWidth = 0; textHeight = 0;

	GfxFont *font = _cache->getFont(fontId);

	if (font) {
		text += from;
		while (len--) {
			curChar = (*(const byte *)text++);
			switch (curChar) {
			case 0x0A:
			case 0x0D:
			case 0x9781: // this one is used by SQ4/japanese as line break as well
				textHeight = MAX<int16> (textHeight, font->getHeight());
				break;
			case 0x7C:
				warning("Code processing isn't implemented in SCI32");
				break;
			default:
				textHeight = MAX<int16> (textHeight, font->getHeight());
				textWidth += font->getCharWidth(curChar);
				break;
			}
		}
	}
}

int16 GfxText32::Size(Common::Rect &rect, const char *text, GuiResourceId fontId, int16 maxWidth) {
	int16 charCount;
	int16 maxTextWidth = 0, textWidth;
	int16 totalHeight = 0, textHeight;

	rect.top = rect.left = 0;
	GfxFont *font = _cache->getFont(fontId);

	if (maxWidth < 0) { // force output as single line
		StringWidth(text, fontId, textWidth, textHeight);
		rect.bottom = textHeight;
		rect.right = textWidth;
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxWidth ? maxWidth : 192);
		const char *curPos = text;
		while (*curPos) {
			charCount = GetLongest(curPos, rect.right, font);
			if (charCount == 0)
				break;
			Width(curPos, 0, charCount, fontId, textWidth, textHeight, false);
			maxTextWidth = MAX(textWidth, maxTextWidth);
			totalHeight += textHeight;
			curPos += charCount;
			while (*curPos == ' ')
				curPos++; // skip over breaking spaces
		}
		rect.bottom = totalHeight;
		rect.right = maxWidth ? maxWidth : MIN(rect.right, maxTextWidth);
	}
	return rect.right;
}

} // End of namespace Sci
