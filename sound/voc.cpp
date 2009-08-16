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
#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"


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
					ret_sound = (byte *)realloc(ret_sound, size + len);
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
			warning("Unhandled code in VOC file : %d", code);
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

int parseVOCFormat(Common::SeekableReadStream& stream, LinearDiskStreamAudioBlock* block, int &rate, int &loops, int &begin_loop, int &end_loop) {
	VocFileHeader fileHeader;
	int currentBlock = 0;
	int size = 0;

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
			warning("Unhandled code in VOC file : %d", code);
			return 0;
		}
	}
	debug(4, "VOC Data Size : %d", size);
	return currentBlock;
}

AudioStream *makeVOCDiskStream(Common::SeekableReadStream &stream, byte flags, bool takeOwnership) {
	const int MAX_AUDIO_BLOCKS = 256;

	LinearDiskStreamAudioBlock *block = new LinearDiskStreamAudioBlock[MAX_AUDIO_BLOCKS];
	int rate, loops, begin_loop, end_loop;

	int numBlocks = parseVOCFormat(stream, block, rate, loops, begin_loop, end_loop);

	AudioStream* audioStream = makeLinearDiskStream(stream, block, numBlocks, rate, flags, takeOwnership, begin_loop, end_loop);

	delete[] block;

	return audioStream;
}
	
#endif


AudioStream *makeVOCStream(Common::SeekableReadStream &stream, byte flags, uint loopStart, uint loopEnd, bool takeOwnershipOfStream) {
#ifdef STREAM_AUDIO_FROM_DISK
	return makeVOCDiskStream(stream, flags, takeOwnershipOfStream);
#else
	int size, rate;

	byte *data = loadVOCFromStream(stream, size, rate);

	if (!data)
		return 0;

	return makeLinearInputStream(data, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE, loopStart, loopEnd);
#endif
}


} // End of namespace Audio
