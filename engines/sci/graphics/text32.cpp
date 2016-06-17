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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/font.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {

int16 GfxText32::_defaultFontId = 0;
int16 GfxText32::_scaledWidth = 0;
int16 GfxText32::_scaledHeight = 0;

GfxText32::GfxText32(SegManager *segMan, GfxCache *fonts) :
	_segMan(segMan),
	_cache(fonts),
	// Not a typo, the original engine did not initialise height, only width
	_width(0),
	_text(""),
	_bitmap(NULL_REG) {
		_fontId = _defaultFontId;
		_font = _cache->getFont(_defaultFontId);

		if (_scaledWidth == 0) {
			// initialize the statics
			_scaledWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
			_scaledHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		}
	}

reg_t GfxText32::createFontBitmap(int16 width, int16 height, const Common::Rect &rect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, const TextAlign alignment, const int16 borderColor, const bool dimmed, const bool doScaling) {

	_borderColor = borderColor;
	_text = text;
	_textRect = rect;
	_width = width;
	_height = height;
	_foreColor = foreColor;
	_backColor = backColor;
	_skipColor = skipColor;
	_alignment = alignment;
	_dimmed = dimmed;

	setFont(fontId);

	if (doScaling) {
		int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

		Ratio scaleX(_scaledWidth, scriptWidth);
		Ratio scaleY(_scaledHeight, scriptHeight);

		_width = (_width * scaleX).toInt();
		_height = (_height * scaleY).toInt();
		mulinc(_textRect, scaleX, scaleY);
	}

	// _textRect represents where text is drawn inside the
	// bitmap; clipRect is the entire bitmap
	Common::Rect bitmapRect(_width, _height);

	if (_textRect.intersects(bitmapRect)) {
		_textRect.clip(bitmapRect);
	} else {
		_textRect = Common::Rect();
	}

	BitmapResource bitmap(_segMan, _width, _height, _skipColor, 0, 0, _scaledWidth, _scaledHeight, 0, false);
	_bitmap = bitmap.getObject();

	erase(bitmapRect, false);

	if (_borderColor > -1) {
		drawFrame(bitmapRect, 1, _borderColor, false);
	}

	drawTextBox();
	return _bitmap;
}

reg_t GfxText32::createFontBitmap(const CelInfo32 &celInfo, const Common::Rect &rect, const Common::String &text, const int16 foreColor, const int16 backColor, const GuiResourceId fontId, const int16 skipColor, const int16 borderColor, const bool dimmed) {
	_borderColor = borderColor;
	_text = text;
	_textRect = rect;
	_foreColor = foreColor;
	_dimmed = dimmed;

	setFont(fontId);

	int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	int borderSize = 1;
	mulinc(_textRect, Ratio(_scaledWidth, scriptWidth), Ratio(_scaledHeight, scriptHeight));

	CelObjView view(celInfo.resourceId, celInfo.loopNo, celInfo.celNo);
	_skipColor = view._transparentColor;
	_width = view._width * _scaledWidth / view._scaledWidth;
	_height = view._height * _scaledHeight / view._scaledHeight;

	Common::Rect bitmapRect(_width, _height);
	if (_textRect.intersects(bitmapRect)) {
		_textRect.clip(bitmapRect);
	} else {
		_textRect = Common::Rect();
	}

	BitmapResource bitmap(_segMan, _width, _height, _skipColor, 0, 0, _scaledWidth, _scaledHeight, 0, false);
	_bitmap = bitmap.getObject();
	Buffer buffer(_width, _height, bitmap.getPixels());

	// NOTE: The engine filled the bitmap pixels with 11 here, which is silly
	// because then it just erased the bitmap using the skip color. So we don't
	// fill the bitmap redundantly here.

	_backColor = _skipColor;
	erase(bitmapRect, false);
	_backColor = backColor;

	view.draw(buffer, bitmapRect, Common::Point(0, 0), false, Ratio(_scaledWidth, view._scaledWidth), Ratio(_scaledHeight, view._scaledHeight));

	if (_backColor != skipColor && _foreColor != skipColor) {
		erase(_textRect, false);
	}

	if (text.size() > 0) {
		if (_foreColor == skipColor) {
			error("TODO: Implement transparent text");
		} else {
			if (borderColor != -1) {
				drawFrame(bitmapRect, borderSize, _borderColor, false);
			}

			drawTextBox();
		}
	}

	return _bitmap;
}

void GfxText32::setFont(const GuiResourceId fontId) {
	// NOTE: In SCI engine this calls FontMgr::BuildFontTable and then a font
	// table is built on the FontMgr directly; instead, because we already have
	// font resources, this code just grabs a font out of GfxCache.
	if (fontId != _fontId) {
		_fontId = fontId == -1 ? _defaultFontId : fontId;
		_font = _cache->getFont(_fontId);
	}
}

void GfxText32::drawFrame(const Common::Rect &rect, const int16 size, const uint8 color, const bool doScaling) {
	Common::Rect targetRect = doScaling ? scaleRect(rect) : rect;

	byte *bitmap = _segMan->getHunkPointer(_bitmap);
	byte *pixels = bitmap + READ_SCI11ENDIAN_UINT32(bitmap + 28) + rect.top * _width + rect.left;

	// NOTE: Not fully disassembled, but this should be right
	int16 rectWidth = targetRect.width();
	int16 sidesHeight = targetRect.height() - size * 2;
	int16 centerWidth = rectWidth - size * 2;
	int16 stride = _width - rectWidth;

	for (int16 y = 0; y < size; ++y) {
		memset(pixels, color, rectWidth);
		pixels += _width;
	}
	for (int16 y = 0; y < sidesHeight; ++y) {
		for (int16 x = 0; x < size; ++x) {
			*pixels++ = color;
		}
		pixels += centerWidth;
		for (int16 x = 0; x < size; ++x) {
			*pixels++ = color;
		}
		pixels += stride;
	}
	for (int16 y = 0; y < size; ++y) {
		memset(pixels, color, rectWidth);
		pixels += _width;
	}
}

void GfxText32::drawChar(const char charIndex) {
	byte *bitmap = _segMan->getHunkPointer(_bitmap);
	byte *pixels = bitmap + READ_SCI11ENDIAN_UINT32(bitmap + 28);

	_font->drawToBuffer(charIndex, _drawPosition.y, _drawPosition.x, _foreColor, _dimmed, pixels, _width, _height);
	_drawPosition.x += _font->getCharWidth(charIndex);
}

uint16 GfxText32::getCharWidth(const char charIndex, const bool doScaling) const {
	uint16 width = _font->getCharWidth(charIndex);
	if (doScaling) {
		width = scaleUpWidth(width);
	}
	return width;
}

void GfxText32::drawTextBox() {
	if (_text.size() == 0) {
		return;
	}

	const char *text = _text.c_str();
	const char *sourceText = text;
	int16 textRectWidth = _textRect.width();
	_drawPosition.y = _textRect.top;
	uint charIndex = 0;
	if (g_sci->getGameId() != GID_PHANTASMAGORIA) {
		if (getLongest(&charIndex, textRectWidth) == 0) {
			error("DrawTextBox GetLongest=0");
		}
	}

	charIndex = 0;
	uint nextCharIndex = 0;
	while (*text != '\0') {
		_drawPosition.x = _textRect.left;

		uint length = getLongest(&nextCharIndex, textRectWidth);
		int16 textWidth = getTextWidth(charIndex, length);

		if (_alignment == kTextAlignCenter) {
			_drawPosition.x += (textRectWidth - textWidth) / 2;
		} else if (_alignment == kTextAlignRight) {
			_drawPosition.x += textRectWidth - textWidth;
		}

		drawText(charIndex, length);
		charIndex = nextCharIndex;
		text = sourceText + charIndex;
		_drawPosition.y += _font->getHeight();
	}
}

void GfxText32::drawTextBox(const Common::String &text) {
	_text = text;
	drawTextBox();
}

void GfxText32::drawText(const uint index, uint length) {
	assert(index + length <= _text.size());

	// NOTE: This draw loop implementation is somewhat different than the
	// implementation in the actual engine, but should be accurate. Primarily
	// the changes revolve around eliminating some extra temporaries and
	// fixing the logic to match.
	const char *text = _text.c_str() + index;
	while (length-- > 0) {
		char currentChar = *text++;

		if (currentChar == '|') {
			const char controlChar = *text++;
			--length;

			if (length == 0) {
				return;
			}

			if (controlChar == 'a' || controlChar == 'c' || controlChar == 'f') {
				uint16 value = 0;

				while (length > 0) {
					const char valueChar = *text;
					if (valueChar < '0' || valueChar > '9') {
						break;
					}

					++text;
					--length;
					value = 10 * value + (valueChar - '0');
				}

				if (length == 0) {
					return;
				}

				if (controlChar == 'a') {
					_alignment = (TextAlign)value;
				} else if (controlChar == 'c') {
					_foreColor = value;
				} else if (controlChar == 'f') {
					setFont(value);
				}
			}

			while (length > 0 && *text != '|') {
				++text;
				--length;
			}
			if (length > 0) {
				++text;
				--length;
			}
		} else {
			drawChar(currentChar);
		}
	}
}

void GfxText32::invertRect(const reg_t bitmap, int16 bitmapStride, const Common::Rect &rect, const uint8 foreColor, const uint8 backColor, const bool doScaling) {
	Common::Rect targetRect = rect;
	if (doScaling) {
		bitmapStride = bitmapStride * _scaledWidth / g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		targetRect = scaleRect(rect);
	}

	byte *bitmapData = _segMan->getHunkPointer(bitmap);

	// NOTE: SCI code is super weird here; it seems to be trying to look at the
	// entire size of the bitmap including the header, instead of just the pixel
	// data size. We just look at the pixel size. This function generally is an
	// odd duck since the stride dimension for a bitmap is built in to the bitmap
	// header, so perhaps it was once an unheadered bitmap format and this
	// function was never updated to match? Or maybe they exploit the
	// configurable stride length somewhere else to do stair stepping inverts...
	uint32 invertSize = targetRect.height() * bitmapStride + targetRect.width();
	uint32 bitmapSize = READ_SCI11ENDIAN_UINT32(bitmapData + 12);

	if (invertSize >= bitmapSize) {
		error("InvertRect too big: %u >= %u", invertSize, bitmapSize);
	}

	// NOTE: Actual engine just added the bitmap header size hardcoded here
	byte *pixel = bitmapData + READ_SCI11ENDIAN_UINT32(bitmapData + 28) + bitmapStride * targetRect.top + targetRect.left;

	int16 stride = bitmapStride - targetRect.width();
	int16 targetHeight = targetRect.height();
	int16 targetWidth = targetRect.width();

	for (int16 y = 0; y < targetHeight; ++y) {
		for (int16 x = 0; x < targetWidth; ++x) {
			if (*pixel == foreColor) {
				*pixel = backColor;
			} else if (*pixel == backColor) {
				*pixel = foreColor;
			}

			++pixel;
		}

		pixel += stride;
	}
}

uint GfxText32::getLongest(uint *charIndex, const int16 width) {
	assert(width > 0);

	uint testLength = 0;
	uint length = 0;

	const uint initialCharIndex = *charIndex;

	// The index of the next word after the last word break
	uint lastWordBreakIndex = *charIndex;

	const char *text = _text.c_str() + *charIndex;

	char currentChar;
	while ((currentChar = *text++) != '\0') {
		// NOTE: In the original engine, the font, color, and alignment were
		// reset here to their initial values

		// The text to render contains a line break; stop at the line break
		if (currentChar == '\r' || currentChar == '\n') {
			// Skip the rest of the line break if it is a Windows-style
			// \r\n or non-standard \n\r
			// NOTE: In the original engine, the `text` pointer had not been
			// advanced yet so the indexes used to access characters were
			// one higher
			if (
				(currentChar == '\r' && text[0] == '\n') ||
				(currentChar == '\n' && text[0] == '\r' && text[1] != '\n')
			) {
				++*charIndex;
			}

			// We are at the end of a line but the last word in the line made
			// it too wide to fit in the text area; return up to the previous
			// word
			if (length && getTextWidth(initialCharIndex, testLength) > width) {
				*charIndex = lastWordBreakIndex;
				return length;
			}

			// Skip the line break and return all text seen up to now
			// NOTE: In original engine, the font, color, and alignment were
			// reset, then getTextWidth was called to use its side-effects to
			// set font, color, and alignment according to the text from
			// `initialCharIndex` to `testLength`
			++*charIndex;
			return testLength;
		} else if (currentChar == ' ') {
			// The last word in the line made it too wide to fit in the text area;
			// return up to the previous word, then collapse the whitespace
			// between that word and its next sibling word into the line break
			if (getTextWidth(initialCharIndex, testLength) > width) {
				*charIndex = lastWordBreakIndex;
				const char *nextChar = _text.c_str() + lastWordBreakIndex;
				while (*nextChar++ == ' ') {
					++*charIndex;
				}

				// NOTE: In original engine, the font, color, and alignment were
				// set here to the values that were seen at the last space character
				return length;
			}

			// NOTE: In the original engine, the values of _fontId, _foreColor,
			// and _alignment were stored for use in the return path mentioned
			// just above here

			// We found a word break that was within the text area, memorise it
			// and continue processing. +1 on the character index because it has
			// not been incremented yet so currently points to the word break
			// and not the word after the break
			length = testLength;
			lastWordBreakIndex = *charIndex + 1;
		}

		// In the middle of a line, keep processing
		++*charIndex;
		++testLength;

		// NOTE: In the original engine, the font, color, and alignment were
		// reset here to their initial values

		// The text to render contained no word breaks yet but is already too
		// wide for the text area; just split the word in half at the point
		// where it overflows
		if (length == 0 && getTextWidth(initialCharIndex, testLength) > width) {
			*charIndex = --testLength + lastWordBreakIndex;
			return testLength;
		}
	}

	// The complete text to render was a single word, or was narrower than
	// the text area, so return the entire line
	if (length == 0 || getTextWidth(initialCharIndex, testLength) <= width) {
		// NOTE: In original engine, the font, color, and alignment were
		// reset, then getTextWidth was called to use its side-effects to
		// set font, color, and alignment according to the text from
		// `initialCharIndex` to `testLength`
		return testLength;
	}

	// The last word in the line made it wider than the text area, so return
	// up to the penultimate word
	*charIndex = lastWordBreakIndex;
	return length;
}

int16 GfxText32::getTextWidth(const uint index, uint length) const {
	int16 width = 0;

	const char *text = _text.c_str() + index;

	GfxFont *font = _font;

	char currentChar = *text++;
	while (length > 0 && currentChar != '\0') {
		// Control codes are in the format `|<code><value>|`
		if (currentChar == '|') {
			// NOTE: Original engine code changed the global state of the
			// FontMgr here upon encountering any color, alignment, or
			// font control code.
			// To avoid requiring all callers to manually restore these
			// values on every call, we ignore control codes other than
			// font change (since alignment and color do not change the
			// width of characters), and simply update the font pointer
			// on stack instead of the member property font.
			currentChar = *text++;
			--length;

			if (length > 0 && currentChar == 'f') {
				GuiResourceId fontId = 0;
				do {
					currentChar = *text++;
					--length;

					fontId = fontId * 10 + currentChar - '0';
				} while (length > 0 && *text >= '0' && *text <= '9');

				if (length > 0) {
					font = _cache->getFont(fontId);
				}
			}

			// Forward through any more unknown control character data
			while (length > 0 && *text != '|') {
				++text;
				--length;
			}
			if (length > 0) {
				++text;
				--length;
			}
		} else {
			width += font->getCharWidth(currentChar);
		}

		if (length > 0) {
			currentChar = *text++;
			--length;
		}
	}

	return width;
}

int16 GfxText32::getTextWidth(const Common::String &text, const uint index, const uint length) {
	_text = text;
	return scaleUpWidth(getTextWidth(index, length));
}

Common::Rect GfxText32::getTextSize(const Common::String &text, int16 maxWidth, bool doScaling) {
	// NOTE: Like most of the text rendering code, this function was pretty
	// weird in the original engine. The initial result rectangle was actually
	// a 1x1 rectangle (0, 0, 0, 0), which was then "fixed" after the main
	// text size loop finished running by subtracting 1 from the right and
	// bottom edges. Like other functions in SCI32, this has been converted
	// to use exclusive rects with inclusive rounding.

	Common::Rect result;

	int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	maxWidth = maxWidth * _scaledWidth / scriptWidth;

	_text = text;

	if (maxWidth >= 0) {
		if (maxWidth == 0) {
			maxWidth = _scaledWidth * 3 / 5;
		}

		result.right = maxWidth;

		int16 textWidth = 0;
		if (_text.size() > 0) {
			const char *rawText = _text.c_str();
			const char *sourceText = rawText;
			uint charIndex = 0;
			uint nextCharIndex = 0;
			while (*rawText != '\0') {
				uint length = getLongest(&nextCharIndex, result.width());
				textWidth = MAX(textWidth, getTextWidth(charIndex, length));
				charIndex = nextCharIndex;
				rawText = sourceText + charIndex;
				// TODO: Due to getLongest and getTextWidth not having side
				// effects, it is possible that the currently loaded font's
				// height is wrong for this line if it was changed inline
				result.bottom += _font->getHeight();
			}
		}

		if (textWidth < maxWidth) {
			result.right = textWidth;
		}
	} else {
		result.right = getTextWidth(0, 10000);
		// NOTE: In the original engine code, the bottom was not decremented
		// by 1, which means that the rect was actually a pixel taller than
		// the height of the font. This was not the case in the other branch,
		// which decremented the bottom by 1 at the end of the loop.
		result.bottom = _font->getHeight() + 1;
	}

	if (doScaling) {
		// NOTE: The original engine code also scaled top/left but these are
		// always zero so there is no reason to do that.
		result.right = ((result.right - 1) * scriptWidth + _scaledWidth - 1) / _scaledWidth + 1;
		result.bottom = ((result.bottom - 1) * scriptHeight + _scaledHeight - 1) / _scaledHeight + 1;
	}

	return result;
}

void GfxText32::erase(const Common::Rect &rect, const bool doScaling) {
	Common::Rect targetRect = doScaling ? scaleRect(rect) : rect;

	byte *bitmap = _segMan->getHunkPointer(_bitmap);
	byte *pixels = bitmap + READ_SCI11ENDIAN_UINT32(bitmap + 28);

	// NOTE: There is an extra optimisation within the SCI code to
	// do a single memset if the scaledRect is the same size as
	// the bitmap, not implemented here.
	Buffer buffer(_width, _height, pixels);
	buffer.fillRect(targetRect, _backColor);
}

int16 GfxText32::getStringWidth(const Common::String &text) {
	return getTextWidth(text, 0, 10000);
}

int16 GfxText32::getTextCount(const Common::String &text, const uint index, const Common::Rect &textRect, const bool doScaling) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	Common::Rect scaledRect(textRect);
	if (doScaling) {
		mulinc(scaledRect, Ratio(_scaledWidth, scriptWidth), Ratio(_scaledHeight, scriptHeight));
	}

	Common::String oldText = _text;
	_text = text;

	uint charIndex = index;
	int16 maxWidth = scaledRect.width();
	int16 lineCount = (scaledRect.height() - 2) / _font->getHeight();
	while (lineCount--) {
		getLongest(&charIndex, maxWidth);
	}

	_text = oldText;
	return charIndex - index;
}

