/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "common/file.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"


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

byte *readVOCFromMemory(byte *ptr, int &size, int &rate, int &loops, int &begin_loop, int &end_loop) {
	
	// Verify the VOC header. We are a little bit lenient here to work around
	// some invalid VOC headers used in various SCUMM games (they have 0x0
	// instead of 0x1A after the "Creative Voice File" string).
	if (memcmp(ptr, "Creative Voice File", 19) != 0)
		error("readVOCFromMemory: Invalid header");
	if (ptr[19] != 0x1A)
		debug(3, "readVOCFromMemory: Partially invalid header");

	int32 offset = READ_LE_UINT16(ptr + 20);
	int16 version = READ_LE_UINT16(ptr + 22);
	int16 code = READ_LE_UINT16(ptr + 24);
	// 0x100 is an invalid VOC version used by German version of DOTT (Disk) and
	// French version of Simon the Sorcerer 2 (CD)
	assert(version == 0x010A || version == 0x0114 || version == 0x0100);
	assert(code == ~version + 0x1234);
	
	int len;
	byte *ret_sound = 0;
	size = 0;
	begin_loop = 0;
	end_loop = 0;
	
	ptr += offset;
	while ((code = *ptr++)) {
		len = *ptr++;
		len |= *ptr++ << 8;
		len |= *ptr++ << 16;

		switch(code) {
		case 1: {
			int time_constant = *ptr++;
			int packing = *ptr++;
			len -= 2;
			rate = getSampleRateFromVOCRate(time_constant);
			debug(9, "VOC Data Block: %d, %d, %d", rate, packing, len);
			if (packing == 0) {
				if (size) {
					ret_sound = (byte *)realloc(ret_sound, size + len);
				} else {
					ret_sound = (byte *)malloc(len);
				}
				memcpy(ret_sound + size, ptr, len);
				begin_loop = size;
				size += len;
				end_loop = size;
			} else {
				warning("VOC file packing %d unsupported", packing);
			}
			} break;
		case 6:	// begin of loop
			loops = (uint16)READ_LE_UINT16(ptr);
			break;
		case 7:	// end of loop
			break;
		default:
			warning("Invalid code in VOC file : %d", code);
			return ret_sound;
		}
		// FIXME some FT samples (ex. 362) has bad length, 2 bytes too short
		ptr += len;
	}
	debug(4, "VOC Data Size : %d", size);
	return ret_sound;
}

// FIXME/TODO: loadVOCFile() essentially duplicates all the code from
// readCreativeVoc(). Obviously this is bad, they should share as much
// code as possible. One way to do that would be to abstract the
// reading from memory / from file into a stream class (similar to the
// RWOps of SDL, for example).
byte *loadVOCFile(File *file, int &size, int &rate) {
	VocFileHeader fileHeader;

	if (file->read(&fileHeader, 8) != 8)
		goto invalid;

	if (!memcmp(&fileHeader, "VTLK", 4)) {
		if (file->read(&fileHeader, sizeof(VocFileHeader)) != sizeof(VocFileHeader))
			goto invalid;
	} else if (!memcmp(&fileHeader, "Creative", 8)) {
		if (file->read(((byte *)&fileHeader) + 8, sizeof(VocFileHeader) - 8) != sizeof(VocFileHeader) - 8)
			goto invalid;
	} else {
	invalid:;
		warning("loadVOCFile: Invalid header");
		return NULL;
	}

	if (memcmp(fileHeader.desc, "Creative Voice File", 19) != 0)
		error("loadVOCFile: Invalid header");
	if (fileHeader.desc[19] != 0x1A)
		debug(3, "loadVOCFile: Partially invalid header");

	//int32 offset = FROM_LE_16(fileHeader.datablock_offset);
	int16 version = FROM_LE_16(fileHeader.version);
	int16 code = FROM_LE_16(fileHeader.id);
	assert(version == 0x010A || version == 0x0114);
	assert(code == ~version + 0x1234);

	int len;
	byte *ret_sound = 0;
	size = 0;

	while ((code = file->readByte())) {
		len = file->readByte();
		len |= file->readByte() << 8;
		len |= file->readByte() << 16;

		switch(code) {
		case 1: {
			int time_constant = file->readByte();
			int packing = file->readByte();
			len -= 2;
			rate = getSampleRateFromVOCRate(time_constant);
			debug(9, "VOC Data Block: %d, %d, %d", rate, packing, len);
			if (packing == 0) {
				if (size) {
					ret_sound = (byte *)realloc(ret_sound, size + len);
				} else {
					ret_sound = (byte *)malloc(len);
				}
				file->read(ret_sound + size, len);
				size += len;
			} else {
				warning("VOC file packing %d unsupported", packing);
			}
			} break;
		default:
			warning("Invalid code in VOC file : %d", code);
			return ret_sound;
		}
	}
	debug(4, "VOC Data Size : %d", size);
	return ret_sound;
}

AudioStream *makeVOCStream(byte *ptr) {
	int size, rate, loops, begin_loop, end_loop;
	byte *data = readVOCFromMemory(ptr, size, rate, loops, begin_loop, end_loop);

	if (!data)
		return 0;

	return makeLinearInputStream(rate, SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_UNSIGNED, data, size, 0, 0);
}

AudioStream *makeVOCStream(File *file) {
	int size, rate;
	byte *data = loadVOCFile(file, size, rate);

	if (!data)
		return 0;

	return makeLinearInputStream(rate, SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_UNSIGNED, data, size, 0, 0);
}

