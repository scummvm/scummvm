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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/smush/saud.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

namespace Scumm {

bool readSaudHeader(Common::SeekableReadStream &stream, SaudHeader &header, bool allowTruncated) {
	int64 end = stream.size();
	if (stream.pos() < 0 || end - stream.pos() < 8 || stream.readUint32BE() != MKTAG('S', 'A', 'U', 'D'))
		return false;
	const uint32 size = stream.readUint32BE();
	if (!allowTruncated && size != end - stream.pos())
		return false;
	end = MIN(end, stream.pos() + size);

	SaudHeader parsed;
	int64 dataOffset = -1;
	while (stream.pos() < end) {
		if (end - stream.pos() < 8)
			return false;
		const uint32 tag = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		const int64 offset = stream.pos();
		if (chunkSize > end - offset) {
			if (!allowTruncated || tag != MKTAG('S', 'D', 'A', 'T'))
				return false;
			chunkSize = end - offset;
		}
		if (tag == MKTAG('S', 'D', 'A', 'T')) {
			if (dataOffset >= 0 || !chunkSize)
				return false;
			dataOffset = offset;
			parsed.dataSize = chunkSize;
			if (allowTruncated)
				break;
		} else if (tag == MKTAG('S', 'H', 'D', 'R') && chunkSize == 4) {
			parsed.sampleRate = stream.readUint32LE();
		} else if (tag == MKTAG('S', 'T', 'R', 'K') && chunkSize >= 14) {
			const byte code = stream.readByte(), length = stream.readByte();
			if (code == 6 && length == 12) {
				if (!stream.skip(8))
					return false;
				parsed.sampleRate = stream.readUint32BE();
			}
		}
		// SAUD chunks have no word-alignment padding.
		if (stream.err() || stream.eos() || !stream.seek(offset + chunkSize))
			return false;
	}
	if (dataOffset < 0 || stream.err() || stream.eos() || !stream.seek(dataOffset))
		return false;
	header = parsed;
	return true;
}

} // End of namespace Scumm
