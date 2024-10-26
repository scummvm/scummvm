/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/font.h"
#include "graphics/managed_surface.h"

#include "common/array.h"
#include "common/util.h"

namespace Graphics {

int Font::getFontAscent() const {
	return -1;
}

int Font::getFontDescent() const {
	return -1;
}

int Font::getFontLeading() const {
	return -1;
}

int Font::getKerningOffset(uint32 left, uint32 right) const {
	return 0;
}

Common::Rect Font::getBoundingBox(uint32 chr) const {
	return Common::Rect(getCharWidth(chr), getFontHeight());
}

namespace {

template<class StringType>
Common::Rect getBoundingBoxImpl(const Font &font, const StringType &str, int x, int y, int w, TextAlign align, int deltax) {
	// We follow the logic of drawStringImpl here. The only exception is
	// that we do allow an empty width to be specified here. This allows us
	// to obtain the complete bounding box of a string.
	const int leftX = x, rightX = w ? (x + w + 1) : 0x7FFFFFFF;
	int width = font.getStringWidth(str);

	if (align == kTextAlignCenter)
		x = x + (w - width)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;
	x += deltax;

	bool first = true;
	Common::Rect bbox;

	typename StringType::unsigned_type last = 0;
	for (typename StringType::const_iterator i = str.begin(), end = str.end(); i != end; ++i) {
		const typename StringType::unsigned_type cur = *i;
		x += font.getKerningOffset(last, cur);
		last = cur;

		Common::Rect charBox = font.getBoundingBox(cur);
		if (x + charBox.right > rightX)
			break;
		if (x + charBox.right >= leftX) {
			charBox.translate(x, y);
			if (first) {
				bbox = charBox;
				first = false;
			} else {
				bbox.extend(charBox);
			}
		}

		x += font.getCharWidth(cur);
	}

	return bbox;
}

template<class StringType>
int getStringWidthImpl(const Font &font, const StringType &str) {
	int space = 0;
	typename StringType::unsigned_type last = 0;

	for (uint i = 0; i < str.size(); ++i) {
		const typename StringType::unsigned_type cur = str[i];
		space += font.getCharWidth(cur) + font.getKerningOffset(last, cur);
		last = cur;
	}

	return space;
}

template<class SurfaceType, class StringType>
void drawStringImpl(const Font &font, SurfaceType *dst, const StringType &str, int x, int y, int w, uint32 color, TextAlign align, int deltax) {
	// The logic in getBoundingImpl is the same as we use here. In case we
	// ever change something here we will need to change it there too.
	assert(dst != 0);

	const int leftX = x, rightX = x + w + 1;
	int width = font.getStringWidth(str);

	if (align == kTextAlignCenter)
		x = x + (w - width)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;
	x += deltax;

	typename StringType::unsigned_type last = 0;
	for (typename StringType::const_iterator i = str.begin(), end = str.end(); i != end; ++i) {
		const typename StringType::unsigned_type cur = *i;
		x += font.getKerningOffset(last, cur);
		last = cur;

		Common::Rect charBox = font.getBoundingBox(cur);
		if (x + charBox.right > rightX)
			break;
		if (x + charBox.right >= leftX)
			font.drawChar(dst, cur, x, y, color);

		x += font.getCharWidth(cur);
	}
}

template<class StringType>
struct WordWrapper {
	Common::Array<StringType> &lines;
	Common::Array<bool> &lineContinuation;
	int actualMaxLineWidth;

	WordWrapper(Common::Array<StringType> &l, Common::Array<bool> &lC) : lines(l), lineContinuation(lC), actualMaxLineWidth(0) {
	}

	void add(StringType &line, bool &wordContinuation, int &w) {
		if (actualMaxLineWidth < w)
			actualMaxLineWidth = w;

		lines.push_back(line);
		lineContinuation.push_back(wordContinuation);

		line.clear();
		wordContinuation = false;
		w = 0;
	}

