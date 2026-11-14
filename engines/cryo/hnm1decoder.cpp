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

#include "cryo/hnm1decoder.h"

#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Cryo {

// Number of bytes the decompressors may write past the logical end of a frame:
// a run or a copy is never cut short, so the last one can overshoot a little.
static const uint32 kDecodeSlack = 512;

// Chunk tags, as the little endian words they are compared against
static const uint16 kChunkSound   = 0x6473; // "sd"
static const uint16 kChunkPalette = 0x6C70; // "pl"

/**
 * The LZ77 variant used by EdenGame::expandHSQ() (resource.cpp), with a 16 bit
 * little endian, least significant bit first queue. Unlike expandHSQ() this
 * takes explicit bounds instead of trusting the stream to end in the right
 * place.
 */
static bool decodeHSQ(const byte *src, uint32 srcSize, byte *dst, uint32 dstSize, uint32 dstCapacity) {
	uint32 pos = 0, out = 0;
	uint16 queue = 0;

	auto nextBit = [&]() -> int {
		int bit = queue & 1;
		queue >>= 1;
		if (!queue) {
			if (pos + 2 > srcSize)
				return -1;
			queue = READ_LE_UINT16(src + pos);
			pos += 2;
			bit = queue & 1;
			queue = (queue >> 1) | 0x8000;
		}
		return bit;
	};

	while (out < dstSize) {
		int bit = nextBit();
		if (bit < 0)
			return false;

		if (bit) {
			if (pos >= srcSize || out >= dstCapacity)
				return false;
			dst[out++] = src[pos++];
			continue;
		}

		uint32 length = 0;
		int32 offset;
		bit = nextBit();
		if (bit < 0)
			return false;

		if (!bit) {
			for (int i = 0; i < 2; i++) {
				bit = nextBit();
				if (bit < 0)
					return false;
				length = (length << 1) | bit;
			}
			if (pos >= srcSize)
				return false;
			offset = (int32)(0xFF00 | src[pos++]) - 0x10000;
		} else {
			if (pos + 2 > srcSize)
				return false;
			uint16 tmp = READ_LE_UINT16(src + pos);
			pos += 2;
			length = tmp & 7;
			offset = (int32)((tmp >> 3) | 0xE000) - 0x10000;
			if (!length) {
				if (pos >= srcSize)
					return false;
				length = src[pos++];
				if (!length)
					return true; // End of stream
			}
		}

		if ((int32)out + offset < 0)
			return false;
		uint32 from = (uint32)((int32)out + offset);
		for (length += 2; length > 0; length--) {
			if (out >= dstCapacity)
				return false;
			dst[out++] = dst[from++];
		}
	}

	return true;
}

/**
 * First stage of the 0xAD scheme (FUN_1000_3b6d in the DOS executable): a byte
 * oriented LZ77. Control bytes below 0x80 are literals, biased by @p bias.
 * Back references come in pairs which share the extra bits of a single second
 * byte, so the first of a pair takes two bytes and the second only one.
 */
static bool decodeLZ(const byte *src, uint32 srcSize, uint32 &srcUsed, byte *dst,
                     uint32 dstPos, uint32 dstEnd, uint32 dstCapacity, byte bias) {
	uint32 pos = 0;
	byte extras = 0;
	bool second = false;

	while (dstPos < dstEnd) {
		if (pos >= srcSize || dstPos >= dstCapacity)
			return false;

		byte control = src[pos];
		if (control < 0x80) {
			// A zero stays zero, anything else picks up the bias
			dst[dstPos++] = control ? (byte)(control + bias) : 0;
			pos++;
			continue;
		}

		uint32 distance, length;
		if (!second) {
			if (pos + 2 > srcSize)
				return false;
			extras = src[pos + 1];
			distance = (byte)((byte)(control << 1) + ((extras >> 4) & 1)) + 1u;
			length = (extras >> 5) + 2u;
			pos += 2;
			second = true;
		} else {
			byte low = extras & 0x0F;
			distance = (byte)((byte)(control << 1) + (low & 1)) + 1u;
			length = (low >> 1) + 2u;
			pos++;
			second = false;
		}

		if (distance > dstPos)
			return false;
		uint32 from = dstPos - distance;
		for (; length > 0; length--) {
			if (dstPos >= dstCapacity)
				return false;
			dst[dstPos++] = dst[from++];
		}
	}

	srcUsed = pos;
	return true;
}

