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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/font.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"
#include "engines/nancy/enginedata.h"

namespace Nancy {

void Font::read(Common::SeekableReadStream &stream) {
	_transColor = g_nancy->_graphics->getTransColor();
	_maxCharWidth = 0;
	_fontHeight = 0;
	uint16 numCharacters = 78;

	Common::Path imageName;
	readFilename(stream, imageName);

	g_nancy->_resource->loadImage(imageName, _image);
	_image.setTransparentColor(g_nancy->_graphics->getTransColor());

	char desc[31];
	stream.read(desc, 30);
	desc[30] = '\0';
	_description = desc;

	_color0CoordsOffset.x = stream.readUint32LE();
	_color0CoordsOffset.y = stream.readUint32LE();
	_color1CoordsOffset.x = stream.readUint32LE();
	_color1CoordsOffset.y = stream.readUint32LE();

	_spaceWidth = stream.readUint16LE();
	_charSpace = stream.readSint16LE() - 1; // Account for the added pixel in readRect

	_uppercaseOffset					= stream.readUint16LE();
	_lowercaseOffset					= stream.readUint16LE();
	_digitOffset						= stream.readUint16LE();
	_periodOffset						= stream.readUint16LE();
	_commaOffset						= stream.readUint16LE();
	_equalitySignOffset					= stream.readUint16LE();
	_colonOffset						= stream.readUint16LE();
	_dashOffset							= stream.readUint16LE();
	_questionMarkOffset					= stream.readUint16LE();
	_exclamationMarkOffset				= stream.readUint16LE();
	_percentOffset						= stream.readUint16LE();
	_ampersandOffset					= stream.readUint16LE();
	_asteriskOffset						= stream.readUint16LE();
	_leftBracketOffset					= stream.readUint16LE();
	_rightBracketOffset					= stream.readUint16LE();
	_plusOffset							= stream.readUint16LE();
	_apostropheOffset					= stream.readUint16LE();
	_semicolonOffset					= stream.readUint16LE();
	_slashOffset						= stream.readUint16LE();

	if (g_nancy->getGameLanguage() == Common::RU_RUS && g_nancy->getGameType() >= kGameTypeNancy5 && g_nancy->getGameType() != kGameTypeNancy6) {
		// Only extract the lowercase/uppercase offsets, since the letters are in order in the FONT data
		_cyrillicLowercaseOffset 		= stream.readUint16LE();
		stream.skip(72);
		_cyrillicUppercaseOffset 		= stream.readUint16LE();
		stream.skip(2);

		numCharacters = 179;
	} else {
		if (g_nancy->getGameType() >= kGameTypeNancy6) {
			// Nancy6 added more characters to its fonts
			_aWithGraveOffset 				= stream.readUint16LE();
			_cWithCedillaOffset				= stream.readUint16LE();
			_eWithGraveOffset 				= stream.readUint16LE();
			_eWithAcuteOffset 				= stream.readUint16LE();
			_eWithCircumflexOffset			= stream.readUint16LE();
			_eWithDiaeresisOffset			= stream.readUint16LE();
			_oWithCircumflexOffset			= stream.readUint16LE();
			_uppercaseAWithGraveOffset		= stream.readUint16LE();
			_aWithCircumflexOffset			= stream.readUint16LE();
			_iWithCircumflexOffset			= stream.readUint16LE();
			_uWithGraveOffset 				= stream.readUint16LE();
			_uppercaseAWithDiaeresisOffset	= stream.readUint16LE();
			_aWithDiaeresisOffset			= stream.readUint16LE();
			_uppercaseOWithDiaeresisOffset	= stream.readUint16LE();
			_oWithDiaeresisOffset			= stream.readUint16LE();
			_uppercaseUWithDiaeresisOffset	= stream.readUint16LE();
			_uWithDiaeresisOffset			= stream.readUint16LE();
			_invertedExclamationMarkOffset	= stream.readUint16LE();
			_invertedQuestionMarkOffset		= stream.readUint16LE();
			_uppercaseNWithTildeOffset		= stream.readUint16LE();
			_nWithTildeOffset				= stream.readUint16LE();
			_uppercaseEWithAcuteOffset		= stream.readUint16LE();
			_aWithAcuteOffset				= stream.readUint16LE();
			_iWithAcuteOffset				= stream.readUint16LE();
			_oWithAcuteOffset				= stream.readUint16LE();
			_uWithAcuteOffset				= stream.readUint16LE();
			_eszettOffset					= stream.readUint16LE();

			numCharacters = 105;
		}
	}

	_characterRects.resize(numCharacters);
	for (uint i = 0; i < numCharacters; ++i) {
		Common::Rect &cur = _characterRects[i];
		readRect(stream, cur);

		if (g_nancy->getGameType() == kGameTypeVampire) {
			++cur.bottom;
			++cur.right;
		}

		_maxCharWidth = MAX<int>(cur.width(), _maxCharWidth);
		_fontHeight = MAX<int>(cur.height(), _fontHeight);
	}

	if (g_nancy->getGameType() >= kGameTypeNancy6) {
		_fontHeight = getCharWidth('o') * 2 - 1;
	}

	_textboxData = GetEngineData(TBOX);
	assert(_textboxData);
}

int Font::getCharWidth(uint32 chr) const {
	return getCharacterSourceRect(chr).width() + _charSpace;
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	Common::Rect srcRect = getCharacterSourceRect(chr);
	if (color == 0) {
		srcRect.translate(_color0CoordsOffset.x, _color0CoordsOffset.y);
	} else if (color == 1) {
		srcRect.translate(_color1CoordsOffset.x, _color1CoordsOffset.y);
	}

	uint vampireAdjust = g_nancy->getGameType() == kGameTypeVampire ? 1 : 0;
	srcRect.setWidth(MAX<int>(srcRect.width() - vampireAdjust, 0));
	uint height = srcRect.height();
	uint yOffset = getFontHeight() - height;
	srcRect.setHeight(MAX<int>(height - vampireAdjust, 0));

	// Create a wrapper ManagedSurface to handle blitting/format differences
	Graphics::ManagedSurface dest;
	dest.w = dst->w;
	dest.h = dst->h;
	dest.pitch = dst->pitch;
	dest.setPixels(dst->getPixels());
	dest.format = dst->format;

	dest.blitFrom(_image, srcRect, Common::Point(x, y + yOffset));
}

void Font::wordWrap(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth) const {
	Common::String temp;
	for (const char *c = str.begin(); c != str.end(); ++c) {
		if (*c == '\n') {
			lines.push_back(temp);
			temp.clear();
			continue;
		}

		temp += *c;
		int size = getStringWidth(temp) + (lines.size() == 0 ? initWidth : 0);
		if (size >= maxWidth) {
			do {
				temp.deleteLastChar();
				--c;
			} while (temp.size() && temp.lastChar() != ' ');

			lines.push_back(temp);
			temp.clear();
		}
	}

	if (temp.size()) {
		lines.push_back(temp);
	}
}

Common::Rect Font::getCharacterSourceRect(char chr) const {
	// Map a character to its source rect inside the font data.
	// The original engine devs had some _interesting_ ideas on how to store font data,
	// which makes the ridiculous switch statements below a necessity
	using namespace Common;
	int offset = -1;
	Common::Rect ret;

	if ((uint8)chr > 127) {
		bool selectedRussian = false;
		if (g_nancy->getGameType() >= kGameTypeNancy5 && g_nancy->getGameType() != kGameTypeNancy6 && g_nancy->getGameLanguage() == Common::RU_RUS) {
			// Handle Russian strings in nancy5 and up, which use Windows-1251 encoding
			// (except for nancy6, which goes back to latinizing russian)
			// We cannot rely on isUpper/isLower, since they use the C locale settings
			if ((uint8)chr >= 0xC0 && (uint8)chr <= 0xDF) {
				// capital letters
				offset = (uint8)chr + _cyrillicUppercaseOffset - 0xC0;
				selectedRussian = true;
			} else if ((uint8)chr >= 0xE0) {
				offset = (uint8)chr + _cyrillicLowercaseOffset - 0xE0;
				selectedRussian = true;
			}
		}

		if (!selectedRussian) {
			// Nancy6 introduced extended ASCII characters
			switch (chr) {
			case '\xe0':
				offset = _aWithGraveOffset;
				break;
			case '\xe7':
				offset = _cWithCedillaOffset;
				break;
			case '\xe8':
				offset = _eWithGraveOffset;
				break;
			case '\xe9':
				offset = _eWithAcuteOffset;
				break;
			case '\xea':
				offset = _eWithCircumflexOffset;
				break;
			case '\xeb':
				offset = _eWithDiaeresisOffset;
				break;
			case '\xf4':
				offset = _oWithCircumflexOffset;
				break;
			case '\xc0':
				offset = _uppercaseAWithGraveOffset;
				break;
			case '\xe2':
				offset = _aWithCircumflexOffset;
				break;
			case '\xee':
				offset = _iWithCircumflexOffset;
				break;
			case '\xf9':
				offset = _uWithGraveOffset;
				break;
			case '\xc4':
				offset = _uppercaseAWithDiaeresisOffset;
				break;
			case '\xe4':
				offset = _aWithDiaeresisOffset;
				break;
			case '\xd6':
				offset = _uppercaseOWithDiaeresisOffset;
				break;
			case '\xf6':
				offset = _oWithDiaeresisOffset;
				break;
			case '\xdc':
				offset = _uppercaseUWithDiaeresisOffset;
				break;
			case '\xfc':
				offset = _uWithDiaeresisOffset;
				break;
			case '\xa1':
				offset = _invertedExclamationMarkOffset;
				break;
			case '\xbf':
				offset = _invertedQuestionMarkOffset;
				break;
			case '\xd1':
				offset = _uppercaseNWithTildeOffset;
				break;
			case '\xf1':
				offset = _nWithTildeOffset;
				break;
			case '\xc9':
				offset = _uppercaseEWithAcuteOffset;
				break;
			case '\xe1':
				offset = _aWithAcuteOffset;
				break;
			case '\xed':
				offset = _iWithAcuteOffset;
				break;
			case '\xf3':
				offset = _oWithAcuteOffset;
				break;
			case '\xfa':
				offset = _uWithAcuteOffset;
				break;
			case '\xdf':
				offset = _eszettOffset;
				break;
			case '\x92':
				if (g_nancy->getGameType() == kGameTypeNancy4) {
					// Improvement: we fix a specific broken string in nancy4 ("It's too dark..." when entering a dark staircase)
					offset = _apostropheOffset;
				} else {
					offset = -1;
				}
				break;
			default:
				offset = -1;
				break;
			}
		}
	} else if (isUpper(chr)) {
		offset = chr + _uppercaseOffset - 0x41;
	} else if (isLower(chr)) {
		offset = chr + _lowercaseOffset - 0x61;
	} else if (isDigit(chr)) {
		offset = chr + _digitOffset - 0x30;
	} else if (chr == '\t') {
		ret.setWidth((_spaceWidth - 1) * _textboxData->tabWidth);
		return ret;
	} else if (isSpace(chr)) {
		ret.setWidth(_spaceWidth - 1);
		return ret;
	} else if (isPunct(chr)) {
		switch (chr) {
		case '.':
			offset = _periodOffset;
			break;
		case ',':
			offset = _commaOffset;
			break;
		case '=':
			offset = _equalitySignOffset;
			break;
		case ':':
			offset = _colonOffset;
			break;
		case '-':
			offset = _dashOffset;
			break;
		case '?':
			offset = _questionMarkOffset;
			break;
		case '!':
			offset = _exclamationMarkOffset;
			break;
		case '%':
			offset = _percentOffset;
			break;
		case '&':
			offset = _ampersandOffset;
			break;
		case '*':
			offset = _asteriskOffset;
			break;
		case '(':
			offset = _leftBracketOffset;
			break;
		case ')':
			offset = _rightBracketOffset;
			break;
		case '+':
			offset = _plusOffset;
			break;
		case '\'':
			offset = _apostropheOffset;
			break;
		case ';':
			offset = _semicolonOffset;
			break;
		case '/':
			offset = _slashOffset;
			break;
		default:
			offset = -1;
			break;
		}
	}

	if (offset == -1) {
		// There is at least one malformed string in nancy4 that will reach this.
		// Also, this helps support displaying saves that were originally made in the GMM
		// inside the in-game load/save menu, since those _may_ contain characters not present in the font

		// When reaching an invalid char, the original engine repeated the last printed character,
		// but we print a question mark instead.
		offset = _questionMarkOffset;
	}

	ret = _characterRects[offset];
	return ret;
}

} // End of namespace Nancy
