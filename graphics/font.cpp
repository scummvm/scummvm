/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 */

#include "common/stdafx.h"
#include "graphics/font.h"

namespace Graphics {

int NewFont::getCharWidth(byte chr) const {
	// If no width table is specified, return the maximum width
	if (!desc.width)
		return desc.maxwidth;
	// If this character is not included in the font, use the default char.
	if (chr < desc.firstchar || desc.firstchar + desc.size < chr) {
		chr = desc.defaultchar;
	}
	return desc.width[chr - desc.firstchar];
}

void NewFont::drawChar(Surface *dst, byte chr, int tx, int ty, uint32 color) const {
	assert(dst != 0);
	byte *ptr = (byte *)dst->getBasePtr(tx, ty);

	assert(desc.bits != 0 && desc.maxwidth <= 17);
	assert(dst->bytesPerPixel == 1 || dst->bytesPerPixel == 2);

	// If this character is not included in the font, use the default char.
	if (chr < desc.firstchar || chr >= desc.firstchar + desc.size) {
		chr = desc.defaultchar;
	}

	const int w = getCharWidth(chr);
	chr -= desc.firstchar;
	const bitmap_t *tmp = desc.bits + (desc.offset ? desc.offset[chr] : (chr * desc.height));

	for (int y = 0; y < desc.height; y++, ptr += dst->pitch) {
		const bitmap_t buffer = *tmp++;
		bitmap_t mask = 0x8000;
		if (ty + y < 0 || ty + y >= dst->h)
			continue;

		for (int x = 0; x < w; x++, mask >>= 1) {
			if (tx + x < 0 || tx + x >= dst->w)
				continue;
			if ((buffer & mask) != 0) {
				if (dst->bytesPerPixel == 1)
					ptr[x] = color;
				else if (dst->bytesPerPixel == 2)
					((uint16 *)ptr)[x] = color;
			}
		}
	}
}


#pragma mark -


int Font::getStringWidth(const Common::String &str) const {
	int space = 0;

	for (uint i = 0; i < str.size(); ++i)
		space += getCharWidth(str[i]);
	return space;
}

void Font::drawString(Surface *dst, const Common::String &s, int x, int y, int w, uint32 color, TextAlignment align, int deltax, bool useEllipsis) const {
	assert(dst != 0);
	const int leftX = x, rightX = x + w;
	uint i;
	int width = getStringWidth(s);
	Common::String str;

	if (useEllipsis && width > w) {
		// String is too wide. So we shorten it "intellegently", by replacing
		// parts of it by an ellipsis ("..."). There are three possibilities
		// for this: replace the start, the end, or the middle of the string.
		// What is best really depends on the context; but unless we want to
		// make this configurable, replacing the middle probably is a good
		// compromise.
		const int ellipsisWidth = getStringWidth("...");

		// SLOW algorithm to remove enough of the middle. But it is good enough
		// for now.
		const int halfWidth = (w - ellipsisWidth) / 2;
		int w2 = 0;

		for (i = 0; i < s.size(); ++i) {
			int charWidth = getCharWidth(s[i]);
			if (w2 + charWidth > halfWidth)
				break;
			w2 += charWidth;
			str += s[i];
		}
		// At this point we know that the first 'i' chars are together 'w2'
		// pixels wide. We took the first i-1, and add "..." to them.
		str += "...";

		// The original string is width wide. Of those we already skipped past
		// w2 pixels, which means (width - w2) remain.
		// The new str is (w2+ellipsisWidth) wide, so we can accomodate about
		// (w - (w2+ellipsisWidth)) more pixels.
		// Thus we skip ((width - w2) - (w - (w2+ellipsisWidth))) =
		// (width + ellipsisWidth - w)
		int skip = width + ellipsisWidth - w;
		for (; i < s.size() && skip > 0; ++i) {
			skip -= getCharWidth(s[i]);
		}

		// Append the remaining chars, if any
		for (; i < s.size(); ++i) {
			str += s[i];
		}

		width = getStringWidth(str);

	} else {
		str = s;
	}

	if (align == kTextAlignCenter)
		x = x + (w - width - 1)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;
	x += deltax;

	for (i = 0; i < str.size(); ++i) {
		w = getCharWidth(str[i]);
		if (x+w > rightX)
			break;
		if (x >= leftX)
			drawChar(dst, str[i], x, y, color);
		x += w;
	}
}


struct WordWrapper {
	Common::StringList &lines;
	int actualMaxLineWidth;

	WordWrapper(Common::StringList &l) : lines(l), actualMaxLineWidth(0) {
	}

	void add(Common::String &line, int &w) {
		if (actualMaxLineWidth < w)
			actualMaxLineWidth = w;

		lines.push_back(line);

		line.clear();
		w = 0;
	}
};

int Font::wordWrapText(const Common::String &str, int maxWidth, Common::StringList &lines) const {
	WordWrapper wrapper(lines);
	Common::String line;
	Common::String tmpStr;
	int lineWidth = 0;
	int tmpWidth = 0;

	// The rough idea behind this algorithm is as follows:
	// We accumulate characters into the string tmpStr. Whenever a full word
	// has been gathered together this way, we 'commit' it to the line buffer
	// 'line', i.e. we add tmpStr to the end of line, then clear it. Before
	// we do that, we check whether it would cause 'line' to exceed maxWidth;
	// in that case, we first add line to lines, then reset it.
	//
	// If a newline character is read, then we also add line to lines and clear it.
	//
	// Special care has to be taken to account for 'words' that exceed the width
	// of a line. If we encounter such a word, we have to wrap it over multiple
	// lines.

	for (Common::String::const_iterator x = str.begin(); x != str.end(); ++x) {
		const char c = *x;
		const int w = getCharWidth(c);
		const bool wouldExceedWidth = (lineWidth + tmpWidth + w > maxWidth);

		// If this char is a whitespace, then it represents a potential
		// 'wrap point' where wrapping could take place. Everything that
		// came before it can now safely be added to the line, as we know
		// that it will not have to be wrapped.
		if (isspace(c)) {
			line += tmpStr;
			lineWidth += tmpWidth;

			tmpStr.clear();
			tmpWidth = 0;

			// If we encounter a line break (\n), or if the new space would
			// cause the line to overflow: start a new line
			if (c == '\n' || wouldExceedWidth) {
				wrapper.add(line, lineWidth);
				continue;
			}
		}

		// If the max line width would be exceeded by adding this char,
		// insert a line break.
		if (wouldExceedWidth) {
			// Commit what we have so far, *if* we have anything.
			// If line is empty, then we are looking at a word
			// which exceeds the maximum line width.
			if (lineWidth > 0) {
				wrapper.add(line, lineWidth);
				// Trim left side
				while (tmpStr.size() && isspace(tmpStr[0])) {
					tmpWidth -= getCharWidth(tmpStr[0]);
					tmpStr.deleteChar(0);
				}
			} else {
				wrapper.add(tmpStr, tmpWidth);
			}
		}


		tmpWidth += w;
		tmpStr += c;
	}

	// If some text is left over, add it as the final line
	line += tmpStr;
	lineWidth += tmpWidth;
	if (lineWidth > 0) {
		wrapper.add(line, lineWidth);
	}
	return wrapper.actualMaxLineWidth;
}


} // End of namespace Graphics