/**
 * The 0xAC scheme (FUN_1000_e850 and the loop at LAB_1000_e79b in the demo's
 * DEMO.EXE). Two stages again, but where the 0xAD scheme keeps everything the
 * second stage needs in one bit stream, this one spreads the work over five,
 * whose offsets sit in a longer chunk header. They are all counted from two
 * bytes before the chunk:
 *
 *   +10, +12  run lengths, a nibble each
 *   +14       the bit stream driving the second stage
 *   +16, +18  the control bytes and the extra bits the first stage reads
 *   +20       how much the first stage leaves for the second
 *
 * The picture can come out a handful of pixels short of its last row, which is
 * how the original behaves too: both stop as soon as the first stage's output
 * has been read through, wherever in the row that leaves them.
 */
static bool decodeAC(const byte *chunk, uint32 size, byte *dst, uint32 dstSize,
                     uint32 dstCapacity) {
	if (size < 22 || dstSize > dstCapacity) {
		return false;
	}

	uint32 lengthsA = READ_LE_UINT16(chunk + 10);
	uint32 lengthsB = READ_LE_UINT16(chunk + 12);
	uint32 bits     = READ_LE_UINT16(chunk + 14);
	uint32 control  = READ_LE_UINT16(chunk + 16);
	uint32 extras   = READ_LE_UINT16(chunk + 18);
	uint32 stage1Size = READ_LE_UINT16(chunk + 20);

	if (lengthsA < 2 || lengthsB < 2 || bits < 2 || control < 2 || extras < 2 ||
	        !stage1Size || stage1Size > dstSize) {
		return false;
	}
	lengthsA -= 2;
	lengthsB -= 2;
	bits -= 2;
	control -= 2;
	extras -= 2;

	// First stage: an LZ77 pass into the tail of the buffer, shaped like the
	// 0xAD one but with the control byte complemented before the distance
	uint32 out = dstSize - stage1Size;
	byte pair = 0;
	bool second = false;
	while (out < dstSize) {
		if (control >= size) {
			return false;
		}
		byte code = chunk[control++];
		if (code < 0x80) {
			dst[out++] = code;
			continue;
		}

		byte inverted = ~code;
		uint32 distance, length;
		if (!second) {
			if (extras >= size) {
				return false;
			}
			pair = chunk[extras++];
			distance = (byte)((byte)(inverted << 1) + 1 + ((pair >> 4) & 1));
			length = (pair >> 5) + 2u;
			second = true;
		} else {
			byte low = pair & 0x0F;
			distance = (byte)((byte)(inverted << 1) + 1 + (low & 1));
			length = (low >> 1) + 2u;
			second = false;
		}

		if (!distance || distance > out) {
			return false;
		}
		uint32 from = out - distance;
		for (; length > 0 && out < dstSize; length--) {
			dst[out++] = dst[from++];
		}
	}

	// Second stage: expand that into the picture. A zero bit copies a byte
	// across, the rest are runs of one, whose length comes from a nibble of one
	// of the two length streams.
	uint32 src = dstSize - stage1Size;
	uint32 pos = 0;
	uint16 queue = 0;
	bool highNibble = true;
	bool failed = false;

	// The same queue as the 0xAD scheme uses: most significant bit first, with a
	// set sentinel appended so that running out reads back as zero.
	auto nextBit = [&]() -> int {
		int bit = (queue >> 15) & 1;
		queue = (uint16)(queue << 1);
		if (!queue) {
			if (bits + 2 > size) {
				failed = true;
				return 0;
			}
			uint16 word = READ_LE_UINT16(chunk + bits);
			bits += 2;
			queue = (uint16)((word << 1) | 1);
			return (word >> 15) & 1;
		}
		return bit;
	};

	while (src < dstSize && !failed) {
		if (!nextBit()) {
			if (pos >= dstCapacity) {
				return false;
			}
			dst[pos++] = dst[src++];
			continue;
		}

		uint32 length;
		if (!nextBit()) {
			// A nibble of the first length stream, taken high half first. The
			// low half belongs to the byte already stepped over.
			if (highNibble) {
				if (lengthsA >= size) {
					return false;
				}
				length = (chunk[lengthsA++] >> 4) + 2u;
			} else {
				length = (chunk[lengthsA - 1] & 0x0F) + 2u;
			}
			highNibble = !highNibble;
		} else if (!nextBit()) {
			length = 2;
		} else {
			if (lengthsB >= size) {
				return false;
			}
			length = chunk[lengthsB++] + 0x12u;
		}

		byte value = dst[src++];
		for (; length > 0; length--) {
			if (pos >= dstCapacity) {
				return false;
			}
			dst[pos++] = value;
		}
	}

	return !failed;
}

