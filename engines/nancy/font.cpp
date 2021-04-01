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

	uint width = srcRect.width();
	uint height = srcRect.height();
	uint yOffset = getFontHeight() - height;

	for (uint curY = 0; curY < height; ++curY) {
		for (uint curX = 0; curX < width; ++curX) {
			switch (g_nancy->_graphicsManager->getInputPixelFormat().bytesPerPixel) {
			case 1:
				// TODO
				break;
			case 2: {
				uint16 curColor = *(const uint16 *)_image.getBasePtr(srcRect.left + curX, srcRect.top +  curY);

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
			error("Unsupported FONT character: %c", chr);
		}
	}
	ret = _symbolRects[offset];
	ret.right += 1;
	ret.bottom += 1;
	return ret;
}

} // End of namespace Nancy
