/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "scumm.h"
#include "scummsys.h"
#include "bundle.h"
#include "file.h"

uint16 imcTable1[] = {
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

byte imxOtherTable1[] = {
	0xFF, 0x04, 0xFF, 0x04
};

byte imxOtherTable2[] = {
	0xFF, 0xFF, 0x02, 0x08, 0xFF, 0xFF, 0x02, 0x08
};

byte imxOtherTable3[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x04, 0x06,
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x04, 0x06
};

byte imxOtherTable4[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x01, 0x02, 0x04, 0x06, 0x08, 0x0C, 0x10, 0x20,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x01, 0x02, 0x04, 0x06, 0x08, 0x0C, 0x10, 0x20
};

byte imxOtherTable5[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x20,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x20
};

byte imxOtherTable6[] = {
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
};

uint16 imxShortTable[] = {
	0x0000, 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F
};

Bundle::Bundle()
{
	_lastSong = -1;
	_initializedImcTables = false;

	_bundleVoiceTable = NULL;
	_bundleMusicTable = NULL;
}

Bundle::~Bundle()
{
 if (_bundleVoiceTable)
	free(_bundleVoiceTable);
 if (_bundleMusicTable)
	free(_bundleMusicTable);
}

void Bundle::initializeImcTables()
{
	if (_initializedImcTables == true)
		return;

	int32 destTablePos = 0;
	int32 imcTable1Pos = 0;
	do {
		int16 put = 1;
		int32 tableValue = ((imcTable1[imcTable1Pos] << 2) / 7) / 2;
		if (tableValue != 0) {
			do {
				tableValue /= 2;
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
		imcTable1Pos++;
		_destImcTable[destTablePos] = (byte)put;
		destTablePos++;
	} while (imcTable1Pos <= 88);
	_destImcTable[89] = 0;

	for (int n = 0; n < 63; n++) {
		imcTable1Pos = 0;
		int32 destTable2Pos = n;
		do {
			int32 count = 32;
			int32 putD = 0;
			int32 tableValue = imcTable1[imcTable1Pos];
	 		do {
				if ((count & n) != 0) {
					putD = putD + tableValue;
				}
				count >>= 1;
				tableValue >>= 1;
			} while (count != 0);
			_destImcTable2[destTable2Pos] = putD;
			destTable2Pos += 64;
			imcTable1Pos++;
		} while (imcTable1Pos <= 88);
	}

	_initializedImcTables = true;
}

bool Bundle::openVoiceFile(const char *filename, const char *directory)
{
	int32 tag, offset;

	initializeImcTables();

	if (_voiceFile.isOpen() == true)
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

bool Bundle::openMusicFile(const char *filename, const char *directory)
{
	int32 tag, offset;

	initializeImcTables();

	if (_musicFile.isOpen() == true)
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

int32 Bundle::decompressVoiceSampleByIndex(int32 index, byte *comp_final, int32 & channels)
{
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
		warning("Bundle: Compressed sound %d invalid (%c%c%c%c)", index, tag >> 24, tag >> 16, tag >> 8,
						tag);
		return 0;
	}

	if (_compVoiceTable)
		free(_compVoiceTable);

	_compVoiceTable = (CompTable*)malloc(sizeof(CompTable) * num);
	for (i = 0; i < num; i++) {
		_compVoiceTable[i].offset = _voiceFile.readUint32BE();
		_compVoiceTable[i].size = _voiceFile.readUint32BE();
		_compVoiceTable[i].codec = _voiceFile.readUint32BE();
		_voiceFile.readUint32BE();
	}

	final_size = 0;

	comp_output = (byte *)malloc(10000);

	for (i = 0; i < num; i++) {
		comp_input = (byte *)malloc(_compVoiceTable[i].size);

		_voiceFile.seek(_bundleVoiceTable[index].offset + _compVoiceTable[i].offset, SEEK_SET);
		_voiceFile.read(comp_input, _compVoiceTable[i].size);

		output_size =
			decompressCodec(_compVoiceTable[i].codec, comp_input, comp_output, _compVoiceTable[i].size, i, channels);
		assert(output_size <= 10000);
		memcpy(comp_final + final_size, comp_output, output_size);
		final_size += output_size;

		free(comp_input);
	}
	free(comp_output);

	return final_size;
}

int32 Bundle::decompressMusicSampleByIndex(int32 index, int32 number, byte *comp_final, int32 & channels)
{
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
			warning("Bundle: Compressed sound %d invalid (%c%c%c%c)", index, tag >> 24, tag >> 16, tag >> 8,
							tag);
			return 0;
		}

		if (_compMusicTable)
			free(_compMusicTable);
		_compMusicTable = (CompTable*)malloc(sizeof(CompTable) * num);

		for (i = 0; i < num; i++) {
			_compMusicTable[i].offset = _musicFile.readUint32BE();
			_compMusicTable[i].size = _musicFile.readUint32BE();
			_compMusicTable[i].codec = _musicFile.readUint32BE();
			_musicFile.readUint32BE();
		}
	}

	comp_input = (byte *)malloc(_compMusicTable[number].size);

	_musicFile.seek(_bundleMusicTable[index].offset + _compMusicTable[number].offset, SEEK_SET);
	_musicFile.read(comp_input, _compMusicTable[number].size);

	// FIXME - if _lastSong == index then i will be 0 - is the right / the desired behaviour?!?
	final_size =
		decompressCodec(_compMusicTable[number].codec, comp_input, comp_final, _compMusicTable[number].size, number, channels);

	free(comp_input);

	_lastSong = index;

	return final_size;
}

int32 Bundle::decompressVoiceSampleByName(char *name, byte *comp_final, int32 & channels)
{
	int32 final_size = 0, i;

	if (_voiceFile.isOpen() == false) {
		warning("Bundle: voice file is not open!");
		return 0;
	}

	for (i = 0; i < _numVoiceFiles; i++) {
		if (!scumm_stricmp(name, _bundleVoiceTable[i].filename)) {
			final_size = decompressVoiceSampleByIndex(i, comp_final, channels);
			return final_size;
		}
	}
	warning("Failed finding voice %s", name);
	return final_size;
}

int32 Bundle::decompressMusicSampleByName(char *name, int32 number, byte *comp_final, int32 & channels) {
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
			final_size = decompressMusicSampleByIndex(i, number, comp_final, channels);
			return final_size;
		}
	}
	warning("Couldn't find sample %s", name);
	return final_size;
}

