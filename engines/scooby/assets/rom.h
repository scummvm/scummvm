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

#ifndef SCOOBY_ROM_H
#define SCOOBY_ROM_H

#include "common/scummsys.h"
#include "common/ptr.h"

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

#include "scooby/common/span.h"

namespace Scooby {

class ScoobyDooRom {
public:
	static Common::ScopedPtr<ScoobyDooRom> load(const Common::Path &path, Common::String &outError);

	Span<const uint8> readBytes(int offset, int length) const {
		return Span<const uint8>(_bytes.data() + offset, static_cast<std::size_t>(length));
	}

	uint8 readByte(int offset) const { return _bytes[static_cast<std::size_t>(offset)]; }

	int16 readInt16(int offset) const { return static_cast<int16>(readUInt16(offset)); }

	uint16 readUInt16(int offset) const {
		return static_cast<uint16>((uint16(_bytes[static_cast<std::size_t>(offset)]) << 8) |
								   uint16(_bytes[static_cast<std::size_t>(offset) + 1]));
	}

	uint32 readUInt32(int offset) const {
		std::size_t o = static_cast<std::size_t>(offset);
		return (uint32(_bytes[o]) << 24) | (uint32(_bytes[o + 1]) << 16) |
			   (uint32(_bytes[o + 2]) << 8) | uint32(_bytes[o + 3]);
	}

private:
	ScoobyDooRom(Common::Array<uint8> bytes);

	Common::Array<uint8> _bytes;
};

} // namespace Scooby

#endif // SCOOBY_ROM_H
