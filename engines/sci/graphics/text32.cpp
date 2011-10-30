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
#include "sci/graphics/compare.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {

#define BITMAP_HEADER_SIZE 46

#define SCI_TEXT32_ALIGNMENT_RIGHT -1
#define SCI_TEXT32_ALIGNMENT_CENTER 1
#define SCI_TEXT32_ALIGNMENT_LEFT	0

GfxText32::GfxText32(SegManager *segMan, GfxCache *fonts, GfxScreen *screen)
	: _segMan(segMan), _cache(fonts), _screen(screen) {
}

GfxText32::~GfxText32() {
}

reg_t GfxText32::createTextBitmap(reg_t textObject, uint16 maxWidth, uint16 maxHeight, reg_t prevHunk) {
	reg_t stringObject = readSelector(_segMan, textObject, SELECTOR(text));

	// The object in the text selector of the item can be either a raw string
	// or a Str object. In the latter case, we need to access the object's data
	// selector to get the raw string.
	if (_segMan->isHeapObject(stringObject))
		stringObject = readSelector(_segMan, stringObject, SELECTOR(data));

	Common::String text = _segMan->getString(stringObject);
	// HACK: The character offsets of the up and down arrow buttons are off by one
	// in GK1, for some unknown reason. Fix them here.
	if (text.size() == 1 && (text[0] == 29 || text[0] == 30)) {
		text.setChar(text[0] + 1, 0);
	}
	GuiResourceId fontId = readSelectorValue(_segMan, textObject, SELECTOR(font));
	GfxFont *font = _cache->getFont(fontId);
	bool dimmed = readSelectorValue(_segMan, textObject, SELECTOR(dimmed));
	int16 alignment = readSelectorValue(_segMan, textObject, SELECTOR(mode));
	uint16 foreColor = readSelectorValue(_segMan, textObject, SELECTOR(fore));
	uint16 backColor = readSelectorValue(_segMan, textObject, SELECTOR(back));

	Common::Rect nsRect = g_sci->_gfxCompare->getNSRect(textObject);
	uint16 width = nsRect.width() + 1;
	uint16 height = nsRect.height() + 1;

	// Limit rectangle dimensions, if requested
	if (maxWidth > 0)
		width = maxWidth;
	if (maxHeight > 0)
		height = maxHeight;

	// Upscale the coordinates/width if the fonts are already upscaled
	if (_screen->fontIsUpscaled()) {
		width = width * _screen->getDisplayWidth() / _screen->getWidth();
		height = height * _screen->getDisplayHeight() / _screen->getHeight();
	}

	int entrySize = width * height + BITMAP_HEADER_SIZE;
	reg_t memoryId = NULL_REG;
	if (prevHunk.isNull()) {
		memoryId = _segMan->allocateHunkEntry("TextBitmap()", entrySize);
		writeSelector(_segMan, textObject, SELECTOR(bitmap), memoryId);
	} else {
		memoryId = prevHunk;
	}
	byte *memoryPtr = _segMan->getHunkPointer(memoryId);

	if (prevHunk.isNull())
		memset(memoryPtr, 0, BITMAP_HEADER_SIZE);

	byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;
	memset(bitmap, backColor, width * height);

	// Save totalWidth, totalHeight
	WRITE_LE_UINT16(memoryPtr, width);
	WRITE_LE_UINT16(memoryPtr + 2, height);

	int16 charCount = 0;
	uint16 curX = 0, curY = 0;
	const char *txt = text.c_str();
	int16 textWidth, textHeight, totalHeight = 0, offsetX = 0, offsetY = 0;
	uint16 start = 0;

	// Calculate total text height
	while (*txt) {
		charCount = GetLongest(txt, width, font);
		if (charCount == 0)
			break;

		Width(txt, 0, (int16)strlen(txt), fontId, textWidth, textHeight, true);

		totalHeight += textHeight;
		txt += charCount;
		while (*txt == ' ')
			txt++; // skip over breaking spaces
	}

	txt = text.c_str();

	// Draw text in buffer
	while (*txt) {
		charCount = GetLongest(txt, width, font);
		if (charCount == 0)
			break;
		Width(txt, start, charCount, fontId, textWidth, textHeight, true);

		switch (alignment) {
		case SCI_TEXT32_ALIGNMENT_RIGHT:
			offsetX = width - textWidth;
			break;
		case SCI_TEXT32_ALIGNMENT_CENTER:
			// Center text both horizontally and vertically
			offsetX = (width - textWidth) / 2;
			offsetY = (height - totalHeight) / 2;
			break;
		case SCI_TEXT32_ALIGNMENT_LEFT:
			offsetX = 0;
			break;

		default:
			warning("Invalid alignment %d used in TextBox()", alignment);
		}

		for (int i = 0; i < charCount; i++) {
			unsigned char curChar = txt[i];
			font->drawToBuffer(curChar, curY + offsetY, curX + offsetX, foreColor, dimmed, bitmap, width, height);
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

void GfxText32::disposeTextBitmap(reg_t hunkId) {
	_segMan->freeHunkEntry(hunkId);
}

void GfxText32::drawTextBitmap(uint16 x, uint16 y, Common::Rect planeRect, reg_t textObject) {
	reg_t hunkId = readSelector(_segMan, textObject, SELECTOR(bitmap));
	// Sanity check: Check if the hunk is set. If not, either the game scripts
	// didn't set it, or an old saved game has been loaded, where it wasn't set.
	if (hunkId.isNull())
		return;

	byte *memoryPtr = _segMan->getHunkPointer(hunkId);

	if (!memoryPtr)
		error("Attempt to draw an invalid text bitmap");

	byte *surface = memoryPtr + BITMAP_HEADER_SIZE;

	int curByte = 0;
	uint16 skipColor = readSelectorValue(_segMan, textObject, SELECTOR(skip));
	uint16 textX = planeRect.left + x;
	uint16 textY = planeRect.top + y;
	// Get totalWidth, totalHeight
	uint16 width = READ_LE_UINT16(memoryPtr);
	uint16 height = READ_LE_UINT16(memoryPtr + 2);

	// Upscale the coordinates/width if the fonts are already upscaled
	if (_screen->fontIsUpscaled()) {
		textX = textX * _screen->getDisplayWidth() / _screen->getWidth();
		textY = textY * _screen->getDisplayHeight() / _screen->getHeight();
	}

	for (int curY = 0; curY < height; curY++) {
		for (int curX = 0; curX < width; curX++) {
			byte pixel = surface[curByte++];
			if (pixel != skipColor)
				_screen->putFontPixel(textY, curX + textX, curY, pixel);
		}
	}
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