/**
 * Second stage of the 0xAD scheme (LAB_1000_39f3 and LAB_1000_3aaf): a run
 * length pass driven by a bit stream which directly follows the first stage's
 * data, while the bytes it works on come from the first stage's output.
 *
 * @p longRunsFirst picks between the two orderings of the length code the
 * original uses, selected by bit 7 of the compression flags.
 */
static bool decodeRLE(const byte *bits, uint32 bitsSize, byte *dst, uint32 dstCapacity,
                      uint32 byteSrc, uint32 dstPos, uint32 dstEnd, bool longRunsFirst) {
	uint32 pos = 0;
	uint16 queue = 0;
	uint32 run = 0;
	bool failed = false;

	// A 16 bit MSB first queue with a set sentinel appended, so it has run out
	// exactly when it reads back as zero
	auto nextBit = [&]() -> int {
		int bit = (queue >> 15) & 1;
		queue = (uint16)(queue << 1);
		if (!queue) {
			if (pos + 2 > bitsSize) {
				failed = true;
				return 0;
			}
			uint16 word = READ_LE_UINT16(bits + pos);
			pos += 2;
			queue = (uint16)((word << 1) | 1);
			return (word >> 15) & 1;
		}
		return bit;
	};

	auto fill = [&](byte value, uint32 count) {
		while (count-- > 0) {
			if (dstPos >= dstCapacity) {
				failed = true;
				return;
			}
			dst[dstPos++] = value;
		}
	};

	// Long runs take their length from the byte stream, and leave a length
	// behind for the next long run to pick up.
	auto longRun = [&](byte value) {
		uint32 count;
		if (run > 4) {
			count = run;
			run = 0;
		} else if (run == 4) {
			if (pos >= bitsSize) {
				failed = true;
				return;
			}
			count = bits[pos++] + 0x14u;
			run = 0;
		} else {
			if (pos >= bitsSize) {
				failed = true;
				return;
			}
			byte length = bits[pos];
			if (length >> 4) {
				count = (length >> 4) + 4u;
				pos++;
			} else {
				if (pos + 2 > bitsSize) {
					failed = true;
					return;
				}
				count = bits[pos + 1] + 0x14u;
				pos += 2;
			}
			run = (length & 0x0F) + 4u;
		}
		fill(value, count);
	};

	while (dstPos < dstEnd && !failed) {
		if (!nextBit()) {
			// Copy one byte straight from the first stage's output
			if (byteSrc >= dstCapacity || dstPos >= dstCapacity)
				return false;
			dst[dstPos++] = dst[byteSrc++];
			continue;
		}

		if (byteSrc >= dstCapacity)
			return false;
		byte value = dst[byteSrc++];

		if (longRunsFirst) {
			if (!nextBit())
				longRun(value);
			else if (!nextBit())
				fill(value, 2);
			else if (!nextBit())
				fill(value, 3);
			else
				fill(value, 4);
		} else {
			if (!nextBit())
				fill(value, 2);
			else if (!nextBit())
				fill(value, 3);
			else if (!nextBit())
				fill(value, 4);
			else
				longRun(value);
		}
	}

	return !failed;
}

