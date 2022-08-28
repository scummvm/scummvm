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


#ifdef ENABLE_SCUMM_7_8

#include "common/config-manager.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/string_v7.h"

namespace Scumm {

TextRenderer_v7::TextRenderer_v7(ScummEngine *vm, GlyphRenderer_v7 *gr)	:
	_gameId(vm->_game.id),
	_lang(vm->_language),
	_2byteCharWidth(vm->_2byteWidth),
	_screenWidth(vm->_screenWidth),
	_useCJKMode(vm->_useCJKMode),
	_direction(vm->_language == Common::HE_ISR ? -1 : 1),
	_rtlCenteredOffset(vm->_language == Common::HE_ISR ? 1 : 0),
	_spacing(vm->_language != Common::JA_JPN ? 1 : 0),
	_lineBreakMarker(vm->_newLineCharacter),
	_newStyle (gr->newStyleWrapping()),
	_gr(gr) {
}

int TextRenderer_v7::getStringWidth(const char *str, uint numBytesMax) {
	assert(str);

	if (!numBytesMax)
		return 0;

	int maxWidth = 0;
	int width = 0;

	while (*str && numBytesMax) {
		if (_newStyle && *str == '^') {
			if (str[1] == 'f') {
				_gr->setFont(str[3] - '0');
				str += 4;
				numBytesMax -= 4;
				continue;
			} else if (str[1] == 'c') {
				str += 5;
				numBytesMax -= 5;
				continue;
			} else if (str[1] == 'l') {
				str += 2;
				numBytesMax -= 2;
				continue;
			}
		} else if (!_newStyle && *str == '@') {
			str++;
			numBytesMax--;
			continue;
		}

		if (is2ByteCharacter(_lang, *str)) {
			width += _2byteCharWidth + _spacing;
			++str;
			--numBytesMax;
		} else if (_newStyle && *str == '\n') {
			maxWidth = MAX<int>(width, maxWidth);
			width = 0;
		} else if (!_newStyle && *str == '\r') {
			break;
		} else if (!_newStyle && *str == '\xff') {
			++str;
			--numBytesMax;
			if (*str == 0 || *str == 3 || *str == 9 || *str == 1 || *str == 2)
				return width;

		} else if (*str != '\r' && !(_newStyle && *str == _lineBreakMarker)) {
			width += _gr->getCharWidth((uint8)*str);
		}
		++str;
		--numBytesMax;
	}

	return MAX<int>(width, maxWidth);
}

int TextRenderer_v7::getStringHeight(const char *str, uint numBytesMax) {
	assert(str);

	if (!numBytesMax)
		return 0;

	int totalHeight = 0;
	int lineHeight = 0;

	while (*str && numBytesMax) {
		if (_newStyle && *str == '^') {
			if (str[1] == 'f') {
				_gr->setFont(str[3] - '0');
				str += 4;
				numBytesMax -= 4;
				continue;
			} else if (str[1] == 'c') {
				str += 5;
				numBytesMax -= 5;
				continue;
			} else if (str[1] == 'l') {
				str += 2;
				numBytesMax -= 2;
				continue;
			}
		}

		if (*str == '\n') {
			totalHeight += (lineHeight ? lineHeight : _gr->getFontHeight()) + 1;
			lineHeight = 0;
		} else if (*str != '\r' && *str != _lineBreakMarker) {
			lineHeight = MAX<int>(lineHeight, _gr->getCharHeight(*str));
			if (is2ByteCharacter(_lang, *str)) {
				++str;
				--numBytesMax;
			}
		}
		++str;
		--numBytesMax;
	}

	return totalHeight + (lineHeight ? lineHeight : _gr->getFontHeight()) + (_newStyle ? 1 : 0);
}

void TextRenderer_v7::drawSubstring(const char *str, uint numBytesMax, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 &col, TextStyleFlags flags) {
	for (int i = 0; str[i] != 0 && numBytesMax; ++i) {
		if (_newStyle && str[i] == '^') {
			if (str[i + 1] == 'f') {
				_gr->setFont(str[i + 3] - '0');
				i += 3;
				numBytesMax -= 4;
				continue;
			} else if (str[i + 1] == 'c') {
				col = str[i + 4] - '0' + 10 *(str[i + 3] - '0');
				i += 4;
				numBytesMax -= 5;
				continue;
			} else if (str[i + 1] == 'l') {
				i++;
				numBytesMax -= 2;
				continue;
			}
		}

		if (is2ByteCharacter(_lang, str[i])) {
			x += _gr->draw2byte(buffer, clipRect, x, y, pitch, col, (byte)str[i] + 256 * (byte)str[i + 1]);
			++i;
			--numBytesMax;
		} else if (str[i] != '\n' && str[i] != _lineBreakMarker) {
			x += _gr->drawCharV7(buffer, clipRect, x, y, pitch, col, flags, str[i]);
		}
		--numBytesMax;
	}
}

#define SCUMM7_MAX_STRINGS		80

void TextRenderer_v7::drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags) {
	debugC(DEBUG_GENERAL, "TextRenderer_v7::drawString(str: '%s', x: %d, y: %d, col: %d, clipRect: (%d, %d, %d, %d), flags: 0x%02x)", str, x, y, col, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom, flags);

	int totalLen = (int)strlen(str);
	int lineStart = 0;
	int xAdj = 0;

	// COMI always does this for CJK strings (before any other possible yPos fixes).
	if (_gameId == GID_CMI) {
		if (_useCJKMode) {
			y += 2;
			if (col != 0)
				xAdj = 1; // x-adjust for left side glyph shadow
		}
		// No idea whether it is actually used. We currently don't handle this flag.
		/*if (flags & 0x40)
			y -= (getStringHeight(str, totalLen) / 2);*/
	}

	int y2 = y;
	int maxWidth = 0;

	for (int pos = 0; pos <= totalLen; ++pos) {
		if (str[pos] != '\0' && str[pos] != '\n')
			continue;

		int len = pos - lineStart;
		int height = getStringHeight(str + lineStart, len);
		if (y < clipRect.bottom) {
			int width = getStringWidth(str + lineStart, len);
			maxWidth = MAX<int>(maxWidth, width);

			int xpos = x;
			if (flags & kStyleAlignCenter)
				xpos = x - _direction * width / 2 + (_rtlCenteredOffset & width);
			else if (((flags & kStyleAlignRight) && _direction == 1) || (!(flags & kStyleAlignRight) && _direction == -1))
				// The original interpreter apparently does not expect a right-to-left written language when the kStyleAlignRight flag is set.
				// It just right-aligns a left-to-right string. So we now move xpos to the left like the original interpreter would if it is a
				// left-to-right string, but leave it on the right in case of a right-to-left string (and vice versa for right-to-left strings
				// with kStyleAlignLeft flag).
				xpos = x - _direction * width;

			drawSubstring(str + lineStart, len, buffer, clipRect, xpos, y, pitch, col, flags);
			y += height;
		}

		lineStart = pos + 1;
	}

	clipRect.left = MAX<int>(0, ((flags & kStyleAlignCenter) ? x - maxWidth / 2 : ((flags & kStyleAlignRight) ? x - maxWidth : x)) - xAdj);
	clipRect.right = MIN<int>(clipRect.right, clipRect.left + xAdj + maxWidth);
	clipRect.top = y2;
	clipRect.bottom = y + (_newStyle ? 0 : 1);
}

