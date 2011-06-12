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
			{   0,  60,  0},	// 198
			{   0, 104,  0},	// 199
			{  20, 172,  0},	// 200
			{  82,  82,  0},	// 201
			{   0, 132, 82},	// 202
			{ 132, 173, 82},	// 203
			{  82,   0,  0},	// 204
			{ 206,   0, 24},	// 205
			{ 255,  33, 33},	// 206
			{ 123,  41,  0},	// 207
			{   0,  41,  0},	// 208
			{   0,   0, 82},	// 209
			{ 132,   0,  0},	// 210
			{ 255,   0,  0},	// 211
			{ 255,  66, 66},	// 212
			{ 148,  66, 16},	// 213
			{   0,  82,  0},	// 214
			{   0,   0,132},	// 215
			{ 173,   0,  0},	// 216
			{ 255,  49,  0},	// 217
			{ 255,  99, 99},	// 218
			{ 181, 107, 49},	// 219
			{   0, 132,  0},	// 220
			{   0,   0,255},	// 221
			{ 173,  41,  0},	// 222
			{ 255,  82,  0},	// 223
			{ 255, 132,132},	// 224
			{ 214, 148, 74},	// 225
			{  41, 214,  0},	// 226
			{   0,  82,173},	// 227
			{ 255, 214,  0},	// 228
			{ 247, 132, 49},	// 229
			{ 255, 165,165},	// 230
			{ 239, 198,123},	// 231
			{ 173, 214,  0},	// 232
			{   0, 132,214},	// 233
			{  57,  57, 57},	// 234
			{ 247, 189, 74},	// 235
			{ 255, 198,198},	// 236
			{ 255, 239,173},	// 237
			{ 214, 255,173},	// 238
			{  82, 173,255},	// 239
			{ 107, 107,107},	// 240
			{ 247, 222, 99},	// 241
			{ 255,   0,255},	// 242
			{ 255, 132,255},	// 243
			{ 132, 132,173},	// 244
			{ 148, 247,255},	// 245
			{ 148, 148,148},	// 246
			{  82,   0, 82},	// 247
			{ 112,  68,112},	// 248
			{ 176,  88,144},	// 249
			{ 214, 132,173},	// 250
			{ 206, 247,255},	// 251
			{ 198, 198,198},	// 252
			{   0, 214,255},	// 253
			{  96, 224,96 },	// 254
			{ 255, 255,255},	// 255
					};

EC void		_fqsort		(void *base, uint16 nelem, uint16 width, int (*fcmp)(const void*, const void*)) {
	warning("STUB: _fqsort");
}

const char *	ProgName	(const char * ext) {
	warning("STUB: ProgName");
	return NULL;
}

char *MergeExt (char *buf, const char *nam, const char *ext) {
//  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
//  fnmerge(buf, dr, di, na, (fnsplit(nam, dr, di, na, ex) & EXTENSION) ? ex : ext);
//  return buf;
	warning("STUB: MergeExt");
	return buf;
}

char *ForceExt (char *buf, const char *nam, const char* ext) {
//  char dr[MAXDRIVE], di[MAXDIR], na[MAXFILE], ex[MAXEXT];
//  fnsplit(nam, dr, di, na, ex);
//  fnmerge(buf, dr, di, na, ext);
//  return buf;
	warning("STUB: ForceExt");
	return buf;
}


#define		BUF	((uint8 *) buf)
static	unsigned	Seed = 1;

unsigned FastRand (void) { return Seed = 257 * Seed + 817; }
unsigned FastRand (unsigned s) { return Seed = 257 * s + 817; }

uint16 RCrypt (void * buf, uint16 siz, uint16 seed) {
/*
	if (buf && siz) {
      uint8 * q = BUF + (siz-1);
      seed = FastRand(seed);
      * (BUF ++) ^= seed;
      while (buf < q) * (BUF ++) ^= FastRand();
      if (buf == q) * BUF ^= (seed = FastRand());
    }
  return seed;
*/
	warning("STUB: RCrypt");
	return 0;
}

