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

#include "mohawk/zoombini_resource.h"
#include "zoombini_resource.h"

#include "common/stream.h"

#include <errno.h>

namespace Mohawk {

// FIXME: These logical substream checks cannot prove physical archive bounds
// until @ref MohawkArchive::openStream() validates archive-table offsets,
// counts, one-based indices, and resource extents before publishing substreams.
// The shared parser currently accepts unchecked allocation counts and table
// indices, and an invalid extent can cross the physical stream or an adjacent
// resource. Parse the complete table transactionally with checked arithmetic
// and exact-read validation before treating these checks as a physical-byte
// guarantee.
bool ZmbResource::hasBytes(Common::SeekableReadStream *stream, int64 byteCount) {
	if (!stream || byteCount < 0)
		return false;

	const int64 streamSize = stream->size();
	const int64 streamPosition = stream->pos();
	if (streamSize < 0 || streamPosition < 0 || streamSize < streamPosition)
		return false;

	return byteCount <= streamSize - streamPosition;
}

bool ZmbResource::hasSize(Common::SeekableReadStream *stream, int64 minimumSize, int64 maximumSize) {
	if (!stream || minimumSize < 0 || maximumSize < minimumSize)
		return false;

	const int64 streamSize = stream->size();
	if (streamSize < 0)
		return false;

	return minimumSize <= streamSize && streamSize <= maximumSize;
}

bool ZmbResource::parseInt(const char *str, int32 &result) {
	if (!str || *str == '\0') {
		warning("Error: Empty string\n");
		return false;
	}

	char *endPtr = nullptr;
	int base = 10;

	// Check if it's a hexadecimal number (starts with "0x" or "0X")
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		base = 16;
	}

	errno = 0;
	long parsed = strtol(str, &endPtr, base);

	// Check for conversion errors
	if (errno != 0) {
		warning("Error: Integer overflow or underflow in '%s'\n", str);
		return false;
	}

	// Check if any characters were converted
	if (endPtr == str) {
		warning("Error: '%s' is not a valid integer\n", str);
		return false;
	}

	// Check if there are trailing characters
	if (*endPtr != '\0') {
		warning("Error: '%s' contains invalid characters\n", str);
		return false;
	}

	result = static_cast<int32>(parsed);
	return true;
}

bool ZmbResource::parse(const char *str, ZmbResource &outRes) {
	bool success = true;
	int32 parsedId = 0;

	// Ex) s:4100, p:4100, s:0x1004, p:0x1004
	if (2 < strlen(str) && str[1] == ':') {
		if (str[0] == 's' || str[0] == 'S')
			outRes._archiveKind = ZmbResource::kSystem;
		else if (str[0] == 'p' || str[0] == 'P')
			outRes._archiveKind = ZmbResource::kPage;
		else
			success = false;

		if (success && !parseInt(str + 2, parsedId))
			success = false;
	} else {
		// Defaults to page, Ex) 4100, 0x1004
		outRes._archiveKind = ZmbResource::kPage;

		if (!parseInt(str, parsedId))
			success = false;
	}

	if (success) {
		if (parsedId < 0 || 0x7FFF < parsedId) {
			warning("Error: Resource ID %d is out of range (0-32767)\n", parsedId);
			success = false;
		} else {
			outRes._id = static_cast<int16>(parsedId);
		}
	}

	if (!success)
		warning("Cannot parse string(%s), try <ID>, <s:ID> or <p:ID> with ID in the 0-32767 range (hex supported with 0x prefix)\n", str);
	return success;
}

bool ZmbResource::operator==(const ZmbResource &other) const {
	return _archiveKind == other._archiveKind && _id == other._id;
}

bool ZmbResource::operator!=(const ZmbResource &other) const {
	return !(*this == other);
}

bool ZmbResource::hasId() const {
	return 0 < _id;
}

Common::String ZmbResource::toString() const {
	char archiveKind = 'p';
	switch (_archiveKind) {
	case ZmbResource::kPage:
		archiveKind = 'p';
		break;
	case ZmbResource::kSystem:
		archiveKind = 's';
		break;
	default:
		archiveKind = '?';
		break;
	}
	return Common::String::format("%c%d", archiveKind, _id);
}

} // End of namespace Mohawk
