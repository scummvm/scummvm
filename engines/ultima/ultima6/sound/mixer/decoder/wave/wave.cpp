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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/sound/decoders/wave.cpp $
 * $Id: wave.cpp 54022 2010-11-01 20:41:03Z fingolfin $
 *
 */

//#include <assert.h>
//#include <string.h>
//#include "common/debug.h"
//#include "common/util.h"
#include "decoder/wave/stream.h"

#include "audiostream.h"
#include "audio/mixer.h"
#include "decoder/wave/wave.h"
#include "decoder/wave/adpcm.h"
#include "decoder/wave/raw.h"

namespace Audio {

bool loadWAVFromStream(Common::SeekableReadStream &stream, int &size, int &rate, uint8 &flags, uint16 *wavType, int *blockAlign_) {
	const sint32 initialPos = stream.pos();
	uint8 buf[4 + 1];

	buf[4] = 0;

	stream.read(buf, 4);
	if (memcmp(buf, "RIFF", 4) != 0) {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: No 'RIFF' header");
		return false;
	}

	sint32 wavLength = stream.readUint32LE();

	stream.read(buf, 4);
	if (memcmp(buf, "WAVE", 4) != 0) {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: No 'WAVE' header");
		return false;
	}

	stream.read(buf, 4);
	if (memcmp(buf, "fmt ", 4) != 0) {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: No 'fmt' header");
		return false;
	}

	uint32 fmtLength = stream.readUint32LE();
	if (fmtLength < 16) {
		// A valid fmt chunk always contains at least 16 bytes
		DEBUG(0, LEVEL_WARNING, "getWavInfo: 'fmt' header is too short");
		return false;
	}

	// Next comes the "type" field of the fmt header. Some typical
	// values for it:
	// 1  -> uncompressed PCM
	// 17 -> IMA ADPCM compressed WAVE
	// See <http://www.saettler.com/RIFFNEW/RIFFNEW.htm> for a more complete
	// list of common WAVE compression formats...
	uint16 type = stream.readUint16LE();    // == 1 for PCM data
	uint16 numChannels = stream.readUint16LE(); // 1 for mono, 2 for stereo
	uint32 samplesPerSec = stream.readUint32LE();   // in Hz
	uint32 avgBytesPerSec = stream.readUint32LE();  // == SampleRate * NumChannels * BitsPerSample/8

	uint16 blockAlign = stream.readUint16LE();  // == NumChannels * BitsPerSample/8
	uint16 bitsPerSample = stream.readUint16LE();   // 8, 16 ...
	// 8 bit data is unsigned, 16 bit data signed


	if (wavType != 0)
		*wavType = type;

	if (blockAlign_ != 0)
		*blockAlign_ = blockAlign;
#if 0
	DEBUG(0, LEVEL_DEBUGGING, "WAVE information:");
	DEBUG(0, LEVEL_DEBUGGING, "  total size: %d", wavLength);
	DEBUG(0, LEVEL_DEBUGGING , "  fmt size: %d", fmtLength);
	DEBUG(0, LEVEL_DEBUGGING , "  type: %d", type);
	DEBUG(0, LEVEL_DEBUGGING , "  numChannels: %d", numChannels);
	DEBUG(0, LEVEL_DEBUGGING , "  samplesPerSec: %d", samplesPerSec);
	DEBUG(0, LEVEL_DEBUGGING , "  avgBytesPerSec: %d", avgBytesPerSec);
	DEBUG(0, LEVEL_DEBUGGING , "  blockAlign: %d", blockAlign);
	DEBUG(0, LEVEL_DEBUGGING , "  bitsPerSample: %d", bitsPerSample);
#endif

	if (type != 1 && type != 2 && type != 17) {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: only PCM, MS ADPCM or IMA ADPCM data is supported (type %d)", type);
		return false;
	}

	if (blockAlign != numChannels * bitsPerSample / 8 && type != 2) {
		DEBUG(0, LEVEL_DEBUGGING , 0, "getWavInfo: blockAlign is invalid");
	}

	if (avgBytesPerSec != samplesPerSec * blockAlign && type != 2) {
		DEBUG(0, LEVEL_DEBUGGING , 0, "getWavInfo: avgBytesPerSec is invalid");
	}

	// Prepare the return values.
	rate = samplesPerSec;

	flags = 0;
	if (bitsPerSample == 8)     // 8 bit data is unsigned
		flags |= Audio::FLAG_UNSIGNED;
	else if (bitsPerSample == 16)   // 16 bit data is signed little endian
		flags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 4 && (type == 2 || type == 17))
		flags |= Audio::FLAG_16BITS;
	else {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: unsupported bitsPerSample %d", bitsPerSample);
		return false;
	}

	if (numChannels == 2)
		flags |= Audio::FLAG_STEREO;
	else if (numChannels != 1) {
		DEBUG(0, LEVEL_WARNING, "getWavInfo: unsupported number of channels %d", numChannels);
		return false;
	}

	// It's almost certainly a WAV file, but we still need to find its
	// 'data' chunk.

	// Skip over the rest of the fmt chunk.
	int offset = fmtLength - 16;

	do {
		stream.seek(offset, SEEK_CUR);
		if (stream.pos() >= initialPos + wavLength + 8) {
			DEBUG(0, LEVEL_WARNING, "getWavInfo: Can't find 'data' chunk");
			return false;
		}
		stream.read(buf, 4);
		offset = stream.readUint32LE();

#if 0
		DEBUG(0, LEVEL_DEBUGGING , "  found a '%s' tag of size %d", buf, offset);
#endif
	} while (memcmp(buf, "data", 4) != 0);

	// Stream now points at 'offset' bytes of sample data...
	size = offset;

	return true;
}

RewindableAudioStream *makeWAVStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	int size, rate;
	uint8 flags;
	uint16 type;
	int blockAlign;

	if (!loadWAVFromStream(*stream, size, rate, flags, &type, &blockAlign)) {
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return 0;
	}

	if (type == 17) // MS IMA ADPCM
		return makeADPCMStream(stream, disposeAfterUse, size, Audio::kADPCMMSIma, rate, (flags & Audio::FLAG_STEREO) ? 2 : 1, blockAlign);
	else if (type == 2) // MS ADPCM
		return makeADPCMStream(stream, disposeAfterUse, size, Audio::kADPCMMS, rate, (flags & Audio::FLAG_STEREO) ? 2 : 1, blockAlign);

	// Raw PCM. Just read everything at once.
	// TODO: More elegant would be to wrap the stream.
	uint8 *data = (uint8 *)malloc(size);
	assert(data);
	stream->read(data, size);

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	return makeRawStream(data, size, rate, flags);
}

} // End of namespace Audio
