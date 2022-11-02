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

#include "common/util.h"
#include "common/stack.h"
#include "common/unicode-bidi.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/engine/tts.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/scifont.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {

int16 GfxText32::_xResolution = 0;
int16 GfxText32::_yResolution = 0;

GfxText32::GfxText32(SegManager *segMan, GfxCache *fonts) :
	_segMan(segMan),
	_cache(fonts),
	// SSCI did not initialise height, so we intentionally do not do so also
	_width(0),
	_text(""),
	_bitmap(NULL_REG) {
		_fontId = kSci32SystemFont;
		_font = _cache->getFont(kSci32SystemFont);
	}

void GfxText32::init() {
	_xResolution = g_sci->_gfxFrameout->getScriptWidth();
	_yResolution = g_sci->_gfxFrameout->getScriptHeight();
	// GK1 Korean patched version uses doubled resolution for fonts
	if (g_sci->getGameId() == GID_GK1 && g_sci->getLanguage() == Common::KO_KOR) {
		_xResolution = _xResolution * 2;
		_yResolution = _yResolution * 2;
	}
}

reg_t GfxText32::createFontBitmap(int16 width, int16 height, const Common::Rect &rect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, const TextAlign alignment, const int16 borderColor, const bool dimmed, const bool doScaling, const bool gc) {

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
		int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
		int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

		Ratio scaleX(_xResolution, scriptWidth);
		Ratio scaleY(_yResolution, scriptHeight);

		_width = (_width * scaleX).toInt();
		_height = (_height * scaleY).toInt();
		mulinc(_textRect, scaleX, scaleY);
	}

	// `_textRect` represents where text is drawn inside the bitmap; `clipRect`
	// is the entire bitmap
	Common::Rect bitmapRect(_width, _height);

	if (_textRect.intersects(bitmapRect)) {
		_textRect.clip(bitmapRect);
	} else {
		_textRect = Common::Rect();
	}

	_segMan->allocateBitmap(&_bitmap, _width, _height, _skipColor, 0, 0, _xResolution, _yResolution, 0, false, gc);

	erase(bitmapRect, false);

	if (_borderColor > -1) {
		drawFrame(bitmapRect, 1, _borderColor, false);
	}

	drawTextBox();
	return _bitmap;
}

reg_t GfxText32::createFontBitmap(const CelInfo32 &celInfo, const Common::Rect &rect, const Common::String &text, const int16 foreColor, const int16 backColor, const GuiResourceId fontId, const int16 skipColor, const int16 borderColor, const bool dimmed, const bool gc) {
	_borderColor = borderColor;
	_text = text;
	_textRect = rect;
	_foreColor = foreColor;
	_dimmed = dimmed;

	setFont(fontId);

	int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	mulinc(_textRect, Ratio(_xResolution, scriptWidth), Ratio(_yResolution, scriptHeight));

	CelObjView view(celInfo.resourceId, celInfo.loopNo, celInfo.celNo);
	_skipColor = view._skipColor;
	_width = view._width * _xResolution / view._xResolution;
	_height = view._height * _yResolution / view._yResolution;

	Common::Rect bitmapRect(_width, _height);
	if (_textRect.intersects(bitmapRect)) {
		_textRect.clip(bitmapRect);
	} else {
		_textRect = Common::Rect();
	}

	SciBitmap &bitmap = *_segMan->allocateBitmap(&_bitmap, _width, _height, _skipColor, 0, 0, _xResolution, _yResolution, 0, false, gc);

	// SSCI filled the bitmap pixels with 11 here, which is silly because then
	// it just erased the bitmap using the skip color. So we don't fill the
	// bitmap redundantly here.

	_backColor = _skipColor;
	erase(bitmapRect, false);
	_backColor = backColor;

	view.draw(bitmap.getBuffer(), bitmapRect, Common::Point(0, 0), false, Ratio(_xResolution, view._xResolution), Ratio(_yResolution, view._yResolution));

	if (_backColor != skipColor && _foreColor != skipColor) {
		erase(_textRect, false);
	}

	if (text.size() > 0) {
		if (_foreColor == skipColor) {
			error("TODO: Implement transparent text");
		} else {
			if (borderColor != -1) {
				drawFrame(bitmapRect, 1, _borderColor, false);
			}

			drawTextBox();
		}
	}

	return _bitmap;
}