void TextRenderer_v7::drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags) {
	debugC(DEBUG_GENERAL, "TextRenderer_v7::drawStringWrap(str: '%s', x: %d, y: %d, col: %d, clipRect: (%d, %d, %d, %d), flags: 0x%02x)", str, x, y, col, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom, flags);
	// This implementation is from COMI. Things are done a bit differently than in the older implementations.
	// In particular, the older version would insert '\0' chars into the string to cut off the sub strings
	// before calling getStringWidth(), getStringHeight() or drawSubstring() and replace these chars with the
	// original values afterwards. COMI allows a byte length limitation in all the functions so that the sub
	// string length can be passed and no cut off '\0' chars are needed.

	int len = (int)strlen(str);
	Common::String spaceSeparators(Common::String::format(" %c", (char)_lineBreakMarker));
	Common::String breakSeparators(Common::String::format(" \n%c", (char)_lineBreakMarker));

	// We have already handled the escape codes used by FT and DIG (demo),
	// so the following assertion is there just for good measure.
	if (!_newStyle) {
		Common::String invalidChars("@\xff\x03\x09\x01\x02\x08");
		for (int i = 0; i < len; ++i) {
			assert(!invalidChars.contains(str[i]));
		}
	}

	int16 substrByteLength[SCUMM7_MAX_STRINGS];
	memset(substrByteLength, 0, sizeof(substrByteLength));
	int16 substrWidths[SCUMM7_MAX_STRINGS];
	memset(substrWidths, 0, sizeof(substrWidths));
	int16 substrStart[SCUMM7_MAX_STRINGS];
	memset(substrStart, 0, sizeof(substrStart));

	int16 numSubstrings = 0;
	int height = 0;
	int lastSubstrHeight = 0;
	int maxWidth = 0;
	int curWidth = 0;
	int curPos = -1;
	int xAdj = 0;

	// COMI does this for CJK strings (before any other possible yPos fixes, see lines 343 - 355).
	if (_gameId == GID_CMI && _useCJKMode) {
		y += 2;
		if (col != 0)
			xAdj = 1; // x-adjust for left side glyph shadow
	}

	while (curPos < len) {
		int textStart = curPos + 1;
		while (str[textStart] && spaceSeparators.contains(str[textStart]))
			++textStart;

		int separatorWidth = curPos > 0 ? getStringWidth(str + curPos, textStart - curPos) : 0;

		int nextSeparatorPos = textStart;
		while (!breakSeparators.contains(str[nextSeparatorPos])) {
			if (++nextSeparatorPos == len)
				break;
		}

		int wordWidth = getStringWidth(str + textStart, nextSeparatorPos - textStart);
		int newWidth = curWidth + separatorWidth + wordWidth;

		if (curWidth && newWidth > clipRect.width()) {
			if (numSubstrings < SCUMM7_MAX_STRINGS) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
			}
			newWidth = wordWidth;
			substrStart[numSubstrings] = textStart;
		}
		curWidth = newWidth;

		curPos = nextSeparatorPos;
		if (!spaceSeparators.contains(str[curPos])) {
			// This one is only triggered by '\n' (which frequently happens in COMI/English).
			if (numSubstrings < SCUMM7_MAX_STRINGS) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
				substrStart[numSubstrings] = curPos + 1;
			}
			curWidth = 0;
		}
	}

	if (curWidth && numSubstrings < SCUMM7_MAX_STRINGS) {
		substrWidths[numSubstrings] = curWidth;
		substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
		numSubstrings++;
	}

	for (int i = 0; i < numSubstrings; ++i) {
		maxWidth = MAX<int>(maxWidth, substrWidths[i]);
		lastSubstrHeight = substrByteLength[i] > 0 ? getStringHeight(str + substrStart[i], substrByteLength[i]) : 0;
		height += lastSubstrHeight;
	}

	// I have verified these y-corrections for DIG (English and Chinese), COMI (English and Chinese) and FT (English).
	// In COMI there seem to be more text flags which we don't support and for which I haven't seen use cases yet. I
	// put some commented-out code in here as a reminder...
	int clipHeight = height + lastSubstrHeight / 2;

	/*if (_vm->_game.id == GID_CMI && (flags & 0x40))
		y -= (lastSubstrHeight / 2);*/

	if (y > clipRect.bottom - clipHeight /*&& !(_vm->_game.id == GID_CMI && (flags & 0x100))*/)
		y = clipRect.bottom - clipHeight;

	if (y < clipRect.top)
		y = clipRect.top;

	if (_newStyle) {
		if (flags & kStyleAlignCenter) {
			if (x + (maxWidth >> 1) > clipRect.right)
				x = clipRect.right - (maxWidth >> 1);
			if (x - (maxWidth >> 1) < clipRect.left)
				x = clipRect.left + (maxWidth >> 1);
		} else if (flags & kStyleAlignRight) {
			if (x > clipRect.right)
				x = clipRect.right;
			if (x < clipRect.left + maxWidth)
				x = clipRect.left + maxWidth;
		} else {
			if (x > clipRect.right - maxWidth)
				x = clipRect.right - maxWidth;
			if (x < clipRect.left)
				x = clipRect.left;
		}
	}

	int y2 = y;

	for (int i = 0; i < numSubstrings; i++) {
		int xpos = x;
		if (flags & kStyleAlignCenter)
			xpos = x - _direction * substrWidths[i] / 2 + (_rtlCenteredOffset & substrWidths[i]);
		else if (((flags & kStyleAlignRight) && _direction == 1) || (!(flags & kStyleAlignRight) && _direction == -1))
			// The original interpreter apparently does not expect a right-to-left written language when the kStyleAlignRight flag is set.
			// It just right-aligns a left-to-right string. So we now move xpos to the left like the original interpreter would if it is a
			// left-to-right string, but leave it on the right in case of a right-to-left string (and vice versa for right-to-left strings
			// with kStyleAlignLeft flag).
			xpos = x - _direction * substrWidths[i];

		if (!_newStyle)
			xpos = (_direction == 1) ? CLIP<int>(xpos, clipRect.left, _screenWidth - substrWidths[i]) : CLIP<int>(xpos, clipRect.left + substrWidths[i], _screenWidth - 1);

		len = substrByteLength[i] > 0 ? substrByteLength[i] : 0;
		drawSubstring(str + substrStart[i], len, buffer, clipRect, xpos, y, pitch, col, flags);
		y += getStringHeight(str + substrStart[i], len);
	}

	clipRect.left = MAX<int>(0, ((flags & kStyleAlignCenter) ? x - maxWidth / 2 : ((flags & kStyleAlignRight) ? x - maxWidth : x)) - xAdj);
	clipRect.right = MIN<int>(clipRect.right, clipRect.left + xAdj + maxWidth);
	clipRect.top = y2;
	clipRect.bottom = y + (_newStyle ? 0 : 1);
}

