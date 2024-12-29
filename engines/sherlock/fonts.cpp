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

#include "common/system.h"
#include "common/platform.h"
#include "sherlock/fonts.h"
#include "sherlock/image_file.h"
#include "sherlock/surface.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

SherlockEngine *Fonts::_vm;
ImageFile *Fonts::_font;
int Fonts::_fontNumber;
int Fonts::_fontHeight;
int Fonts::_widestChar;
uint16 Fonts::_charCount;
byte Fonts::_yOffsets[255];
bool Fonts::_isModifiedEucCn;
bool Fonts::_isBig5;
byte *Fonts::_chineseFont;
Graphics::Big5Font *Fonts::_big5Font;

void Fonts::setVm(SherlockEngine *vm) {
	_vm = vm;
	_font = nullptr;
	_charCount = 0;
	_isModifiedEucCn = (_vm->getLanguage() == Common::Language::ZH_CHN && _vm->getGameID() == GameType::GType_RoseTattoo);
	_isBig5 = (_vm->getLanguage() == Common::Language::ZH_TWN && _vm->getGameID() == GameType::GType_SerratedScalpel);
}

void Fonts::freeFont() {
	delete _font;
	delete _chineseFont;
	_chineseFont = nullptr;
}

void Fonts::setFont(int fontNum) {
	_fontNumber = fontNum;

	// Discard previous font
	delete _font;

	if (IS_SERRATED_SCALPEL) {
		// Scalpel
		if ((_vm->isDemo()) && (!_vm->_interactiveFl)) {
			// Do not set up any font for the non-interactive demo of scalpel
			// The non-interactive demo does not contain any font at all
			return;
		}
		if (_vm->getLanguage() == Common::Language::ZH_TWN)
			fontNum = 2;
	}

	Common::Path fontFilename;

	if (_isModifiedEucCn && _chineseFont == nullptr) {
		Common::File hzk;
		if (!hzk.open("Hzk16.lib")) {
			_isModifiedEucCn = false;
		} else {
			_chineseFont = new byte[hzk.size()];
			hzk.read(_chineseFont, hzk.size());
		}
	}

	if (_isBig5 && _chineseFont == nullptr) {
		Common::File pat;
		if (!pat.open("TEXTPAT.FNT")) {
			_isBig5 = false;
		} else {
			_big5Font = new Graphics::Big5Font();
			_big5Font->loadPrefixedRaw(pat, 14);
		}
	}

	if (_vm->getPlatform() != Common::kPlatform3DO) {
		// PC
		// use FONT[number].VGS, which is a regular sherlock graphic file
		fontFilename = Common::Path(Common::String::format("FONT%d.VGS", fontNum + 1));

		// load font data
		_font = new ImageFile(fontFilename);

		if (IS_SERRATED_SCALPEL) {
			if (_vm->getLanguage() == Common::ES_ESP) {
				if (_fontNumber == 1) {
					// Create a new character - inverted exclamation mark (0x88)
					// Seems this wasn't included originally, but some text has it
					// This was obviously not done in the original game interpreter
					ImageFrame &frameExclamationMark = (*_font)[0]; // get actual exclamation mark
					ImageFrame frameRevExclamationMark;

					frameRevExclamationMark._width = frameExclamationMark._width;
					frameRevExclamationMark._height = frameExclamationMark._height;
					frameRevExclamationMark._paletteBase = frameExclamationMark._paletteBase;
					frameRevExclamationMark._rleEncoded = frameExclamationMark._rleEncoded;
					frameRevExclamationMark._size = frameExclamationMark._size;
					frameRevExclamationMark._frame.create(frameExclamationMark._width, frameExclamationMark._height, Graphics::PixelFormat::createFormatCLUT8());

					byte *frameExclMarkPixels = (byte *)frameExclamationMark._frame.getPixels();
					byte *frameRevExclMarkPixels = (byte *)frameRevExclamationMark._frame.getPixels();

					frameRevExclMarkPixels += frameExclamationMark._width * (frameExclamationMark._height - 1);
					for (uint16 exclMarkY = 0; exclMarkY < frameExclamationMark._height; exclMarkY++) {
						memcpy(frameRevExclMarkPixels, frameExclMarkPixels, frameExclamationMark._width);
						frameRevExclMarkPixels -= frameExclamationMark._width;
						frameExclMarkPixels += frameExclamationMark._width;
					}

					frameRevExclamationMark._offset.x = frameExclamationMark._offset.x;
					frameRevExclamationMark._offset.y = frameExclamationMark._offset.y + 1;

					_font->push_back(frameRevExclamationMark);
				}
			}
		}

	} else {
		// 3DO
		switch (fontNum) {
		case 0:
		case 1:
			fontFilename = "helvetica14.font";
			break;
		case 2:
			fontFilename = "darts.font";
			break;
		default:
			error("setFont(): unsupported 3DO font number");
		}

		// load font data
		_font = new ImageFile3DO(fontFilename, kImageFile3DOType_Font);
	}

	_charCount = _font->size();

	// Iterate through the frames to find the widest and tallest font characters
	_fontHeight = _widestChar = 0;
	for (uint idx = 0; idx < MIN<uint>(_charCount, 128 - 32); ++idx) {
		_fontHeight = MAX(_fontHeight, (*_font)[idx]._frame.h + (*_font)[idx]._offset.y);
		_widestChar = MAX(_widestChar, (*_font)[idx]._frame.w + (*_font)[idx]._offset.x);
	}

	// Initialize the Y offset table for the extended character set
	for (int idx = 0; idx < 255; ++idx) {
		_yOffsets[idx] = 0;

		if (IS_ROSE_TATTOO) {
			if ((idx >= 129 && idx < 135) || (idx >= 136 && idx < 143) || (idx >= 147 && idx < 155) ||
					(idx >= 156 && idx < 165))
				_yOffsets[idx] = 1;
			else if ((idx >= 143 && idx < 146) || idx == 165)
				_yOffsets[idx] = 2;
		}
	}
}