/**
 * Read a palette in the format used both by the container header and by "pl"
 * chunks: runs of (start, count, RGB triples), terminated by 0xFF,0xFF.
 * Components are 6 bits wide.
 */
static bool readPalette(const byte *data, uint32 size, uint32 &used, Graphics::Palette &palette,
                        uint16 *first = nullptr, uint16 *last = nullptr) {
	uint32 pos = 0;
	for (;;) {
		if (pos + 2 > size)
			return false;
		byte start = data[pos];
		byte count = data[pos + 1];
		pos += 2;
		if (start == 0xFF && count == 0xFF)
			break;
		uint16 num = count ? count : 256;
		if (start + num > 256 || pos + num * 3 > size)
			return false;
		for (uint16 i = 0; i < num; i++, pos += 3)
			palette.set(start + i, data[pos] * 4, data[pos + 1] * 4, data[pos + 2] * 4);
		// A movie names only the colours it uses. The rest belong to whatever
		// else is on screen - the cursor and the friezes among them - and are
		// not the movie's to give away.
		if (first && start < *first)
			*first = start;
		if (last && start + num - 1 > *last)
			*last = start + num - 1;
	}
	used = pos;
	return true;
}

/**
 * Walk the chunk list of a frame record, reporting the image chunk (if any)
 * and the extent of the sound data.
 */
static bool parseRecord(const byte *record, uint32 size, uint32 &imageChunk,
                        uint32 &soundStart, uint32 &soundSize) {
	imageChunk = 0;
	soundStart = 0;
	soundSize = 0;

	uint32 pos = 2;
	while (pos + 4 <= size) {
		uint16 tag = READ_LE_UINT16(record + pos);
		if (tag != kChunkSound && tag != kChunkPalette) {
			// A picture needs ten bytes of header at least. Records exist which
			// carry only sound and then four bytes of padding, and that padding
			// is not a short picture.
			if (pos + 10 <= size)
				imageChunk = pos;
			return true;
		}

		uint32 length = READ_LE_UINT16(record + pos + 2);
		if (length < 4 || pos + length > size)
			return false;
		if (tag == kChunkSound) {
			soundStart = pos + 4;
			soundSize = length - 4;
		}
		pos += length;
	}

	// A record without an image chunk is legal: it only carries sound
	return true;
}

HNM1Decoder::HNM1Decoder() : _stream(nullptr), _videoTrack(nullptr) {
}

HNM1Decoder::~HNM1Decoder() {
	close();
}

