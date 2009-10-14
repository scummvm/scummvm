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
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/util.h"
#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/wave.h"
#include "sound/adpcm.h"

namespace Audio {

bool loadWAVFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags, uint16 *wavType, int *blockAlign_) {
	const int32 initialPos = stream.pos();
	byte buf[4+1];

	buf[4] = 0;

	stream.read(buf, 4);
	if (memcmp(buf, "RIFF", 4) != 0) {
		warning("getWavInfo: No 'RIFF' header");
		return false;
	}

	int32 wavLength = stream.readUint32LE();

	stream.read(buf, 4);
	if (memcmp(buf, "WAVE", 4) != 0) {
		warning("getWavInfo: No 'WAVE' header");
		return false;
	}

	stream.read(buf, 4);
	if (memcmp(buf, "fmt ", 4) != 0) {
		warning("getWavInfo: No 'fmt' header");
		return false;
	}

	uint32 fmtLength = stream.readUint32LE();
	if (fmtLength < 16) {
		// A valid fmt chunk always contains at least 16 bytes
		warning("getWavInfo: 'fmt' header is too short");
		return false;
	}

	// Next comes the "type" field of the fmt header. Some typical
	// values for it:
	// 1  -> uncompressed PCM
	// 17 -> IMA ADPCM compressed WAVE
	// See <http://www.saettler.com/RIFFNEW/RIFFNEW.htm> for a more complete
	// list of common WAVE compression formats...
	uint16 type = stream.readUint16LE();	// == 1 for PCM data
	uint16 numChannels = stream.readUint16LE();	// 1 for mono, 2 for stereo
	uint32 samplesPerSec = stream.readUint32LE();	// in Hz
	uint32 avgBytesPerSec = stream.readUint32LE();	// == SampleRate * NumChannels * BitsPerSample/8

	uint16 blockAlign = stream.readUint16LE();	// == NumChannels * BitsPerSample/8
	uint16 bitsPerSample = stream.readUint16LE();	// 8, 16 ...
	// 8 bit data is unsigned, 16 bit data signed


	if (wavType != 0)
		*wavType = type;

	if (blockAlign_ != 0)
		*blockAlign_ = blockAlign;
#if 0
	printf("WAVE information:\n");
	printf("  total size: %d\n", wavLength);
	printf("  fmt size: %d\n", fmtLength);
	printf("  type: %d\n", type);
	printf("  numChannels: %d\n", numChannels);
	printf("  samplesPerSec: %d\n", samplesPerSec);
	printf("  avgBytesPerSec: %d\n", avgBytesPerSec);
	printf("  blockAlign: %d\n", blockAlign);
	printf("  bitsPerSample: %d\n", bitsPerSample);
#endif

	if (type != 1 && type != 2 && type != 17) {
		warning("getWavInfo: only PCM, MS ADPCM or IMA ADPCM data is supported (type %d)", type);
		return false;
	}

	if (blockAlign != numChannels * bitsPerSample / 8 && type != 2) {
		debug(0, "getWavInfo: blockAlign is invalid");
	}

	if (avgBytesPerSec != samplesPerSec * blockAlign && type != 2) {
		debug(0, "getWavInfo: avgBytesPerSec is invalid");
	}

	// Prepare the return values.
	rate = samplesPerSec;

	flags = 0;
	if (bitsPerSample == 8)		// 8 bit data is unsigned
		flags |= Audio::Mixer::FLAG_UNSIGNED;
	else if (bitsPerSample == 16)	// 16 bit data is signed little endian
		flags |= (Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 4 && (type == 2 || type == 17))
		flags |= Audio::Mixer::FLAG_16BITS;
	else {
		warning("getWavInfo: unsupported bitsPerSample %d", bitsPerSample);
		return false;
	}

	if (numChannels == 2)
		flags |= Audio::Mixer::FLAG_STEREO;
	else if (numChannels != 1) {
		warning("getWavInfo: unsupported number of channels %d", numChannels);
		return false;
	}

	// It's almost certainly a WAV file, but we still need to find its
	// 'data' chunk.

	// Skip over the rest of the fmt chunk.
	int offset = fmtLength - 16;

	do {
		stream.seek(offset, SEEK_CUR);
		if (stream.pos() >= initialPos + wavLength + 8) {
			warning("getWavInfo: Can't find 'data' chunk");
			return false;
		}
		stream.read(buf, 4);
		offset = stream.readUint32LE();

#if 0
		printf("  found a '%s' tag of size %d\n", buf, offset);
#endif
	} while (memcmp(buf, "data", 4) != 0);

	// Stream now points at 'offset' bytes of sample data...
	size = offset;

	return true;
}

AudioStream *makeWAVStream(Common::SeekableReadStream *stream, bool disposeAfterUse, bool loop) {
	int size, rate;
	byte flags;
	uint16 type;
	int blockAlign;

	if (!loadWAVFromStream(*stream, size, rate, flags, &type, &blockAlign)) {
		if (disposeAfterUse)
			delete stream;
		return 0;
	}

	if (type == 17) // MS IMA ADPCM
		return makeADPCMStream(stream, disposeAfterUse, size, Audio::kADPCMMSIma, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign, loop ? 0 : 1);
	else if (type == 2) // MS ADPCM
		return makeADPCMStream(stream, disposeAfterUse, size, Audio::kADPCMMS, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign, loop ? 0 : 1);
	
	// Raw PCM. Just read everything at once.
	// TODO: More elegant would be to wrap the stream.
	byte *data = (byte *)malloc(size);
	assert(data);
	stream->read(data, size);

	if (disposeAfterUse)
		delete stream;

	// Since we allocated our own buffer for the data, we must set the autofree flag.
	flags |= Audio::Mixer::FLAG_AUTOFREE;
	
	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;

	return makeLinearInputStream(data, size, rate, flags, 0, 0);
}

} // End of namespace Audio
