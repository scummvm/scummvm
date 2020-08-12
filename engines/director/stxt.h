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

#ifndef DIRECTOR_STXT_H
#define DIRECTOR_STXT_H

namespace Common {
class ReadStreamEndian;
}

namespace Director {

struct FontStyle {
	uint32 formatStartOffset;
	uint16 height;
	uint16 ascent;

	uint16 fontId;
	byte textSlant;

	uint16 fontSize;

	uint16 r, g, b;

	FontStyle();
	void read(Common::ReadStreamEndian &textStream);
};

class Stxt {
public:
	Stxt(Cast *cast, Common::SeekableReadStreamEndian &textStream);

public:
	Cast *_cast;
	Common::String _ftext;
	Common::String _ptext;
	TextType _textType;
	TextAlignType _textAlign;
	SizeType _textShadow;

	uint32 _size;

	FontStyle _style;
	uint16 _unk1f;
	uint16 _unk2f;
	byte _unk3f;
};

} // End of namespace Director

#endif
