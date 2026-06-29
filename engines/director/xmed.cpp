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

#include "common/array.h"
#include "common/stream.h"

#include "director/director.h"
#include "director/xmed.h"

namespace Director {

static bool isHexDigit(byte c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint hexValue(byte c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return c - 'a' + 10;
}

XMED::XMED(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast), _isText(false) {
	uint32 size = stream.size();
	if (size < 4)
		return;

	Common::Array<byte> data(size);
	if (stream.read(data.data(), size) != size)
		return;

	// The "Text" Asset Xtra serialization starts with the ASCII bytes "FFFF".
	// Other XMED payloads (e.g. an embedded "PFR1" font from the "Font" Asset
	// Xtra) are not text and are left alone.
	if (memcmp(data.data(), "FFFF", 4) != 0)
		return;

	_isText = true;

	// The displayed string is serialized as: 0x00 <ASCII hex length> ',' <raw bytes>.
	// Scan for the first such record whose payload is predominantly printable;
	// that is the editable text of the field.
	for (uint32 i = 0; i + 2 < size; i++) {
		if (data[i] != 0x00)
			continue;

		uint32 j = i + 1;
		uint32 len = 0;
		int digits = 0;
		while (j < size && isHexDigit(data[j]) && digits < 6) {
			len = len * 16 + hexValue(data[j]);
			j++;
			digits++;
		}

		if (digits == 0 || len == 0 || j >= size || data[j] != ',')
			continue;
		j++; // skip the ',' separator

		if (j + len > size)
			continue;

		uint32 printable = 0;
		for (uint32 k = 0; k < len; k++) {
			byte c = data[j + k];
			if ((c >= 0x20 && c <= 0x7e) || c >= 0x80 || c == '\r' || c == '\n' || c == '\t')
				printable++;
		}

		// Require at least 80% printable to reject false matches.
		if (printable * 5 < len * 4)
			continue;

		_text = Common::String((const char *)&data[j], len);
		break;
	}
}

} // End of namespace Director
