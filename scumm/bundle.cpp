/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/bundle.h"

namespace Scumm {

#ifdef __PALM_OS__
static const int16 *imcTable;
#else
static const int16 imcTable[] = {
	0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010, 0x0011,
	0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F, 0x0022, 0x0025, 0x0029, 0x002D,
	0x0032, 0x0037, 0x003C, 0x0042, 0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076,
	0x0082, 0x008F, 0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
	0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292, 0x02D4, 0x031C,
	0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583, 0x0610, 0x06AB, 0x0756, 0x0812,
	0x08E0, 0x09C3, 0x0ABD, 0x0BD0, 0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE,
	0x1706, 0x1954, 0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
	0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF
};
#endif

static const byte imxOtherTable[6][128] = {
	{
		0xFF, 0x04, 0xFF, 0x04
	},
	
	{
		0xFF, 0xFF, 0x02, 0x08, 0xFF, 0xFF, 0x02, 0x08
	},
	
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x04, 0x06,
		0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x04, 0x06
	},
	
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x04, 0x06, 0x08, 0x0C, 0x10, 0x20,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x04, 0x06, 0x08, 0x0C, 0x10, 0x20
	},
	
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
		0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x20,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
		0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x20
	},
	
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20
	}
};

const byte imxShortTable[] = {
	0, 0, 1, 3, 7, 15, 31, 63
};


Bundle::Bundle() {
	_compVoiceTable = NULL;
	_compMusicTable = NULL;

	_bundleVoiceTable = NULL;
	_bundleMusicTable = NULL;
	
	_numVoiceFiles = 0;
	_numMusicFiles = 0;

	_lastSong = -1;

	initializeImcTables();
}

Bundle::~Bundle() {
	free(_bundleVoiceTable);
	free(_bundleMusicTable);
	free(_compVoiceTable);
	free(_compMusicTable);
}

void Bundle::initializeImcTables() {
	int32 destTablePos = 0;
	int32 imcTable1Pos = 0;
	do {
		byte put = 1;
		int32 tableValue = ((imcTable[imcTable1Pos] << 2) / 7) >> 1;
		if (tableValue != 0) {
			do {
				tableValue >>= 1;
				put++;
			} while (tableValue != 0);
		}
		if (put < 3) {
			put = 3;
		}
		if (put > 8) {
			put = 8;
		}
		put--;
		_destImcTable[destTablePos] = put;
		destTablePos++;
	} while (++imcTable1Pos <= 88);
	_destImcTable[89] = 0;

	for (int n = 0; n < 64; n++) {
		imcTable1Pos = 0;
		destTablePos = n;
		do {
			int32 count = 32;
			int32 put = 0;
			int32 tableValue = imcTable[imcTable1Pos];
	 		do {
				if ((count & n) != 0) {
					put += tableValue;
				}
				count >>= 1;
				tableValue >>= 1;
			} while (count != 0);
			_destImcTable2[destTablePos] = put;
			destTablePos += 64;
		} while (++imcTable1Pos <= 88);
	}
}

bool Bundle::openVoiceFile(const char *filename, const char *directory) {
	int32 tag, offset;

	if (_voiceFile.isOpen())
		return true;

	if (_voiceFile.open(filename, directory) == false) {
		warning("Bundle: Can't open voice bundle file: %s", filename);
		return false;
	}

	tag = _voiceFile.readUint32BE();
	offset = _voiceFile.readUint32BE();
	_numVoiceFiles = _voiceFile.readUint32BE();

	_bundleVoiceTable = (BundleAudioTable *) malloc(_numVoiceFiles * sizeof(BundleAudioTable));

	_voiceFile.seek(offset, SEEK_SET);

	for (int32 i = 0; i < _numVoiceFiles; i++) {
		char name[13], c;
		int32 z = 0;
		int32 z2;

		for (z2 = 0; z2 < 8; z2++)
			if ((c = _voiceFile.readByte()) != 0)
				name[z++] = c;
		name[z++] = '.';
		for (z2 = 0; z2 < 4; z2++)
			if ((c = _voiceFile.readByte()) != 0)
				name[z++] = c;
		name[z] = '\0';
		strcpy(_bundleVoiceTable[i].filename, name);
		_bundleVoiceTable[i].offset = _voiceFile.readUint32BE();
		_bundleVoiceTable[i].size = _voiceFile.readUint32BE();
	}

	return true;
}