Common::Rect TextRenderer_v7::calcStringDimensions(const char *str, int x, int y, TextStyleFlags flags) {
	int width = getStringWidth(str);
	if (_gameId == GID_CMI && _useCJKMode)
		y += 2;

	if (flags & kStyleAlignCenter)
		x -= width / 2;
	else if (flags & kStyleAlignRight)
		x -= width;

	return Common::Rect(x, y, x + width, y + getStringHeight(str));
}

void ScummEngine_v7::createTextRenderer(GlyphRenderer_v7 *gr) {
	assert(gr);
	_textV7 = new TextRenderer_v7(this, gr);
}

#pragma mark -
#pragma mark --- V7 blast text queue code ---
#pragma mark -

void ScummEngine_v7::enqueueText(const byte *text, int x, int y, byte color, byte charset, TextStyleFlags flags) {
	assert(_blastTextQueuePos + 1 <= ARRAYSIZE(_blastTextQueue));

	if (_useCJKMode) {
		// The Dig expressly checks for x == 160 && y == 189 && charset == 3. Usually, if the game wants to print CJK text at the bottom
		// of the screen it will use y = 183. So maybe this is a hack to fix some script texts that were forgotten in the CJK converting
		// process.
		if (_game.id == GID_DIG && x == 160 && y == 189 && charset == 3)
			y -= 6;
	}

	BlastText &bt = _blastTextQueue[_blastTextQueuePos];
	convertMessageToString(text, bt.text, sizeof(bt.text));

	// The original DIG interpreter expressly checks for " " strings here. And the game also sends these quite frequently...
	if (!bt.text[0] || (bt.text[0] == (byte)' ' && !bt.text[1]))
		return;

	_blastTextQueuePos++;
	bt.xpos = x;
	bt.ypos = y;
	bt.color = color;
	bt.charset = charset;
	bt.flags = flags;
}

