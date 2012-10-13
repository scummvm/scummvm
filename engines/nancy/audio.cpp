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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/substream.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Nancy {

Audio::RewindableAudioStream *makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	// HIS files are just WAVE files with the first 22 bytes of the file
	// overwritten with a string

	char buf[22];

	stream->read(buf, 22);
	buf[21] = 0;

	if (Common::String(buf) != "Her Interactive Sound") {
		warning("Invalid header found in HIS file");
		return 0;
	}

	// Most of this is copied from the standard WAVE decoder
	uint16 numChannels = stream->readUint16LE();
	uint32 samplesPerSec = stream->readUint32LE();
	stream->skip(6);
	uint16 bitsPerSample = stream->readUint16LE();

	byte flags = 0;
	if (bitsPerSample == 8)		// 8 bit data is unsigned
		flags |= Audio::FLAG_UNSIGNED;
	else if (bitsPerSample == 16)	// 16 bit data is signed little endian
		flags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	else {
		warning("Unsupported bitsPerSample %d found in HIS file", bitsPerSample);
		return 0;
	}

	if (numChannels == 2)
		flags |= Audio::FLAG_STEREO;
	else if (numChannels != 1) {
		warning("Unsupported number of channels %d found in HIS file", numChannels);
		return 0;
	}

	stream->read(buf, 4);
	buf[4] = 0;

	if (Common::String(buf) != "data") {
		warning("Data chunk not found in HIS file");
		return 0;
	}

	uint32 size = stream->readUint32LE();

	if (stream->eos() || stream->err()) {
		warning("Error reading HIS file");
		return 0;
	}

	// Raw PCM, make sure the last packet is complete
	uint sampleSize = (flags & Audio::FLAG_16BITS ? 2 : 1) * (flags & Audio::FLAG_STEREO ? 2 : 1);
	if (size % sampleSize != 0) {
		warning("Trying to play an HIS file with an incomplete PCM packet");
		size &= ~(sampleSize - 1);
	}

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(stream, stream->pos(), size, disposeAfterUse);
	return Audio::makeRawStream(subStream, samplesPerSec, flags);
}

} // End of namespace Nancy
