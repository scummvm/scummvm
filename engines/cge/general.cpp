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
#include "cge/wav.h"

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

void _fqsort(void *base, uint16 nelem, uint16 width, int (*fcmp)(const void *, const void *)) {
	warning("STUB: _fqsort");
}

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

char *mergeExt(char *buf, const char *nam, const char *ext) {
	strcpy(buf, nam);
	char *dot = strrchr(buf, '.');
	if (!dot)
		strcat(buf, ext);

	return buf;
}

char *forceExt(char *buf, const char *nam, const char *ext) {
	strcpy(buf, nam);
	char *dot = strrchr(buf, '.');
	if (dot)
		*dot = '\0';
	strcat(buf, ext);

	return buf;
}

static  unsigned    Seed = 0xA5;

unsigned fastRand() {
	return Seed = 257 * Seed + 817;
}
unsigned fastRand(unsigned s) {
	return Seed = 257 * s + 817;
}

uint16 RCrypt(void *buf, uint16 siz, uint16 seed) {
	if (buf && siz) {
		byte *b = static_cast<byte *>(buf);
		byte *q = b + (siz - 1);
		seed = fastRand(seed);
		*b++ ^= seed;
		while (buf < q)
			*b++ ^= fastRand();
		if (buf == q)
			*b ^= (seed = fastRand());
	}
	return seed;
}

uint16 XCrypt(void *buf, uint16 siz, uint16 seed) {
	byte *b = static_cast<byte *>(buf);

	for (uint16 i = 0; i < siz; i++)
		*b++ ^= seed;
	
	return seed;
}

uint16 atow(const char *a) {
	uint16 w = 0;
	if (a)
		while (IsDigit(*a))
			w = (10 * w) + (*(a++) & 0xF);
	return w;
}

uint16 xtow(const char *x) {
	uint16 w = 0;
	if (x) {
		while (IsHxDig(*x)) {
			register uint16 d = *(x++);
			if (d > '9')
				d -= 'A' - ('9' + 1);
			w = (w << 4) | (d & 0xF);
		}
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

IoHand::IoHand(IOMode mode, CRYPT *crpt)
	: XFile(mode), _crypt(crpt), _seed(kCryptSeed) {
	_file = new Common::File();
}

IoHand::IoHand(const char *name, IOMode mode, CRYPT *crpt)
		: XFile(mode), _crypt(crpt), _seed(kCryptSeed) {
	// TODO: Check if WRI and/or UPD modes are needed, and map to a save file
	assert(mode == kModeRead);

	_file = new Common::File();
	_file->open(name);
}

IoHand::~IoHand() {
	_file->close();
	delete _file;
}

uint16 IoHand::read(void *buf, uint16 len) {
	if (_mode == kModeWrite || !_file->isOpen())
		return 0;

	uint16 bytesRead = _file->read(buf, len);
	if (!bytesRead)
		error("Read %s - %d bytes", _file->getName(), len);
	if (_crypt)
		_seed = _crypt(buf, len, Seed);
	return bytesRead;
}

uint16 IoHand::write(void *buf, uint16 len) {
	warning("IOHAND::Write not supported");
	return 0;
/*
	if (len) {
		if (Mode == kModeRead || Handle < 0)
			return 0;
		if (Crypt)
			Seed = Crypt(buf, len, Seed);
		Error = _dos_write(Handle, buf, len, &len);
		if (Crypt)
			Seed = Crypt(buf, len, Seed); //------$$$$$$$
	}
	return len;
*/
}

long IoHand::mark() {
	return _file->pos();
}

long IoHand::seek(long pos) {
	_file->seek(pos, SEEK_SET);
	return _file->pos();
}

long IoHand::size() {
	return _file->size();
}

bool IoHand::exist(const char *name) {
	Common::File f;
	return f.exists(name);
}

void sndInit() {
	warning("STUB: SNDInit");
}

void sndDone() {
	// FIXME: STUB: SNDDone
}

void sndSetVolume() {
	warning("STUB: SNDSetVolume");
}

void sndDigiStart(SmpInfo *PSmpInfo) {
	warning("STUB: SNDDigitStart");
}

void sndDigiStop(SmpInfo *PSmpInfo) {
	warning("STUB: SNDDigiStop");
}

void sndMidiStart(uint8 *MIDFile) {
	warning("STUB: SNDMIDIStart");
}

void sndMidiStop() {
	// FIXME: STUB: sndMIDIStop
}

DataCk *loadWave(XFile *file) {
	warning("STUB: LoadWave");
	return NULL;
}

int takeEnum(const char **tab, const char *txt) {
	const char **e;
	if (txt) {
		for (e = tab; *e; e++) {
			if (scumm_stricmp(txt, *e) == 0) {
				return e - tab;
			}
		}
	}
	return -1;
}

long timer() {
/*
  asm	mov	ax,0x40
  asm	mov	es,ax
  asm	mov	cx,es:[0x6C]
  asm	mov	dx,es:[0x6E]
  return  ((long) _DX << 16) | _CX;
*/
	warning("STUB: timer");
	return 0;
}

int new_random(int range) {
	return ((CGEEngine *)g_engine)->_randomSource.getRandomNumber(range - 1);
}

//void interrupt	(* Engine_::oldTimer) (...) = NULL;

Engine_::Engine_(uint16 tdiv) {
/*
  // steal timer interrupt
  OldTimer = getvect(TIMER_INT);
  setvect(TIMER_INT, NewTimer);

  // set turbo-timer mode
  asm	mov	al,0x36
  asm	out	0x43,al
  asm	mov	ax,TMR_DIV
  asm	out	0x40,al
  asm	mov	al,ah
  asm	out	0x40,al
*/
	warning("STUB: Engine_::Engine_");
}

Engine_::~Engine_() {
/*
  // reset timer
  asm	mov	al,0x36
  asm	out	0x43,al
  asm	xor	al,al
  asm	out	0x40,al
  asm	out	0x40,al
  // bring back timer interrupt
  setvect(TIMER_INT, OldTimer);
*/
	warning("STUB: Engine_::~Engine_");
}

DataCk::~DataCk() {
  if (!_e && _buf)
	  free(_buf);
}
} // End of namespace CGE