uint16 XCrypt (void *buf, uint16 siz, uint16 seed) {
//  for (uint16 i = 0; i < siz; i ++) 
//	  *(BUF ++) ^= seed;
	warning("STUB: XCrypt");
	return seed;
}

uint16 atow (const char *a) {
  uint16 w = 0;
	if (a)
		while (IsDigit(*a))
			w = (10 * w) + (*(a ++) & 0xF);
  return w;
}

uint16 xtow (const char *x) {
  uint16 w = 0;
  if (x) {
      while (IsHxDig(*x)) {
		register uint16 d = * (x ++);
		if (d > '9')
		  d -= 'A' - ('9' + 1);
		w = (w << 4) | (d & 0xF);
	  }
  }
  return w;
}

char *wtom (uint16 val, char *str, int radix, int len) {
  while (-- len >= 0) {
      uint16 w = val % radix;
      if (w > 9) w += ('A' - ('9'+1));
      str[len] = '0' + w;
      val /= radix;
    }
  return str;
}

IOHAND::IOHAND (IOMODE mode, CRYPT * crpt)
: XFILE(mode), Handle(-1), Crypt(crpt), Seed(SEED)
{
}

IOHAND::IOHAND (const char *name, IOMODE mode, CRYPT *crpt)
: XFILE(mode), Crypt(crpt), Seed(SEED)
{
/*  switch (mode)
    {
      case REA : Error = _dos_open(name, O_RDONLY | O_DENYNONE, &Handle); break;
      case WRI : Error = _dos_creat(name, FA_ARCH, &Handle); break;
      case UPD : Error = _dos_open(name, O_RDWR | O_DENYALL, &Handle); break;
    }
  if (Error) Handle = -1;
*/
	warning("STUB: IOHAND::IOHAND");
}

IOHAND::~IOHAND(void) {
/*
	if (Handle != -1)
    {
      Error = _dos_close(Handle);
      Handle = -1;
    }
*/
	warning("STUB: IOHAND::~IOHAND");
}

uint16 IOHAND::Read(void *buf, uint16 len) {
/*
  if (Mode == WRI || Handle < 0) return 0;
  if (len) Error = _dos_read(Handle, buf, len, &len);
  if (Crypt) Seed = Crypt(buf, len, Seed);
  return len;
*/
	warning("STUB: IOHAND::Read");
	return 0;
}

uint16 IOHAND::Write(void *buf, uint16 len) {
/*
  if (len) {
      if (Mode == REA || Handle < 0) return 0;
      if (Crypt) Seed = Crypt(buf, len, Seed);
      Error = _dos_write(Handle, buf, len, &len);
      if (Crypt) Seed = Crypt(buf, len, Seed); //------$$$$$$$
    }
  return len;
*/
	warning("STUB: IOHAND::Write");
	return 0;
}

long IOHAND::Mark (void)
{
  return (Handle < 0) ? 0 : tell(Handle);
}

long IOHAND::Seek (long pos)
{
  if (Handle < 0) return 0;
  lseek(Handle, pos, SEEK_SET);
  return tell(Handle);
}

long IOHAND::Size (void)
{
  if (Handle < 0) return 0;
  return filelength(Handle);
}

bool IOHAND::Exist (const char * name) {
  return access(name, 0) == 0;
}

//#define		EMS_ADR(a)	(FP_SEG(a) > 0xA000)
//#define		HNODE_OK(p)	(heapchecknode(p)==4)

MEM_TYPE MemType (void *mem) {
/*	if (FP_SEG(mem) == _DS) {
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

DATACK *LoadWave(XFILE * file, EMM * emm) {
	warning("STUB: LoadWave");
	return NULL;
}

int TakeEnum(const char **tab, const char *txt) {
  const char **e;
  if (txt)
    {
      for (e = tab; *e; e ++)
	{
	  if (scumm_stricmp(txt, *e) == 0)
	    {
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

} // End of namespace CGE

