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

#include "cge/boot.h"
#include "cge/general.h"
#include "cge/snddrv.h"
#include "cge/wav.h"

namespace CGE {

DAC StdPal[] =  {// R    G   B
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

DRVINFO SNDDrvInfo;

EC void     _fqsort(void *base, uint16 nelem, uint16 width, int (*fcmp)(const void *, const void *)) {
	warning("STUB: _fqsort");
}

const char *ProgName(const char *ext) {
	warning("ProgName");

	static Common::String buf = "CGE";
	if (ext)
		buf += ext;
	return buf.c_str();
}

char *MergeExt(char *buf, const char *nam, const char *ext) {
//  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
//  fnmerge(buf, dr, di, na, (fnsplit(nam, dr, di, na, ex) & EXTENSION) ? ex : ext);
//  return buf;
	warning("MergeExt");

	strcpy(buf, nam);
	char *dot = strrchr(buf, '.');
	if (!dot)
		strcat(buf, ext);

	return buf;
}

char *ForceExt(char *buf, const char *nam, const char *ext) {
//  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
//  fnsplit(nam, dr, di, na, ex);
//  fnmerge(buf, dr, di, na, ext);
//  return buf;
	warning("ForceExt");
	strcpy(buf, nam);
	char *dot = strrchr(buf, '.');
	if (dot)
		*dot = '\0';
	strcat(buf, ext);

	return buf;
}


#define     BUF ((uint8 *) buf)
static  unsigned    Seed = 1;

unsigned FastRand(void) {
	return Seed = 257 * Seed + 817;
}
unsigned FastRand(unsigned s) {
	return Seed = 257 * s + 817;
}

uint16 RCrypt(void *buf, uint16 siz, uint16 seed) {
	if (buf && siz) {
		byte *b = static_cast<byte *>(buf);
		byte *q = b + (siz - 1);
		seed = FastRand(seed);
		*b++ ^= seed;
		while (buf < q)
			*b++ ^= FastRand();
		if (buf == q)
			*b ^= (seed = FastRand());
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

IOHAND::IOHAND(IOMODE mode, CRYPT *crpt)
	: XFILE(mode), Crypt(crpt), Seed(SEED) {
	_file = new Common::File();
}

IOHAND::IOHAND(const char *name, IOMODE mode, CRYPT *crpt)
		: XFILE(mode), Crypt(crpt), Seed(SEED) {
	// TODO: Check if WRI and/or UPD modes are needed, and map to a save file
	assert(mode == REA);

	_file = new Common::File();
	_file->open(name);
}

IOHAND::~IOHAND(void) {
	_file->close();
	delete _file;
}

uint16 IOHAND::Read(void *buf, uint16 len) {
	if (Mode == WRI || !_file->isOpen())
		return 0;

	uint16 bytesRead = _file->read(buf, len);
	if (Crypt) Seed = Crypt(buf, len, Seed);
	return bytesRead;
}

uint16 IOHAND::Write(void *buf, uint16 len) {
	error("IOHAND::Write not supported");
/*
	if (len) {
		if (Mode == REA || Handle < 0)
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

long IOHAND::Mark(void) {
	return _file->pos();
}

long IOHAND::Seek(long pos) {
	_file->seek(pos, SEEK_SET);
	return _file->pos();
}

long IOHAND::Size(void) {
	return _file->size();
}

bool IOHAND::Exist(const char *name) {
	Common::File f;
	return f.exists(name);
}

//#define       EMS_ADR(a)  (FP_SEG(a) > 0xA000)
//#define       HNODE_OK(p) (heapchecknode(p)==4)

MEM_TYPE MemType(void *mem) {
	/*  if (FP_SEG(mem) == _DS) {
	      if (heapchecknode((void *)mem)==4)
	          return NEAR_MEM;
	    } else {
	      if (FP_SEG(mem) > 0xA000)
	          return EMS_MEM;
	      else if (farheapchecknode(mem)==4)
	          return FAR_MEM;
	    }
	  return BAD_MEM;
	*/
	warning("STUB: MemType");
	return FAR_MEM;
}

bool IsVga() {
	return true;
}

EC void SNDInit() {
	warning("STUB: SNDInit");
}

EC void SNDDone() {
	warning("STUB: SNDDone");
}

EC void SNDSetVolume() {
	warning("STUB: SNDSetVolume");
}

EC void SNDDigiStart(SMPINFO *PSmpInfo) {
	warning("STUB: SNDDigitStart");
}

EC void SNDDigiStop(SMPINFO *PSmpInfo) {
	warning("STUB: SNDDigiStop");
}

EC void SNDMIDIStart(uint8 *MIDFile) {
	warning("STUB: SNDMIDIStart");
}

EC void SNDMIDIStop() {
	warning("STUB: SNDMIDIStop");
}

DATACK *LoadWave(XFILE *file, EMM *emm) {
	warning("STUB: LoadWave");
	return NULL;
}

int TakeEnum(const char **tab, const char *txt) {
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

Boot *ReadBoot(int drive) {
	/*
	  struct fatinfo fi; Boot *b;
	  getfat(drive+1, &fi);
	  if (fi.fi_sclus & 0x80) return NULL;
	  if ((b = malloc(fi.fi_bysec)) == NULL) return NULL;
	  // read boot sector
	  if (absread(drive, 1, 0L, b) == 0) return b;
	  free(b);
	  return NULL;
	*/
	warning("STUB: ReadBoot");
	return NULL;
}

long Timer(void) {
/*
  asm	mov	ax,0x40
  asm	mov	es,ax
  asm	mov	cx,es:[0x6C]
  asm	mov	dx,es:[0x6E]
  return  ((long) _DX << 16) | _CX;
*/
	warning("STUB: Timer");
	return 0;
}

int new_random(int range) {
	warning("STUB: new_random(a)");
	return 0;
}

#define		TIMER_INT	0x08
//void interrupt	(* ENGINE::OldTimer) (...) = NULL;

ENGINE::ENGINE (uint16 tdiv)
{
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
	warning("STUB: ENGINE::ENGINE");
}

ENGINE::~ENGINE (void)
{
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
	warning("STUB: ENGINE::~ENGINE");
}

DATACK::~DATACK (void)
{
  if (!e && Buf) free(Buf);
}
} // End of namespace CGE