void Bundle::closeVoiceFile() {
	if (_voiceFile.isOpen()) {
		_voiceFile.close();
		free(_bundleVoiceTable);
		_bundleVoiceTable = NULL;
	}
}

bool Bundle::openMusicFile(const char *filename, const char *directory) {
	int32 tag, offset;

	if (_musicFile.isOpen())
		return true;

	if (_musicFile.open(filename, directory) == false) {
		warning("Bundle: Can't open music bundle file: %s", filename);
		return false;
	}

	tag = _musicFile.readUint32BE();
	offset = _musicFile.readUint32BE();
	_numMusicFiles = _musicFile.readUint32BE();

	_bundleMusicTable = (BundleAudioTable *) malloc(_numMusicFiles * sizeof(BundleAudioTable));

	_musicFile.seek(offset, SEEK_SET);

	for (int32 i = 0; i < _numMusicFiles; i++) {
		char name[13], c;
		int z = 0;
		int z2;

		for (z2 = 0; z2 < 8; z2++)
			if ((c = _musicFile.readByte()) != 0)
				name[z++] = c;
		name[z++] = '.';
		for (z2 = 0; z2 < 4; z2++)
			if ((c = _musicFile.readByte()) != 0)
				name[z++] = c;
		name[z] = '\0';
		strcpy(_bundleMusicTable[i].filename, name);
		_bundleMusicTable[i].offset = _musicFile.readUint32BE();
		_bundleMusicTable[i].size = _musicFile.readUint32BE();
	}

	return true;
}

void Bundle::closeMusicFile() {
	if (_musicFile.isOpen()) {
		_musicFile.close();
		free(_bundleMusicTable);
		_bundleMusicTable = NULL;
		_lastSong = -1;
	}
}


int32 Bundle::decompressVoiceSampleByIndex(int32 index, byte **comp_final) {
	int32 i, tag, num, final_size, output_size;
	byte *comp_input, *comp_output;

	if (_voiceFile.isOpen() == false) {
		warning("Bundle: voice file is not open!");
		return 0;
	}

	_voiceFile.seek(_bundleVoiceTable[index].offset, SEEK_SET);
	tag = _voiceFile.readUint32BE();
	num = _voiceFile.readUint32BE();
	_voiceFile.readUint32BE();
	_voiceFile.readUint32BE();

	if (tag != MKID_BE('COMP')) {
		warning("Bundle: Compressed sound %d invalid (%c%c%c%c)", index, tag >> 24, tag >> 16, tag >> 8, tag);
		return 0;
	}

	free(_compVoiceTable);
	_compVoiceTable = (CompTable *)malloc(sizeof(CompTable) * num);
	for (i = 0; i < num; i++) {
		_compVoiceTable[i].offset = _voiceFile.readUint32BE();
		_compVoiceTable[i].size = _voiceFile.readUint32BE();
		_compVoiceTable[i].codec = _voiceFile.readUint32BE();
		_voiceFile.readUint32BE();
	}

	final_size = 0;

	comp_output = (byte *)malloc(0x2000);

	*comp_final = (byte *)malloc(0x2000 * num);

	for (i = 0; i < num; i++) {
		// CMI hack: one more zero byte at the end of input buffer
		comp_input = (byte *)malloc(_compVoiceTable[i].size + 1);
		comp_input[_compVoiceTable[i].size] = 0;

		_voiceFile.seek(_bundleVoiceTable[index].offset + _compVoiceTable[i].offset, SEEK_SET);
		_voiceFile.read(comp_input, _compVoiceTable[i].size);

		output_size = decompressCodec(_compVoiceTable[i].codec, comp_input, comp_output, _compVoiceTable[i].size);
		assert(output_size <= 0x2000);
		memcpy(*comp_final + final_size, comp_output, output_size);
		final_size += output_size;

		free(comp_input);
	}
	free(comp_output);

	return final_size;
}

