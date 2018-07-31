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

#include "common/iff_container.h"
#include "common/substream.h"
#include "common/util.h"

namespace Common {

IFFParser::IFFParser(ReadStream *stream, bool disposeStream) : _stream(stream), _disposeStream(disposeStream) {
	setInputStream(stream);
}

IFFParser::~IFFParser() {
	if (_disposeStream) {
		delete _stream;
	}
	_stream = nullptr;
}

void IFFParser::setInputStream(ReadStream *stream) {
	assert(stream);
	_formChunk.setInputStream(stream);
	_chunk.setInputStream(stream);

	_formChunk.readHeader();
	if (_formChunk.id != ID_FORM) {
		error("IFFParser input is not a FORM type IFF file");
	}
	_formSize = _formChunk.size;
	_formType = _formChunk.readUint32BE();
}

void IFFParser::parse(IFFCallback &callback) {
	bool stop;
	do {
		_chunk.feed();
		_formChunk.incBytesRead(_chunk.size);

		if (_formChunk.hasReadAll()) {
			break;
		}

		_formChunk.incBytesRead(8);
		_chunk.readHeader();

		// invoke the callback
		SubReadStream stream(&_chunk, _chunk.size);
		IFFChunk chunk(_chunk.id, _chunk.size, &stream);
		stop = callback(chunk);

		// eats up all the remaining data in the chunk
		while (!stream.eos()) {
			stream.readByte();
		}

	} while (!stop);
}


PackBitsReadStream::PackBitsReadStream(Common::ReadStream &input) : _input(&input) {
}

PackBitsReadStream::~PackBitsReadStream() {
}

bool PackBitsReadStream::eos() const {
	return _input->eos();
}

uint32 PackBitsReadStream::read(void *dataPtr, uint32 dataSize) {
	byte *out = (byte *)dataPtr;
	uint32 left = dataSize;

	uint32 lenR = 0, lenW = 0;
	while (left > 0 && !_input->eos()) {
		lenR = _input->readByte();

		if (lenR == 128) {
			// no-op
			lenW = 0;
		} else if (lenR <= 127) {
			// literal run
			lenR++;
			lenW = MIN(lenR, left);
			for (uint32 j = 0; j < lenW; j++) {
				*out++ = _input->readByte();
			}
			for (; lenR > lenW; lenR--) {
				_input->readByte();
			}
		} else {  // len > 128
			// expand run
			lenW = MIN((256 - lenR) + 1, left);
			byte val = _input->readByte();
			memset(out, val, lenW);
			out += lenW;
		}

		left -= lenW;
	}

	return dataSize - left;
}

} // End of namespace Common
