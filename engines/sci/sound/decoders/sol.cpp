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

#include "audio/decoders/raw.h"
#include "common/substream.h"
#include "common/util.h"
#include "engines/sci/sci.h"
#include "engines/sci/sound/decoders/sol.h"

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

static const byte tableDPCM8[8] = { 0, 1, 2, 3, 6, 10, 15, 21 };

/**
 * Decompresses 16-bit DPCM compressed audio. Each byte read
 * outputs one sample into the decompression buffer.
 */
static int16 deDPCM16(int16 *out, Common::SeekableReadStream &audioStream, size_t numBytes, int16 s = 0) {
	for (size_t i = 0; i < numBytes; i++) {
		byte b = audioStream.readByte();
		if (b & 0x80) {
			s -= tableDPCM16[b & 0x7f];
		} else {
			s += tableDPCM16[b];
		}
		s = CLIP<int16>(s, -32768, 32767);
		*out++ = TO_LE_16(s);
	}

	return s;
}

/**
 * Decompresses one half of an 8-bit DPCM compressed audio
 * byte.
 */
static void deDPCM8Nibble(int16 *out, byte &s, byte b) {
	if (b & 8) {
		s -= tableDPCM8[b & 7];
	} else {
		s += tableDPCM8[b & 7];
	}
	s = CLIP<byte>(s, 0, 255);
	*out = (s << 8) ^ 0x8000;
}

/**
 * Decompresses 8-bit DPCM compressed audio. Each byte read
 * outputs two samples into the decompression buffer.
 */
static int32 deDPCM8(int16 *out, Common::SeekableReadStream &audioStream, size_t numBytes, byte s = 0x80) {
	for (size_t i = 0; i < numBytes; i++) {
		byte b = audioStream.readByte();
		deDPCM8Nibble(out++, s, b >> 4);
		deDPCM8Nibble(out++, s, b & 0xf);
	}

	return s;
}

# pragma mark -

template<bool STEREO, bool S16BIT>
SOLStream<STEREO, S16BIT>::SOLStream(Common::SeekableReadStream *stream, const DisposeAfterUse::Flag disposeAfterUse, const int32 dataOffset, const uint16 sampleRate, const int32 rawDataSize) :
	_stream(stream, disposeAfterUse),
	_dataOffset(dataOffset),
	_sampleRate(sampleRate),
	// SSCI aligns the size of SOL data to 32 bits
	_rawDataSize(rawDataSize & ~3),
	// TODO: This is not valid for stereo SOL files, which
	// have interleaved L/R compression so need to store the
	// carried values for each channel separately. See
	// 60900.aud from Lighthouse for an example stereo file
	_dpcm8Carry(0x80),
	_dpcm16Carry(0) {
		const uint8 compressionRatio = 2;
		const uint8 numChannels = STEREO ? 2 : 1;
		const uint8 bytesPerSample = S16BIT ? 2 : 1;
		_length = Audio::Timestamp((_rawDataSize * compressionRatio * 1000) / (_sampleRate * numChannels * bytesPerSample), 60);
	}

/**
 * Seeks to a given offset in the stream.
 *
 * @param where offset as timestamp
 * @return true on success, false on failure.
 */
template <bool STEREO, bool S16BIT>
bool SOLStream<STEREO, S16BIT>::seek(const Audio::Timestamp &where) override {
	if (where < 0 || where > getLength()) {
		warning("Seeking to %d which is outside the data", where.msecs());
		return false;
	}

	return _stream->seek(_dataOffset + _rawDataSize * where.msecs() / getLength().msecs(), SEEK_SET);
}

template <bool STEREO, bool S16BIT>
Audio::Timestamp SOLStream<STEREO, S16BIT>::getLength() const override {
	return _length;
}

