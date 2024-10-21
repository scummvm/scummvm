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

//=============================================================================
//
// WFNFont - an immutable AGS font object.
//
//-----------------------------------------------------------------------------
//
// WFN format:
// - signature            ( 15 )
// - offsets table offset (  2 )
// - characters table (for unknown number of char items):
// -     width            (  2 )
// -     height           (  2 )
// -     pixel bits       ( (width / 8 + 1) * height )
// -     any unknown data
// - offsets table (for X chars):
// -     character offset (  2 )
//
// NOTE: unfortunately, at the moment the format does not provide means to
// know the number of supported characters for certain, and the size of the
// data (file) is used to determine that.
//
//=============================================================================

#ifndef AGS_SHARED_FONT_WFN_FONT_H
#define AGS_SHARED_FONT_WFN_FONT_H

#include "common/std/vector.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

enum WFNError {
	kWFNErr_NoError,
	kWFNErr_BadSignature,
	kWFNErr_BadTableAddress,
	kWFNErr_HasBadCharacters
};

struct WFNChar {
	uint16_t       Width;
	uint16_t       Height;
	const uint8_t *Data;

	WFNChar();

	inline size_t GetRowByteCount() const {
		return (Width + 7) / 8;
	}

	inline size_t GetRequiredPixelSize() const {
		return GetRowByteCount() * Height;
	}

	// Ensure character's width & height fit in given number of pixel bytes
	void RestrictToBytes(size_t bytes);
};


class WFNFont {
public:
	inline uint16_t GetCharCount() const {
		return static_cast<uint16_t>(_refs.size());
	}

	// Get WFN character for the given code; if the character is missing, returns empty character
	const WFNChar &GetChar(uint16_t code) const;

	void Clear();
	// Reads WFNFont object, using data_size bytes from stream; if data_size = 0,
	// the available stream's length is used instead. Returns error code.
	WFNError ReadFromFile(AGS::Shared::Stream *in, const soff_t data_size = 0);

protected:
	std::vector<const WFNChar *> _refs;      // reference array, contains pointers to elements of _items
	std::vector<WFNChar>        _items;     // actual character items
	std::vector<uint8_t>        _pixelData; // pixel data array
};

} // namespace AGS3

#endif
