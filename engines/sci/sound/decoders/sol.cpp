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

#include "audio/audiostream.h"
#include "audio/rate.h"
#include "audio/decoders/raw.h"
#include "common/substream.h"
#include "common/util.h"
#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/sound/decoders/sol.h"
#include "sci/resource/resource.h"

namespace Sci {

// Note that the 16-bit version is also used in coktelvideo.cpp
static const uint16 tableDPCM16[128] = {
	0x0000, 0x0008, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
	0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x0100, 0x0110, 0x0120,
	0x0130, 0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01A0, 0x01B0, 0x01C0,
	0x01D0, 0x01E0, 0x01F0, 0x0200, 0x0208, 0x0210, 0x0218, 0x0220, 0x0228, 0x0230,
	0x0238, 0x0240, 0x0248, 0x0250, 0x0258, 0x0260, 0x0268, 0x0270, 0x0278, 0x0280,
	0x0288, 0x0290, 0x0298, 0x02A0, 0x02A8, 0x02B0, 0x02B8, 0x02C0, 0x02C8, 0x02D0,
	0x02D8, 0x02E0, 0x02E8, 0x02F0, 0x02F8, 0x0300, 0x0308, 0x0310, 0x0318, 0x0320,
	0x0328, 0x0330, 0x0338, 0x0340, 0x0348, 0x0350, 0x0358, 0x0360, 0x0368, 0x0370,
	0x0378, 0x0380, 0x0388, 0x0390, 0x0398, 0x03A0, 0x03A8, 0x03B0, 0x03B8, 0x03C0,
	0x03C8, 0x03D0, 0x03D8, 0x03E0, 0x03E8, 0x03F0, 0x03F8, 0x0400, 0x0440, 0x0480,
	0x04C0, 0x0500, 0x0540, 0x0580, 0x05C0, 0x0600, 0x0640, 0x0680, 0x06C0, 0x0700,
	0x0740, 0x0780, 0x07C0, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00,
	0x0F00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
};

// Each 4-bit nibble indexes into this table to refer to one of the 16 delta values.
// deDPCM8Nibble() currently uses the first 8 values, with logic to order the negative
//  deltas differently depending on the exact encoding used.
// deDPCM8NibbleWithRepair() uses the whole table, since it matches the order of "old"
//  encoding as-is. This saves a tiny bit of computation in the more complex function.
static const int8 tableDPCM8[16] = {
	0, 1, 2, 3, 6, 10, 15, 21, -21, -15, -10, -6, -3, -2, -1, -0
};

/**
 * Decompresses one channel of 16-bit DPCM compressed audio.
 */
static void deDPCM16Channel(int16 *out, int16 &sample, uint8 delta) {
	int32 nextSample = sample;
	if (delta & 0x80) {
		nextSample -= tableDPCM16[delta & 0x7f];
	} else {
		nextSample += tableDPCM16[delta];
	}

	// Emulating x86 16-bit signed register overflow
	if (nextSample > 32767) {
		nextSample -= 65536;
	} else if (nextSample < -32768) {
		nextSample += 65536;
	}

	*out = sample = nextSample;
}

/**
 * Decompresses 16-bit DPCM compressed audio. Each byte read
 * outputs one sample into the decompression buffer.
 */
static void deDPCM16Mono(int16 *out, Common::ReadStream &audioStream, const uint32 numBytes, int16 &sample) {
	for (uint32 i = 0; i < numBytes; ++i) {
		const uint8 delta = audioStream.readByte();
		deDPCM16Channel(out++, sample, delta);
	}
}

// Used by Robot
void deDPCM16Mono(int16 *out, const byte *in, const uint32 numBytes, int16 &sample) {
	for (uint32 i = 0; i < numBytes; ++i) {
		const uint8 delta = *in++;
		deDPCM16Channel(out++, sample, delta);
	}
}

static void deDPCM16Stereo(int16 *out, Common::ReadStream &audioStream, const uint32 numBytes, int16 &sampleL, int16 &sampleR) {
	assert((numBytes % 2) == 0);
	for (uint32 i = 0; i < numBytes / 2; ++i) {
		deDPCM16Channel(out++, sampleL, audioStream.readByte());
		deDPCM16Channel(out++, sampleR, audioStream.readByte());
	}
}

/**
 * Decompresses one half of an 8-bit DPCM compressed audio
 * byte.
 */
template <bool OLD>
static void deDPCM8Nibble(int16 *out, uint8 &sample, uint8 delta) {
	const uint8 lastSample = sample;
	if (delta & 8) {
		sample -= tableDPCM8[OLD ? (7 - (delta & 7)) : (delta & 7)];
	} else {
		sample += tableDPCM8[delta & 7];
	}
	*out = ((lastSample + sample) << 7) ^ 0x8000;
}

/**
 * Decompresses one half of an 8-bit DPCM compressed audio
 * byte. Attempts to repair overflows on the fly.
 */
static void deDPCM8NibbleWithRepair(int16 *const out, uint8 &sample, const uint8 delta,
									uint8 &repairState, uint8 &preRepairSample) {
	const uint8 lastSample = sample;

	// In Gabriel Knight: Sins of the Fathers CD, the DPCM8-encoded speech contains overflows.
	// Overflows wrap from positive to negative, or negative to positive. This continues
	// until the wave settles at a new (wrapped) zero DC offset.

	// We can't look ahead to see where the wave "reconnects" to valid data, so we
	// decay the wave on an artificial slope until it does. This seems to take on average
	// around 9-10 samples. The slope value below was chosen through analysing spectrographs
	// of the decoded/repaired wave data to find the slope which removed the pops most
	// cleanly across a test selection of game speech.

#define REPAIR_SLOPE 12

	switch (repairState) {
	case 0: {
		const int16 newSampleOverflow = (int16)sample + tableDPCM8[delta & 15];

		if (newSampleOverflow > 255) {
			// Positive overflow has occurred; begin artificial negative slope.
			repairState = 1;
			sample = lastSample - REPAIR_SLOPE;
			// We also begin tracking the un-repaired waveform, so we can tell when to stop.
			preRepairSample = (uint8)newSampleOverflow;

			debugC(1, kDebugLevelSound, "DPCM8 OVERFLOW (+)");

		} else if (newSampleOverflow < 0) {
			// Negative overflow has occurred; begin artificial positive slope.
			repairState = 2;
			sample = lastSample + REPAIR_SLOPE;
			// We also begin tracking the un-repaired waveform, so we can tell when to stop.
			preRepairSample = (uint8)newSampleOverflow;

			debugC(1, kDebugLevelSound, "DPCM8 OVERFLOW (-)");

		} else {
			sample = (uint8)newSampleOverflow;
		}
	}	break;
	case 1: {
		// Check for a slope wrap. This circumstance should never happen in reality;
		// the unrepaired wave would somehow need to be stuck near minimum
		// value over the entire course of the slope.
		if (lastSample < REPAIR_SLOPE)
			warning("Negative slope wrap!");

		const uint8 slopeSample = lastSample - REPAIR_SLOPE;
		preRepairSample += tableDPCM8[delta & 15];

		// Stop the repair if the artificial slope has intersected with real data.
		if (preRepairSample >= slopeSample) {
			// Return to real data.
			repairState = 0;
			sample = preRepairSample;
		} else {
			sample = slopeSample;
		}
	}	break;
	case 2: {
		// Check for a slope wrap. This circumstance should never happen in reality;
		// the unrepaired wave would somehow need to be stuck near maximum
		// value over the entire course of the slope.
		if (lastSample > (255 - REPAIR_SLOPE))
			warning("Positive slope wrap!");

		const uint8 slopeSample = lastSample + REPAIR_SLOPE;
		preRepairSample += tableDPCM8[delta & 15];

		// Stop the repair if the artificial slope has intersected with real data.
		if (preRepairSample <= slopeSample) {
			// Return to real data.
			repairState = 0;
			sample = preRepairSample;
		} else {
			sample = slopeSample;
		}
	}	break;
	default:
		warning("Invalid repair state!");
		repairState = 0;
		break;
	}

	*out = ((lastSample + sample) << 7) ^ 0x8000;
}

/**
 * Decompresses 8-bit DPCM compressed audio. Each byte read
 * outputs two samples into the decompression buffer.
 */
template <bool OLD>
static void deDPCM8Mono(int16 *out, Common::ReadStream &audioStream, const uint32 numBytes, uint8 &sample,
						const bool popfixEnabled, uint8 &repairState, uint8 &preRepairSample) {
	if (popfixEnabled) {
		for (uint32 i = 0; i < numBytes; ++i) {
			const uint8 delta = audioStream.readByte();
			deDPCM8NibbleWithRepair(out++, sample, delta >> 4, repairState, preRepairSample);
			deDPCM8NibbleWithRepair(out++, sample, delta & 0xf, repairState, preRepairSample);
		}
	} else {
		for (uint32 i = 0; i < numBytes; ++i) {
			const uint8 delta = audioStream.readByte();
			deDPCM8Nibble<OLD>(out++, sample, delta >> 4);
			deDPCM8Nibble<OLD>(out++, sample, delta & 0xf);
		}
	}
}

static void deDPCM8Stereo(int16 *out, Common::ReadStream &audioStream, uint32 numBytes, uint8 &sampleL, uint8 &sampleR) {
	for (uint32 i = 0; i < numBytes; ++i) {
		const uint8 delta = audioStream.readByte();
		deDPCM8Nibble<false>(out++, sampleL, delta >> 4);
		deDPCM8Nibble<false>(out++, sampleR, delta & 0xf);
	}
}

# pragma mark -

template<bool STEREO, bool S16BIT, bool OLDDPCM8>
SOLStream<STEREO, S16BIT, OLDDPCM8>::SOLStream(Common::SeekableReadStream *stream, const DisposeAfterUse::Flag disposeAfterUse, const uint16 sampleRate, const int32 rawDataSize) :
	_stream(stream, disposeAfterUse),
	_sampleRate(sampleRate),
	// SSCI aligns the size of SOL data to 32 bits
	_rawDataSize(rawDataSize & ~3),
	// The pop fix is only verified with (relevant to?) "old" DPCM8, so we enforce that here.
	_popfixDPCM8(g_sci->_features->useAudioPopfix() && OLDDPCM8) {
		if (S16BIT) {
			_dpcmCarry16.l = _dpcmCarry16.r = 0;
		} else {
			_dpcmCarry8.l = _dpcmCarry8.r = 0x80;
		}

		const uint8 compressionRatio = 2;
		const uint8 numChannels = STEREO ? 2 : 1;
		const uint8 bytesPerSample = S16BIT ? 2 : 1;
		_length = ((uint64)_rawDataSize * compressionRatio * 1000) / (_sampleRate * numChannels * bytesPerSample);
	}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
bool SOLStream<STEREO, S16BIT, OLDDPCM8>::seek(const Audio::Timestamp &where) {
	if (where != 0) {
		// In order to seek in compressed SOL files, all previous bytes must be
		// known since it uses differential compression. Therefore, only seeking
		// to the beginning is supported now (SSCI does not offer seeking
		// anyway)
		return false;
	}

	if (S16BIT) {
		_dpcmCarry16.l = _dpcmCarry16.r = 0;
	} else {
		_dpcmCarry8.l = _dpcmCarry8.r = 0x80;
	}

	return _stream->seek(0, SEEK_SET);
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
Audio::Timestamp SOLStream<STEREO, S16BIT, OLDDPCM8>::getLength() const {
	return _length;
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
int SOLStream<STEREO, S16BIT, OLDDPCM8>::readBuffer(int16 *buffer, const int numSamples) {
	// Reading an odd number of 8-bit samples will result in a loss of samples
	// since one byte represents two samples and we do not store the second
	// nibble in this case; it should never happen in reality
	assert(S16BIT || (numSamples % 2) == 0);

	const int samplesPerByte = S16BIT ? 1 : 2;

	int32 bytesToRead = numSamples / samplesPerByte;
	if (_stream->pos() + bytesToRead > _rawDataSize) {
		bytesToRead = _rawDataSize - _stream->pos();
	}

	if (S16BIT) {
		if (STEREO) {
			deDPCM16Stereo(buffer, *_stream, bytesToRead, _dpcmCarry16.l, _dpcmCarry16.r);
		} else {
			deDPCM16Mono(buffer, *_stream, bytesToRead, _dpcmCarry16.l);
		}
	} else {
		if (STEREO) {
			deDPCM8Stereo(buffer, *_stream, bytesToRead, _dpcmCarry8.l, _dpcmCarry8.r);
		} else {
			deDPCM8Mono<OLDDPCM8>(buffer, *_stream, bytesToRead, _dpcmCarry8.l,
								  _popfixDPCM8.enabled, _popfixDPCM8.state, _popfixDPCM8.preRepairSample);
		}
	}

	const int samplesRead = bytesToRead * samplesPerByte;
	return samplesRead;
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
bool SOLStream<STEREO, S16BIT, OLDDPCM8>::isStereo() const {
	return STEREO;
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
int SOLStream<STEREO, S16BIT, OLDDPCM8>::getRate() const {
	return _sampleRate;
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
bool SOLStream<STEREO, S16BIT, OLDDPCM8>::endOfData() const {
	return _stream->eos() || _stream->pos() >= _rawDataSize;
}

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
bool SOLStream<STEREO, S16BIT, OLDDPCM8>::rewind() {
	return seek(0);
}

Audio::SeekableAudioStream *makeSOLStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	int32 initialPosition = stream->pos();

	byte header[6];
	if (stream->read(header, sizeof(header)) != sizeof(header)) {
		stream->seek(initialPosition, SEEK_SET);
		return nullptr;
	}

	if ((header[0] & 0x7f) != kResourceTypeAudio || READ_BE_UINT32(header + 2) != MKTAG('S', 'O', 'L', 0)) {
		stream->seek(initialPosition, SEEK_SET);
		return nullptr;
	}

	const uint8 headerSize = header[1] + kResourceHeaderSize;
	const uint16 sampleRate = stream->readUint16LE();
	const byte flags = stream->readByte();
	const uint32 dataSize = stream->readUint32LE();

	initialPosition += headerSize;

	if (flags & kCompressed) {
		if (flags & kStereo && flags & k16Bit) {
			return new SOLStream<true, true, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), DisposeAfterUse::YES, sampleRate, dataSize);
		} else if (flags & kStereo) {
			if (getSciVersion() < SCI_VERSION_2_1_EARLY) {
				error("SCI2 and earlier did not support stereo SOL audio");
			}

			return new SOLStream<true, false, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), DisposeAfterUse::YES, sampleRate, dataSize);
		} else if (flags & k16Bit) {
			return new SOLStream<false, true, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), DisposeAfterUse::YES, sampleRate, dataSize);
		} else {
			if (getSciVersion() < SCI_VERSION_2_1_EARLY) {
				return new SOLStream<false, false, true>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), DisposeAfterUse::YES, sampleRate, dataSize);
			} else {
				return new SOLStream<false, false, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), DisposeAfterUse::YES, sampleRate, dataSize);
			}
		}
	}

	byte rawFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (flags & k16Bit) {
		rawFlags |= Audio::FLAG_16BITS;
	} else {
		rawFlags |= Audio::FLAG_UNSIGNED;
	}

	if (flags & kStereo) {
		rawFlags |= Audio::FLAG_STEREO;
	}

	return Audio::makeRawStream(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), sampleRate, rawFlags, disposeAfterUse);
}
}
