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
 */

#ifndef NANCY_FONT_H
#define NANCY_FONT_H

#include "common/array.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;

class Font : public Graphics::Font {
public:
	Font() = default;
	~Font() = default;

	void read(Common::SeekableReadStream &stream);

	int getFontHeight() const override { return _fontHeight - 1; }
	int getMaxCharWidth() const override { return _maxCharWidth; }
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override { return 1; }

	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	const Graphics::ManagedSurface &getImageSurface() const { return _image; }

	// Custom word wrapping function to fix an edge case with overflowing whitespaces
	void wordWrap(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth = 0) const;

private:
	Common::Rect getCharacterSourceRect(char chr) const;

	Common::String _description;
	Common::Point _color0CoordsOffset;
	Common::Point _color1CoordsOffset; // Added to source rects when colored text is requested
	int16 _charSpace = 0;
	uint16 _spaceWidth = 0;

	// Specific offsets into the _characterRects array
	uint16 _uppercaseOffset			= 0;
	uint16 _lowercaseOffset			= 0;
	uint16 _digitOffset				= 0;
	uint16 _periodOffset			= 0;
	uint16 _commaOffset				= 0;
	uint16 _equalitySignOffset		= 0;
	uint16 _colonOffset				= 0;
	uint16 _dashOffset				= 0;
	uint16 _questionMarkOffset		= 0;
	uint16 _exclamationMarkOffset	= 0;
	uint16 _percentOffset			= 0;
	uint16 _ampersandOffset			= 0;
	uint16 _asteriskOffset			= 0;
	uint16 _leftBracketOffset		= 0;
	uint16 _rightBracketOffset		= 0;
	uint16 _plusOffset				= 0;
	uint16 _apostropheOffset		= 0;
	uint16 _semicolonOffset			= 0;
	uint16 _slashOffset				= 0;

	// More specific offsets for extended ASCII characters. Introduced in nancy6
	int16 _aWithGraveOffset					= -1;
	int16 _cWithCedillaOffset				= -1;
	int16 _eWithGraveOffset					= -1;
	int16 _eWithAcuteOffset					= -1;
	int16 _eWithCircumflexOffset			= -1;
	int16 _eWithDiaeresisOffset				= -1;
	int16 _oWithCircumflexOffset			= -1;
	int16 _uppercaseAWithGraveOffset		= -1;
	int16 _aWithCircumflexOffset			= -1;
	int16 _iWithCircumflexOffset			= -1;
	int16 _uWithGraveOffset 				= -1;
	int16 _uppercaseAWithDiaeresisOffset	= -1;
	int16 _aWithDiaeresisOffset				= -1;
	int16 _uppercaseOWithDiaeresisOffset	= -1;
	int16 _oWithDiaeresisOffset				= -1;
	int16 _uppercaseUWithDiaeresisOffset	= -1;
	int16 _uWithDiaeresisOffset				= -1;
	int16 _invertedExclamationMarkOffset	= -1;
	int16 _invertedQuestionMarkOffset		= -1;
	int16 _uppercaseNWithTildeOffset		= -1;
	int16 _nWithTildeOffset					= -1;
	int16 _uppercaseEWithAcuteOffset		= -1;
	int16 _aWithAcuteOffset					= -1;
	int16 _iWithAcuteOffset					= -1;
	int16 _oWithAcuteOffset					= -1;
	int16 _uWithAcuteOffset					= -1;
	int16 _eszettOffset						= -1;

	Common::Array<Common::Rect> _characterRects;

	Graphics::ManagedSurface _image;

	const struct TBOX *_textboxData = nullptr;

	int _fontHeight = 0;
	int _maxCharWidth = 0;
	uint _transColor = 0;
};

} // End of namespace Nancy

#endif // NANCY_FONT_H
