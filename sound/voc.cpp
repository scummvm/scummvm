/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "sound/voc.h"


byte *readCreativeVoc(byte *ptr, int32 &size, int &rate, int32 &loops) {
	
	assert(strncmp((char *)ptr, "Creative Voice File\x1A", 20) == 0);
	int32 offset = READ_LE_UINT16(ptr + 20);
	int16 version = READ_LE_UINT16(ptr + 22);
	int16 code = READ_LE_UINT16(ptr + 24);
	assert(version == 0x010A || version == 0x0114);
	assert(code == ~version + 0x1234);
	
	bool quit = 0;
	byte *ret_sound = 0;
	size = 0;

	while (!quit) {
		int len = READ_LE_UINT32(ptr + offset);
		offset += 4;
		code = len & 0xFF;
		len >>= 8;
		switch(code) {
		case 0: quit = 1;
			break;
		case 1: {
			int time_constant = ptr[offset++];
			int packing = ptr[offset++];
			len -= 2;
			rate = getSampleRateFromVOCRate(time_constant);
			debug(9, "VOC Data Bloc : %d, %d, %d", rate, packing, len);
			if (packing == 0) {
				if (size) {
					ret_sound = (byte *)realloc(ret_sound, size + len);
				} else {
					ret_sound = (byte *)malloc(len);
				}
				memcpy(ret_sound + size, ptr + offset, len);
				size += len;
			} else {
				warning("VOC file packing %d unsupported", packing);
			}
			} break;
		case 6:	// begin of loop
			loops = (uint16)READ_LE_UINT16(ptr + offset);
			warning("voc loops: %d", loops);
			break;
		case 7:	// end of loop
			break;
		default:
			warning("Invalid code in VOC file : %d", code);
			quit = 1;
			break;
		}
		// FIXME some FT samples (ex. 362) has bad length, 2 bytes too short
		offset += len;
	}
	debug(9, "VOC Data Size : %d", size);
	return ret_sound;
}

enum {
	SOUND_HEADER_SIZE = 26,
	SOUND_HEADER_BIG_SIZE = 26 + 8
};

// FIXME/TODO: loadVocSample() essentially duplicates all the code from
// readCreativeVoc(). Obviously this is bad, it should rather use that function
// (after some tweaks to readCreativeVoc, to deal with the alternate VTLK
// header).
byte *loadVocSample(File *file, int32 &size, int &rate) {
	char ident[8];

	if (file->read(ident, 8) != 8)
		goto invalid;

	if (!memcmp(ident, "VTLK", 4)) {
		file->seek(SOUND_HEADER_BIG_SIZE - 8, SEEK_CUR);
	} else if (!memcmp(ident, "Creative", 8)) {
		file->seek(SOUND_HEADER_SIZE - 8, SEEK_CUR);
	} else {
	invalid:;
		warning("loadVocSample: invalid header");
		return NULL;
	}

	VocBlockHeader voc_block_hdr;

	file->read(&voc_block_hdr, sizeof(voc_block_hdr));
	if (voc_block_hdr.blocktype != 1) {
		warning("loadVocSample: Expecting block_type == 1, got %d", voc_block_hdr.blocktype);
		return NULL;
	}

	size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
	rate = getSampleRateFromVOCRate(voc_block_hdr.sr);
	int comp = voc_block_hdr.pack;

	if (comp != 0) {
		warning("loadVocSample: Unsupported compression type %d", comp);
		return NULL;
	}

	byte *data = (byte *)malloc(size);
	if (data == NULL) {
		error("loadVocSample: out of memory");
	}

	if ((int)file->read(data, size) != size) {
		/* no need to free the memory since error will shut down */
		error("startSfxSound: cannot read %d bytes", size);
	}

	return data;
}

int getSampleRateFromVOCRate(int vocSR) {
	if (vocSR == 0xa5 || vocSR == 0xa6 || vocSR == 0x83) {
		return 11025;
	} else if (vocSR == 0xd2 || vocSR == 0xd3) {
		return 22050;
	} else {
		int sr = 1000000L / (256L - vocSR);
		warning("inexact sample rate used: %i (0x%x)", sr, vocSR);
		return sr;
	}
}
