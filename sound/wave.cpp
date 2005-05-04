/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/util.h"
#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/wave.h"
#include "sound/adpcm.h"

bool loadWAVFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags, uint16 *wavType) {
	const uint32 initialPos = stream.pos();
	byte buf[4+1];

	buf[4] = 0;

	stream.read(buf, 4);
	if (memcmp(buf, "RIFF", 4) != 0) {
		warning("getWavInfo: No 'RIFF' header");
		return false;
	}

	uint32 wavLength = stream.readUint32LE();

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

	if (type != 1 && type != 17) {
		warning("getWavInfo: only PCM or IMA ADPCM data is supported (type %d)", type);
		return false;
	}

	if (blockAlign != numChannels * bitsPerSample / 8) {
		debug(0, "getWavInfo: blockAlign is invalid");
	}

	if (avgBytesPerSec != samplesPerSec * blockAlign) {
		debug(0, "getWavInfo: avgBytesPerSec is invalid");
	}

	// Prepare the return values.
	rate = samplesPerSec;

	flags = 0;
	if (bitsPerSample == 8)		// 8 bit data is unsigned
		flags |= SoundMixer::FLAG_UNSIGNED;
	else if (bitsPerSample == 16)	// 16 bit data is signed little endian
		flags |= (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 4 && type == 17)	// IMA ADPCM compressed. We decompress it
		flags |= (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN);
	else {
		warning("getWavInfo: unsupported bitsPerSample %d", bitsPerSample);
		return false;
	}
	
	if (numChannels == 2)
		flags |= SoundMixer::FLAG_STEREO;
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

AudioStream *makeWAVStream(Common::SeekableReadStream &stream) {
	int size, rate;
	byte flags;
	uint16 type;
	
	if (!loadWAVFromStream(stream, size, rate, flags, &type))
		return 0;
	
	if (type == 17) // IMA ADPCM
		return makeADPCMStream(stream, size, kADPCMIma);

	byte *data = (byte *)malloc(size);
	assert(data);
	stream.read(data, size);
	flags |= SoundMixer::FLAG_AUTOFREE;

	return makeLinearInputStream(rate, flags, data, size, 0, 0);
}