int32 Bundle::decompressMusicSampleByIndex(int32 index, int32 number, byte *comp_final) {
	int32 i = 0;
	int tag, num, final_size;
	byte *comp_input;

	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	if (_lastSong != index) {
		_musicFile.seek(_bundleMusicTable[index].offset, SEEK_SET);
		tag = _musicFile.readUint32BE();
		num = _musicFile.readUint32BE();
		_musicFile.readUint32BE();
		_musicFile.readUint32BE();

		if (tag != MKID_BE('COMP')) {
			warning("Bundle: Compressed sound %d invalid (%c%c%c%c)", index, tag >> 24, tag >> 16, tag >> 8, tag);
			return 0;
		}

		free(_compMusicTable);
		_compMusicTable = (CompTable *)malloc(sizeof(CompTable) * num);

		for (i = 0; i < num; i++) {
			_compMusicTable[i].offset = _musicFile.readUint32BE();
			_compMusicTable[i].size = _musicFile.readUint32BE();
			_compMusicTable[i].codec = _musicFile.readUint32BE();
			_musicFile.readUint32BE();
		}

		_lastSong = index;
	}

	// CMI hack: one more zero byte at the end of input buffer
	comp_input = (byte *)malloc(_compMusicTable[number].size + 1);
	comp_input[_compMusicTable[number].size] = 0;

	_musicFile.seek(_bundleMusicTable[index].offset + _compMusicTable[number].offset, SEEK_SET);
	_musicFile.read(comp_input, _compMusicTable[number].size);

	final_size = decompressCodec(_compMusicTable[number].codec, comp_input, comp_final, _compMusicTable[number].size);

	free(comp_input);

	return final_size;
}

int32 Bundle::decompressVoiceSampleByName(const char *name, byte **comp_final) {
	int32 final_size = 0, i;

	if (_voiceFile.isOpen() == false) {
		warning("Bundle: voice file is not open!");
		return 0;
	}

	for (i = 0; i < _numVoiceFiles; i++) {
		if (!scumm_stricmp(name, _bundleVoiceTable[i].filename)) {
			final_size = decompressVoiceSampleByIndex(i, comp_final);
			return final_size;
		}
	}
	debug(2, "Failed finding voice %s", name);
	return final_size;
}

int32 Bundle::decompressMusicSampleByName(const char *name, int32 number, byte *comp_final) {
	int32 final_size = 0, i;

	if (!name) {
		warning("Bundle: decompressMusicSampleByName called with no name!");
		return 0;
	}

	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	for (i = 0; i < _numMusicFiles; i++) {
		if (!scumm_stricmp(name, _bundleMusicTable[i].filename)) {
			final_size = decompressMusicSampleByIndex(i, number, comp_final);
			return final_size;
		}
	}
	warning("Couldn't find sample %s", name);
	return final_size;
}

int32 Bundle::getNumberOfMusicSamplesByIndex(int32 index) {
	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	_musicFile.seek(_bundleMusicTable[index].offset, SEEK_SET);
	_musicFile.readUint32BE();
	return _musicFile.readUint32BE();
}

int32 Bundle::getNumberOfMusicSamplesByName(const char *name) {
	int32 number = 0, i;

	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	for (i = 0; i < _numMusicFiles; i++) {
		if (!scumm_stricmp(name, _bundleMusicTable[i].filename)) {
			number = getNumberOfMusicSamplesByIndex(i);
			return number;
		}
	}
	warning("Couldn't find numsample %s", name);
	return number;
}