bool HNM1Decoder::loadStream(Common::SeekableReadStream *stream) {
	close();
	_stream = stream;

	uint32 streamSize = stream->size();
	if (streamSize < 8)
		return false;

	uint16 dataOffset = stream->readUint16LE();
	if (dataOffset < 8 || dataOffset >= streamSize)
		return false;

	// The palette, the marker byte and the frame offset table all have to fit
	// exactly into the space before the frame records.
	byte *header = new byte[dataOffset];
	stream->seek(0, SEEK_SET);
	if (stream->read(header, dataOffset) != dataOffset) {
		delete[] header;
		return false;
	}

	Graphics::Palette palette(256);
	uint32 used = 0;
	uint16 palFirst = 255, palLast = 0;
	if (!readPalette(header + 2, dataOffset - 2, used, palette, &palFirst, &palLast)) {
		delete[] header;
		return false;
	}

	uint32 pos = 2 + used;
	if (pos >= dataOffset) {
		delete[] header;
		return false;
	}
	// A spare 0xFF sits between the palette and the frame offsets in some
	// movies and not in others; the demo disc has both kinds. Where it is
	// missing the offsets start right after the palette's own terminator.
	if (header[pos] == 0xFF)
		pos++;

	Common::Array<uint32> frameOffsets;
	uint32 remaining = streamSize - dataOffset;
	for (;;) {
		if (pos + 4 > dataOffset) {
			delete[] header;
			return false;
		}
		uint32 value = READ_LE_UINT32(header + pos);
		pos += 4;
		frameOffsets.push_back(value);
		if (value == remaining)
			break;
		if (value > remaining) {
			delete[] header;
			return false;
		}
	}

	byte rawPalette[256 * 3];
	memcpy(rawPalette, palette.data(), sizeof(rawPalette));
	delete[] header;

	// Everything before the frame records must be accounted for, and there
	// has to be at least one record (the table always ends with a marker).
	if (pos != dataOffset || frameOffsets.size() < 2)
		return false;

	// Walk the records to pick up the sound, work out how tall the movie is,
	// and make sure this really is a stream we can decode. Sound is stored as
	// one VOC file sliced across the records.
	Common::MemoryWriteStreamDynamic soundData(DisposeAfterUse::YES);
	uint16 height = 0;
	for (uint frame = 0; frame + 1 < frameOffsets.size(); frame++) {
		uint32 start = dataOffset + frameOffsets[frame];
		uint32 size = frameOffsets[frame + 1] - frameOffsets[frame];
		if (size < 4)
			return false;

		byte *record = new byte[size];
		stream->seek(start, SEEK_SET);
		if (stream->read(record, size) != size) {
			delete[] record;
			return false;
		}

		// A record opens with its own length, and the offsets can reach past it:
		// one movie on the demo disc has a record trailed by leftovers from
		// whatever built the file, command line and all. Go by the record.
		size = MIN(size, (uint32)MAX<uint16>(READ_LE_UINT16(record), 4));

		uint32 imageChunk, soundStart, soundSize;
		bool usable = parseRecord(record, size, imageChunk, soundStart, soundSize);

		if (usable && imageChunk) {
			if (imageChunk + 10 > size) {
				usable = false;
			} else {
				byte checksum = 0;
				for (int i = 0; i < 6; i++)
					checksum += record[imageChunk + 4 + i];
				uint16 heightMode = READ_LE_UINT16(record + imageChunk + 2);
				uint16 chunkHeight = heightMode & 0xFF;
				byte mode = heightMode >> 8;

				// 0xFE is a complete picture and 0xFF a strip laid over it. A
				// frame that is neither is left to fail when its turn comes,
				// which holds the picture rather than losing the whole movie
				if ((mode == 0xFE || mode == 0xFF) && chunkHeight &&
				        chunkHeight <= kMaxHeight &&
				        (checksum == 0xAB || checksum == 0xAC || checksum == 0xAD))
					// Only a frame we can draw has a say in how tall the movie is
					height = MAX(height, chunkHeight);
			}
		}

		if (usable && soundSize)
			soundData.write(record + soundStart, soundSize);

		delete[] record;

		if (!usable)
			return false;
	}

	if (!height)
		return false;

	if (soundData.size() > 0) {
		// soundData frees its own buffer, so the audio stream gets a copy it
		// can own: the read stream takes it over, and makeVOCStream() takes
		// over the read stream, including when it rejects the data.
		byte *sound = (byte *)malloc(soundData.size());
		if (!sound)
			return false;
		memcpy(sound, soundData.getData(), soundData.size());
		Common::MemoryReadStream *voc = new Common::MemoryReadStream(sound,
		        soundData.size(), DisposeAfterUse::YES);
		Audio::SeekableAudioStream *audio = Audio::makeVOCStream(voc, Audio::FLAG_UNSIGNED,
		                                    DisposeAfterUse::YES);
		if (audio)
			addTrack(new HNM1AudioTrack(audio, getSoundType()));
	}

	_videoTrack = new HNM1VideoTrack(stream, dataOffset, frameOffsets, rawPalette, height, palFirst, palLast);
	addTrack(_videoTrack);
	return true;
}