int16 GfxText32::getTextCount(const Common::String &text, const uint index, const GuiResourceId fontId, const Common::Rect &textRect, const bool doScaling) {
	setFont(fontId);
	return getTextCount(text, index, textRect, doScaling);
}

void GfxText32::scrollLine(const Common::String &lineText, int numLines, uint8 color, TextAlign align, GuiResourceId fontId, ScrollDirection dir) {
	BitmapResource bmr(_bitmap);
	byte *pixels = bmr.getPixels();

	int h = _font->getHeight();

	if (dir == kScrollUp) {
		// Scroll existing text down
		for (int i = 0; i < (numLines - 1) * h; ++i) {
			int y = _textRect.top + numLines * h - i - 1;
			memcpy(pixels + y * _width + _textRect.left,
			       pixels + (y - h) * _width + _textRect.left,
			       _textRect.width());
		}
	} else {
		// Scroll existing text up
		for (int i = 0; i < (numLines - 1) * h; ++i) {
			int y = _textRect.top + i;
			memcpy(pixels + y * _width + _textRect.left,
			       pixels + (y + h) * _width + _textRect.left,
			       _textRect.width());
		}
	}

	Common::Rect lineRect = _textRect;

	if (dir == kScrollUp) {
		lineRect.bottom = lineRect.top + h;
	} else {
		// It is unclear to me what the purpose of this bottom++ is.
		// It does not seem to be the usual inc/exc issue.
		lineRect.top += (numLines - 1) * h;
		lineRect.bottom++;
	}

	erase(lineRect, false);

	_drawPosition.x = _textRect.left;
	_drawPosition.y = _textRect.top;
	if (dir == kScrollDown) {
		_drawPosition.y += (numLines - 1) * h;
	}

	_foreColor = color;
	_alignment = align;
	//int fc = _foreColor;

	setFont(fontId);

	_text = lineText;
	int16 textWidth = getTextWidth(0, lineText.size());

	if (_alignment == kTextAlignCenter) {
		_drawPosition.x += (_textRect.width() - textWidth) / 2;
	} else if (_alignment == kTextAlignRight) {
		_drawPosition.x += _textRect.width() - textWidth;
	}

	//_foreColor = fc;
	//setFont(fontId);

	drawText(0, lineText.size());
}


} // End of namespace Sci