	void clear() {
		lines.clear();
		lineContinuation.clear();
		actualMaxLineWidth = 0;
	}
};

template<class StringType>
int wordWrapTextImpl(const Font &font, const StringType &str, int maxWidth, Common::Array<StringType> &lines, Common::Array<bool> &lineContinuation, int initWidth, uint32 mode) {
	WordWrapper<StringType> wrapper(lines, lineContinuation);
	StringType line;
	StringType tmpStr;
	bool wordContinuation = false;
	int lineWidth = initWidth;
	int tmpWidth = 0;
	int fullTextWidthEWL = initWidth; // this replaces new line characters (if any) with single spaces - it is used in Even Width Lines mode

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

	typename StringType::unsigned_type last = 0;

	// When EvenWidthLines mode is enabled then we require an early loop over the entire string
	// in order to get the full width of the text
	//
	// If both "Wrap On Explicit New Lines" and "Even Width Lines" modes are set,
	// and there are new line characters in the text,
	// then "Wrap On Explicit New Lines" takes precedence and "Even Width Lines" is ignored for that text.
	// However, if both are set, but there are no new lines in the text,
	// then the "Even Width Lines" auto-wrapping is applied.
	//
	if (mode & kWordWrapEvenWidthLines) {
		// Early loop to get the full width of the text
		for (typename StringType::const_iterator x = str.begin(); x != str.end(); ++x) {
			typename StringType::unsigned_type c = *x;

			// Check for Windows and Mac line breaks
			if (c == '\r') {
				if (x != str.end() && *(x + 1) == '\n') {
					++x;
				}
				c = '\n';
			}

			if (c == '\n') {
				if (!(mode & kWordWrapOnExplicitNewLines)) {
					c = ' ';
				} else {
					mode &= ~kWordWrapEvenWidthLines;
					break;
				}
			}

			const int w = font.getCharWidth(c) + font.getKerningOffset(last, c);
			last = c;
			fullTextWidthEWL += w;
		}
	}

	int targetTotalLinesNumberEWL = 0;
	int targetMaxLineWidth = 0;
	do {
		if (mode & kWordWrapEvenWidthLines) {
			wrapper.clear();
			targetTotalLinesNumberEWL += 1;
			// We add +2 to the fullTextWidthEWL to account for possible shadow pixels
			// We add +10 * font.getCharWidth(' ') to the quotient since we want to allow some extra margin (about an extra wprd's length)
			// since that yields better looking results
			targetMaxLineWidth = ((fullTextWidthEWL + 2) / targetTotalLinesNumberEWL) + 10 * font.getCharWidth(' ');
			if (targetMaxLineWidth > maxWidth) {
				// repeat the loop with increased targetTotalLinesNumberEWL
				continue;
			}
		} else {
			targetMaxLineWidth = maxWidth;
		}

		last = 0;
		tmpWidth = 0;

		for (typename StringType::const_iterator x = str.begin(); x != str.end(); ++x) {
			typename StringType::unsigned_type c = *x;

			// Convert Windows and Mac line breaks into plain \n
			if (c == '\r') {
				if (x != str.end() && *(x + 1) == '\n') {
					++x;
				}
				c = '\n';
			}
			// if wrapping on explicit new lines is disabled, then new line characters should be treated as a single white space char
			if (!(mode & kWordWrapOnExplicitNewLines) && c == '\n')  {
				c = ' ';
			}

			const int currentCharWidth = font.getCharWidth(c);
			const int w = currentCharWidth + font.getKerningOffset(last, c);
			last = c;
			const bool wouldExceedWidth =
				(lineWidth + tmpWidth + w > targetMaxLineWidth) &&
				!(mode & kWordWrapAllowTrailingWhitespace && Common::isSpace(c));

			// If this char is a whitespace, then it represents a potential
			// 'wrap point' where wrapping could take place. Everything that
			// came before it can now safely be added to the line, as we know
			// that it will not have to be wrapped.
			if (Common::isSpace(c)) {
				line += tmpStr;
				lineWidth += tmpWidth;

				tmpStr.clear();
				tmpWidth = 0;

				// If we encounter a line break (\n), or if the new space would
				// cause the line to overflow: start a new line
				if (((mode & kWordWrapOnExplicitNewLines) && c == '\n') || wouldExceedWidth) {
					wrapper.add(line, wordContinuation, lineWidth);
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
					wrapper.add(line, wordContinuation, lineWidth);
					// Trim left side
					while (tmpStr.size() && Common::isSpace(tmpStr[0])) {
						tmpStr.deleteChar(0);
						// This is not very fast, but it is the simplest way to
						// assure we do not mess something up because of kerning.
						tmpWidth = font.getStringWidth(tmpStr);
					}

					if (tmpStr.empty()) {
						// If tmpStr is empty, we might have removed the space before 'c'.
						// That means we have to recompute the kerning.

						tmpWidth += currentCharWidth + font.getKerningOffset(0, c);
						tmpStr += c;
						continue;
					}
				} else {
					wordContinuation = true;
					wrapper.add(tmpStr, wordContinuation, tmpWidth);
				}
			}

			tmpWidth += w;
			tmpStr += c;
		}

		// If some text is left over, add it as the final line
		line += tmpStr;
		lineWidth += tmpWidth;
		if (lineWidth > 0) {
			wrapper.add(line, wordContinuation, lineWidth);
		}
	} while ((mode & kWordWrapEvenWidthLines)
	         && (targetMaxLineWidth > maxWidth));
	return wrapper.actualMaxLineWidth;
}

template<typename StringType>
StringType handleEllipsis(const Font &font, const StringType &input, int w) {
	StringType s = input;
	int width = font.getStringWidth(s);
	bool hasEllipsisAtEnd = false;

	if (s.size() > 3 && s[s.size() - 1] == '.' && s[s.size() - 2] == '.' && s[s.size() - 3] == '.') {
		hasEllipsisAtEnd = true;
	}

	if (width > w && hasEllipsisAtEnd) {
		// String is too wide. Check whether it ends in an ellipsis
		// ("..."). If so, remove that and try again!
		s.deleteLastChar();
		s.deleteLastChar();
		s.deleteLastChar();
		width = font.getStringWidth(s);
	}

	if (width > w) {
		StringType str;
		StringType ellipsis("...");

		// String is too wide. So we shorten it "intelligently" by
		// replacing parts of the string by an ellipsis. There are
		// three possibilities for this: replace the start, the end, or
		// the middle of the string. What is best really depends on the
		// context; but unless we want to make this configurable,
		// replacing the middle seems to be a good compromise.

		const int ellipsisWidth = font.getStringWidth(ellipsis);

		// SLOW algorithm to remove enough of the middle. But it is good enough
		// for now.
		const int halfWidth = (w - ellipsisWidth) / 2;
		int w2 = 0;
		typename StringType::unsigned_type last = 0;
		uint i = 0;

		for (; i < s.size(); ++i) {
			const typename StringType::unsigned_type cur = s[i];
			int charWidth = font.getCharWidth(cur) + font.getKerningOffset(last, cur);
			if (w2 + charWidth > halfWidth)
				break;
			last = cur;
			w2 += charWidth;
			str += cur;
		}

		// At this point we know that the first 'i' chars are together 'w2'
		// pixels wide. We took the first i-1, and add "..." to them.
		str += ellipsis;
		last = '.';

		// The original string is width wide. Of those we already skipped past
		// w2 pixels, which means (width - w2) remain.
		// The new str is (w2+ellipsisWidth) wide, so we can accommodate about
		// (w - (w2+ellipsisWidth)) more pixels.
		// Thus we skip ((width - w2) - (w - (w2+ellipsisWidth))) =
		// (width + ellipsisWidth - w)
		int skip = width + ellipsisWidth - w;
		for (; i < s.size() && skip > 0; ++i) {
			const typename StringType::unsigned_type cur = s[i];
			skip -= font.getCharWidth(cur) + font.getKerningOffset(last, cur);
			last = cur;
		}

		// Append the remaining chars, if any
		for (; i < s.size(); ++i) {
			str += s[i];
		}

		return str;
	}

	return s;
}

} // End of anonymous namespace

Common::Rect Font::getBoundingBox(const Common::String &input, int x, int y, const int w, TextAlign align, int deltax, bool useEllipsis) const {
	// In case no width was given we cannot use ellipsis or any alignment
	// apart from left alignment.
	if (w == 0) {
		if (useEllipsis) {
			warning("Font::getBoundingBox: Requested ellipsis when no width was specified");
		}

		if (align != kTextAlignLeft) {
			warning("Font::getBoundingBox: Requested text alignment when no width was specified");
		}

		useEllipsis = false;
		align = kTextAlignLeft;
	}

	const Common::String str = useEllipsis ? handleEllipsis(*this, input, w) : input;
	return getBoundingBoxImpl(*this, str, x, y, w, align, deltax);
}

Common::Rect Font::getBoundingBox(const Common::U32String &input, int x, int y, const int w, TextAlign align, int deltax, bool useEllipsis) const {
	// In case no width was given we cannot any alignment apart from left
	// alignment.
	if (w == 0) {
		if (useEllipsis) {
			warning("Font::getBoundingBox: Requested ellipsis when no width was specified");
		}

		if (align != kTextAlignLeft) {
			warning("Font::getBoundingBox: Requested text alignment when no width was specified");
		}

		useEllipsis = false;
		align = kTextAlignLeft;
	}

	const Common::U32String str = useEllipsis ? handleEllipsis(*this, input, w) : input;
	return getBoundingBoxImpl(*this, str, x, y, w, align, 0);
}

int Font::getStringWidth(const Common::String &str) const {
	return getStringWidthImpl(*this, str);
}

int Font::getStringWidth(const Common::U32String &str) const {
	return getStringWidthImpl(*this, str);
}

void Font::drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const {
	drawChar(dst->surfacePtr(), chr, x, y, color);

	Common::Rect charBox = getBoundingBox(chr);
	charBox.translate(x, y);
	dst->addDirtyRect(charBox);
}

void Font::drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) const {
	Common::String renderStr = useEllipsis ? handleEllipsis(*this, str, w) : str;
	drawStringImpl(*this, dst, renderStr, x, y, w, color, align, deltax);
}