inline byte Fonts::translateChar(byte c) {
	switch (c) {
	case ' ':
		return 0; // translate to first actual character
	case 225:
		// This was done in the German interpreter
		// SH1: happens, when talking to the kid in the 2nd room
		// SH2: happens, when looking at the newspaper right at the start in the backalley
		// Special handling for 0xE1 (German Sharp-S character)
		if (IS_ROSE_TATTOO) {
			return 136; // it got translated to this for SH2
		}
		return 135; // and this for SH1
	default:
		if (IS_SERRATED_SCALPEL) {
			if (_vm->getLanguage() == Common::ES_ESP) {
				if (_fontNumber == 1) {
					// Special workarounds for translated game text, which was skipped because of effectively a bug
					// This was not done in the original interpreter
					// It seems at least the inverted exclamation mark was skipped by the original interpreter /
					// wasn't shown at all.
					// This character is used for example in the alley room, when talking with the inspector after
					// searching the corpse. "[0xAD]Claro! Mi experiencia profesional revela que esta mujer fue asesinada..."
					// The same text gets put inside Watson's journal as well and should be on page 10 right after
					// talking with the inspector. For further study see bug #6931
					// Inverted question mark was also skipped, but at least that character is inside the font already.
					if (c == 0xAD) {
						// inverted exclamation mark
						return 0x88; // our own font character, created during setFont()
					}
					// Inverted question mask is 0x86 (mapped from 0x88)
				}
			}
			if (c >= 0x80) { // German SH1 version did this, but not German SH2
				c--;
			}
			// Spanish SH1 did this (reverse engineered code)
			//if ((c >= 0xA0) && (c <= 0xAD) || (c == 0x82)) {
			//	c--;
			//}
		}
		assert(c > 32); // anything above space is allowed
		return c - 33;
	}
}

Common::String Fonts::unescape(const Common::String& in) {
	if (!_isModifiedEucCn)
		return in;

	bool isInEucEscape = false;
	Common::String out;

	for (const char *curCharPtr = in.c_str(); *curCharPtr; ++curCharPtr) {
		byte curChar = *curCharPtr;
		byte nextChar = curCharPtr[1];

		if (!isInEucEscape && curChar == '@' && nextChar == '$') {
			curCharPtr++;
			isInEucEscape = true;
			out += ' ';
			continue;
		}

		if (isInEucEscape && curChar == '$' && nextChar == '@') {
			curCharPtr++;
			isInEucEscape = false;
			out += ' ';
			continue;
		}

		if (isInEucEscape && curChar >= 0x41 && curChar < 0xa0) {
			out += curChar + 0x60;
			continue;
		}

		out += curChar;
	}

	return out;
}

