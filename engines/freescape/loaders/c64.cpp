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

#include "common/compression/unp64.h"
#include "common/endian.h"

#include "freescape/freescape.h"

namespace Freescape {

Common::Array<byte> FreescapeEngine::unpackC64Snapshot(Common::SeekableReadStream *file, const Common::Path &continuation) {
	if (file->size() <= 2 || file->size() > 0x10002)
		error("Invalid C64 program size");

	Common::Array<byte> packed;
	packed.resize(file->size());
	file->seek(0);
	if (file->read(packed.data(), packed.size()) != packed.size())
		error("Unable to read C64 program");

	uint32 endAddress = READ_LE_UINT16(packed.data()) + packed.size() - 2;
	if (endAddress > 0x10000)
		error("Invalid C64 program load address");

	Common::File part;
	if (!part.open(continuation))
		error("Unable to open C64 continuation %s", continuation.toString().c_str());
	if (part.size() <= 2 || part.size() - 2 > 0x10000 - endAddress || part.readUint16LE() != endAddress)
		error("Invalid C64 continuation %s", continuation.toString().c_str());

	uint32 offset = packed.size();
	uint32 partSize = part.size() - 2;
	packed.resize(offset + partSize);
	if (part.read(packed.data() + offset, partSize) != partSize)
		error("Truncated C64 continuation %s", continuation.toString().c_str());

	return unpackC64Snapshot(packed);
}

Common::Array<byte> FreescapeEngine::unpackC64Snapshot(const Common::Array<byte> &packed) {
	Common::Array<byte> data;
	data.resize(0x10000);
	uint32 size = 0;
	if (!Common::Unp64::unp64(packed.data(), packed.size(), data.data(), &size, nullptr))
		error("Unable to unpack C64 snapshot");
	if (size != data.size() || READ_LE_UINT16(data.data()) != 2)
		error("Incomplete C64 snapshot");

	// The PRG covers $0002..$ffff, so its payload is already at the RAM offsets.
	data[0] = data[1] = 0;
	return data;
}

} // namespace Freescape