int32 Bundle::getNumberOfMusicSamplesByIndex(int32 index)
{
	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	_musicFile.seek(_bundleMusicTable[index].offset, SEEK_SET);
	_musicFile.readUint32BE();
	return _musicFile.readUint32BE();
}

int32 Bundle::getNumberOfMusicSamplesByName(char *name)
{
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
	warning("Couldn't find numsample %s\n", name);
	return number;
}

#define NextBit bit = mask & 1; mask >>= 1; if (!--bitsleft) {mask = READ_LE_UINT16(srcptr); srcptr += 2; bitsleft=16;}

int32 Bundle::compDecode(byte *src, byte *dst)
{
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

int32 Bundle::decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size, int32 index, int32 & channels)
{
	int32 output_size;
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
				ptr = src + length + (k / 2);
				if (k & 1) {
					r = c / 8;
					*(t_table + r + 2) = ((*(src + j) & 0x0f) << 4) | ((*(ptr + 1) & 0xf0) >> 4);
					*(t_table + r + 1) = (*(src + j) & 0xf0) | (*(t_table + r + 1));
				} else {
					r = s / 8;
					*(t_table + r + 0) = ((*(src + j) & 0x0f) << 4) | (*ptr & 0x0f);
					*(t_table + r + 1) = (*(src + j) & 0xf0) >> 4;
				}
				s += 12;
				k++;
				j++;
				c += 12;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) / 2;
		*(t_table + offset1 + 1) = (*(t_table + offset1 + 1)) | *(src + length - 1) & 0xf0;
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
		*t_table = (*(src + length)) >> 4;
		t = length + k;
		j = 1;
		if (t > k) {
			do {
				ptr = src + length + (k / 2);
				if (k & 1) {
					r = c / 8;
					*(t_table + r + 0) = (*(src + j - 1) & 0xf0) | (*(t_table + r));
					*(t_table + r + 1) = ((*(src + j - 1) & 0x0f) << 4) | (*ptr & 0x0f);
				} else {
					r = s / 8;
					*(t_table + r + 0) = (*(src + j - 1) & 0xf0) >> 4;
					*(t_table + r - 1) = ((*(src + j - 1) & 0x0f) << 4) | ((*ptr & 0xf0) >> 4);
				}
				s += 12;
				k++;
				j++;
				c += 12;
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
		*t_table = *(output_size + src - 1);
		*(t_table + output_size - 1) = *(src + length - 1);
		t = length - 1;
		if (t > 0) {
			do {
				ptr = src + length + (k / 2);
				if (k & 1) {
					r = s / 8;
					*(t_table + r + 2) = (*(src + j) & 0xf0) | *(t_table + r + 2);
					*(t_table + r + 3) = ((*(src + j) & 0x0f) << 4) | ((*ptr & 0xf0) >> 4);
				} else {
					r = c / 8;
					*(t_table + r + 2) = (*(src + j) & 0xf0) >> 4;
					*(t_table + r + 1) = ((*(src + j) & 0x0f) << 4) | (*ptr & 0x0f);
				}
				s += 12;
				k++;
				j++;
				c += 12;
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
			*(t_table + offset2 + 0) = *(src + offset1);
			*(t_table + offset2 + 1) = *(src + offset3);
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		if (length > 0) {
			c = -12;
			s = 0;
			do {
				j = length + (k / 2);
				if (k & 1) {
					r = c / 8;
					t_tmp1 = *(t_table + k);
					t_tmp2 = *(t_table + j + 1);
					*(src + r + 2) = ((t_tmp1 & 0x0f) << 4) | ((t_tmp2 & 0xf0) >> 4);
					*(src + r + 1) = (*(src + r + 1)) | (t_tmp1 & 0xf0);
				} else {
					r = s / 8;
					t_tmp1 = *(t_table + k);
					t_tmp2 = *(t_table + j);
					*(src + r + 0) = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
					*(src + r + 1) = ((t_tmp1 & 0xf0) >> 4);
				}
				s += 12;
				k++;
				c += 12;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) / 2;
		*(src + offset1 + 1) = (*(t_table + length) & 0xf0) | *(src + offset1 + 1);
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
			*(t_table + offset2 + 0) = *(src + offset1);
			*(t_table + offset2 + 1) = *(src + offset3);
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 1;
		c = 0;
		s = 12;
		t_tmp1 = (*(t_table + length)) >> 4;
		*(src) = t_tmp1;
		t = length + k;
		if (t > k) {
			do {
				j = length + (k / 2);
				if (k & 1) {
					r = c / 8;
					t_tmp1 = *(t_table + k - 1);
					t_tmp2 = *(t_table + j);
					*(src + r + 0) = (*(src + r)) | (t_tmp1 & 0xf0);
					*(src + r + 1) = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				} else {
					r = s / 8;
					t_tmp1 = *(t_table + k - 1);
					t_tmp2 = *(t_table + j);
					*(src + r + 0) = (t_tmp1 & 0xf0) >> 4;
					*(src + r - 1) = ((t_tmp1 & 0x0f) << 4) | ((t_tmp2 & 0xf0) >> 4);
				}
				s += 12;
				k++;
				c += 12;
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
			*(t_table + offset2 + 0) = *(src + offset1);
			*(t_table + offset2 + 1) = *(src + offset3);
		} while (1);

		src = comp_output;
		length = (output_size * 8) / 12;
		k = 0;
		c = 0;
		s = -12;
		*(src) = *(output_size + t_table - 1);
		*(src + output_size - 1) = *(t_table + length - 1);
		t = length - 1;
		if (t > 0) {
			do {
				j = length + (k / 2);
				if (k & 1) {
					r = s / 8;
					t_tmp1 = *(t_table + k);
					t_tmp2 = *(t_table + j);
					*(src + r + 2) = (*(src + r + 2)) | (t_tmp1 & 0xf0);
					*(src + r + 3) = ((t_tmp1 & 0x0f) << 4) | ((t_tmp2 & 0xf0) >> 4);
				} else {
					r = c / 8;
					t_tmp1 = *(t_table + k);
					t_tmp2 = *(t_table + j);
					*(src + r + 2) = (t_tmp1 & 0xf0) >> 4;
					*(src + r + 1) = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				}
				s += 12;
				k++;
				c += 12;
			} while (k < t);
		}
		free(t_table);
		break;

	case 13:
	case 15:
		{
			uint16 firstWord;
			int32 startPos;
			byte sByte[4];
			int32 sDWord[4];
			int32 channel;
			int32 left, origLeft;
			int32 tableEntrySum;
			int32 curTablePos;
			int32 outputWord;
			int32 imcTableEntry;
			int32 destPos = 0;
			int32 curTableEntry;
			byte decompTable;
			uint16 readWord;
			byte * readPos;
			byte otherTablePos;
			int32 esiReg;
			byte var3b;
			int32 adder;
			
			byte *endPos = comp_input + input_size;
			
			src = comp_input;
			memset (comp_output, 0, 0x2000);
			firstWord = READ_BE_UINT16(src);
			src += 2;
			assert(src < endPos);


			if (firstWord != 0) {
				if (index != 0) {
					startPos = 0;
				} else {
					memcpy(comp_output, src, firstWord);
					startPos = firstWord;
					uint32 tag;
					ptr = src;
					do {
						tag = READ_BE_UINT32(ptr); ptr += 4;
					} while (tag != MKID_BE('FRMT'));
					channels = READ_BE_UINT32(ptr + 20);
				}
				src += firstWord;
				origLeft = 0x2000 - firstWord;
				// At this point we are at the start of the content of the 'DATA' chunk.
			} else {
				sByte[0] = *src++;
				sDWord[0] = (int32)READ_BE_UINT32(src);
				src += 4;
				sDWord[1] = (int32)READ_BE_UINT32(src);
				src += 4;
				if (channels > 1) {
					sByte[1] = *src++;
					sDWord[2] = (int32)READ_BE_UINT32(src);
					src += 4;
					sDWord[3] = (int32)READ_BE_UINT32(src);
					src += 4;
				}
				startPos = 0;
				origLeft = 0x2000;
			}
			assert(src < endPos);

			tableEntrySum = 0;
			for (channel = 0; channel < channels; channel++) {
				if (firstWord == 0) {
					curTablePos = sByte[channel];
					outputWord = sDWord[channel * 2 + 1];
					imcTableEntry = sDWord[channel * 2];
				} else {
					curTablePos = 0;
					outputWord = 0;
					imcTableEntry = 7;
				}
				left = (origLeft - 1) / (2 * channels) + 1;
				destPos = startPos + 2 * channel;
				while (left--) {
					curTableEntry = _destImcTable[curTablePos];
					decompTable = curTableEntry - 2;
					var3b = (1 << decompTable) << 1;
					readPos = src + (tableEntrySum >> 3);
					if (readPos >= endPos) {
						error("readPos exceeds endPos: %d >= %d (%d, %d)!" , readPos, endPos, left, origLeft);
					}
					assert(readPos < endPos);
					readWord = (uint16)(READ_BE_UINT16(readPos) << (tableEntrySum & 7));
					otherTablePos = (byte)(readWord >> (16 - curTableEntry));
					tableEntrySum += curTableEntry;
					esiReg = imxShortTable[curTableEntry];
					esiReg = (esiReg & otherTablePos) << (7 - curTableEntry);
					esiReg += (curTablePos << 6);
					imcTableEntry >>= (curTableEntry - 1);
					adder = imcTableEntry + _destImcTable2[esiReg];
					if ((otherTablePos & var3b) != 0) {
						adder = -adder;
					}
					outputWord += adder;
					if (outputWord > 0x7fff) 
						outputWord = 0x7fff;
					if (outputWord < -0x8000)
						outputWord = -0x8000;
					comp_output[destPos] = (byte)(outputWord >> 8);
					comp_output[destPos + 1] = (byte)(outputWord);
					switch (decompTable) {
						case 0: curTablePos += (signed char)imxOtherTable1[otherTablePos];
							break;
						case 1: curTablePos += (signed char)imxOtherTable2[otherTablePos];
							break;
						case 2: curTablePos += (signed char)imxOtherTable3[otherTablePos];
							break;
						case 3: curTablePos += (signed char)imxOtherTable4[otherTablePos];
							break;
						case 4: curTablePos += (signed char)imxOtherTable5[otherTablePos];
							break;
						case 5: curTablePos += (signed char)imxOtherTable6[otherTablePos];
							break;
					}
					if (curTablePos < 0)
						curTablePos = 0;
					if (curTablePos > 88)
						curTablePos = 88;
					destPos += 2 * channels;
					imcTableEntry = imcTable1[curTablePos];
				}
			}
			if (index == 0) {
				output_size = 0x2000 - firstWord;
			} else {
				output_size = 0x2000;
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
