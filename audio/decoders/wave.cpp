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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/substream.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave_types.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/g711.h"

namespace Audio {

bool loadWAVFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags, uint16 *wavType, int *blockAlign_, int *samplesPerBlock_) {
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
	if (memcmp(buf, "fact", 4) == 0) {
		// Initial fact chunk, so skip over it
		uint32 factLen = stream.readUint32LE();
		stream.skip(factLen);
		stream.read(buf, 4);
	}

	if (memcmp(buf, "JUNK", 4) == 0) {
		uint32 junksize = stream.readUint32LE();
		// skip junk padding (add 1 byte if odd)
		stream.skip(junksize + (junksize % 2));
		stream.read(buf, 4);
	}

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
	uint32 fmtRemaining = fmtLength;

	// Next comes the "type" field of the fmt header. Some typical
	// values for it:
	// 1  -> uncompressed PCM
	// 17 -> IMA ADPCM compressed WAVE
	// See <http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html>
	// for a more complete list of common WAVE compression formats...
	uint16 type = stream.readUint16LE();	// == 1 for PCM data
	uint16 numChannels = stream.readUint16LE();	// 1 for mono, 2 for stereo
	uint32 samplesPerSec = stream.readUint32LE();	// in Hz
	uint32 avgBytesPerSec = stream.readUint32LE();	// == SampleRate * NumChannels * BitsPerSample/8

	uint16 blockAlign = stream.readUint16LE();	// == NumChannels * BitsPerSample/8
	uint16 bitsPerSample = stream.readUint16LE();	// 8, 16 ...
	// 8 bit data is unsigned, 16 bit data signed
	fmtRemaining -= 16;

	uint16 samplesPerBlock = 1;
	if (type == kWaveFormatMSADPCM) {
		// TODO: There is a samplesPerBlock in this header. It should be parsed and the below warning removed.
		// (NB: The FMT header for MSADPCM has different information from the MSIMAADPCM)
		warning("getWavInfo: 'fmt' header not parsed in entirety for MSADPCM");
	} else if (type == kWaveFormatMSIMAADPCM) {
		if (fmtRemaining != 4) {
			// A valid IMA ADPCM fmt chunk is always 20 bytes long
			warning("getWavInfo: 'fmt' header is wrong length for IMA ADPCM");
			return false;
		}
		stream.readUint16LE(); // cbSize
		samplesPerBlock = stream.readUint16LE();
		fmtRemaining -= 4;
	}

	if (wavType != nullptr)
		*wavType = type;

	if (blockAlign_ != nullptr)
		*blockAlign_ = blockAlign;

	if (samplesPerBlock_ != nullptr)
		*samplesPerBlock_ = samplesPerBlock;
#if 0
	debug("WAVE information:");
	debug("  total size: %d", wavLength);
	debug("  fmt size: %d", fmtLength);
	debug("  type: %d", type);
	debug("  numChannels: %d", numChannels);
	debug("  samplesPerSec: %d", samplesPerSec);
	debug("  avgBytesPerSec: %d", avgBytesPerSec);
	debug("  blockAlign: %d", blockAlign);
	debug("  bitsPerSample: %d", bitsPerSample);
#endif

	switch (type) {
	case kWaveFormatPCM:
	case kWaveFormatMSADPCM:
	case kWaveFormatALawPCM:
	case kWaveFormatMuLawPCM:
	case kWaveFormatMSIMAADPCM:
	#ifdef USE_MAD
	case kWaveFormatMP3:
	#endif
		break;
	default:
		warning("getWavInfo: unsupported format (type %d)", type);
		return false;
	}

	if (type == kWaveFormatMP3) {
		bitsPerSample = 8;
	} else if (type != kWaveFormatMSADPCM && type != kWaveFormatMSIMAADPCM) {
		if (blockAlign != numChannels * bitsPerSample / 8) {
			debug(0, "getWavInfo: blockAlign is invalid");
		}

		if (avgBytesPerSec != samplesPerSec * blockAlign) {
			debug(0, "getWavInfo: avgBytesPerSec is invalid");
		}
	}

	// Prepare the return values.
	rate = samplesPerSec;

	flags = 0;
	if (bitsPerSample == 8)		// 8 bit data is unsigned
		flags |= Audio::FLAG_UNSIGNED;
	else if (bitsPerSample == 16)	// 16 bit data is signed little endian
		flags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 24)	// 24 bit data is signed little endian
		flags |= (Audio::FLAG_24BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 4 && (type == kWaveFormatMSADPCM || type == kWaveFormatMSIMAADPCM))
		flags |= Audio::FLAG_16BITS;
	else {
		warning("getWavInfo: unsupported bitsPerSample %d", bitsPerSample);
		return false;
	}

	if (numChannels == 2)
		flags |= Audio::FLAG_STEREO;
	else if (numChannels != 1) {
		warning("getWavInfo: unsupported number of channels %d", numChannels);
		return false;
	}

	// It's almost certainly a WAV file, but we still need to find its
	// 'data' chunk.

	// Skip over the rest of the fmt chunk.
	int offset = fmtRemaining;

	do {
		stream.seek(offset, SEEK_CUR);
		if (stream.pos() >= initialPos + wavLength + 8) {
			warning("getWavInfo: Can't find 'data' chunk");
			return false;
		}
		stream.read(buf, 4);
		offset = stream.readUint32LE();

#if 0
		debug("  found a '%s' tag of size %d", buf, offset);
#endif
	} while (memcmp(buf, "data", 4) != 0);

	// Stream now points at 'offset' bytes of sample data...
	size = offset;

	return true;
}

SeekableAudioStream *makeWAVStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	int size, rate;
	byte flags;
	uint16 type;
	int blockAlign;

	if (!loadWAVFromStream(*stream, size, rate, flags, &type, &blockAlign)) {
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return nullptr;
	}
	int channels = (flags & Audio::FLAG_STEREO) ? 2 : 1;
	int bytesPerSample = (flags & Audio::FLAG_24BITS) ? 3 : ((flags & Audio::FLAG_16BITS) ? 2 : 1);

	// Raw PCM, make sure the last packet is complete
	if (type == kWaveFormatPCM) {
		uint sampleSize = bytesPerSample * channels;
		if (size % sampleSize != 0) {
			warning("makeWAVStream: Trying to play a WAVE file with an incomplete PCM packet");
			size &= ~(sampleSize - 1);
		}
	}
	Common::SeekableReadStream *dataStream = new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + size, disposeAfterUse);

	switch (type) {
	case kWaveFormatMSIMAADPCM:
		return makeADPCMStream(dataStream, DisposeAfterUse::YES, 0, Audio::kADPCMMSIma, rate, channels, blockAlign);
	case kWaveFormatMSADPCM:
		return makeADPCMStream(dataStream, DisposeAfterUse::YES, 0, Audio::kADPCMMS, rate, channels, blockAlign);
	#ifdef USE_MAD
	case kWaveFormatMP3:
		return makeMP3Stream(dataStream, DisposeAfterUse::YES);
	#endif
	case kWaveFormatALawPCM:
		return makeALawStream(dataStream, DisposeAfterUse::YES, rate, channels);
	case kWaveFormatMuLawPCM:
		return makeMuLawStream(dataStream, DisposeAfterUse::YES, rate, channels);
	case kWaveFormatPCM:
		return makeRawStream(dataStream, rate, flags);
	}

	// If the format is unsupported, we already returned earlier, but just in case
	delete dataStream;
	return nullptr;
}

} // End of namespace Audio