void ScummEngine_v7::drawTextImmediately(const byte *text, Common::Rect *clipRect, int x, int y, byte color, byte charset, TextStyleFlags flags) {
	// This function allows for a string to be immediately
	// drawn on the screen without having to enqueueing it.
	byte msg[256];
	Common::Rect rect = clipRect ? *clipRect : _defaultTextClipRect;
	int effX = x;
	TextStyleFlags effFlags = flags;
	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	convertMessageToString(text, msg, sizeof(msg));

	_charset->setCurID(charset);

	// If a Hebrew String comes up that is still marked as kStyleAlignLeft we fix it here...
	if (_language == Common::HE_ISR && !(flags & (kStyleAlignCenter | kStyleAlignRight))) {
		effFlags = (TextStyleFlags)(flags | kStyleAlignRight);
		effX = _screenWidth - 1 - effX;
	}

	_textV7->drawString((const char *)msg, (byte *)vs->getPixels(0, _screenTop), rect, effX, y, vs->pitch, color, effFlags);

	rect.top += _screenTop;
	rect.bottom += _screenTop;
	markRectAsDirty(vs->number, rect);
}

void ScummEngine_v7::drawBlastTexts() {
	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	for (int i = 0; i < _blastTextQueuePos; i++) {
		BlastText &bt = _blastTextQueue[i];

		_charset->setCurID(_blastTextQueue[i].charset);

		// If a Hebrew String comes up that is still marked as kStyleAlignLeft we fix it here...
		if (_language == Common::HE_ISR && !(bt.flags & (kStyleAlignCenter | kStyleAlignRight))) {
			bt.flags = (TextStyleFlags)(bt.flags | kStyleAlignRight);
			bt.xpos = _screenWidth - 1 - bt.xpos;
		}

		if (bt.flags & kStyleWordWrap) {
			bt.rect = _wrappedTextClipRect;

			// This is for the "narrow" paragraph wrapping type that the older interpreters (e. g. FT, DIG English) do.
			if (!_newTextRenderStyle) {
				bt.xpos = CLIP<int>(bt.xpos, 80, 240);
				bt.ypos = CLIP<int>(bt.ypos, 1, 160);
				int maxWidth = _string[0].right - bt.xpos - 1;
				if (bt.flags & kStyleAlignCenter) {
					if (maxWidth > bt.xpos)
						maxWidth = bt.xpos;
					bt.rect.left = MAX<int>(0, bt.xpos - maxWidth);
					bt.rect.right = MIN<int>(_screenWidth, bt.xpos + maxWidth);
				}
			}

			_textV7->drawStringWrap((const char*)bt.text, (byte*)vs->getPixels(0, _screenTop), bt.rect, bt.xpos, bt.ypos, vs->pitch, bt.color, bt.flags);
		} else {
			bt.rect = _defaultTextClipRect;
			_textV7->drawString((const char*)bt.text, (byte*)vs->getPixels(0, _screenTop), bt.rect, bt.xpos, bt.ypos, vs->pitch, bt.color, bt.flags);
		}

		bt.rect.top += _screenTop;
		bt.rect.bottom += _screenTop;
		markRectAsDirty(vs->number, bt.rect);
	}
}

