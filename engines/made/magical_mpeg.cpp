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

#include "made/magical_mpeg.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Made {

// MPEG-1 start codes (ISO/IEC 11172), preceded by the 00 00 01 prefix
enum {
	kStartCodePicture  = 0x00,
	kStartCodeSequence = 0xB3,
	kStartCodeEnd      = 0xB9,
	kStartCodePack     = 0xBA,
	kStartCodeVideoMin = 0xE0,
	kStartCodeVideoMax = 0xEF,
	kStartCodeAudioMin = 0xC0,
	kStartCodeAudioMax = 0xDF
};

enum {
	kPictureTypeP = 2,
	kPictureTypeB = 3
};

// The card sets the high bit of the four-bit frame-rate code as its marker.
// Codes 9-15 are reserved by MPEG-1; the low three bits retain the real rate.
static const byte kMagicalFrameRateCode = 0x9;

struct MagicKeyPattern {
	uint32 key;
	byte evenPattern[4];
};

static const MagicKeyPattern kMagicKeyPatterns[] = {
	{ 0x40044041, { 4, 3, 2, 3 } },
	{ 0xC39D7088, { 1, 3, 3, 3 } }
};

static const uint kDeltaPeriod = 56;

// ReelMagic encodes each forward/backward f_code by subtracting a value which
// advances with the picture's temporal sequence number. The two known card
// keys select the increment pattern used for even sequence numbers; odd ones
// always add six. Both the pattern and the MPEG f_code value repeat modulo 7,
// giving this table a 56-picture period.
static byte computeDeltaFCode(uint tsn, const byte *evenPattern) {
	uint result = 2;
	for (uint i = 0; i <= tsn; i++) {
		if ((i & 1) == 0)
			result += evenPattern[(i >> 1) & 3];
		else
			result += 6;
	}
	return result % 7;
}

static byte recoverFCode(byte encoded, byte delta) {
	int value = ((int)encoded - 1 + (int)delta) % 7;
	if (value < 0)
		value += 7;
	return (byte)(value + 1);
}

struct PayloadRange {
	uint32 offset;
	uint32 size;
};

/** Collect video or audio payload ranges from an MPEG-1 program stream. */
static void collectPesRanges(const byte *data, uint32 size, bool audio,
		Common::Array<PayloadRange> &ranges) {
	const byte streamMin = audio ? kStartCodeAudioMin : kStartCodeVideoMin;
	const byte streamMax = audio ? kStartCodeAudioMax : kStartCodeVideoMax;

	// The game's short overlay animations are bare video elementary streams.
	if (size >= 4 && data[0] == 0 && data[1] == 0 && data[2] == 1 && data[3] != kStartCodePack) {
		if (!audio) {
			PayloadRange range = { 0, size };
			ranges.push_back(range);
		}
		return;
	}

	uint32 pos = 0;
	while (pos + 4 <= size) {
		if (data[pos] != 0 || data[pos + 1] != 0 || data[pos + 2] != 1) {
			pos++;
			continue;
		}

		const byte streamId = data[pos + 3];
		pos += 4;

		if (streamId == kStartCodePack) {
			if (size - pos < 8)
				break;
			pos += 8;
			continue;
		}

		if (streamId == kStartCodeEnd)
			break;

		if (pos + 2 > size)
			break;

		const uint32 packetSize = (data[pos] << 8) | data[pos + 1];
		pos += 2;
		if (packetSize > size - pos)
			break;

		if (streamId >= streamMin && streamId <= streamMax) {
			// Skip stuffing, an optional buffer size, and optional PTS/DTS.
			uint32 skip = 0;
			while (skip < packetSize && data[pos + skip] == 0xFF)
				skip++;
			if (skip < packetSize && (data[pos + skip] & 0xC0) == 0x40)
				skip += 2;
			if (skip < packetSize) {
				const byte flags = data[pos + skip] & 0xF0;
				if (flags == 0x20)
					skip += 5;
				else if (flags == 0x30)
					skip += 10;
				else if (data[pos + skip] == 0x0F)
					skip++;
			}

			if (skip < packetSize) {
				PayloadRange range = { pos + skip, packetSize - skip };
				ranges.push_back(range);
			}
		}

		pos += packetSize;
	}
}

static const uint16 kLayer2BitRates[] = {
	0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0
};

static const uint32 kLayer2SampleRates[] = { 44100, 48000, 32000, 0 };

static bool isLayer2Header(const byte *p) {
	return p[0] == 0xFF && (p[1] & 0xE0) == 0xE0 &&
		((p[1] >> 3) & 0x03) == 0x03 && ((p[1] >> 1) & 0x03) == 0x02;
}