#define NextBit bit = mask & 1; mask >>= 1;				\
								if (!--bitsleft) {								\
									mask = READ_LE_UINT16(srcptr);	\
									srcptr += 2;										\
									bitsleft = 16;									\
								}

static int32 compDecode(byte *src, byte *dst) {
	byte *result, *srcptr = src, *dstptr = dst;
	int data, size, bit, bitsleft = 16, mask = READ_LE_UINT16(srcptr);
	srcptr += 2;

	while (1) {
		NextBit if (bit) {
			*dstptr++ = *srcptr++;
		} else {
			NextBit if (!bit) {
				NextBit size = bit << 1;
				NextBit size = (size | bit) + 3;
				data = *srcptr++ | 0xffffff00;
			} else {
				data = *srcptr++;
				size = *srcptr++;

				data |= 0xfffff000 + ((size & 0xf0) << 4);
				size = (size & 0x0f) + 3;

				if (size == 3)
					if (((*srcptr++) + 1) == 1)
						return dstptr - dst;
			}
			result = dstptr + data;
			while (size--)
				*dstptr++ = *result++;
		}
	}
}
#undef NextBit

int32 Bundle::decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size) {
	int32 output_size, channels;
	int32 offset1, offset2, offset3, length, k, c, s, j, r, t, z;
	byte *src, *t_table, *p, *ptr;
	byte t_tmp1, t_tmp2;

	switch (codec) {
	case 0:
		memcpy(comp_output, comp_input, input_size);
		output_size = input_size;
		break;

	case 1:
		output_size = compDecode(comp_input, comp_output);
		break;

	case 2:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];
		break;

	case 3:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];
		break;

	case 4:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memset(t_table, 0, output_size);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		if (length > 0) {
			c = -12;
			s = 0;
			j = 0;
			do {
				ptr = src + length + (k >> 1);
				if (k & 1) {
					r = c >> 3;
					t_table[r + 2] = ((src[j] & 0x0f) << 4) | (ptr[1] >> 4);
					t_table[r + 1] = (src[j] & 0xf0) | (t_table[r + 1]);
				} else {
					r = s >> 3;
					t_table[r + 0] = ((src[j] & 0x0f) << 4) | (ptr[0] & 0x0f);
					t_table[r + 1] = src[j] >> 4;
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) / 2;
		t_table[offset1 + 1] = (t_table[offset1 + 1]) | (src[length - 1] & 0xf0);
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 5:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memset(t_table, 0, output_size);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 1;
		c = 0;
		s = 12;
		t_table[0] = src[length] / 16;
		t = length + k;
		j = 1;
		if (t > k) {
			do {
				ptr = src + length + (k >> 1);
				if (k & 1) {
					r = c >> 3;
					t_table[r + 0] = (src[j - 1] & 0xf0) | t_table[r];
					t_table[r + 1] = ((src[j - 1] & 0x0f) << 4) | (ptr[0] & 0x0f);
				} else {
					r = s >> 3;
					t_table[r + 0] = src[j - 1] >> 4;
					t_table[r - 1] = ((src[j - 1] & 0x0f) << 4) | (ptr[0] >> 4);
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < t);
		}
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 6:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memset(t_table, 0, output_size);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		c = 0;
		j = 0;
		s = -12;
		t_table[0] = src[output_size - 1];
		t_table[output_size - 1] = src[length - 1];
		t = length - 1;
		if (t > 0) {
			do {
				ptr = src + length + (k >> 1);
				if (k & 1) {
					r = s >> 3;
					t_table[r + 2] = (src[j] & 0xf0) | *(t_table + r + 2);
					t_table[r + 3] = ((src[j] & 0x0f) << 4) | (ptr[0] >> 4);
				} else {
					r = c >> 3;
					t_table[r + 2] = src[j] >> 4;
					t_table[r + 1] = ((src[j] & 0x0f) << 4) | (ptr[0] & 0x0f);
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < t);
		}
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 10:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 * 2;
		offset3 = offset2;
		src = comp_output;
		do {
			if (offset1 == 0)
				break;
			offset1--;
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		if (length > 0) {
			c = -12;
			s = 0;
			do {
				j = length + (k >> 1);
				if (k & 1) {
					r = c >> 3;
					t_tmp1 = t_table[k];
					t_tmp2 = t_table[j + 1];
					src[r + 2] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
					src[r + 1] = (src[r + 1]) | (t_tmp1 & 0xf0);
				} else {
					r = s >> 3;
					t_tmp1 = t_table[k];
					t_tmp2 = t_table[j];
					src[r + 0] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
					src[r + 1] = t_tmp1 >> 4;
				}
				s += 12;
				c += 12;
				k++;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) / 2;
		src[offset1 + 1] = (t_table[length] & 0xf0) | src[offset1 + 1];
		free(t_table);
		break;

	case 11:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 * 2;
		offset3 = offset2;
		src = comp_output;
		do {
			if (offset1 == 0)
				break;
			offset1--;
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 1;
		c = 0;
		s = 12;
		t_tmp1 = t_table[length] / 16;
		src[0] = t_tmp1;
		t = length + k;
		if (t > k) {
			do {
				j = length + (k / 2);
				if (k & 1) {
					r = c >> 3;
					t_tmp1 = t_table[k - 1];
					t_tmp2 = t_table[j];
					src[r + 0] = (src[r]) | (t_tmp1 & 0xf0);
					src[r + 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				} else {
					r = s >> 3;
					t_tmp1 = t_table[k - 1];
					t_tmp2 = t_table[j];
					src[r + 0] = t_tmp1 >> 4;
					src[r - 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
				}
				s += 12;
				c += 12;
				k++;
			} while (k < t);
		}
		free(t_table);
		break;

	case 12:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 * 2;
		offset3 = offset2;
		src = comp_output;
		do {
			if (offset1 == 0)
				break;
			offset1--;
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		c = 0;
		s = -12;
		src[0] = t_table[output_size - 1];
		src[output_size - 1] = t_table[length - 1];
		t = length - 1;
		if (t > 0) {
			do {
				j = length + (k >> 1);
				if (k & 1) {
					r = s >> 3;
					t_tmp1 = t_table[k];
					t_tmp2 = t_table[j];
					src[r + 2] = (src[r + 2]) | (t_tmp1 & 0xf0);
					src[r + 3] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
				} else {
					r = c >> 3;
					t_tmp1 = t_table[k];
					t_tmp2 = t_table[j];
					src[r + 2] = t_tmp1 >> 4;
					src[r + 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				}
				s += 12;
				c += 12;
				k++;
			} while (k < t);
		}
		free(t_table);
		break;

	case 13:
	case 15:
		if (codec == 13) {
			channels = 1;
		} else {
			channels = 2;
		}

		{
			const int MAX_CHANNELS = 2;
			int32 left, startPos, origLeft, curTableEntry, destPos, esiReg;
			int16 firstWord;
			byte sByte[MAX_CHANNELS] = {0, 0};
			int32 sDWord1[MAX_CHANNELS] = {0, 0};
			int32 sDWord2[MAX_CHANNELS] = {0, 0};
			int32 tableEntrySum, imcTableEntry, curTablePos, outputWord, adder;
			byte decompTable, otherTablePos, bitMask;
			byte *readPos, *dst;
			uint16 readWord;
			
			assert(0 <= channels && channels <= MAX_CHANNELS);

			src = comp_input;
			dst = comp_output;
			if (channels == 2) {
				output_size = left = 0x2000;
			} else {
				left = 0x1000;
				output_size = 0x2000;
			}
			firstWord = READ_BE_UINT16(src);
			src += 2;
			if (firstWord != 0) {
				memcpy(dst, src, firstWord);
				dst += firstWord;
				src += firstWord;
				startPos = 0;
				if (channels == 2) {
					left = 0x2000 - firstWord;
					output_size = left;
				} else {
					left = 0x1000 - (firstWord >> 1);
					output_size = left << 1;
				}
			} else {
				startPos = 1;
				for (int i = 0; i < channels; i++) {
					sByte[i] = *(src++);
					sDWord1[i] = READ_BE_UINT32(src);
					src += 4;
					sDWord2[i] = READ_BE_UINT32(src);
					src += 4;
				}
			}

			origLeft = left >> (channels - 1);
			tableEntrySum = 0;
			for (int l = 0; l < channels; l++) {
				if (startPos != 0) {
					curTablePos = sByte[l];
					imcTableEntry = sDWord1[l];
					outputWord = sDWord2[l];
				} else {
					curTablePos = 0;
					imcTableEntry = 7;
					outputWord = 0;
				}

				left = origLeft;
				destPos = l * 2;

				if (channels == 2) {
					if (l == 0)
						left++;
					left >>= 1;
				}

				while (left--) {
					curTableEntry = _destImcTable[curTablePos];
					decompTable = (byte)(curTableEntry - 2);
					bitMask = 2 << decompTable;
					readPos = src + (tableEntrySum >> 3);
					
					// FIXME - it seems the decoder often reads exactly one byte too
					// far - that is, it reads 2 bytes at once, and the second byte
					// is just outside the buffer. However, it seems of these two bytes,
					// only the upper one is actually used, so this should be fine.
					// Still, I put this error message into place. If somebody one day
					// encounters a situation where the second byte would be used, too,
					// then this would indicate there is a bug in the decoder...
					if (readPos + 1 >= comp_input + input_size) {
						// OK an overflow... if it is more than one byte or if we
						// need more than 8 bit of data -> error
						if (readPos + 1 > comp_input + input_size ||
						    curTableEntry + (tableEntrySum & 7) > 8) {
							error("decompressCodec: input buffer overflow: %d bytes over (we need %d bits of data)",
									(int)((readPos+1) - (comp_input+input_size))+1,
									curTableEntry + (tableEntrySum & 7)
								);
						}
					}
					readWord = (uint16)(READ_BE_UINT16(readPos) << (tableEntrySum & 7));
					otherTablePos = (byte)(readWord >> (16 - curTableEntry));
					tableEntrySum += curTableEntry;
					esiReg = ((imxShortTable[curTableEntry] & otherTablePos)
						<< (7 - curTableEntry)) + (curTablePos << 6);
					imcTableEntry >>= (curTableEntry - 1);
					adder = imcTableEntry + _destImcTable2[esiReg];
					if ((otherTablePos & bitMask) != 0) {
						adder = -adder;
					}
					outputWord += adder;

					// Clip outputWord to 16 bit signed, and write it into the destination stream
					if (outputWord > 0x7fff)
						outputWord = 0x7fff;
					if (outputWord < -0x8000)
						outputWord = -0x8000;
					dst[destPos] = ((int16)outputWord) >> 8;
					dst[destPos + 1] = (byte)(outputWord);

					// Adjust the curTablePos / imcTableEntry
					assert(decompTable < 6);
					curTablePos += (signed char)imxOtherTable[decompTable][otherTablePos];
					if (curTablePos > 88)
						curTablePos = 88;
					if (curTablePos < 0)
						curTablePos = 0;
					imcTableEntry = imcTable[curTablePos];

					destPos += channels << 1;
				}
			}
		}
		break;

	default:
		warning("Bundle: Unknown codec %d!", (int)codec);
		output_size = 0;
		break;
	}

	return output_size;
}

} // End of namespace Scumm

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Bundle)
_GSETPTR(Scumm::imcTable, GBVARS_IMCTABLE_INDEX, int16, GBVARS_SCUMM)
_GEND

_GRELEASE(Bundle)
_GRELEASEPTR(GBVARS_IMCTABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif


