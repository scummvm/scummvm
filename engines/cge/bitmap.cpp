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

#include "cge/bitmap.h"
#include "cge/cfile.h"
#include "cge/jbw.h"
#include "cge/vol.h"
#include "cge/cfile.h"
#include "cge/vga13h.h"
#include "common/system.h"

namespace CGE {

Dac *Bitmap::_pal = NULL;
#define MAXPATH  128

void Bitmap::init() {
	_pal = NULL;
}

void Bitmap::deinit() {
}

#pragma argsused
Bitmap::Bitmap(const char *fname, bool rem) : _m(NULL), _v(NULL) {
	char pat[MAXPATH];
	forceExt(pat, fname, ".VBM");

#if (BMP_MODE < 2)
	if (rem && PIC_FILE::exist(pat)) {
		PIC_FILE file(pat);
		if ((file._error == 0) && (!loadVBM(&file)))
			error("Bad VBM [%s]", fname);
	} else
#endif
	{
#if (BMP_MODE)
		ForceExt(pat, fname, ".BMP");
		PIC_FILE file(pat);
		if (file._error == 0) {
			if (loadBMP(&file)) {
				Code();
				if (rem) {
					free(_m);
					_m = NULL;
				}
			} else
				error("Bad BMP [%s]", fname);
		}
#else
		error("Bad VBM [%s]", fname);
#endif
	}
}


Bitmap::Bitmap(uint16 w, uint16 h, uint8 *map) : _w(w), _h(h), _m(map), _v(NULL) {
	if (map)
		code();
}


// following routine creates filled rectangle
// immediately as VGA video chunks, in near memory as fast as possible,
// especially for text line real time display
Bitmap::Bitmap(uint16 w, uint16 h, uint8 fill)
	: _w((w + 3) & ~3),                              // only full uint32 allowed!
	  _h(h),
	  _m(NULL) {
	uint16 dsiz = _w >> 2;                           // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
	uint16 psiz = _h * lsiz;                         // - last gape, but + plane trailer
	uint8 *v = new uint8[4 * psiz + _h * sizeof(*_b)];// the same for 4 planes
	                                                // + room for wash table
	if (v == NULL)
		error("No core");

	*(uint16 *) v = CPY | dsiz;                     // data chunk hader
	memset(v + 2, fill, dsiz);                      // data bytes
	*(uint16 *)(v + lsiz - 2) = SKP | ((SCR_WID / 4) - dsiz);  // gap

	// Replicate lines
	byte *destP;
	for (destP = v + lsiz; destP < (v + psiz); destP += lsiz)
		Common::copy(v, v + lsiz, destP);

	*(uint16 *)(v + psiz - 2) = EOI;                // plane trailer uint16

	// Repliccate planes
	for (destP = v + psiz; destP < (v + 4 * psiz); destP += psiz)
		Common::copy(v, v + psiz, destP);

	HideDesc *b = (HideDesc *)(v + 4 * psiz);
	b->skip = (SCR_WID - _w) >> 2;
	b->hide = _w >> 2;
	memcpy(b + 1, b, (_h - 1) * sizeof(*b));         // tricky fill entire table
	b->skip = 0;                                    // fix the first entry
	_v = v;
	_b = b;
}


Bitmap::Bitmap(const Bitmap &bmp) : _w(bmp._w), _h(bmp._h), _m(NULL), _v(NULL) {
	uint8 *v0 = bmp._v;
	if (v0) {
		uint16 vsiz = (uint8 *)(bmp._b) - (uint8 *)(v0);
		uint16 siz = vsiz + _h * sizeof(HideDesc);
		uint8 *v1 = farnew(uint8, siz);
		if (v1 == NULL)
			error("No core");
		memcpy(v1, v0, siz);
		_b = (HideDesc *)((_v = v1) + vsiz);
	}
}


Bitmap::~Bitmap(void) {
	if (memType(_m) == FAR_MEM)
		free(_m);

	switch (memType(_v)) {
	case NEAR_MEM :
		delete[](uint8 *) _v;
		break;
	case FAR_MEM  :
		free(_v);
	default:
		warning("Unhandled MemType in Bitmap destructor");
		break;
		break;
	}
}


Bitmap &Bitmap::operator = (const Bitmap &bmp) {
	uint8 *v0 = bmp._v;
	_w = bmp._w;
	_h = bmp._h;
	_m = NULL;
	if (memType(_v) == FAR_MEM)
		free(_v);
	if (v0 == NULL)
		_v = NULL;
	else {
		uint16 vsiz = (uint8 *)bmp._b - (uint8 *)v0;
		uint16 siz = vsiz + _h * sizeof(HideDesc);
		uint8 *v1 = farnew(uint8, siz);
		if (v1 == NULL)
			error("No core");
		memcpy(v1, v0, siz);
		_b = (HideDesc *)((_v = v1) + vsiz);
	}
	return *this;
}


uint16 Bitmap::moveVmap(uint8 *buf) {
	if (_v) {
		uint16 vsiz = (uint8 *)_b - (uint8 *)_v;
		uint16 siz = vsiz + _h * sizeof(HideDesc);
		memcpy(buf, _v, siz);
		if (memType(_v) == FAR_MEM)
			free(_v);
		_b = (HideDesc *)((_v = buf) + vsiz);
		return siz;
	}
	return 0;
}


BMP_PTR Bitmap::code(void) {
	if (_m) {
		uint16 i, cnt;

		if (_v) {                                        // old X-map exists, so remove it
			switch (memType(_v)) {
			case NEAR_MEM :
				delete[](uint8 *) _v;
				break;
			case FAR_MEM  :
				free(_v);
				break;
			default:
				warning("Unhandled MemType in Bitmap::Code()");
				break;
			}
			_v = NULL;
		}

		while (true) {                                  // at most 2 times: for (V == NULL) & for allocated block;
			uint8 *im = _v + 2;
			uint16 *cp = (uint16 *) _v;
			int bpl;

			if (_v) {                                      // 2nd pass - fill the hide table
				for (i = 0; i < _h; i++) {
					_b[i].skip = 0xFFFF;
					_b[i].hide = 0x0000;
				}
			}
			for (bpl = 0; bpl < 4; bpl++) {              // once per each bitplane
				uint8 *bm = _m;
				bool skip = (bm[bpl] == TRANS);
				uint16 j;

				cnt = 0;
				for (i = 0; i < _h; i++) {                  // once per each line
					uint8 pix;
					for (j = bpl; j < _w; j += 4) {
						pix = bm[j];
						if (_v && pix != TRANS) {
							if (j < _b[i].skip)
								_b[i].skip = j;

							if (j >= _b[i].hide)
								_b[i].hide = j + 1;
						}
						if ((pix == TRANS) != skip || cnt >= 0x3FF0) { // end of block
							cnt |= (skip) ? SKP : CPY;
							if (_v)
								*cp = cnt;                          // store block description uint16

							cp = (uint16 *) im;
							im += 2;
							skip = (pix == TRANS);
							cnt = 0;
						}
						if (! skip) {
							if (_v)
								*im = pix;
							++ im;
						}
						++ cnt;
					}

					bm += _w;
					if (_w < SCR_WID) {
						if (skip) {
							cnt += (SCR_WID - j + 3) / 4;
						} else {
							cnt |= CPY;
							if (_v)
								*cp = cnt;

							cp = (uint16 *) im;
							im += 2;
							skip = true;
							cnt = (SCR_WID - j + 3) / 4;
						}
					}
				}
				if (cnt && ! skip) {
					cnt |= CPY;
					if (_v)
						*cp = cnt;

					cp = (uint16 *) im;
					im += 2;
				}
				if (_v)
					*cp = EOI;
				cp = (uint16 *) im;
				im += 2;
			}
			if (_v)
				break;

			uint16 sizV = (uint16)(im - 2 - _v);
			_v = farnew(uint8, sizV + _h * sizeof(*_b));
			if (!_v)
				error("No core");

			_b = (HideDesc *)(_v + sizV);
		}
		cnt = 0;
		for (i = 0; i < _h; i++) {
			if (_b[i].skip == 0xFFFF) {                    // whole line is skipped
				_b[i].skip = (cnt + SCR_WID) >> 2;
				cnt = 0;
			} else {
				uint16 s = _b[i].skip & ~3;
				uint16 h = (_b[i].hide + 3) & ~3;
				_b[i].skip = (cnt + s) >> 2;
				_b[i].hide = (h - s) >> 2;
				cnt = SCR_WID - h;
			}
		}
	}
	return this;
}


bool Bitmap::solidAt(int x, int y) {
	uint8 *m;
	uint16 r, n, n0;

	if ((x >= _w) || (y >= _h))
		return false;

	m = _v;
	r = static_cast<uint16>(x) % 4;
	n0 = (SCR_WID * y + x) / 4, n = 0;

	while (r) {
		uint16 w, t;

		w = *(uint16 *) m;
		m += 2;
		t = w & 0xC000;
		w &= 0x3FFF;

		switch (t) {
		case EOI :
			r--;
		case SKP :
			w = 0;
			break;
		case REP :
			w = 1;
			break;
		}
		m += w;
	}

	while (true) {
		uint16 w, t;

		w = *(uint16 *) m;
		m += 2;
		t = w & 0xC000;
		w &= 0x3FFF;

		if (n > n0)
			return false;

		n += w;
		switch (t) {
		case EOI :
			return false;
		case SKP :
			w = 0;
			break;
		case REP :
		case CPY :
			if (n - w <= n0 && n > n0)
				return true;
			break;
		}
		m += ((t == REP) ? 1 : w);
	}
}


bool Bitmap::saveVBM(XFile *f) {
	uint16 p = (_pal != NULL),
	       n = ((uint16)(((uint8 *)_b) - _v)) + _h * sizeof(HideDesc);
	if (f->_error == 0)
		f->write((uint8 *)&p, sizeof(p));

	if (f->_error == 0)
		f->write((uint8 *)&n, sizeof(n));

	if (f->_error == 0)
		f->write((uint8 *)&_w, sizeof(_w));

	if (f->_error == 0)
		f->write((uint8 *)&_h, sizeof(_h));

	if (f->_error == 0)
		if (p)
			f->write((uint8 *)_pal, 256 * 3);

	if (f->_error == 0)
		f->write(_v, n);

	return (f->_error == 0);
}


bool Bitmap::loadVBM(XFile *f) {
	uint16 p = 0, n = 0;
	if (f->_error == 0)
		f->read((uint8 *)&p, sizeof(p));

	if (f->_error == 0)
		f->read((uint8 *)&n, sizeof(n));

	if (f->_error == 0)
		f->read((uint8 *)&_w, sizeof(_w));

	if (f->_error == 0)
		f->read((uint8 *)&_h, sizeof(_h));

	if (f->_error == 0) {
		if (p) {
			if (_pal) {
				// Read in the palette
				byte palData[PAL_SIZ];
				f->read(palData, PAL_SIZ);
				
				const byte *srcP = palData;
				for (int idx = 0; idx < PAL_CNT; ++idx, srcP += 3) {
					_pal[idx]._r = *srcP;
					_pal[idx]._g = *(srcP + 1);
					_pal[idx]._b = *(srcP + 2);
				}
			} else
				f->seek(f->mark() + PAL_SIZ);
		}
	}
	if ((_v = farnew(uint8, n)) == NULL)
		return false;

	if (f->_error == 0)
		f->read(_v, n);

	_b = (HideDesc *)(_v + n - _h * sizeof(HideDesc));
	return (f->_error == 0);
}

bool Bitmap::loadBMP(XFile *f) {
  struct {
	   char BM[2];
	   union { int16 len; int32 len_; };
	   union { int16 _06; int32 _06_; };
	   union { int16 hdr; int32 hdr_; };
	   union { int16 _0E; int32 _0E_; };
	   union { int16 wid; int32 wid_; };
	   union { int16 hig; int32 hig_; };
	   union { int16 _1A; int32 _1A_; };
	   union { int16 _1E; int32 _1E_; };
	   union { int16 _22; int32 _22_; };
	   union { int16 _26; int32 _26_; };
	   union { int16 _2A; int32 _2A_; };
	   union { int16 _2E; int32 _2E_; };
	   union { int16 _32; int32 _32_; };
	 } hea;

  Bgr4 bpal[256];

  f->read((byte *)&hea, sizeof(hea));
  if (f->_error == 0) {
      if (hea.hdr == 0x436L) {
	  int16 i = (hea.hdr - sizeof(hea)) / sizeof(Bgr4);
	  f->read((byte *)&bpal, sizeof(bpal));
	  if (f->_error == 0) {
		if (_pal) {
			for (i = 0; i < 256; i ++) {
				_pal[i]._r = bpal[i]._R;
				_pal[i]._g = bpal[i]._G;
				_pal[i]._b = bpal[i]._B;
			}
			_pal = NULL;
		}
	      _h = hea.hig;
	      _w = hea.wid;
	      if ((_m = farnew(byte, _h * _w)) != NULL) {
		  int16 r = (4 - (hea.wid & 3)) % 4;
		  byte buf[3];
		  for (i = _h - 1; i >= 0; i--) {
		      f->read(_m + (_w * i), _w);
		      if (r && f->_error == 0)
				  f->read(buf, r);
		      if (f->_error)
				  break;
		  }
		  if (i < 0)
			  return true;
		}
	    }
	}
    }
  return false;
}

} // End of namespace CGE