uint32 MagicalMpeg::fixAudioPadding(byte *data, uint32 size) {
	Common::Array<PayloadRange> ranges;
	collectPesRanges(data, size, true, ranges);
	if (ranges.empty())
		return 0;

	// Reassemble the payload so frames crossing packet boundaries stay intact.
	uint64 totalAudioSize = 0;
	for (uint i = 0; i < ranges.size(); i++)
		totalAudioSize += ranges[i].size;
	if (totalAudioSize > 0xFFFFFFFFU)
		return 0;
	const uint32 audioSize = (uint32)totalAudioSize;

	Common::Array<byte> audio;
	audio.resize(audioSize);

	Common::Array<uint32> starts;
	starts.reserve(ranges.size());
	uint32 audioPos = 0;
	for (uint i = 0; i < ranges.size(); i++) {
		starts.push_back(audioPos);
		memcpy(&audio[audioPos], data + ranges[i].offset, ranges[i].size);
		audioPos += ranges[i].size;
	}

	uint rangeHint = 0;
	auto audioToSource = [&](uint32 offset) -> uint32 {
		while (rangeHint + 1 < ranges.size() && starts[rangeHint + 1] <= offset)
			rangeHint++;
		return ranges[rangeHint].offset + offset - starts[rangeHint];
	};

	uint32 patched = 0;
	uint32 pos = 0;
	while (pos + 4 <= audioSize) {
		if (!isLayer2Header(&audio[pos])) {
			pos++;
			continue;
		}

		const uint16 bitRate = kLayer2BitRates[(audio[pos + 2] >> 4) & 0x0F];
		const uint32 sampleRate = kLayer2SampleRates[(audio[pos + 2] >> 2) & 0x03];
		if (bitRate == 0 || sampleRate == 0) {
			pos++;
			continue;
		}

		const bool padded = (audio[pos + 2] & 0x02) != 0;
		const uint32 base = (144 * (uint32)bitRate * 1000) / sampleRate;
		uint32 length = base + (padded ? 1 : 0);

		// Some ReelMagic assets set the padding bit without adding the byte.
		if (padded && pos + length + 4 <= audioSize &&
			!isLayer2Header(&audio[pos + length]) &&
			isLayer2Header(&audio[pos + length - 1])) {
			data[audioToSource(pos + 2)] &= ~0x02;
			audio[pos + 2] &= ~0x02;
			length--;
			patched++;
		}

		pos += length;
	}

	return patched;
}

uint32 MagicalMpeg::unlockBuffer(byte *data, uint32 size, uint32 magicKey) {
	const byte *evenPattern = nullptr;
	for (uint i = 0; i < ARRAYSIZE(kMagicKeyPatterns); i++) {
		if (kMagicKeyPatterns[i].key == magicKey) {
			evenPattern = kMagicKeyPatterns[i].evenPattern;
			break;
		}
	}

	if (!evenPattern) {
		warning("MagicalMpeg: unknown magic key 0x%08X, using the default", magicKey);
		evenPattern = kMagicKeyPatterns[0].evenPattern;
	}

	byte deltaTable[kDeltaPeriod];
	for (uint tsn = 0; tsn < kDeltaPeriod; tsn++)
		deltaTable[tsn] = computeDeltaFCode(tsn, evenPattern);

	Common::Array<PayloadRange> ranges;
	collectPesRanges(data, size, false, ranges);
	if (ranges.empty())
		return 0;

	uint64 totalVideoSize = 0;
	for (uint i = 0; i < ranges.size(); i++)
		totalVideoSize += ranges[i].size;
	if (totalVideoSize > 0xFFFFFFFFU)
		return 0;
	const uint32 videoSize = (uint32)totalVideoSize;

	Common::Array<byte> video;
	video.resize(videoSize);

	Common::Array<uint32> starts;
	starts.reserve(ranges.size());
	uint32 videoPos = 0;
	for (uint i = 0; i < ranges.size(); i++) {
		starts.push_back(videoPos);
		memcpy(&video[videoPos], data + ranges[i].offset, ranges[i].size);
		videoPos += ranges[i].size;
	}

	uint rangeHint = 0;
	auto videoToSource = [&](uint32 offset) -> uint32 {
		while (rangeHint + 1 < ranges.size() && starts[rangeHint + 1] <= offset)
			rangeHint++;
		return ranges[rangeHint].offset + offset - starts[rangeHint];
	};

	uint32 patchedPictures = 0;
	for (uint32 pos = 0; pos + 9 <= videoSize; pos++) {
		if (video[pos] != 0 || video[pos + 1] != 0 || video[pos + 2] != 1)
			continue;

		const byte streamId = video[pos + 3];
		const byte *header = &video[pos + 4];
		if (streamId == kStartCodeSequence) {
			if ((header[3] & 0x0F) >= kMagicalFrameRateCode)
				data[videoToSource(pos + 7)] = header[3] & 0xF7;
			continue;
		}

		if (streamId != kStartCodePicture)
			continue;

		const uint tsn = (header[0] << 2) | (header[1] >> 6);
		const byte pictureType = (header[1] >> 3) & 0x07;
		if (pictureType != kPictureTypeP && pictureType != kPictureTypeB)
			continue;

		const byte delta = deltaTable[tsn % kDeltaPeriod];
		// P pictures carry a three-bit forward f_code split across bytes 3 and
		// 4. B pictures add a three-bit backward f_code in byte 4.
		const byte forward = ((header[3] & 0x03) << 1) | (header[4] >> 7);
		const byte backward = (header[4] >> 3) & 0x07;
		const byte newForward = recoverFCode(forward, delta);
		const byte byte3 = (header[3] & 0xFC) | ((newForward >> 1) & 0x03);
		byte byte4 = (header[4] & 0x7F) | ((newForward & 0x01) << 7);

		if (pictureType == kPictureTypeB) {
			const byte newBackward = recoverFCode(backward, delta);
			byte4 = (byte4 & 0xC7) | ((newBackward & 0x07) << 3);
		}

		data[videoToSource(pos + 7)] = byte3;
		data[videoToSource(pos + 8)] = byte4;
		patchedPictures++;
	}

	return patchedPictures;
}

