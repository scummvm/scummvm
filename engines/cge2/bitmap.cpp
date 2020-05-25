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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "cge2/bitmap.h"
#include "cge2/cge2.h"
#include "cge2/vga13h.h"
#include "cge2/talk.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"

namespace CGE2 {

Bitmap::Bitmap() : _w(0), _h(0), _v(nullptr), _b(nullptr), _map(0), _vm(nullptr) {
}

void Bitmap::setVM(CGE2Engine *vm) {
	_vm = vm;
}

Bitmap::Bitmap(CGE2Engine *vm, const char *fname) : _w(0), _h(0), _v(nullptr), _b(nullptr), _map(0), _vm(vm) {
	Common::String path;

	if (!strcmp(fname, "04tal201")) {
		path = "04tal202";
		warning("Workaround for missing VBM: 04tal201");
	} else if (!strcmp(fname, "11oqlist-")) {
		path = "11oqlist";
		warning("Workaround for wrong VBM name: 11oqlist-");
	} else
		path = fname;

	path = setExtension(path, ".VBM");

	if (_vm->_resman->exist(path.c_str())) {
		EncryptedStream file(_vm, path.c_str());
		if (file.err())
			error("Unable to find VBM [%s]", fname);
		if (!loadVBM(&file))
			error("Bad VBM [%s]", fname);
	} else {
		warning("Missing VBM [%s]", path.c_str());
	}
}

Bitmap::Bitmap(CGE2Engine *vm, uint16 w, uint16 h, uint8 *map) : _w(w), _h(h), _v(nullptr), _map(0), _b(nullptr), _vm(vm) {
	if (map)
		code(map);
}

// following routine creates filled rectangle
// immediately as VGA video chunks, in near memory as fast as possible,
// especially for text line real time display
Bitmap::Bitmap(CGE2Engine *vm, uint16 w, uint16 h, uint8 fill)
	: _w((w + 3) & ~3),                              // only full uint32 allowed!
	  _h(h), _map(0), _b(nullptr), _vm(vm) {

	uint16 dsiz = _w >> 2;                           // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
	uint16 psiz = _h * lsiz;                         // - last gape, but + plane trailer
	uint8 *v = new uint8[4 * psiz + _h * sizeof(*_b)];// the same for 4 planes
	                                                // + room for wash table

	WRITE_LE_UINT16(v, (kBmpCPY | dsiz));                 // data chunk hader
	memset(v + 2, fill, dsiz);                      // data bytes
	WRITE_LE_UINT16(v + lsiz - 2, (kBmpSKP | ((kScrWidth / 4) - dsiz)));  // gap

	// Replicate lines
	byte *destP;
	for (destP = v + lsiz; destP < (v + psiz); destP += lsiz)
		Common::copy(v, v + lsiz, destP);

	WRITE_LE_UINT16(v + psiz - 2, kBmpEOI);            // plane trailer uint16

	// Replicate planes
	for (destP = v + psiz; destP < (v + 4 * psiz); destP += psiz)
		Common::copy(v, v + psiz, destP);

	HideDesc *b = (HideDesc *)(v + 4 * psiz);
	b->_skip = (kScrWidth - _w) >> 2;
	b->_hide = _w >> 2;

	// Replicate across the entire table
	for (HideDesc *hdP = b + 1; hdP < (b + _h); hdP++)
		*hdP = *b;

	b->_skip = 0;                                    // fix the first entry
	_v = v;
	_b = b;
}

Bitmap::Bitmap(CGE2Engine *vm, const Bitmap &bmp) : _w(bmp._w), _h(bmp._h), _v(nullptr), _map(0), _b(nullptr), _vm(vm) {
	uint8 *v0 = bmp._v;
	if (!v0)
		return;

	uint16 vsiz = (uint8 *)(bmp._b) - (uint8 *)(v0);
	uint16 siz = vsiz + _h * sizeof(HideDesc);
	uint8 *v1 = new uint8[siz];
	memcpy(v1, v0, siz);
	_b = (HideDesc *)((_v = v1) + vsiz);
}

Bitmap::~Bitmap() {
	release();
}

void Bitmap::release() {
	if (_v != nullptr)
		delete[] _v;
	_v = nullptr;
}

Bitmap &Bitmap::operator=(const Bitmap &bmp) {
	if (this == &bmp)
		return *this;

	uint8 *v0 = bmp._v;
	_w = bmp._w;
	_h = bmp._h;
	_map = 0;
	_vm = bmp._vm;
	delete[] _v;
	_v = nullptr;

	if (v0) {
		uint16 vsiz = (uint8 *)bmp._b - (uint8 *)v0;
		uint16 siz = vsiz + _h * sizeof(HideDesc);
		uint8 *v1 = new uint8[siz];
		memcpy(v1, v0, siz);
		_b = (HideDesc *)((_v = v1) + vsiz);
	}
	return *this;
}

// Blatant rip from hopkins engine where it's ripped from gob engine. Hi DrMcCoy, hi Strangerke! ;>
Common::String Bitmap::setExtension(const Common::String &str, const Common::String &ext) {
	if (str.empty())
		return str;

	const char *dot = strrchr(str.c_str(), '.');
	if (dot)
		return Common::String(str.c_str(), dot - str.c_str()) + ext;

	return str + ext;
}

BitmapPtr Bitmap::code(uint8 *map) {
	if (!map)
		return nullptr;

	uint16 cnt;

	if (_v) {                                        // old X-map exists, so remove it
		delete[] _v;
		_v = nullptr;
	}

	while (true) {                                  // at most 2 times: for (V == NULL) & for allocated block;
		uint8 *im = _v + 2;
		uint16 *cp = (uint16 *) _v;

		if (_v) {                                      // 2nd pass - fill the hide table
			for (uint i = 0; i < _h; i++) {
				_b[i]._skip = 0xFFFF;
				_b[i]._hide = 0x0000;
			}
		}
		for (int bpl = 0; bpl < 4; bpl++) {              // once per each bitplane
			uint8 *bm = map;
			bool skip = (bm[bpl] == kPixelTransp);
			uint16 j;

			cnt = 0;
			for (uint i = 0; i < _h; i++) {                  // once per each line
				uint8 pix;
				for (j = bpl; j < _w; j += 4) {
					pix = bm[j];
					if (_v && (pix != kPixelTransp)) {
						if (j < _b[i]._skip)
							_b[i]._skip = j;

						if (j >= _b[i]._hide)
							_b[i]._hide = j + 1;
					}
					if (((pix == kPixelTransp) != skip) || (cnt >= 0x3FF0)) { // end of block
						cnt |= (skip) ? kBmpSKP : kBmpCPY;
						if (_v)
							WRITE_LE_UINT16(cp, cnt); // store block description uint16

						cp = (uint16 *) im;
						im += 2;
						skip = (pix == kPixelTransp);
						cnt = 0;
					}
					if (!skip) {
						if (_v)
							*im = pix;
						im++;
					}
					cnt++;
				}

				bm += _w;
				if (_w < kScrWidth) {
					if (skip)
						cnt += (kScrWidth - j + 3) / 4;
					else {
						cnt |= kBmpCPY;
						if (_v)
							WRITE_LE_UINT16(cp, cnt);

						cp = (uint16 *) im;
						im += 2;
						skip = true;
						cnt = (kScrWidth - j + 3) / 4;
					}
				}
			}
			if (cnt && ! skip) {
				cnt |= kBmpCPY;
				if (_v)
					WRITE_LE_UINT16(cp, cnt);

				cp = (uint16 *) im;
				im += 2;
			}
			if (_v)
				WRITE_LE_UINT16(cp, kBmpEOI);
			cp = (uint16 *) im;
			im += 2;
		}
		if (_v)
			break;

		uint16 sizV = (uint16)(im - 2 - _v);
		_v = new uint8[sizV + _h * sizeof(*_b)];
		_b = (HideDesc *)(_v + sizV);
	}
	cnt = 0;
	for (uint i = 0; i < _h; i++) {
		if (_b[i]._skip == 0xFFFF) {                    // whole line is skipped
			_b[i]._skip = (cnt + kScrWidth) >> 2;
			cnt = 0;
		} else {
			uint16 s = _b[i]._skip & ~3;
			uint16 h = (_b[i]._hide + 3) & ~3;
			_b[i]._skip = (cnt + s) >> 2;
			_b[i]._hide = (h - s) >> 2;
			cnt = kScrWidth - h;
		}
	}

	return this;
}

bool Bitmap::solidAt(V2D pos) {
	pos.x += _w >> 1;
	pos.y = _h - pos.y;

	if (!pos.limited(V2D(_vm, _w, _h)))
		return false;

	uint8 *m = _v;
	uint16 r = static_cast<uint16>(pos.x) % 4;
	uint16 n0 = (kScrWidth * pos.y + pos.x) / 4;
	uint16 n = 0;

	while (r) {
		uint16 w, t;

		w = READ_LE_UINT16(m);
		m += 2;
		t = w & 0xC000;
		w &= 0x3FFF;

		switch (t) {
		case kBmpEOI:
			r--;
			// fall through
		case kBmpSKP:
			w = 0;
			break;
		case kBmpREP:
			w = 1;
			break;
		case kBmpCPY:
		default:
			break;
		}
		m += w;
	}

	while (true) {
		uint16 w, t;

		w = READ_LE_UINT16(m);
		m += 2;
		t = w & 0xC000;
		w &= 0x3FFF;

		if (n > n0)
			return false;

		n += w;
		switch (t) {
		default:
		case kBmpEOI:
			return false;
		case kBmpSKP:
			w = 0;
			break;
		case kBmpREP:
		case kBmpCPY:
			if ((n - w <= n0) && (n > n0))
				return true;
			break;
		}
		m += ((t == kBmpREP) ? 1 : w);
	}
}

bool Bitmap::loadVBM(EncryptedStream *f) {
	uint16 p = 0, n = 0;
	if (!f->err())
		f->read((uint8 *)&p, sizeof(p));
	p = FROM_LE_16(p);

	if (!f->err())
		f->read((uint8 *)&n, sizeof(n));
	n = FROM_LE_16(n);

	if (!f->err())
		f->read((uint8 *)&_w, sizeof(_w));
	_w = FROM_LE_16(_w);

	if (!f->err())
		f->read((uint8 *)&_h, sizeof(_h));
	_h = FROM_LE_16(_h);

	if (!f->err()) {
		if (p) {
			if (_vm->_bitmapPalette) {
				// Read in the palette
				byte palData[kPalSize];
				f->read(palData, kPalSize);

				const byte *srcP = palData;
				for (int idx = 0; idx < kPalCount; idx++, srcP += 3) {
					_vm->_bitmapPalette[idx]._r = *srcP;
					_vm->_bitmapPalette[idx]._g = *(srcP + 1);
					_vm->_bitmapPalette[idx]._b = *(srcP + 2);
				}
			} else
				f->seek(f->pos() + kPalSize);
		}
	}
	_v = new uint8[n];

	if (!f->err())
		f->read(_v, n);

	_b = (HideDesc *)(_v + n - _h * sizeof(HideDesc));
	return (!f->err());
}

void Bitmap::xLatPos(V2D& p) {
	p.x -= (_w >> 1);
	p.y = kWorldHeight - p.y - _h;
}

#define	_    kPixelTransp,
#define L               1,
#define G               2,
#define D               3,
#define kDesignSize   240

uint8 *Bitmap::makeSpeechBubbleTail(int which, uint8 colorSet[][4]) {
	static const uint8 kSLDesign[kDesignSize] = {
		G G G G G G G G G _ _ _ _ _ _
		L G G G G G G G G D _ _ _ _ _
		_ L G G G G G G G D _ _ _ _ _
		_ _ L G G G G G G G D _ _ _ _
		_ _ _ L G G G G G G D _ _ _ _
		_ _ _ _ L G G G G G D _ _ _ _
		_ _ _ _ _ L G G G G G D _ _ _
		_ _ _ _ _ _ L G G G G D _ _ _
		_ _ _ _ _ _ _ L G G G D _ _ _
		_ _ _ _ _ _ _ _ L G G G D _ _
		_ _ _ _ _ _ _ _ _ L G G D _ _
		_ _ _ _ _ _ _ _ _ _ L G D _ _
		_ _ _ _ _ _ _ _ _ _ _ L G D _
		_ _ _ _ _ _ _ _ _ _ _ _ L D _
		_ _ _ _ _ _ _ _ _ _ _ _ _ L D
		_ _ _ _ _ _ _ _ _ _ _ _ _ _ D
	};

	static const uint8 kSRDesign[kDesignSize] = {
		_ _ _ _ _ _ G G G G G G G G G
		_ _ _ _ _ L G G G G G G G G D
		_ _ _ _ _ L G G G G G G G D _
		_ _ _ _ L G G G G G G G D _ _
		_ _ _ _ L G G G G G G D _ _ _
		_ _ _ _ L G G G G G D _ _ _ _
		_ _ _ L G G G G G D _ _ _ _ _
		_ _ _ L G G G G D _ _ _ _ _ _
		_ _ _ L G G G D _ _ _ _ _ _ _
		_ _ L G G G D _ _ _ _ _ _ _ _
		_ _ L G G D _ _ _ _ _ _ _ _ _
		_ _ L G D _ _ _ _ _ _ _ _ _ _
		_ L G D _ _ _ _ _ _ _ _ _ _ _
		_ L D _ _ _ _ _ _ _ _ _ _ _ _
		L D _ _ _ _ _ _ _ _ _ _ _ _ _
		D _ _ _ _ _ _ _ _ _ _ _ _ _ _
	};

	uint8 *des = new uint8[kDesignSize];
	switch (which) {
	case 0:
		memcpy(des, kSLDesign, sizeof(kSLDesign));
		break;
	case 1:
		memcpy(des, kSRDesign, sizeof(kSRDesign));
		break;
	default:
		error("Wrong parameter in Bitmap::makeSpeechBubbleTail!");
		break;
	}

	for (int i = 0; i < kDesignSize; i++) {
		if ((des[i] >= 1) && (des[i] <= 3))
			des[i] = colorSet[kCBSay][des[i]];
	}

	return des;
}

} // End of namespace CGE2
