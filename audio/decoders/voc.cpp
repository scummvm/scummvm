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
#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"


namespace Audio {

int getSampleRateFromVOCRate(int vocSR) {
	if (vocSR == 0xa5 || vocSR == 0xa6) {
		return 11025;
	} else if (vocSR == 0xd2 || vocSR == 0xd3) {
		return 22050;
	} else {
		int sr = 1000000L / (256L - vocSR);
		// inexact sampling rates occur e.g. in the kitchen in Monkey Island,
		// very easy to reach right from the start of the game.
		//warning("inexact sample rate used: %i (0x%x)", sr, vocSR);
		return sr;
	}
}

static byte *loadVOCFromStream(Common::ReadStream &stream, int &size, int &rate, int &loops, int &begin_loop, int &end_loop) {
	VocFileHeader fileHeader;

	debug(2, "loadVOCFromStream");

	if (stream.read(&fileHeader, 8) != 8)
		goto invalid;

	if (!memcmp(&fileHeader, "VTLK", 4)) {
		if (stream.read(&fileHeader, sizeof(VocFileHeader)) != sizeof(VocFileHeader))
			goto invalid;
	} else if (!memcmp(&fileHeader, "Creative", 8)) {
		if (stream.read(((byte *)&fileHeader) + 8, sizeof(VocFileHeader) - 8) != sizeof(VocFileHeader) - 8)
			goto invalid;
	} else {
	invalid:;
		warning("loadVOCFromStream: Invalid header");
		return NULL;
	}

	if (memcmp(fileHeader.desc, "Creative Voice File", 19) != 0)
		error("loadVOCFromStream: Invalid header");
	if (fileHeader.desc[19] != 0x1A)
		debug(3, "loadVOCFromStream: Partially invalid header");

	int32 offset = FROM_LE_16(fileHeader.datablock_offset);
	int16 version = FROM_LE_16(fileHeader.version);
	int16 code = FROM_LE_16(fileHeader.id);
	assert(offset == sizeof(VocFileHeader));
	// 0x100 is an invalid VOC version used by German version of DOTT (Disk) and
	// French version of Simon the Sorcerer 2 (CD)
	assert(version == 0x010A || version == 0x0114 || version == 0x0100);
	assert(code == ~version + 0x1234);

	int len;
	byte *ret_sound = 0;
	size = 0;
	begin_loop = 0;
	end_loop = 0;

	while ((code = stream.readByte())) {
		len = stream.readByte();
		len |= stream.readByte() << 8;
		len |= stream.readByte() << 16;

		debug(2, "Block code %d, len %d", code, len);

		switch (code) {
		case 1:
		case 9: {
			int packing;
			if (code == 1) {
				int time_constant = stream.readByte();
				packing = stream.readByte();
				len -= 2;
				rate = getSampleRateFromVOCRate(time_constant);
			} else {
				rate = stream.readUint32LE();
				int bits = stream.readByte();
				int channels = stream.readByte();
				if (bits != 8 || channels != 1) {
					warning("Unsupported VOC file format (%d bits per sample, %d channels)", bits, channels);
					break;
				}
				packing = stream.readUint16LE();
				stream.readUint32LE();
				len -= 12;
			}
			debug(9, "VOC Data Block: %d, %d, %d", rate, packing, len);
			if (packing == 0) {
				if (size) {
					byte *tmp = (byte *)realloc(ret_sound, size + len);
					if (!tmp)
						error("Cannot reallocate memory for VOC Data Block");

					ret_sound = tmp;
				} else {
					ret_sound = (byte *)malloc(len);
				}
				stream.read(ret_sound + size, len);
				size += len;
				begin_loop = size;
				end_loop = size;
			} else {
				warning("VOC file packing %d unsupported", packing);
			}
			} break;
		case 3: // silence
			// occur with a few Igor sounds, voc file starts with a silence block with a
			// frequency different from the data block. Just ignore fow now (implementing
			// it wouldn't make a big difference anyway...)
			assert(len == 3);
			stream.readUint16LE();
			stream.readByte();
			break;
		case 6:	// begin of loop
			assert(len == 2);
			loops = stream.readUint16LE();
			break;
		case 7:	// end of loop
			assert(len == 0);
			break;
		case 8: { // "Extended"
			// This occures in the LoL Intro demo.
			// This block overwrites the next parameters of a block 1 "Sound data".
			// To assure we never get any bad data here, we will assert in case
			// this tries to define a stereo sound block or tries to use something
			// different than 8bit unsigned sound data.
			// TODO: Actually we would need to check the frequency divisor (the
			// first word) here too. It is used in the following equation:
			// sampleRate = 256000000/(channels * (65536 - frequencyDivisor))
			assert(len == 4);
			stream.readUint16LE();
			uint8 codec = stream.readByte();
			uint8 channels = stream.readByte() + 1;
			assert(codec == 0 && channels == 1);
			} break;
		default:
			warning("Unhandled code %d in VOC file (len %d)", code, len);
			return ret_sound;
		}
	}
	debug(4, "VOC Data Size : %d", size);
	return ret_sound;
}

byte *loadVOCFromStream(Common::ReadStream &stream, int &size, int &rate) {
	int loops, begin_loop, end_loop;
	return loadVOCFromStream(stream, size, rate, loops, begin_loop, end_loop);
}


#ifdef STREAM_AUDIO_FROM_DISK

int parseVOCFormat(Common::SeekableReadStream& stream, RawStreamBlock* block, int &rate, int &loops, int &begin_loop, int &end_loop) {
	VocFileHeader fileHeader;
	int currentBlock = 0;
	int size = 0;

	debug(2, "parseVOCFormat");

	if (stream.read(&fileHeader, 8) != 8)
		goto invalid;

	if (!memcmp(&fileHeader, "VTLK", 4)) {
		if (stream.read(&fileHeader, sizeof(VocFileHeader)) != sizeof(VocFileHeader))
			goto invalid;
	} else if (!memcmp(&fileHeader, "Creative", 8)) {
		if (stream.read(((byte *)&fileHeader) + 8, sizeof(VocFileHeader) - 8) != sizeof(VocFileHeader) - 8)
			goto invalid;
	} else {
	invalid:;
		warning("loadVOCFromStream: Invalid header");
		return 0;
	}

	if (memcmp(fileHeader.desc, "Creative Voice File", 19) != 0)
		error("loadVOCFromStream: Invalid header");
	if (fileHeader.desc[19] != 0x1A)
		debug(3, "loadVOCFromStream: Partially invalid header");

	int32 offset = FROM_LE_16(fileHeader.datablock_offset);
	int16 version = FROM_LE_16(fileHeader.version);
	int16 code = FROM_LE_16(fileHeader.id);
	assert(offset == sizeof(VocFileHeader));
	// 0x100 is an invalid VOC version used by German version of DOTT (Disk) and
	// French version of Simon the Sorcerer 2 (CD)
	assert(version == 0x010A || version == 0x0114 || version == 0x0100);
	assert(code == ~version + 0x1234);

	int len;
	size = 0;
	begin_loop = 0;
	end_loop = 0;

	while ((code = stream.readByte())) {
		len = stream.readByte();
		len |= stream.readByte() << 8;
		len |= stream.readByte() << 16;

		debug(2, "Block code %d, len %d", code, len);

		switch (code) {
		case 1:
		case 9: {
			int packing;
			if (code == 1) {
				int time_constant = stream.readByte();
				packing = stream.readByte();
				len -= 2;
				rate = getSampleRateFromVOCRate(time_constant);
			} else {
				rate = stream.readUint32LE();
				int bits = stream.readByte();
				int channels = stream.readByte();
				if (bits != 8 || channels != 1) {
					warning("Unsupported VOC file format (%d bits per sample, %d channels)", bits, channels);
					break;
				}
				packing = stream.readUint16LE();
				stream.readUint32LE();
				len -= 12;
			}
			debug(9, "VOC Data Block: %d, %d, %d", rate, packing, len);
			if (packing == 0) {

				// Found a data block - so add it to the block list
				block[currentBlock].pos = stream.pos();
				block[currentBlock].len = len;
				currentBlock++;

				stream.seek(len, SEEK_CUR);

				size += len;
				begin_loop = size;
				end_loop = size;
			} else {
				warning("VOC file packing %d unsupported", packing);
			}
			} break;
		case 3: // silence
			// occur with a few Igor sounds, voc file starts with a silence block with a
			// frequency different from the data block. Just ignore fow now (implementing
			// it wouldn't make a big difference anyway...)
			assert(len == 3);
			stream.readUint16LE();
			stream.readByte();
			break;
		case 6:	// begin of loop
			assert(len == 2);
			loops = stream.readUint16LE();
			break;
		case 7:	// end of loop
			assert(len == 0);
			break;
		case 8: // "Extended"
			// This occures in the LoL Intro demo. This block can usually be used to create stereo
			// sound, but the LoL intro has only an empty block, thus this dummy implementation will
			// work.
			assert(len == 4);
			stream.readUint16LE();
			stream.readByte();
			stream.readByte();
			break;
		default:
			warning("Unhandled code %d in VOC file (len %d)", code, len);
			return 0;
		}
	}
	debug(4, "VOC Data Size : %d", size);
	return currentBlock;
}

AudioStream *makeVOCDiskStream(Common::SeekableReadStream *stream, byte flags, DisposeAfterUse::Flag disposeAfterUse) {
	const int MAX_AUDIO_BLOCKS = 256;

	RawStreamBlock *block = new RawStreamBlock[MAX_AUDIO_BLOCKS];
	int rate, loops, begin_loop, end_loop;

	int numBlocks = parseVOCFormat(*stream, block, rate, loops, begin_loop, end_loop);

	AudioStream *audioStream = 0;

	// Create an audiostream from the data. Note the numBlocks may be 0,
	// e.g. when invalid data is encountered. See bug #2890038.
	if (numBlocks)
		audioStream = makeRawDiskStream_OLD(stream, block, numBlocks, rate, flags, disposeAfterUse/*, begin_loop, end_loop*/);

	delete[] block;

	return audioStream;
}

SeekableAudioStream *makeVOCDiskStreamNoLoop(Common::SeekableReadStream *stream, byte flags, DisposeAfterUse::Flag disposeAfterUse) {
	const int MAX_AUDIO_BLOCKS = 256;

	RawStreamBlock *block = new RawStreamBlock[MAX_AUDIO_BLOCKS];
	int rate, loops, begin_loop, end_loop;

	int numBlocks = parseVOCFormat(*stream, block, rate, loops, begin_loop, end_loop);

	SeekableAudioStream *audioStream = 0;

	// Create an audiostream from the data. Note the numBlocks may be 0,
	// e.g. when invalid data is encountered. See bug #2890038.
	if (numBlocks)
		audioStream = makeRawDiskStream_OLD(stream, block, numBlocks, rate, flags, disposeAfterUse);

	delete[] block;

	return audioStream;
}

#endif


AudioStream *makeVOCStream(Common::SeekableReadStream *stream, byte flags, uint loopStart, uint loopEnd, DisposeAfterUse::Flag disposeAfterUse) {
#ifdef STREAM_AUDIO_FROM_DISK
	return makeVOCDiskStream(stream, flags, disposeAfterUse);
#else
	int size, rate;

	byte *data = loadVOCFromStream(*stream, size, rate);

	if (!data) {
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return 0;
	}

	SeekableAudioStream *s = Audio::makeRawStream(data, size, rate, flags);

	if (loopStart != loopEnd) {
		const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
		const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;

		if (loopEnd == 0)
			loopEnd = size;
		assert(loopStart <= loopEnd);
		assert(loopEnd <= (uint)size);

		// Verify the buffer sizes are sane
		if (is16Bit && isStereo)
			assert((loopStart & 3) == 0 && (loopEnd & 3) == 0);
		else if (is16Bit || isStereo)
			assert((loopStart & 1) == 0 && (loopEnd & 1) == 0);

		const uint32 extRate = s->getRate() * (is16Bit ? 2 : 1) * (isStereo ? 2 : 1);

		return new SubLoopingAudioStream(s, 0, Timestamp(0, loopStart, extRate), Timestamp(0, loopEnd, extRate));
	} else {
		return s;
	}
#endif
}

SeekableAudioStream *makeVOCStream(Common::SeekableReadStream *stream, byte flags, DisposeAfterUse::Flag disposeAfterUse) {
#ifdef STREAM_AUDIO_FROM_DISK
	return makeVOCDiskStreamNoLoop(stream, flags, disposeAfterUse);
#else
	int size, rate;

	byte *data = loadVOCFromStream(*stream, size, rate);

	if (!data) {
		if (disposeAfterUse == DisposeAfterUse::YES)
			delete stream;
		return 0;
	}

	return makeRawStream(data, size, rate, flags);
#endif
}

} // End of namespace Audio