void ScummEngine_v7::removeBlastTexts() {
	if (_game.version == 8) {
		if (_blastTextQueuePos != 0)
			_blastTextRectsQueue = _blastTextQueuePos;
		_blastTextQueuePos = 0;
		return;
	}

	for (int i = 0; i < _blastTextQueuePos; i++) {
		restoreBackground(_blastTextQueue[i].rect);
	}
	_blastTextQueuePos = 0;
}

void ScummEngine_v7::restoreBlastTextsRects() {
	if (_game.version < 8)
		return;

	for (int i = 0; i < _blastTextRectsQueue; i++) {
		restoreBackground(_blastTextQueue[i].rect);
	}

	_blastTextRectsQueue = 0;
}

void ScummEngine_v8::printString(int m, const byte *msg) {
	if (m == 4) {
		const StringTab &st = _string[m];
		int flags = st.wrapping ? kStyleWordWrap : 0;
		if (st.center)
			flags |= kStyleAlignCenter;
		enqueueText(msg, st.xpos, st.ypos, st.color, st.charset, (TextStyleFlags)flags);
	} else {
		ScummEngine::printString(m, msg);
	}
}

void ScummEngine_v7::showMessageDialog(const byte *msg) {
	if (isUsingOriginalGUI()) {
		int textColor = _string[3].color;
		if (textColor)
			setBannerColors(
				26,
				_currentPalette[3 * textColor],
				_currentPalette[3 * textColor + 1],
				_currentPalette[3 * textColor + 2]);
		Common::KeyState ks = showBannerAndPause(2, -1, (const char *)msg);

		if (VAR_KEYPRESS != 0xFF)
			VAR(VAR_KEYPRESS) = ks.ascii;
	} else {
		ScummEngine::showMessageDialog(msg);
	}
}