void Font::drawString(Surface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) const {
	Common::U32String renderStr = useEllipsis ? handleEllipsis(*this, str, w) : str;
	drawStringImpl(*this, dst, renderStr, x, y, w, color, align, deltax);
}

void Font::drawString(ManagedSurface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) const {
	Common::String renderStr = useEllipsis ? handleEllipsis(*this, str, w) : str;
	drawStringImpl(*this, dst, renderStr, x, y, w, color, align, deltax);

	if (w != 0) {
		dst->addDirtyRect(getBoundingBox(str, x, y, w, align, deltax, useEllipsis));
	}
}

void Font::drawString(ManagedSurface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) const {
	Common::U32String renderStr = useEllipsis ? handleEllipsis(*this, str, w) : str;
	drawStringImpl(*this, dst, renderStr, x, y, w, color, align, deltax);

	if (w != 0) {
		dst->addDirtyRect(getBoundingBox(str, x, y, w, align, useEllipsis));
	}
}

int Font::wordWrapText(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth, uint32 mode) const {
	Common::Array<bool> dummyLineContinuation;
	return wordWrapTextImpl(*this, str, maxWidth, lines, dummyLineContinuation, initWidth, mode);
}

int Font::wordWrapText(const Common::U32String &str, int maxWidth, Common::Array<Common::U32String> &lines, int initWidth, uint32 mode) const {
	Common::Array<bool> dummyLineContinuation;
	return wordWrapTextImpl(*this, str, maxWidth, lines, dummyLineContinuation, initWidth, mode);
}