void Fonts::writeString(BaseSurface *surface, const Common::String &str,
		const Common::Point &pt, int overrideColor) {
	Common::Point charPos = pt;

	if (!_font)
		return;

	bool isInEucEscape = false;

	for (const char *curCharPtr = str.c_str(); *curCharPtr; ++curCharPtr) {
		byte curChar = *curCharPtr;
		byte nextChar = curCharPtr[1];

		if (_isModifiedEucCn && !isInEucEscape && curChar == '@' && nextChar == '$') {
			charPos.x += 10;
			curCharPtr++;
			isInEucEscape = true;
			continue;
		}

		if (_isModifiedEucCn && isInEucEscape && curChar == '$' && nextChar == '@') {
			charPos.x += 10;
			curCharPtr++;
			isInEucEscape = false;
			continue;
		}

		if (_isModifiedEucCn && curChar >= 0x41 && nextChar >= 0x41 && (isInEucEscape || ((curChar >= 0xa1) && (nextChar >= 0xa1)))) {
			int a = curChar >= 0xa1 ? curChar - 0xa1 : curChar - 0x41;
			int b = nextChar >= 0xa1 ? nextChar - 0xa1 : nextChar - 0x41;
			curCharPtr++;
			if (a >= 0 && a <= 93 && b >= 0 && b <= 93) {
				surface->SHbitmapBlitFrom(_chineseFont + 32 * (94 * a + b), kChineseWidth, kChineseHeight, kChineseWidth / 8,
							  Common::Point(charPos.x, charPos.y), overrideColor);
				charPos.x += kChineseWidth;
				continue;
			} else {
				curChar = '?';
			}
		}

		if (curChar == ' ') {
			charPos.x += 5; // hardcoded space
			continue;
		}

		if (_isBig5 && (curChar & 0x80) && nextChar) {
			curCharPtr++;
			uint16 point = (curChar << 8) | nextChar;
			if (_big5Font->drawBig5Char(surface->surfacePtr(), point, charPos, overrideColor)) {
				charPos.x += Graphics::Big5Font::kChineseTraditionalWidth;
				continue;
			}
			curChar = '?';
		}
		
		curChar = translateChar(curChar);

		if (curChar < _charCount) {
			ImageFrame &frame = (*_font)[curChar];
			if (overrideColor) {
				surface->SHoverrideBlitFrom(frame, Common::Point(charPos.x, charPos.y + _yOffsets[curChar]), overrideColor);
			} else {
				surface->SHtransBlitFrom(frame, Common::Point(charPos.x, charPos.y + _yOffsets[curChar]));
			}
			charPos.x += frame._frame.w + 1;
		} else {
			warning("Invalid character encountered - %d", (int)curChar);
		}
	}
}

int Fonts::stringWidth(const Common::String &str) {
	int width = 0;

	if (!_font)
		return 0;

	bool isInEucEscape = false;

	for (int idx = 0; idx < (int) str.size(); ) {
		byte curChar = str.c_str()[idx];
		byte nextChar = str.c_str()[idx+1];

		if (_isModifiedEucCn && !isInEucEscape && curChar == '@' && nextChar == '$') {
			width += charWidth(' ');
			idx += 2;
			isInEucEscape = true;
			continue;
		}

		if (_isModifiedEucCn && isInEucEscape && curChar == '$' && nextChar == '@') {
			width += charWidth(' ');
			idx += 2;
			isInEucEscape = false;
			continue;
		}

		if (_isModifiedEucCn && curChar >= 0x41 && nextChar >= 0x41 && (isInEucEscape || ((curChar >= 0xa1) && (nextChar >= 0xa1)))) {
			width += kChineseWidth;
			idx += 2;
			continue;
		}

		width += charWidth(str.c_str(), idx);
	}

	return width;
}

int Fonts::stringHeight(const Common::String &str) {
	int height = 0;

	if (!_font)
		return 0;

	bool isInEucEscape = false;

	for (const char *c = str.c_str(); *c; ++c) {
		byte curChar = *c;
		byte nextChar = c[1];

		if (_isModifiedEucCn && !isInEucEscape && curChar == '@' && nextChar == '$') {
			height = MAX(height, charHeight(' '));
			c++;
			isInEucEscape = true;
			continue;
		}

		if (_isModifiedEucCn && isInEucEscape && curChar == '$' && nextChar == '@') {
			height = MAX(height, charHeight(' '));
			c++;
			isInEucEscape = false;
			continue;
		}

		if (_isModifiedEucCn && curChar >= 0x41 && nextChar >= 0x41 && (isInEucEscape || ((curChar >= 0xa1) && (nextChar >= 0xa1)))) {
			height = MAX(height, kChineseHeight);
			c++;
			continue;
		}

		if (_isBig5 && _big5Font && (curChar & 0x80) && nextChar) {
			height = MAX(height, _big5Font->getFontHeight());
			c++;
			continue;
		}

		height = MAX(height, charHeight(*c));
	}

	return height;
}


int Fonts::charWidth(const char *p, int &idx) {
	byte curChar = p[idx];
	byte nextChar = p[idx + 1];
	if (_isBig5 && (curChar & 0x80) && nextChar) {
		idx += 2;
		return Graphics::Big5Font::kChineseTraditionalWidth;
	}

	idx++;

	if (!_font)
		return 0;

	if (curChar == ' ') {
		return 5; // hardcoded space
	}

	byte translatedChar = translateChar(curChar);

	if (translatedChar < _charCount)
		return (*_font)[translatedChar]._frame.w + 1;
	return 0;
}

