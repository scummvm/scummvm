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

namespace Nancy {

void Font::read(Common::SeekableReadStream &stream) {
	_transColor = g_nancy->_graphicsManager->getTransColor();
	_maxCharWidth = 0;
	_fontHeight = 0;

	Common::String imageName;
	readFilename(stream, imageName);

	g_nancy->_resource->loadImage(imageName, _image);

	if (g_nancy->getGameType() == kGameTypeVampire) {
		// Hacky fix for load/save menu
		_image.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
	}

	char desc[0x20];
	stream.read(desc, 0x20);
	desc[0x1F] = '\0';
	_description = desc;
	stream.skip(8);
	_colorCoordsOffset.x = stream.readUint16LE();
	_colorCoordsOffset.y = stream.readUint16LE();

	stream.skip(2);
	_spaceWidth = stream.readUint16LE();
	stream.skip(2);
	_uppercaseOffset = stream.readUint16LE();
	_lowercaseOffset = stream.readUint16LE();
	_digitOffset = stream.readUint16LE();
	_periodOffset = stream.readUint16LE();
	_commaOffset = stream.readUint16LE();
	_equalitySignOffset = stream.readUint16LE();
	_colonOffset = stream.readUint16LE();
	_dashOffset = stream.readUint16LE();
	_questionMarkOffset = stream.readUint16LE();
	_exclamationMarkOffset = stream.readUint16LE();
	_percentOffset = stream.readUint16LE();
	_ampersandOffset = stream.readUint16LE();
	_asteriskOffset = stream.readUint16LE();
	_leftBracketOffset = stream.readUint16LE();
	_rightBracketOffset = stream.readUint16LE();
	_plusOffset = stream.readUint16LE();
	_apostropheOffset = stream.readUint16LE();
	_semicolonOffset = stream.readUint16LE();
	_slashOffset = stream.readUint16LE();

	_symbolRects.reserve(78);
	for (uint i = 0; i < 78; ++i) {
		_symbolRects.push_back(Common::Rect());
		Common::Rect &cur = _symbolRects[i];
		readRect(stream, cur);

		if (g_nancy->getGameType() == kGameTypeVampire) {
			++cur.bottom;
			++cur.right;
		}

		_maxCharWidth = MAX<int>(cur.width(), _maxCharWidth);
		_fontHeight = MAX<int>(cur.height(), _maxCharWidth);
	}
}

int Font::getCharWidth(uint32 chr) const {
	return getCharacterSourceRect(chr).width();
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	Common::Rect srcRect = getCharacterSourceRect(chr);
	if (color != 0) {
		srcRect.translate(_colorCoordsOffset.x, _colorCoordsOffset.y);
	}

	uint vampireAdjust = g_nancy->getGameType() == kGameTypeVampire ? 1 : 0;
	uint width = MAX<int>(srcRect.width() - vampireAdjust, 0);
	uint height = srcRect.height();
	uint yOffset = getFontHeight() - height;
	height = MAX<int>(height - vampireAdjust, 0);

	for (uint curY = 0; curY < height; ++curY) {
		for (uint curX = 0; curX < width; ++curX) {
			switch (g_nancy->_graphicsManager->getInputPixelFormat().bytesPerPixel) {
			case 1: {
				byte colorID = *(const byte *)_image.getBasePtr(srcRect.left + curX, srcRect.top + curY);

				if (colorID != _transColor) {
					uint8 palette[1 * 3];
					_image.grabPalette(palette, colorID, 1);
					*(uint16 *)dst->getBasePtr(x + curX, y + yOffset + curY) = dst->format.RGBToColor(palette[0], palette[1], palette[2]);
				}

				break;
			}
			case 2: {
				uint16 curColor = *(const uint16 *)_image.getBasePtr(srcRect.left + curX, srcRect.top + curY);

				if (curColor != _transColor) {
					uint8 r, g, b;
					_image.format.colorToRGB(curColor, r, g, b);
					*(uint16 *)dst->getBasePtr(x + curX, y + yOffset + curY) = dst->format.RGBToColor(r, g, b);
				}

				break;
			}
			default:
				break;
			}
		}
	}
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
	using namespace Common;
	uint offset = 0;
	Common::Rect ret;

	if (isUpper(chr)) {
		offset = chr + _uppercaseOffset - 0x41;
	} else if (isLower(chr)) {
		offset = chr + _lowercaseOffset - 0x61;
	} else if (isDigit(chr)) {
		offset = chr + _digitOffset - 0x30;
	} else if (isSpace(chr)) {
		ret.setWidth(_spaceWidth - 1); // Not sure why we sutract 1
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
			// Replace unknown characters with question marks. This shouldn't happen normally,
			// but we need it to support displaying saves that were originally made in the GMM
			// inside the in-game load/save menu.
			offset = _questionMarkOffset;
			break;
		}
	}
	ret = _symbolRects[offset];
	return ret;
}

} // End of namespace Nancy
