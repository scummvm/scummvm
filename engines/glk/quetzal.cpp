/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/quetzal.h"
#include "common/memstream.h"

namespace Glk {

bool QuetzalReader::open(Common::SeekableReadStream *stream, uint32 formType) {
	_chunks.clear();
	stream->seek(0);
	if (stream->readUint32BE() != ID_FORM)
		return false;

	uint32 size = stream->readUint32BE();
	uint32 fileFormType = stream->readUint32BE();

	if (formType != ID_IFSF && fileFormType != formType)
		return false;
	if ((int)size > stream->size() || (size & 1) || (size < 4))
		return false;
	size -= 4;

	// Iterate through reading chunk headers
	while (size > 0) {
		if (size < 8)
			// Couldn't contain a chunk
			return false;

		// Get in the chunk header
		Chunk c;
		c._id = stream->readUint32BE();
		c._size = stream->readUint32BE() - 8;
		c._offset = stream->pos();
		_chunks.push_back(c);

		int chunkRemainder = c._size + (c._size & 1);
		if ((stream->pos() + chunkRemainder) > stream->size())
			// Chunk goes beyond the file size
			return false;

		size -= 8 + chunkRemainder;
		stream->skip(chunkRemainder);
	}

	return true;
}

/*--------------------------------------------------------------------------*/

Common::WriteStream &QuetzalWriter::add(uint32 chunkId) {
	// Sanity check to prevent adding the same chunk multiple times
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		if (_chunks[idx]._id == chunkId)
			error("Duplicate chunk added");
	}

	_chunks.push_back(Chunk());
	return _chunks.back()._stream;
}

void QuetzalWriter::save(Common::WriteStream *out, uint32 formType) {
	// Calculate the size of the chunks
	uint size = 4;
	for (uint idx = 0; idx < _chunks.size(); ++idx)
		size += _chunks[idx]._stream.size();

	// Write out the header
	out->writeUint32BE(ID_FORM);
	out->writeUint32BE(size);
	out->writeUint32BE(formType);

	// Loop through writing the chunks
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		Common::MemoryWriteStreamDynamic &s = _chunks[idx]._stream;
		uint chunkSize = s.size() + (s.size() & 1);

		out->writeUint32BE(_chunks[idx]._id);
		out->writeUint32BE(chunkSize);
		out->write(s.getData(), s.size());
		if (s.size() & 1)
			out->writeByte(0);
	}
}

} // End of namespace Glk
