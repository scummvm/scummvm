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

#ifndef DIRECTOR_XMED_H
#define DIRECTOR_XMED_H

#include "common/str.h"

namespace Common {
class SeekableReadStreamEndian;
}

namespace Director {

class Cast;

// XMED ("Xtra MEDia") is the child resource used by Director 7+ Xtra cast
// members. The "Text" Asset Xtra stores its editable rich text here (a custom
// serialized property list), while the "Font" Asset Xtra stores an embedded
// PFR (Portable Font Resource) font.
//
// We only decode the Text Asset variant far enough to recover the displayed
// string so it can be rendered through the regular text pipeline. The format
// is a tagged property list where the displayed string is serialized as a
// 0x00 byte, an ASCII hexadecimal byte-length, a ',' separator and then the
// raw (platform-encoded) characters.
class XMED {
public:
	XMED(Cast *cast, Common::SeekableReadStreamEndian &stream);

	Cast *_cast;
	bool _isText;            // true for the "Text" Asset Xtra, false otherwise (e.g. an embedded font)
	Common::String _text;    // raw displayed text, platform encoded, with '\r' line breaks
};

} // End of namespace Director

#endif
