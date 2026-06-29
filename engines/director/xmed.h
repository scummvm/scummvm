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

// XMED ("Xtra MEDia") child resource of an Asset Xtra cast member; observed
// in Physikus (D7.0.2)'s "Text" and "Font" Asset Xtras. Only the Text variant
// is decoded, to recover its displayed string (0x00, ASCII hex length, ',',
// raw platform-encoded bytes).
class XMED {
public:
	XMED(Cast *cast, Common::SeekableReadStreamEndian &stream);

	Cast *_cast;
	bool _isText;            // true for the "Text" Asset Xtra, false otherwise (e.g. an embedded font)
	Common::String _text;    // raw displayed text, platform encoded, with '\r' line breaks
};

} // End of namespace Director

#endif
