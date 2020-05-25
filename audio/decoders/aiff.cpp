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

/*
 * The code in this file is based on information found at
 * http://www.borg.com/~jglatt/tech/aiff.htm
 *
 * Also partially based on libav's aiffdec.c
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/3do.h"

namespace Audio {

uint32 readExtended(Common::SeekableReadStream &stream) {
	// The sample rate is stored as an "80 bit IEEE Standard 754 floating
	// point number (Standard Apple Numeric Environment [SANE] data type
	// Extended).

	byte buf[10];
	uint32 mantissa;
	uint32 last = 0;
	byte exp;

	stream.read(buf, 10);
	mantissa = READ_BE_UINT32(buf + 2);
	exp = 30 - buf[1];

	while (exp--) {
		last = mantissa;
		mantissa >>= 1;
	}

	if (last & 0x00000001)
		mantissa++;

	return mantissa;
}

// AIFF versions
static const uint32 kVersionAIFF = MKTAG('A', 'I', 'F', 'F');
static const uint32 kVersionAIFC = MKTAG('A', 'I', 'F', 'C');

// Codecs
static const uint32 kCodecPCM = MKTAG('N', 'O', 'N', 'E'); // very original

RewindableAudioStream *makeAIFFStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (stream->readUint32BE() != MKTAG('F', 'O', 'R', 'M')) {
		warning("makeAIFFStream: No 'FORM' header");

		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;

		return 0;
	}

	stream->readUint32BE(); // file size

	uint32 version = stream->readUint32BE();

	if (version != kVersionAIFF && version != kVersionAIFC) {
		warning("makeAIFFStream: No 'AIFF' or 'AIFC' header");

		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;

		return 0;
	}

	// From here on, we only care about the COMM and SSND chunks, which are
	// the only required chunks.

	bool foundCOMM = false;
	bool foundSSND = false;

	uint16 channels = 0, bitsPerSample = 0;
	uint32 rate = 0;
	uint32 codec = kCodecPCM; // AIFF default
	Common::SeekableReadStream *dataStream = 0;

	while (!(foundCOMM && foundSSND) && !stream->err() && !stream->eos()) {
		uint32 tag = stream->readUint32BE();
		uint32 length = stream->readUint32BE();
		uint32 pos = stream->pos();

		if (stream->eos() || stream->err())
			break;

		switch (tag) {
		case MKTAG('C', 'O', 'M', 'M'):
			foundCOMM = true;
			channels = stream->readUint16BE();
			/* frameCount = */ stream->readUint32BE();
			bitsPerSample = stream->readUint16BE();
			rate = readExtended(*stream);

			if (version == kVersionAIFC)
				codec = stream->readUint32BE();
			break;
		case MKTAG('S', 'S', 'N', 'D'):
			foundSSND = true;
			/* uint32 offset = */ stream->readUint32BE();
			/* uint32 blockAlign = */ stream->readUint32BE();
			if (dataStream)
				delete dataStream;
			dataStream = new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + length - 8, disposeAfterUse);
			break;
		case MKTAG('F', 'V', 'E', 'R'):
			switch (stream->readUint32BE()) {
			case 0:
				version = kVersionAIFF;
				break;
			case 0xA2805140:
				version = kVersionAIFC;
				break;
			default:
				warning("Unknown AIFF version chunk version");
				break;
			}
			break;
		case MKTAG('w', 'a', 'v', 'e'):
			warning("Found unhandled AIFF-C extra data chunk");

			if (!dataStream && disposeAfterUse == DisposeAfterUse::YES)
				delete stream;

			delete dataStream;
			return 0;
		default:
			debug(1, "Skipping AIFF '%s' chunk", tag2str(tag));
			break;
		}

		uint32 seekPos = pos + length;
		if (seekPos < (uint32)stream->size()) {
			seekPos += (length & 1); // ensure we're word-aligned
		}
		stream->seek(seekPos);
	}

	if (!foundCOMM) {
		warning("makeAIFFStream: Could not find 'COMM' chunk");

		if (!dataStream && disposeAfterUse == DisposeAfterUse::YES)
			delete stream;

		delete dataStream;
		return 0;
	}

	if (!foundSSND) {
		warning("makeAIFFStream: Could not find 'SSND' chunk");

		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;

		return 0;
	}

	// We only implement a subset of the AIFF standard.

	if (channels < 1 || channels > 2) {
		warning("makeAIFFStream: Only 1 or 2 channels are supported, not %d", channels);
		delete dataStream;
		return 0;
	}

	// Seek to the start of dataStream, required for at least FileStream
	dataStream->seek(0);

	switch (codec) {
	case kCodecPCM:
	case MKTAG('t', 'w', 'o', 's'):
	case MKTAG('s', 'o', 'w', 't'): {
		// PCM samples are always signed.
		byte rawFlags = 0;
		if (bitsPerSample == 16)
			rawFlags |= Audio::FLAG_16BITS;
		if (channels == 2)
			rawFlags |= Audio::FLAG_STEREO;
		if (codec == MKTAG('s', 'o', 'w', 't'))
			rawFlags |= Audio::FLAG_LITTLE_ENDIAN;

		return makeRawStream(dataStream, rate, rawFlags);
	}
	case MKTAG('i', 'm', 'a', '4'):
		// TODO: Use QT IMA ADPCM
		warning("Unhandled AIFF-C QT IMA ADPCM compression");
		break;
	case MKTAG('Q', 'D', 'M', '2'):
		// TODO: Need to figure out how to integrate this
		// (But hopefully never needed)
		warning("Unhandled AIFF-C QDM2 compression");
		break;
	case MKTAG('A', 'D', 'P', '4'):
		// ADP4 on 3DO
		return make3DO_ADP4AudioStream(dataStream, rate, channels == 2);
	case MKTAG('S', 'D', 'X', '2'):
		// SDX2 on 3DO
		return make3DO_SDX2AudioStream(dataStream, rate, channels == 2);
	default:
		warning("Unhandled AIFF-C compression tag '%s'", tag2str(codec));
	}

	delete dataStream;
	return 0;
}

} // End of namespace Audio
