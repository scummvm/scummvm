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

#include "chewy/audio/chewy_voc.h"

#include "common/stream.h"

namespace Chewy {

ChewyVocStream::ChewyVocStream(Common::SeekableReadStream* stream, DisposeAfterUse::Flag disposeAfterUse) :
		VocStream(stream, true, disposeAfterUse) {
	removeHeaders();
}

void ChewyVocStream::removeHeaders() {
	// Check the sample blocks for non-standard headers.
	for (BlockList::iterator i = _blocks.begin(), end = _blocks.end(); i != end; ++i) {
		if (i->code == 1 && i->sampleBlock.samples > 80) {
			// Found a sample block. Check for the headers.
			int headerSize = 0;
			if (_stream->readUint32BE() == FOURCC_RIFF) {
				// Found a RIFF header. 
				headerSize = 44;
			} else {
				_stream->seek(i->sampleBlock.offset + 76);
				if (_stream->readUint32BE() == FOURCC_SCRS) {
					// Found an SCRS (?) header.
					headerSize = 80;
				}
			}

			if (headerSize > 0) {
				// Move the offset past the header and adjust the length.
				i->sampleBlock.offset += headerSize;
				i->sampleBlock.samples -= headerSize;
				_length = _length.addFrames(-headerSize);
			}
		}
	}

	// Reset the stream.
	rewind();
}

} // End of namespace Chewy