void HNM1Decoder::getPaletteRange(uint16 &first, uint16 &last) const {
	first = _videoTrack ? _videoTrack->getPaletteFirst() : 0;
	last = _videoTrack ? _videoTrack->getPaletteLast() : 0;
}

void HNM1Decoder::close() {
	VideoDecoder::close();
	_videoTrack = nullptr;

	delete _stream;
	_stream = nullptr;
}

HNM1Decoder::HNM1VideoTrack::HNM1VideoTrack(Common::SeekableReadStream *stream, uint32 dataOffset,
        const Common::Array<uint32> &frameOffsets, const byte *palette, uint16 height,
        uint16 palFirst, uint16 palLast) :
	_stream(stream), _dataOffset(dataOffset), _frameOffsets(frameOffsets), _curFrame(-1),
	_height(height), _palette(palette, 256), _dirtyPalette(true), _record(nullptr),
	_palFirst(palFirst), _palLast(palLast),
	_recordAlloc(0) {

	_surface.create(kWidth, _height, Graphics::PixelFormat::createFormatCLUT8());
	_decodeBuffer = new byte[(uint32)kWidth * kMaxHeight + 4 + kDecodeSlack]();
}

HNM1Decoder::HNM1VideoTrack::~HNM1VideoTrack() {
	_surface.free();
	delete[] _record;
	delete[] _decodeBuffer;
}

void HNM1Decoder::HNM1VideoTrack::updatePalette(const byte *data, uint32 size) {
	uint32 used = 0;
	if (readPalette(data, size, used, _palette, &_palFirst, &_palLast))
		_dirtyPalette = true;
}

bool HNM1Decoder::HNM1VideoTrack::decodeImage(const byte *chunk, uint32 size) {
	if (size < 10)
		return false;

	// A complete picture is as wide as the frame; a strip says how wide it is in
	// the low nine bits of its flags
	uint16 width = READ_LE_UINT16(chunk) & 0x1FF;
	uint16 heightMode = READ_LE_UINT16(chunk + 2);
	uint16 height = heightMode & 0xFF;
	byte mode = heightMode >> 8;
	const byte *compHeader = chunk + 4;
	const byte *payload = chunk + 10;
	uint32 payloadSize = size - 10;

	byte checksum = 0;
	for (int i = 0; i < 6; i++)
		checksum += compHeader[i];

	uint32 uncompressed = READ_LE_UINT16(compHeader);
	uint32 imageSize = (uint32)width * height;
	if (!width || width > kWidth || !height || height > _height || uncompressed < imageSize)
		return false;

	// The image can be preceded by a few bytes of padding, which the original
	// leaves in place and draws from just past.
	uint32 imageOffset = uncompressed - imageSize;
	if (imageOffset > 4)
		return false;

	const uint32 capacity = (uint32)kWidth * kMaxHeight + 4 + kDecodeSlack;

	// The 0xAD scheme can keep the first four bytes of the chunk unpacked at the
	// head of the buffer, which pushes the picture along by that much
	uint32 plainBytes = 0;

	if (checksum == 0xAB) {
		if (!decodeHSQ(payload, payloadSize, _decodeBuffer, uncompressed, capacity))
			return false;
	} else if (checksum == 0xAC) {
		// This one reads streams from all over the chunk, so it gets the lot
		if (!decodeAC(chunk, size, _decodeBuffer, uncompressed, capacity))
			return false;
	} else if (checksum == 0xAD) {
		uint32 stage1Size = READ_LE_UINT16(compHeader + 2);
		byte compFlags = compHeader[4];

		uint32 out = 0;
		const byte *src = payload;
		uint32 srcSize = payloadSize;
		if (!(compFlags & 0x04)) {
			// The first four bytes of the image are stored plainly
			if (payloadSize < 4)
				return false;
			memcpy(_decodeBuffer, payload, 4);
			out = 4;
			src += 4;
			srcSize -= 4;
			plainBytes = 4;
		}

		uint32 end = out + uncompressed;
		if (stage1Size > uncompressed || end > capacity)
			return false;

		uint32 srcUsed = 0;
		uint32 scratch = end - stage1Size;
		if (!decodeLZ(src, srcSize, srcUsed, _decodeBuffer, scratch, end, capacity,
		              (compFlags & 0x40) ? 0x80 : 0x00))
			return false;
		if (!decodeRLE(src + srcUsed, srcSize - srcUsed, _decodeBuffer, capacity, scratch,
		               out, end, (compFlags & 0x80) != 0))
			return false;
	} else {
		return false;
	}

	const byte *src = _decodeBuffer + plainBytes + imageOffset;

	if (mode == 0xFF) {
		// Zeroes are left alone so what is already there shows through. Two
		// words at the head of the buffer give left and top; a strip without
		// them goes at the origin, as the frame-wide credit strips do
		uint16 left = 0;
		uint16 top = 0;
		if (plainBytes == 4) {
			left = READ_LE_UINT16(_decodeBuffer);
			top = READ_LE_UINT16(_decodeBuffer + 2);
		}
		if (left + width > kWidth || top + height > _height)
			return false;
		byte *dst = (byte *)_surface.getPixels() + (uint32)top * kWidth + left;
		for (uint16 row = 0; row < height; row++) {
			for (uint16 col = 0; col < width; col++) {
				if (src[col])
					dst[col] = src[col];
			}
			src += width;
			dst += kWidth;
		}
		return true;
	}

	// A complete picture is laid down as it comes, and rows past its height keep
	// whatever the last frame put there
	byte *dst = (byte *)_surface.getPixels();
	for (uint16 row = 0; row < height; row++) {
		memcpy(dst, src, width);
		src += width;
		dst += kWidth;
	}
	return true;
}