#pragma mark -
#pragma mark --- V7 subtitle queue code ---
#pragma mark -

void ScummEngine_v7::processSubtitleQueue() {
	bool usingOldSystem = (_game.id == GID_FT) || (_game.id == GID_DIG && _game.features & GF_DEMO);
	for (int i = 0; i < _subtitleQueuePos; ++i) {
		SubtitleText *st = &_subtitleQueue[i];
		if (!st->actorSpeechMsg && (!ConfMan.getBool("subtitles") || VAR(VAR_VOICE_MODE) == 0))
			// no subtitles and there's a speech variant of the message, don't display the text
			continue;
		if (usingOldSystem) {
			if (st->center || VAR(VAR_VOICE_MODE)) {
				enqueueText(st->text, st->xpos, st->ypos, st->color, st->charset, (TextStyleFlags)0);
			}
		} else {
			int flags = st->wrap ? kStyleWordWrap : 0;
			if (st->center)
				flags |= kStyleAlignCenter;
			enqueueText(st->text, st->xpos, st->ypos, st->color, st->charset, (TextStyleFlags)flags);
		}
	}
}

void ScummEngine_v7::addSubtitleToQueue(const byte *text, const Common::Point &pos, byte color, byte charset, bool center, bool wrap) {
	if (text[0] && strcmp((const char *)text, " ") != 0) {
		assert(_subtitleQueuePos < ARRAYSIZE(_subtitleQueue));
		SubtitleText *st = &_subtitleQueue[_subtitleQueuePos];
		int i = 0;
		while (1) {
			st->text[i] = text[i];
			if (!text[i])
				break;
			++i;
		}
		st->xpos = pos.x;
		st->ypos = pos.y;
		st->color = color;
		st->charset = charset;
		st->actorSpeechMsg = _haveActorSpeechMsg;
		st->center = center;
		st->wrap = wrap;
		++_subtitleQueuePos;
	}
}

void ScummEngine_v7::clearSubtitleQueue() {
	memset(_subtitleQueue, 0, sizeof(_subtitleQueue));
	_subtitleQueuePos = 0;
}

