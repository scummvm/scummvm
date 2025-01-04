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

#include "mediastation/chunk.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Chunk::Chunk(Common::SeekableReadStream *stream) : _input(stream), _dataStartOffset(0), _dataEndOffset(0) {
	// READ THE HEADER.
	_id = _input->readUint32BE();
	_length = _input->readUint32LE();
	_dataStartOffset = pos();
	_dataEndOffset = _dataStartOffset + _length;
	debugC(5, kDebugLoading, "Chunk::Chunk(): Got chunk with ID \"%s\" and size 0x%x", tag2str(_id), _length);
	if (_length == 0) {
		error("Encountered a zero-length chunk. This usually indicates corrupted data - maybe a CD-ROM read error.");
	}
}

} // End of namespace MediaStation