Common::Array<Common::String> Fonts::wordWrap(const Common::String &str, uint maxWidth, Common::String &rem, uint maxChars, uint maxLines, bool skipHeadAt) {
	Common::Array<Common::String> lines;
	int strIdx = 0;

	bool isInEucEscape = false;
	do {
		uint width = 0;
		uint numChars = 0;
		int spaceIdx = 0;
		bool spacePNeedsEndEscape = false;
		bool spacePNeedsBeginEscape = false;
		int lineStartIdx = strIdx;
		// Invariant: lastCharIdx is either -1
		// or is exactly one character behind strP
		// and in the same escape state.
		int lastCharIdx = -1;
		bool isLineStartPInEucEscape = isInEucEscape;

		if (skipHeadAt) {
			// If the first character is a '@' flagging a title line, then move
			// past it, so the @ won't be included in the line width calculation
			if (strIdx + 1 < (int)str.size() && str[strIdx] == '@' && str[strIdx + 1] != '$')
				++strIdx;
		}

		// Find how many characters will fit on the next line
		while (width < maxWidth && numChars < maxChars && strIdx < (int)str.size() && str[strIdx] != '\n') {
			if (_isModifiedEucCn) {
				byte curChar = str[strIdx];
				byte nextChar = strIdx + 1 < (int)str.size() ? str[strIdx + 1] : 0;
				if (!isInEucEscape && curChar == '@' && nextChar == '$') {
					width += charWidth(' ');
					numChars++;
					if (lineStartIdx != strIdx) {
						spaceIdx = strIdx;
						spacePNeedsEndEscape = isInEucEscape;
						spacePNeedsBeginEscape = true;
					}
					lastCharIdx = -1;
					strIdx += 2;
					isInEucEscape = true;
					continue;
				}

				if (isInEucEscape && curChar == '$' && nextChar == '@') {
					width += charWidth(' ');
					numChars++;
					spaceIdx = strIdx;
					lastCharIdx = -1;
					strIdx += 2;
					spacePNeedsEndEscape = isInEucEscape;
					spacePNeedsBeginEscape = false;
					isInEucEscape = false;
					continue;
				}

				if (curChar >= 0x41 && nextChar >= 0x41 && (isInEucEscape || ((curChar >= 0xa1) && (nextChar >= 0xa1)))) {
					width += kChineseWidth;
					lastCharIdx = strIdx;
					strIdx += 2;
					numChars++;
					continue;
				}
			}

			// Keep track of the last space
			if (str[strIdx] == ' ') {
				spacePNeedsEndEscape = isInEucEscape;
				spacePNeedsBeginEscape = isInEucEscape;
				spaceIdx = strIdx;
			}
			lastCharIdx = strIdx;
			width += charWidth(str.c_str(), strIdx);
			numChars++;
		}

		bool previousEucEscape = isInEucEscape;

		// If the line was too wide to fit on a single line, go back to the last space
		// if there was one, or otherwise simply break the line at this point
		if (width >= maxWidth || numChars >= maxChars) {
			if (spaceIdx > 0) {
				previousEucEscape = spacePNeedsEndEscape;
				isInEucEscape = spacePNeedsBeginEscape;
				strIdx = spaceIdx;
			} else if (lastCharIdx > 0 && lastCharIdx != lineStartIdx) {
				strIdx = lastCharIdx;
			}
		}

		Common::String line = str.substr(lineStartIdx, strIdx - lineStartIdx);
		assert(!line.contains('\n'));
		if (!line.hasPrefix("@$") && isLineStartPInEucEscape)
			line = "@$" + line;

		if (!line.hasSuffix("$@") && previousEucEscape)
			line = line + "$@";

		// Add the line to the output array
		lines.push_back(line);

		// Move the string ahead to the next line
		while (strIdx < (int)str.size() && (str[strIdx] == '\n' || str[strIdx] == ' ' || str[strIdx] == '\r'))
			++strIdx;
	} while (strIdx < (int)str.size() && lines.size() < maxLines);

	rem = str.substr(strIdx);

	return lines;
}

Common::Array<Common::String> Fonts::wordWrap(const Common::String &str, uint maxWidth, uint maxChars, uint maxLines, bool skipHeadAt) {
	Common::String rem;
	return wordWrap(str, maxWidth, rem, maxChars, maxLines, skipHeadAt);
}

int Fonts::charWidth(char c) {
	char s[2] = { c, '\0' };
	int idx = 0;
	return charWidth(s, idx);
}

int Fonts::charHeight(unsigned char c) {
	byte curChar;

	if (!_font)
		return 0;

	// Space is supposed to be handled like the first actual character (which is decimal 33)
	curChar = translateChar(c);

	assert(curChar < _charCount);
	const ImageFrame &img = (*_font)[curChar];
	return img._height + img._offset.y + 1;
}

} // End of namespace Sherlock