reg_t GfxText32::createTitledFontBitmap(int16 width, int16 height, const Common::Rect &textRect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, const TextAlign alignment, const int16 borderColor, const Common::String &title, const uint8 titleForeColor, const uint8 titleBackColor, const GuiResourceId titleFontId, const bool doScaling, const bool gc) {

	_borderColor = borderColor;
	_width = width;
	_height = height;
	_skipColor = skipColor;

	setFont(titleFontId);

	int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
	Ratio scaleX(_xResolution, scriptWidth);
	Ratio scaleY(_yResolution, scriptHeight);

	if (doScaling) {
		_width = (_width * scaleX).toInt();
		_height = (_height * scaleY).toInt();
	}

	_text = title;
	int16 titleWidth;
	int16 titleHeight;
	getTextDimensions(0, 10000, titleWidth, titleHeight);
	if (getSciVersion() < SCI_VERSION_3) {
		GfxFont *titleFont = _cache->getFont(titleFontId);
		titleHeight = titleFont->getHeight();
	}
	titleWidth += 2;
	titleHeight += 1;
	if (borderColor != -1) {
		titleWidth += 2;
		titleHeight += 2;
	}

	// allocate memory for the bitmap
	_segMan->allocateBitmap(&_bitmap, _width, _height, _skipColor, 0, 0, _xResolution, _yResolution, 0, false, gc);

	// draw background
	_backColor = backColor;
	_textRect = Common::Rect(0, 0, width, height);
	erase(_textRect, false);

	// draw title background
	_foreColor = titleForeColor;
	_backColor = titleBackColor;
	_alignment = kTextAlignCenter;
	_dimmed = false;
	_textRect.setHeight(titleHeight);
	erase(_textRect, false);

	// draw title border
	if (borderColor != -1) {
		drawFrame(_textRect, 1, borderColor, false);
		_textRect.grow(-2);
	}

	// draw title text
	drawTextBox();

	setFont(fontId);
	_text = text;
	_foreColor = foreColor;
	_backColor = backColor;
	_alignment = alignment;
	_textRect = textRect;
	if (doScaling) {
		mulinc(_textRect, scaleX, scaleY);
	}

	// draw text border
	Common::Rect textBorderRect(0, titleHeight - 1, _width, _height);
	_textRect.clip(textBorderRect);
	if (borderColor != -1) {
		drawFrame(textBorderRect, 1, borderColor, false);
	}

	// draw text
	GfxFont *font = _cache->getFont(fontId);
	if (_textRect.height() >= font->getHeight()) {
		drawTextBox();
	}

	return _bitmap;
}

void GfxText32::setFont(const GuiResourceId fontId) {
	// In SSCI, this calls FontMgr::BuildFontTable, and then a font table is
	// built on the FontMgr directly; instead, because we already have GfxFont
	// resources from SCI16 and those resources did not change in SCI32, this
	// code just grabs those out of GfxCache
	if (fontId != _fontId) {
		_fontId = fontId;
		_font = _cache->getFont(_fontId);
	}
}

