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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/cge.h"
#include "cge/general.h"
#include "cge/snddrv.h"

namespace CGE {

uint16 XCrypt(void *buf, uint16 siz) {
	byte *b = static_cast<byte *>(buf);

	for (uint16 i = 0; i < siz; i++)
		*b++ ^= kCryptSeed;
	
	return kCryptSeed;
}

char *mergeExt(char *buf, const char *name, const char *ext) {
	strcpy(buf, name);
	char *dot = strrchr(buf, '.');
	if (!dot)
		strcat(buf, ext);

	return buf;
}

char *forceExt(char *buf, const char *name, const char *ext) {
	strcpy(buf, name);
	char *dot = strrchr(buf, '.');
	if (dot)
		*dot = '\0';
	strcat(buf, ext);

	return buf;
}

void sndSetVolume() {
	// USeless for ScummVM
}

DataCk *loadWave(EncryptedStream *file) {
	byte *data = (byte *)malloc(file->size());
	file->read(data, file->size());

	return new DataCk(data, file->size());
}

int takeEnum(const char **tab, const char *text) {
	const char **e;
	if (text) {
		for (e = tab; *e; e++) {
			if (scumm_stricmp(text, *e) == 0) {
				return e - tab;
			}
		}
	}
	return -1;
}

int newRandom(int range) {
	return ((CGEEngine *)g_engine)->_randomSource.getRandomNumber(range - 1);
}

DataCk::DataCk(byte *buf, int bufSize) {
	_buf = buf;
	_ckSize = bufSize;
}

DataCk::~DataCk() {
	free(_buf);
}

} // End of namespace CGE