template <bool STEREO, bool S16BIT>
int SOLStream<STEREO, S16BIT>::readBuffer(int16 *buffer, const int numSamples) override {
	// Reading an odd number of 8-bit samples will result in a loss of samples
	// since one byte represents two samples and we do not store the second
	// nibble in this case; it should never happen in reality
	assert(S16BIT || (numSamples % 2) == 0);

	const int samplesPerByte = S16BIT ? 1 : 2;

	int bytesToRead = numSamples / samplesPerByte;
	if (_stream->pos() + bytesToRead > _rawDataSize) {
		bytesToRead = _rawDataSize - _stream->pos();
	}

	if (S16BIT) {
		_dpcm16Carry = deDPCM16(buffer, *_stream, bytesToRead, _dpcm16Carry);
	} else {
		_dpcm8Carry = deDPCM8(buffer, *_stream, bytesToRead, _dpcm8Carry);
	}

	const int samplesRead = bytesToRead * samplesPerByte;
	return samplesRead;
}

template <bool STEREO, bool S16BIT>
bool SOLStream<STEREO, S16BIT>::isStereo() const override {
	return STEREO;
}

template <bool STEREO, bool S16BIT>
int SOLStream<STEREO, S16BIT>::getRate() const override {
	return _sampleRate;
}

template <bool STEREO, bool S16BIT>
bool SOLStream<STEREO, S16BIT>::endOfData() const override {
	return _stream->eos() || _stream->pos() >= _dataOffset + _rawDataSize;
}

template <bool STEREO, bool S16BIT>
bool SOLStream<STEREO, S16BIT>::rewind() override {
	_dpcm8Carry = 0x80;
	_dpcm16Carry = 0;
	return seek(0);
}

Audio::SeekableAudioStream *makeSOLStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {

	// TODO: Might not be necessary? Makes seeking work, but
	// not sure if audio is ever actually seeked in SSCI.
	const int32 initialPosition = stream->pos();

	byte header[6];
	if (stream->read(header, sizeof(header)) != sizeof(header)) {
		return nullptr;
	}

	if (header[0] != 0x8d || READ_BE_UINT32(header + 2) != MKTAG('S', 'O', 'L', 0)) {
		return nullptr;
	}

	const uint8 headerSize = header[1];
	const uint16 sampleRate = stream->readUint16LE();
	const byte flags = stream->readByte();
	const uint32 dataSize = stream->readUint32LE();

	if (flags & kCompressed) {
		if (flags & kStereo && flags & k16Bit) {
			return new SOLStream<true, true>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), disposeAfterUse, headerSize, sampleRate, dataSize);
		} else if (flags & kStereo) {
			return new SOLStream<true, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), disposeAfterUse, headerSize, sampleRate, dataSize);
		} else if (flags & k16Bit) {
			return new SOLStream<false, true>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), disposeAfterUse, headerSize, sampleRate, dataSize);
		} else {
			return new SOLStream<false, false>(new Common::SeekableSubReadStream(stream, initialPosition, initialPosition + dataSize, disposeAfterUse), disposeAfterUse, headerSize, sampleRate, dataSize);
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

	return Audio::makeRawStream(new Common::SeekableSubReadStream(stream, initialPosition + headerSize, initialPosition + headerSize + dataSize, disposeAfterUse), sampleRate, rawFlags, disposeAfterUse);
}

// TODO: This needs to be removed when resource manager is fixed
// to not split audio into two parts
Audio::SeekableAudioStream *makeSOLStream(Common::SeekableReadStream *headerStream, Common::SeekableReadStream *dataStream, DisposeAfterUse::Flag disposeAfterUse) {

	if (headerStream->readUint32BE() != MKTAG('S', 'O', 'L', 0)) {
		return nullptr;
	}

	const uint16 sampleRate = headerStream->readUint16LE();
	const byte flags = headerStream->readByte();
	const int32 dataSize = headerStream->readSint32LE();

	if (flags & kCompressed) {
		if (flags & kStereo && flags & k16Bit) {
			return new SOLStream<true, true>(dataStream, disposeAfterUse, 0, sampleRate, dataSize);
		} else if (flags & kStereo) {
			return new SOLStream<true, false>(dataStream, disposeAfterUse, 0, sampleRate, dataSize);
		} else if (flags & k16Bit) {
			return new SOLStream<false, true>(dataStream, disposeAfterUse, 0, sampleRate, dataSize);
		} else {
			return new SOLStream<false, false>(dataStream, disposeAfterUse, 0, sampleRate, dataSize);
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

	return Audio::makeRawStream(dataStream, sampleRate, rawFlags, disposeAfterUse);
}

}
