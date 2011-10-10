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
}

reg_t GfxText32::createTextBitmap(reg_t textObject, uint16 maxWidth, uint16 maxHeight) {
	reg_t stringObject = readSelector(_segMan, textObject, SELECTOR(text));

	// The object in the text selector of the item can be either a raw string
	// or a Str object. In the latter case, we need to access the object's data
	// selector to get the raw string.
	if (_segMan->isHeapObject(stringObject))
		stringObject = readSelector(_segMan, stringObject, SELECTOR(data));

	Common::String text = _segMan->getString(stringObject);
	GfxFont *font = _cache->getFont(readSelectorValue(_segMan, textObject, SELECTOR(font)));
	bool dimmed = readSelectorValue(_segMan, textObject, SELECTOR(dimmed));
	uint16 foreColor = readSelectorValue(_segMan, textObject, SELECTOR(fore));

	Common::Rect planeRect = getPlaneRect(textObject);
	uint16 width = planeRect.width();
	uint16 height = planeRect.height();

	// Limit rectangle dimensions, if requested
	if (maxWidth > 0)
		width = maxWidth;
	if (maxHeight > 0)
		height = maxHeight;

	int entrySize = width * height;
	reg_t memoryId = _segMan->allocateHunkEntry("TextBitmap()", entrySize);
	writeSelector(_segMan, textObject, SELECTOR(bitmap), memoryId);
	byte *memoryPtr = _segMan->getHunkPointer(memoryId);
	memset(memoryPtr, 0, entrySize);

	int16 charCount = 0;
	uint16 curX = 0, curY = 0;
	const char *txt = text.c_str();

	while (*txt) {
		charCount = GetLongest(txt, width, font);
		if (charCount == 0)
			break;

		for (int i = 0; i < charCount; i++) {
			unsigned char curChar = txt[i];
			font->drawToBuffer(curChar, curY, curX, foreColor, dimmed, memoryPtr, width, height);
			curX += font->getCharWidth(curChar);
		}

		curX = 0;
		curY += font->getHeight();
		txt += charCount;
		while (*txt == ' ')
			txt++; // skip over breaking spaces
	}

	return memoryId;
}

reg_t GfxText32::createTextBitmapSci21(uint16 width, uint16 height, byte skip, byte back, uint16 width2, uint16 height2, byte transparentFlag) {
	// TODO: skip, width2, height2, transparentFlag
	int entrySize = width * height;
	reg_t memoryId = _segMan->allocateHunkEntry("TextBitmap()", entrySize);
	byte *memoryPtr = _segMan->getHunkPointer(memoryId);
	memset(memoryPtr, back, entrySize);
	return memoryId;
}

void GfxText32::disposeTextBitmap(reg_t hunkId) {
	_segMan->freeHunkEntry(hunkId);
}

void GfxText32::drawTextBitmap(reg_t textObject) {
	reg_t hunkId = readSelector(_segMan, textObject, SELECTOR(bitmap));
	byte *surface = _segMan->getHunkPointer(hunkId);

	if (!surface)
		error("Attempt to draw an invalid text bitmap");

	int curByte = 0;
	Common::Rect nsRect = getNSRect(textObject);
	Common::Rect planeRect = getPlaneRect(textObject);
	uint16 x = readSelectorValue(_segMan, textObject, SELECTOR(x));
	uint16 y = readSelectorValue(_segMan, textObject, SELECTOR(y));
	uint16 textX = planeRect.left + x;
	uint16 textY = planeRect.top + y;
	uint16 width = nsRect.width();
	uint16 height = nsRect.height();

	// Upscale the coordinates/width if the fonts are already upscaled
	if (_screen->fontIsUpscaled()) {
		textX = textX * _screen->getDisplayWidth() / _screen->getWidth();
		textY = textY * _screen->getDisplayHeight() / _screen->getHeight();
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			byte pixel = surface[curByte++];
			if (pixel)
				_screen->putFontPixel(textY, x + textX, y, pixel);
		}
	}
}

Common::Rect GfxText32::getPlaneRect(reg_t textObject) {
	Common::Rect planeRect(0, 0, _screen->getWidth(), _screen->getHeight());

	reg_t planeObject = readSelector(_segMan, textObject, SELECTOR(plane));
	if (!planeObject.isNull()) {
		planeRect.top = readSelectorValue(_segMan, planeObject, SELECTOR(top));
		planeRect.left = readSelectorValue(_segMan, planeObject, SELECTOR(left));
		planeRect.bottom = readSelectorValue(_segMan, planeObject, SELECTOR(bottom)) + 1;
		planeRect.right = readSelectorValue(_segMan, planeObject, SELECTOR(right)) + 1;
	}

	return planeRect;
}

Common::Rect GfxText32::getNSRect(reg_t textObject) {
	Common::Rect nsRect;
	nsRect.top = readSelectorValue(_segMan, textObject, SELECTOR(nsTop));
	nsRect.left = readSelectorValue(_segMan, textObject, SELECTOR(nsLeft));
	nsRect.bottom = readSelectorValue(_segMan, textObject, SELECTOR(nsBottom)) + 1;
	nsRect.right = readSelectorValue(_segMan, textObject, SELECTOR(nsRight)) + 1;
	return nsRect;
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
