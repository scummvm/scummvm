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

#include "twine/parser/parser.h"
#include "common/stream.h"
#include "twine/resources/hqr.h"
#include "twine/shared.h"

namespace TwinE {

bool Parser::loadFromBuffer(const uint8 *buf, uint32 size, bool lba1) {
	if (size == 0) {
		return false;
	}
	Common::MemoryReadStream stream(buf, size);
	return loadFromStream(stream, lba1);
}

bool Parser::loadFromHQR(const char *name, int index, bool lba1) {
	Common::SeekableReadStream *stream = HQR::makeReadStream(name, index);
	if (stream == nullptr) {
		warning("Failed to load %s with index %i", name, index);
		return false;
	}
	if (!loadFromStream(*stream, lba1)) {
		delete stream;
		return false;
	}
	_hqrIndex = index;
	delete stream;
	return true;
}

} // End of namespace TwinE
