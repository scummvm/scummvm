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

Dac _stdPal[] =  {// R    G   B
	{   0,  60,  0},    // 198
	{   0, 104,  0},    // 199
	{  20, 172,  0},    // 200
	{  82,  82,  0},    // 201
	{   0, 132, 82},    // 202
	{ 132, 173, 82},    // 203
	{  82,   0,  0},    // 204
	{ 206,   0, 24},    // 205
	{ 255,  33, 33},    // 206
	{ 123,  41,  0},    // 207
	{   0,  41,  0},    // 208
	{   0,   0, 82},    // 209
	{ 132,   0,  0},    // 210
	{ 255,   0,  0},    // 211
	{ 255,  66, 66},    // 212
	{ 148,  66, 16},    // 213
	{   0,  82,  0},    // 214
	{   0,   0, 132},   // 215
	{ 173,   0,  0},    // 216
	{ 255,  49,  0},    // 217
	{ 255,  99, 99},    // 218
	{ 181, 107, 49},    // 219
	{   0, 132,  0},    // 220
	{   0,   0, 255},   // 221
	{ 173,  41,  0},    // 222
	{ 255,  82,  0},    // 223
	{ 255, 132, 132},   // 224
	{ 214, 148, 74},    // 225
	{  41, 214,  0},    // 226
	{   0,  82, 173},   // 227
	{ 255, 214,  0},    // 228
	{ 247, 132, 49},    // 229
	{ 255, 165, 165},   // 230
	{ 239, 198, 123},   // 231
	{ 173, 214,  0},    // 232
	{   0, 132, 214},   // 233
	{  57,  57, 57},    // 234
	{ 247, 189, 74},    // 235
	{ 255, 198, 198},   // 236
	{ 255, 239, 173},   // 237
	{ 214, 255, 173},   // 238
	{  82, 173, 255},   // 239
	{ 107, 107, 107},   // 240
	{ 247, 222, 99},    // 241
	{ 255,   0, 255},   // 242
	{ 255, 132, 255},   // 243
	{ 132, 132, 173},   // 244
	{ 148, 247, 255},   // 245
	{ 148, 148, 148},   // 246
	{  82,   0, 82},    // 247
	{ 112,  68, 112},   // 248
	{ 176,  88, 144},   // 249
	{ 214, 132, 173},   // 250
	{ 206, 247, 255},   // 251
	{ 198, 198, 198},   // 252
	{   0, 214, 255},   // 253
	{  96, 224, 96 },   // 254
	{ 255, 255, 255},   // 255
};

const char *progName(const char *ext) {
	static char buf[kMaxFile];
	strcpy(buf, "CGE");
	if (ext) {
		strcat(buf, ".");
		if (*ext == '.')
			ext++;
		strcat(buf, ext);
	}

	return buf;
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

uint16 atow(const char *a) {
	if (!a)
		return 0;

	uint16 w = 0;
	while (IsDigit(*a))
		w = (10 * w) + (*(a++) & 0xF);
	return w;
}

uint16 xtow(const char *x) {
	if (!x)
		return 0;

	uint16 w = 0;
	while (IsHxDig(*x)) {
		register uint16 d = *(x++);
		if (d > '9')
			d -= 'A' - ('9' + 1);
		w = (w << 4) | (d & 0xF);
	}
	return w;
}

char *wtom(uint16 val, char *str, int radix, int len) {
	while (--len >= 0) {
		uint16 w = val % radix;
		if (w > 9)
			w += ('A' - ('9' + 1));
		str[len] = '0' + w;
		val /= radix;
	}
	return str;
}

char *dwtom(uint32 val, char *str, int radix, int len) {
	while (--len >= 0) {
		uint16 w = (uint16) (val % radix);
		if (w > 9)
			w += ('A' - ('9' + 1));
		str[len] = '0' + w;
		val /= radix;
	}
	return str;
}

void sndSetVolume() {
	// USeless for ScummVM
}

DataCk *loadWave(XFile *file) {
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
	if (_buf)
		free(_buf);
}

} // End of namespace CGE