void ScummEngine_v7::CHARSET_1() {
	processSubtitleQueue();

	bool usingOldSystem = (_game.id == GID_FT) || (_game.id == GID_DIG && _game.features & GF_DEMO);

	byte subtitleBuffer[200];
	byte *subtitleLine = subtitleBuffer;
	Common::Point subtitlePos;

	if (!_haveMsg)
		return;

	Actor *a = NULL;
	if (getTalkingActor() != 0xFF)
		a = derefActorSafe(getTalkingActor(), "CHARSET_1");

	StringTab saveStr = _string[0];
	if (a && _string[0].overhead) {
		int s;
		_string[0].xpos = a->getPos().x - _virtscr[kMainVirtScreen].xstart;
		s = a->_scalex * a->_talkPosX / 255;
		_string[0].xpos += (a->_talkPosX - s) / 2 + s;

		_string[0].ypos = a->getPos().y - a->getElevation() - _screenTop;
		s = a->_scaley * a->_talkPosY / 255;
		_string[0].ypos += (a->_talkPosY - s) / 2 + s;

		if (usingOldSystem) {
			if (_string[0].ypos > _screenHeight - 40)
				_string[0].ypos = _screenHeight - 40;

			if (_string[0].ypos < 1)
				_string[0].ypos = 1;

			if (_string[0].xpos < 80)
				_string[0].xpos = 80;

			if (_string[0].xpos > _screenWidth - 80)
				_string[0].xpos = _screenWidth - 80;
		}
	}
	_charset->setColor(_charsetColor);

	if (usingOldSystem) {
		_charset->_top = _string[0].ypos + _screenTop;
		_charset->_startLeft = _charset->_left = _string[0].xpos;
		_charset->_right = _string[0].right;
		_charset->_center = _string[0].center;
		memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);
	}

	if (usingOldSystem && a && a->_charset) {
		_charset->setCurID(a->_charset);
	} else {
		_charset->setCurID(_string[0].charset);
	}

	if (_talkDelay)
		return;

	if ((!usingOldSystem && VAR(VAR_HAVE_MSG)) || (usingOldSystem && _haveMsg != 1)) {
		if ((_sound->_sfxMode & 2) == 0) {
			stopTalk();
		}
		return;
	}

	if (a && !_string[0].no_talk_anim) {
		a->runActorTalkScript(a->_talkStartFrame);
	}

	int tmpNextLeft = _string[0].xpos;

	if (!_keepText) {
		clearSubtitleQueue();
		if (usingOldSystem) {
			_nextLeft = _string[0].xpos;
			_nextTop = _string[0].ypos + _screenTop;
		}
	} else {
		tmpNextLeft = _nextLeft;
	}

	if (usingOldSystem) {
		_talkDelay = VAR(VAR_DEFAULT_TALK_DELAY);

		int maxwidth = _charset->_right - tmpNextLeft - 1;
		if (_charset->_center) {
			if (maxwidth > _nextLeft)
				maxwidth = _nextLeft;
			maxwidth *= 2;
		}

		_charset->addLinebreaks(0, _charsetBuffer + _charsetBufPos, 0, maxwidth);

		if (_charset->_center) {
			_nextLeft -= _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) / 2;
			if (_nextLeft <= 0)
				_nextLeft = 0;
		}

		int c = 0;

		while (handleNextCharsetCode(a, &c)) {
			if (c == 0) {
				// End of text
				_haveMsg =  2;
				_keepText = false;
				break;
			}

			if (c == 13) {
				// New line
				if (subtitleLine != subtitleBuffer) {
					addSubtitleToQueue(subtitleBuffer, subtitlePos, _charsetColor, _charset->getCurID(), false, false);
					subtitleLine = subtitleBuffer;
				}

				if (!newLine())
					break;
				continue;
			}

			_charset->_left = _nextLeft;
			_charset->_top = _nextTop;

			if (subtitleLine == subtitleBuffer) {
				subtitlePos.x = _charset->_left;
				// BlastText position is relative to the top of the screen, adjust y-coordinate
				subtitlePos.y = _charset->_top - _screenTop;
			}
			*subtitleLine++ = c;
			*subtitleLine = '\0';

			_talkDelay += (int)VAR(VAR_CHARINC);
		}

		if (subtitleLine != subtitleBuffer) {
			addSubtitleToQueue(subtitleBuffer, subtitlePos, _charsetColor, _charset->getCurID(), false, false);
		}
	} else {
		_talkDelay = VAR(VAR_DEFAULT_TALK_DELAY);
		int newPos = _charsetBufPos;
		while (_charsetBuffer[newPos++])
			_talkDelay += VAR(VAR_CHARINC);

		subtitlePos.x = _string[0].xpos;
		subtitlePos.y = _string[0].ypos;

		addSubtitleToQueue(_charsetBuffer + _charsetBufPos, subtitlePos, _charsetColor, _charset->getCurID(), _string[0].center, _string[0].wrapping);
		_charsetBufPos = newPos;

		_haveMsg = VAR(VAR_HAVE_MSG) = (_game.version == 8 && _string[0].no_talk_anim) ? 2 : 1;
		_keepText = false;
	}

	_string[0] = saveStr;
}

} // End of namespace Scumm

#endif
