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

Bundle::Bundle() {
	_lastSong = -1;
}

Bundle::~Bundle() {
}

bool Bundle::openVoiceFile(const char *filename, const char *directory) {
	int32 tag, offset;

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

bool Bundle::openMusicFile(const char *filename, const char *directory) {
	int32 tag, offset;

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

int32 Bundle::decompressVoiceSampleByIndex(int32 index, byte *comp_final) {
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
			decompressCodec(_compVoiceTable[i].codec, comp_input, comp_output, _compVoiceTable[i].size);
		memcpy((byte *)&comp_final[final_size], comp_output, output_size);
		final_size += output_size;

		free(comp_input);
	}
	free(comp_output);

	return final_size;
}

int32 Bundle::decompressMusicSampleByIndex(int32 index, int32 number, byte *comp_final) {
	int32 i, tag, num, final_size;
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
	final_size =
		decompressCodec(_compMusicTable[number].codec, comp_input, comp_final, _compMusicTable[number].size);

	free(comp_input);

	_lastSong = index;

	return final_size;
}

int32 Bundle::decompressVoiceSampleByName(char *name, byte *comp_final, bool use_extended) {
	int32 final_size = 0, i;
	char realName[255];

	if (use_extended)
		sprintf(realName, "%s.IMX", name);
	else
		strcpy(realName, name);

	if (_voiceFile.isOpen() == false) {
		warning("Bundle: voice file is not open!");
		return 0;
	}

	for (i = 0; i < _numVoiceFiles; i++) {
		if (!scumm_stricmp(realName, _bundleVoiceTable[i].filename)) {
			final_size = decompressVoiceSampleByIndex(i, comp_final);
			return final_size;
		}
	}
	printf("Failed finding voice %s\n", realName);
	return final_size;
}

int32 Bundle::decompressMusicSampleByName(char *name, int32 number, byte *comp_final, bool fuzzy) {
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
		if (fuzzy) // Fuzzy matching, only look at the first part of the song
			if (strstr(_bundleMusicTable[i].filename, name) == _bundleMusicTable[i].filename) {
				final_size = decompressMusicSampleByIndex(i, number, comp_final);
				return final_size;
			}
		else
			if (!scumm_stricmp(name, _bundleMusicTable[i].filename)) {
				final_size = decompressMusicSampleByIndex(i, number, comp_final);
				return final_size;
			}
	}
	printf("Couldn't find sample %s\n", name);
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

int32 Bundle::getNumberOfMusicSamplesByName(char *name, bool fuzzy) {
	int32 number = 0, i;

	if (_musicFile.isOpen() == false) {
		warning("Bundle: music file is not open!");
		return 0;
	}

	for (i = 0; i < _numMusicFiles; i++) {
		if (fuzzy) // Fuzzy matching, only look at the first part of the song
			if (strstr(_bundleMusicTable[i].filename, name) == _bundleMusicTable[i].filename) {
				number = getNumberOfMusicSamplesByIndex(i);
				return number;
			}
		else
			if (!scumm_stricmp(name, _bundleMusicTable[i].filename)) {
				number = getNumberOfMusicSamplesByIndex(i);
				return number;
			}

	}
	printf("Couldn't find numsample %s\n", name);
	return number;
}

#define NextBit bit = mask & 1; mask >>= 1; if (!--bitsleft) {mask = READ_LE_UINT16(srcptr); srcptr += 2; bitsleft=16;}

int32 Bundle::compDecode(byte *src, byte *dst) {
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
	int32 output_size = input_size;
	int32 offset1, offset2, offset3, length, k, c, s, j, r, t, z;
	byte *src, *t_table, *p, *ptr;
	byte t_tmp1, t_tmp2;

	switch (codec) {
	case 0:
		memcpy(comp_output, comp_input, output_size);
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

	default:
		warning("Bundle: Unknown codec %d!", (int)codec);
		output_size = 0;
		break;
	}

	return output_size;
}
