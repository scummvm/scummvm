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

#ifndef DUNE_MOVIE_STREAM_H
#define DUNE_MOVIE_STREAM_H

#include "common/array.h"
#include "common/util.h"

namespace Dune {

/**
 * CA1B/CB1A/CC0C/CC4E's silent movie ring. C9E8 leaves its cursor in SI,
 * which DD7E uses as the previous mouse sample. Preserve that input guard
 * from packet sizes and real buffering, not a list of per-frame decisions.
 */
class MovieInputStream {
public:
	MovieInputStream(const Common::Array<uint32> &offsets, uint32 headerSize, uint16 capacity = 64000)
		: _offsets(offsets), _capacity(capacity), _extent(capacity), _fileOffset(headerSize) {
		assert(offsets.size() > 1 && capacity <= 64000);
		_fileOffset += offsets[1];
		for (uint i = 0; i + 1 < offsets.size(); ++i)
			assert(offsets[i + 1] > offsets[i] + 2 && offsets[i + 1] - offsets[i] < capacity);
		// CA1B presents packet1, CE1A clears the ring, then prefetches50 times.
		for (uint i = 0; i < 50; ++i)
			prefetch();
	}

	uint16 present(uint frame) {
		assert(frame == _presented + 1 && frame < _offsets.size());
		const uint16 size = packetSize(frame - 1);
		uint attempts = 0;
		while (_available < size && attempts++ < 100)
			prefetch();
		assert(_available >= size);
		const uint32 end = _read + 2U + size;
		_cursor = (uint16)end;
		_available -= size;
		_read = end > 65535 || _cursor > _capacity ? size - 2 : (uint16)(_read + size);
		prefetch();
		_presented = frame;
		return _cursor;
	}

private:
	Common::Array<uint32> _offsets;
	uint16 _capacity, _extent;
	uint32 _fileOffset;
	uint16 _read = 0, _write = 0, _available = 0, _left = 0, _cursor = 0;
	uint _nextPacket = 1, _presented = 1;

	uint16 packetSize(uint index) const {
		return _offsets[index + 1] - _offsets[index];
	}

	bool canWrite(uint16 count) const {
		if (_write < _read && _read < (uint16)(_write + count + 18))
			return false;
		const uint32 total = _available + 10U + count;
		return total <= 65535 && _extent >= total;
	}

	void readBytes(uint16 count) {
		_write += count;
		_available += count;
		_fileOffset += count;
	}

	void prefetch() {
		if (!_left) {
			if (_nextPacket + 1 == _offsets.size())
				return;
			const uint16 size = packetSize(_nextPacket++);
			readBytes(2); // CD8F: read the packet header before its data.
			const uint32 end = _write + (uint32)size;
			_cursor = (uint16)end;
			if (end > 65535 || _cursor > _capacity) {
				_extent = _write;
				_write = 0;
			}
			_left = size - 2;
		}
		const uint16 maximum = ((-_fileOffset) & 0x7ff) + 0x800;
		const uint16 count = MIN<uint16>(_left, maximum);
		if (canWrite(count)) {
			_left -= count;
			readBytes(count);
		}
	}
};

inline bool movieTimerEligible(uint16 previousButtons, uint16 buttons) {
	return !(((previousButtons ^ buttons) & buttons) & 7);
}

} // End of namespace Dune

#endif