const Graphics::Surface *HNM1Decoder::HNM1VideoTrack::decodeNextFrame() {
	if (endOfTrack())
		return nullptr;

	_curFrame++;

	uint32 start = _dataOffset + _frameOffsets[_curFrame];
	uint32 size = _frameOffsets[_curFrame + 1] - _frameOffsets[_curFrame];
	if (size < 4)
		return &_surface;

	if (_recordAlloc < size) {
		delete[] _record;
		_record = new byte[size];
		_recordAlloc = size;
	}
	_stream->seek(start, SEEK_SET);
	if (_stream->read(_record, size) != size)
		return &_surface;

	// The record's own length wins over the offsets, which can reach past it
	size = MIN(size, (uint32)MAX<uint16>(READ_LE_UINT16(_record), 4));

	// Palette updates have to be applied before the image is decoded
	uint32 pos = 2;
	while (pos + 4 <= size) {
		uint16 tag = READ_LE_UINT16(_record + pos);
		if (tag != kChunkSound && tag != kChunkPalette)
			break;
		uint32 length = READ_LE_UINT16(_record + pos + 2);
		if (length < 4 || pos + length > size)
			return &_surface;
		if (tag == kChunkPalette)
			updatePalette(_record + pos + 4, length - 4);
		pos += length;
	}

	if (pos + 10 <= size && !decodeImage(_record + pos, size - pos)) {
		// Not everything in this format is understood; leaving the previous
		// image alone looks better than showing garbage.
		debug(5, "HNM1: Could not decode frame %d", _curFrame);
	}

	return &_surface;
}

HNM1Decoder::HNM1AudioTrack::HNM1AudioTrack(Audio::SeekableAudioStream *stream,
        Audio::Mixer::SoundType soundType) : AudioTrack(soundType), _stream(stream) {
}

HNM1Decoder::HNM1AudioTrack::~HNM1AudioTrack() {
	delete _stream;
}

Audio::AudioStream *HNM1Decoder::HNM1AudioTrack::getAudioStream() const {
	return _stream;
}

} // End of namespace Cryo