void GfxText32::drawFrame(const Common::Rect &rect, const int16 size, const uint8 color, const bool doScaling) {
	Common::Rect targetRect = doScaling ? scaleRect(rect) : rect;

	SciBitmap &bitmap = *_segMan->lookupBitmap(_bitmap);
	byte *pixels = bitmap.getPixels() + rect.top * _width + rect.left;

	// Not fully disassembled, but appears to be correct in all cases
	int16 rectWidth = targetRect.width();
	int16 heightRemaining = targetRect.height();
	int16 sidesHeight = heightRemaining - size * 2;
	int16 centerWidth = rectWidth - size * 2;
	int16 stride = _width - rectWidth;

	for (int16 y = 0; y < size && y < heightRemaining; ++y) {
		memset(pixels, color, rectWidth);
		pixels += _width;
		--heightRemaining;
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
	for (int16 y = 0; y < size && y < heightRemaining; ++y) {
		memset(pixels, color, rectWidth);
		pixels += _width;
		--heightRemaining;
	}
}

void GfxText32::drawChar(const uint16 charIndex) {
	SciBitmap &bitmap = *_segMan->lookupBitmap(_bitmap);
	byte *pixels = bitmap.getPixels();

	_font->drawToBuffer(charIndex, _drawPosition.y, _drawPosition.x, _foreColor, _dimmed, pixels, _width, _height);
	_drawPosition.x += _font->getCharWidth(charIndex);
}

int16 GfxText32::getScaledFontHeight() const {
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
	return (_font->getHeight() * scriptHeight + _yResolution - 1) / _yResolution;
}

uint16 GfxText32::getCharWidth(const uint16 charIndex, const bool doScaling) const {
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

	g_sci->_tts->text(_text);

	const char *text = _text.c_str();
	const char *sourceText = text;
	int16 textRectWidth = _textRect.width();
	_drawPosition.y = _textRect.top;
	uint charIndex = 0;

	if (g_sci->getGameId() == GID_SQ6 || g_sci->getGameId() == GID_MOTHERGOOSEHIRES) {
		if (getLongest(&charIndex, textRectWidth) == 0) {
			error("DrawTextBox GetLongest=0");
		}
	}

	// Check for Korean text
	if (g_sci->getLanguage() == Common::KO_KOR)
		SwitchToFont1001OnKorean(text);

	charIndex = 0;
	uint nextCharIndex = 0;
	while (*text != '\0') {
		_drawPosition.x = _textRect.left;

		uint length = getLongest(&nextCharIndex, textRectWidth);
		int16 textWidth = getTextWidth(charIndex, length);

		if (!g_sci->isLanguageRTL()) {
			if (_alignment == kTextAlignCenter) {
				_drawPosition.x += (textRectWidth - textWidth) / 2;
			} else if (_alignment == kTextAlignRight) {
				_drawPosition.x += textRectWidth - textWidth;
			}
		} else {
			if (_alignment == kTextAlignCenter) {
				_drawPosition.x += (textRectWidth - textWidth) / 2;
			} else if (_alignment == kTextAlignLeft) {
				_drawPosition.x += textRectWidth - textWidth;
			}
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

	// This draw loop implementation is somewhat different than the
	// implementation in SSCI, but is accurate. Primarily the changes revolve
	// around eliminating some extra temporaries and fixing the logic to match.

	Common::String textString;
	const char *text;
	if (!g_sci->isLanguageRTL()) {
		text = _text.c_str() + index;
	} else {
		const char *textOrig = _text.c_str() + index;
		Common::String textLogical = Common::String(textOrig, (uint32)length);
		textString = Common::convertBiDiString(textLogical, g_sci->getLanguage(), Common::BiDiParagraph::BIDI_PAR_RTL);
		text = textString.c_str();
	}

	while (length-- > 0) {
		uint16 currentChar = *(const byte *)text++;
		if (_font->isDoubleByte(currentChar)) {
			currentChar |= *text++ << 8;
		}

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

void GfxText32::invertRect(const reg_t bitmapId, int16 bitmapStride, const Common::Rect &rect, const uint8 foreColor, const uint8 backColor, const bool doScaling) {
	Common::Rect targetRect = rect;
	if (doScaling) {
		bitmapStride = bitmapStride * _xResolution / g_sci->_gfxFrameout->getScriptWidth();
		targetRect = scaleRect(rect);
	}

	SciBitmap &bitmap = *_segMan->lookupBitmap(bitmapId);

	// SSCI is super weird here; it seems to be trying to look at the entire
	// size of the bitmap including the header, instead of just the pixel data
	// size. We just look at the pixel size. This function generally is an odd
	// duck since the stride dimension for a bitmap is built in to the bitmap
	// header, so perhaps it was once an unheadered bitmap format and this
	// function was never updated to match? Or maybe they exploit the
	// configurable stride length somewhere else to do stair stepping inverts...
	uint32 invertSize = targetRect.height() * bitmapStride + targetRect.width();
	uint32 bitmapSize = bitmap.getDataSize();

	if (invertSize >= bitmapSize) {
		error("InvertRect too big: %u >= %u", invertSize, bitmapSize);
	}

	// SSCI just added a hardcoded bitmap header size here
	byte *pixel = bitmap.getPixels() + bitmapStride * targetRect.top + targetRect.left;

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

	uint16 currentChar = 0;
	while ((currentChar = *(const byte *)text++) != '\0') {
		if (_font->isDoubleByte(currentChar)) {
			currentChar |= (*text++) << 8;
		}
		// In SSCI, the font, color, and alignment were reset here to their
		// initial values; this does not seem to be necessary and really
		// complicates the font system, so we do not do it

		// The text to render contains a line break; stop at the line break
		if (currentChar == '\r' || currentChar == '\n') {
			// Skip the rest of the line break if it is a Windows-style \r\n (or
			// non-standard \n\r)

			// In SSCI, the `text` pointer had not been advanced yet here, so
			// the indexes used to access characters were one higher there
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
			// In SSCI, the font, color, and alignment were reset, then
			// getTextWidth was called to use its side-effects to set font,
			// color, and alignment according to the text from
			// `initialCharIndex` to `testLength`. This is complicated and
			// apparently not necessary for correct operation, so we do not do
			// it

			++*charIndex;
			return testLength;
		} else if (currentChar == ' ') {
			// The last word in the line made it too wide to fit in the text
			// area; return up to the previous word, then collapse the
			// whitespace between that word and its next sibling word into the
			// line break
			if (getTextWidth(initialCharIndex, testLength) > width) {
				*charIndex = lastWordBreakIndex;
				const char *nextChar = _text.c_str() + lastWordBreakIndex;
				while (*nextChar++ == ' ') {
					++*charIndex;
				}

				// In SSCI, the font, color, and alignment were set here to the
				// values that were seen at the last space character, but this
				// is complicated and unnecessary so we do not do it

				return length;
			}

			// In SSCI, the values of `_fontId`, `_foreColor`, and `_alignment`
			// were stored for use in the return path mentioned just above here,
			// but we do not need to do this because we do not cause
			// side-effects when calculating text dimensions

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
		if (_font->isDoubleByte(currentChar)) {
			++*charIndex;
		}

		// In SSCI, the font, color, and alignment were reset here to their
		// initial values, but we do not need to do this because we do not cause
		// side-effects when calculating text dimensions

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
		// In SSCI, the font, color, and alignment were reset, then getTextWidth
		// was called to use its side-effects to set font, color, and alignment
		// according to the text from `initialCharIndex` to `testLength`. This
		// is not necessary because we do not cause side-effects when
		// calculating text dimensions
		return testLength;
	}

	// The last word in the line made it wider than the text area, so return
	// up to the penultimate word
	*charIndex = lastWordBreakIndex;
	return length;
}

int16 GfxText32::getTextWidth(const uint index, uint length) const {
	int16 width;
	int16 height;
	getTextDimensions(index, length, width, height);
	return width;
}

void GfxText32::getTextDimensions(const uint index, uint length, int16 &width, int16& height) const {
	width = 0;
	height = 0;

	const char *text = _text.c_str() + index;

	GfxFont *font = _font;

	uint16 currentChar = *(const byte *)text++;
	while (length > 0 && currentChar != '\0') {
		if (_font->isDoubleByte(currentChar)) {
			currentChar |= (*text++) << 8;
		}
		// Control codes are in the format `|<code><value>|`
		if (currentChar == '|') {
			// SSCI changed the global state of the FontMgr here upon
			// encountering any color, alignment, or font control code. To avoid
			// requiring all callers to manually restore these values on every
			// call, we ignore control codes other than font change (since
			// alignment and color do not change the width of characters), and
			// simply update the font pointer on stack instead of the member
			// property font to avoid these unnecessary side-effects.
			currentChar = *text++;
			--length;

			if (length > 0 && currentChar == 'f') {
				GuiResourceId fontId = 0;
				while (length > 0 && *text >= '0' && *text <= '9') {
					currentChar = *text++;
					--length;

					fontId = fontId * 10 + currentChar - '0';
				}

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
			width += font->getCharWidth((unsigned char)currentChar);
			byte charHeight = font->getCharHeight((unsigned char)currentChar);
			if (height < charHeight) {
				height = charHeight;
			}
		}

		if (length > 0) {
			currentChar = *text++;
			--length;
		}
	}
}

int16 GfxText32::getTextWidth(const Common::String &text, const uint index, const uint length) {
	_text = text;
	return scaleUpWidth(getTextWidth(index, length));
}

Common::Rect GfxText32::getTextSize(const Common::String &text, int16 maxWidth, bool doScaling) {
	// Like most of the text rendering code, this function was pretty weird in
	// SSCI. The initial result rectangle was actually a 1x1 rectangle
	// (0, 0, 0, 0), which was then "fixed" after the main text size loop
	// finished running by subtracting 1 from the right and bottom edges. Like
	// other functions in SCI32, this has been converted to use exclusive rects
	// with inclusive rounding.

	Common::Rect result;

	int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	maxWidth = maxWidth * _xResolution / scriptWidth;

	_text = text;

	if (maxWidth >= 0) {
		if (maxWidth == 0) {
			maxWidth = _xResolution * 3 / 5;
		}

		result.right = maxWidth;

		int16 textWidth = 0;
		if (_text.size() > 0) {
			const char *rawText = _text.c_str();
			const char *sourceText = rawText;

			// Check for Korean text
			if (g_sci->getLanguage() == Common::KO_KOR)
				SwitchToFont1001OnKorean(rawText);

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

		if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
			result.bottom = 0;
		} else {
			// In SSCI, the bottom was not decremented by 1, which means that
			// the rect was actually a pixel taller than the height of the font.
			// This was not the case in the other branch, which decremented the
			// bottom by 1 at the end of the loop. For accuracy, we do what SSCI
			// did, even though this means the result is a pixel off
			result.bottom = _font->getHeight() + 1;
		}
	}

	if (doScaling) {
		// SSCI also scaled top/left but these are always zero so there is no
		// reason to do that
		result.right = ((result.right - 1) * scriptWidth + _xResolution - 1) / _xResolution + 1;
		result.bottom = ((result.bottom - 1) * scriptHeight + _yResolution - 1) / _yResolution + 1;
	}

	return result;
}

void GfxText32::erase(const Common::Rect &rect, const bool doScaling) {
	Common::Rect targetRect = doScaling ? scaleRect(rect) : rect;

	SciBitmap &bitmap = *_segMan->lookupBitmap(_bitmap);
	bitmap.getBuffer().fillRect(targetRect, _backColor);
}

int16 GfxText32::getStringWidth(const Common::String &text) {
	return getTextWidth(text, 0, 10000);
}

int16 GfxText32::getTextCount(const Common::String &text, const uint index, const Common::Rect &textRect, const bool doScaling) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	Common::Rect scaledRect(textRect);
	if (doScaling) {
		mulinc(scaledRect, Ratio(_xResolution, scriptWidth), Ratio(_yResolution, scriptHeight));
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
	SciBitmap &bmr = *_segMan->lookupBitmap(_bitmap);
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

	// As with other font functions, SSCI saved _foreColor here so it could be
	// restored after the getTextWidth call, but this call is side-effect-free
	// in our implementation so this is not necessary

	setFont(fontId);

	_text = lineText;
	int16 textWidth = getTextWidth(0, lineText.size());

	if (_alignment == kTextAlignCenter) {
		_drawPosition.x += (_textRect.width() - textWidth) / 2;
	} else if (_alignment == kTextAlignRight) {
		_drawPosition.x += _textRect.width() - textWidth;
	}

	// _foreColor and font were restored here in SSCI due to side-effects of
	// getTextWidth which do not exist in our implementation

	drawText(0, lineText.size());
}

// Check for Korean strings, and use font 1001 to render them
bool GfxText32::SwitchToFont1001OnKorean(const char *text) {
	const byte *ptr = (const byte *)text;
	// Check if the text contains at least one Korean character
	while (*ptr) {
		byte ch = *ptr++;
		if (ch >= 0xB0 && ch <= 0xC8) {
			ch = *ptr++;
			if (!ch)
				return false;

			if (ch >= 0xA1 && ch <= 0xFE) {
				setFont(1001);
				return true;
			}
		}
	}
	return false;
}

} // End of namespace Sci