int Font::wordWrapText(const Common::U32String &str, int maxWidth, Common::Array<Common::U32String> &lines, Common::Array<bool> &lineContinuation, int initWidth, uint32 mode) const {
	return wordWrapTextImpl(*this, str, maxWidth, lines, lineContinuation, initWidth, mode);
}

TextAlign convertTextAlignH(TextAlign alignH, bool rtl) {
	switch (alignH) {
	case kTextAlignStart:
		return rtl ? kTextAlignRight : kTextAlignLeft;
	case kTextAlignEnd:
		return rtl ? kTextAlignLeft : kTextAlignRight;
	default:
		return alignH;
	}
}

#define wholedivide(x, y)	(((x)+((y)-1))/(y))

static void countupScore(int *dstGray, int x, int y, int bbw, int bbh, float scale) {
	int newbbw = bbw * scale;
	int newbbh = bbh * scale;
	int x_ = x * newbbw;
	int y_ = y * newbbh;
	int x1 = x_ + newbbw;
	int y1 = y_ + newbbh;

	int newxbegin = x_ / bbw;
	int newybegin = y_ / bbh;
	int newxend = wholedivide(x1, bbw);
	int newyend = wholedivide(y1, bbh);

	for (int newy = newybegin; newy < newyend; newy++) {
		for (int newx = newxbegin; newx < newxend; newx++) {
			int newX = newx * bbw;
			int newY = newy * bbh;
			int newX1 = newX + bbw;
			int newY1 = newY + bbh;
			dstGray[newy * newbbw + newx] += (MIN(x1, newX1) - MAX(x_, newX)) *
											 (MIN(y1, newY1) - MAX(y_, newY));
		}
	}
}

static void magnifyGray(Surface *src, int *dstGray, int width, int height, float scale) {
	for (uint16 y = 0; y < height; y++) {
		for (uint16 x = 0; x < width; x++) {
			if (*((byte *)src->getBasePtr(x, y)) == 1)
				countupScore(dstGray, x, y, width, height, scale);
		}
	}
}

void Font::scaleSingleGlyph(Surface *scaleSurface, int *grayScaleMap, int grayScaleMapSize, int width, int height, int xOffset, int yOffset, int grayLevel, int chr, int srcheight, int srcwidth, float scale) const {

	scaleSurface->fillRect(Common::Rect(scaleSurface->w, scaleSurface->h), 0);
	drawChar(scaleSurface, chr, xOffset, yOffset, 1);
	memset(grayScaleMap, 0, grayScaleMapSize * sizeof(int));
	magnifyGray(scaleSurface, grayScaleMap, srcwidth, srcheight, scale);
	int *grayPtr = grayScaleMap;
	for (int y = 0; y < height; y++) {
		byte *dst = (byte *)scaleSurface->getBasePtr(0, y);
		for (int x = 0; x < width; x++, grayPtr++, dst++) {
			if (*grayPtr > grayLevel)
				*dst = 1;
			else
				*dst = 0;
		}
	}

}

} // End of namespace Graphics