static bool readFrameRateCode(Common::SeekableReadStream &stream, byte &code) {
	const int64 startPos = stream.pos();
	const int64 remaining = stream.size() - startPos;
	if (remaining <= 0)
		return false;

	const uint32 kScanSize = 16 * 1024;
	const uint32 size = (uint32)MIN<int64>(kScanSize, remaining);
	Common::Array<byte> buffer;
	buffer.resize(size);

	bool found = false;
	const uint32 read = stream.read(&buffer[0], size);
	for (uint32 i = 0; i + 8 <= read; i++) {
		if (buffer[i] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 1 &&
			buffer[i + 3] == kStartCodeSequence) {
			code = buffer[i + 7] & 0x0F;
			found = true;
			break;
		}
	}

	stream.seek(startPos);
	return found;
}

bool MagicalMpeg::isProgramStream(Common::SeekableReadStream &stream) {
	const int64 startPos = stream.pos();
	const bool programStream = stream.readUint32BE() == 0x000001BA;
	stream.seek(startPos);
	return programStream;
}

bool MagicalMpeg::isMagical(Common::SeekableReadStream &stream) {
	byte code = 0;
	return readFrameRateCode(stream, code) && code >= kMagicalFrameRateCode;
}

bool MagicalMpeg::getFrameDuration(Common::SeekableReadStream &stream, uint32 &num, uint32 &den) {
	static const struct {
		uint32 num;
		uint32 den;
	} kFrameDurations[] = {
		{    0,  0 },
		{ 1001, 24 },
		{ 1000, 24 },
		{ 1000, 25 },
		{ 1001, 30 },
		{ 1000, 30 },
		{ 1000, 50 },
		{ 1001, 60 },
		{ 1000, 60 }
	};

	byte code = 0;
	if (!readFrameRateCode(stream, code))
		return false;

	// ReelMagic sets bit 3 as its marker; the low three bits are the real rate.
	code &= 0x07;
	if (code == 0 || code >= ARRAYSIZE(kFrameDurations))
		return false;

	num = kFrameDurations[code].num;
	den = kFrameDurations[code].den;
	return true;
}

Common::SeekableReadStream *MagicalMpeg::unlock(Common::SeekableReadStream &stream,
		uint32 magicKey, bool magical) {
	const int64 startPos = stream.pos();
	const int64 streamSize = stream.size() - startPos;
	if (streamSize <= 0 || streamSize > 0xFFFFFFFFU)
		return nullptr;

	const uint32 size = (uint32)streamSize;
	byte *data = (byte *)malloc(size);
	if (!data)
		return nullptr;

	if (stream.read(data, size) != size) {
		free(data);
		return nullptr;
	}

	if (magical) {
		const uint32 patched = unlockBuffer(data, size, magicKey);
		debug(1, "MagicalMpeg: restored %u picture headers", patched);
	}

	const uint32 repadded = fixAudioPadding(data, size);
	if (repadded)
		debug(1, "MagicalMpeg: corrected %u audio padding bits", repadded);

	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

} // End of namespace Made
