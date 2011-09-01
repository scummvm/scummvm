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

#include "cge/general.h"
#include "cge/talk.h"
#include "cge/vol.h"
#include "cge/game.h"
#include "cge/events.h"

namespace CGE {
Font::Font(const char *name) {
	_map = (uint8 *) malloc(sizeof(uint8) * kMapSize);
	_pos = (uint16 *) malloc(sizeof(uint16) * kPosSize);
	_wid = (uint8 *) malloc(sizeof(uint8) * kWidSize);
	assert((_map != NULL) && (_pos != NULL) && (_wid != NULL));
	mergeExt(_path, name, kFontExt);
	load();
}


Font::~Font() {
	free(_map);
	free(_pos);
	free(_wid);
}


void Font::load() {
	INI_FILE f(_path);
	if (!f._error) {
		f.read(_wid, kWidSize);
		if (!f._error) {
			uint16 i, p = 0;
			for (i = 0; i < kPosSize; i++) {
				_pos[i] = p;
				p += _wid[i];
			}
			f.read(_map, p);
		}
	}
}


uint16 Font::width(const char *text) {
	uint16 w = 0;
	if (text)
		while (* text)
			w += _wid[(unsigned char)*(text++)];
	return w;
}

Talk::Talk(CGEEngine *vm, const char *text, TextBoxStyle mode)
	: Sprite(vm, NULL), _mode(mode), _vm(vm) {
	
	_ts = NULL;
	_flags._syst = true;
	update(text);
}


Talk::Talk(CGEEngine *vm)
	: Sprite(vm, NULL), _mode(kTBPure), _vm(vm) {
	_ts = NULL;
	_flags._syst = true;
}

Font *Talk::_font;

void Talk::init() {
	_font = new Font(progName());
}

void Talk::deinit() {
	delete _font;
}


void Talk::update(const char *text) {
	uint16 vmarg = (_mode) ? kTextVMargin : 0;
	uint16 hmarg = (_mode) ? kTextHMargin : 0;
	uint16 mw = 0;
	uint16 ln = vmarg;
	const char *p;
	uint8 *m;

	if (!_ts) {
		uint16 k = 2 * hmarg;
		uint16 mh = 2 * vmarg + kFontHigh;
		for (p = text; *p; p++) {
			if (*p == '|' || *p == '\n') {
				mh += kFontHigh + kTextLineSpace;
				if (k > mw)
					mw = k;
				k = 2 * hmarg;
			} else
				k += _font->_wid[(unsigned char)*p];
		}
		if (k > mw)
			mw = k;

		_ts = new BitmapPtr[2];
		_ts[0] = box(mw, mh);
		_ts[1] = NULL;
	}

	m = _ts[0]->_m + ln * mw + hmarg;

	while (*text) {
		if (*text == '|' || *text == '\n')
			m = _ts[0]->_m + (ln += kFontHigh + kTextLineSpace) * mw + hmarg;
		else {
			int cw = _font->_wid[(unsigned char)*text], i;
			uint8 *f = _font->_map + _font->_pos[(unsigned char)*text];
			for (i = 0; i < cw; i++) {
				uint8 *pp = m;
				uint16 n;
				register uint16 b = *(f++);
				for (n = 0; n < kFontHigh; n++) {
					if (b & 1)
						*pp = kTextColFG;
					b >>= 1;
					pp += mw;
				}
				m++;
			}
		}
		text++;
	}
	_ts[0]->code();
	setShapeList(_ts);
}

Bitmap *Talk::box(uint16 w, uint16 h) {
	uint8 *b, * p, * q;
	uint16 r = (_mode == kTBRound) ? kTextRoundCorner : 0;

	if (w < 8)
		w = 8;
	if (h < 8)
		h = 8;
	uint16 n = w * h;
	b = (uint8 *) malloc(sizeof(uint8) * n);
	assert(b != NULL);
	memset(b, kTextColBG, n);

	if (_mode) {
		p = b;
		q = b + n - w;
		memset(p, LGRAY, w);
		memset(q, DGRAY, w);
		while (p < q) {
			p += w;
			*(p - 1) = DGRAY;
			*p = LGRAY;
		}
		p = b;
		for (int i = 0; i < r; i++) {
			int j;
			for (j = 0; j < r - i; j++) {
				p[j] = kPixelTransp;
				p[w - j - 1] = kPixelTransp;
				q[j] = kPixelTransp;
				q[w - j - 1] = kPixelTransp;
			}
			p[j] = LGRAY;
			p[w - j - 1] = DGRAY;
			q[j] = LGRAY;
			q[w - j - 1] = DGRAY;
			p += w;
			q -= w;
		}
	}
	return new Bitmap(w, h, b);
}


void Talk::putLine(int line, const char *text) {
// Note: (TS[0].W % 4) have to be 0
	uint16 w = _ts[0]->_w;
	uint16 h = _ts[0]->_h;
	uint8 *v = _ts[0]->_v, * p;
	uint16 dsiz = w >> 2;     // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;   // uint16 for line header, uint16 for gap
	uint16 psiz = h * lsiz;       // - last gap, but + plane trailer
	uint16 size = 4 * psiz;       // whole map size
	uint16 rsiz = kFontHigh * lsiz;    // length of whole text row map

	// set desired line pointer
	v += (kTextVMargin + (kFontHigh + kTextLineSpace) * line) * lsiz;
	p = v;                // assume blanked line above text

	// clear whole rectangle
	assert((rsiz % lsiz) == 0);
	for (int planeCtr = 0; planeCtr < 4; planeCtr++, p += psiz) {
		for (byte *pDest = p; pDest < (p + (rsiz - lsiz)); pDest += lsiz)
			Common::copy(p - lsiz, p, pDest);
	}

	// paint text line
	if (text) {
		uint8 *q;
		p = v + 2 + kTextHMargin / 4 + (kTextHMargin % 4) * psiz;
		q = v + size;

		while (* text) {
			uint16 cw = _font->_wid[(unsigned char)*text], i;
			uint8 *fp = _font->_map + _font->_pos[(unsigned char)*text];

			for (i = 0; i < cw; i++) {
				register uint16 b = fp[i];
				uint16 n;
				for (n = 0; n < kFontHigh; n++) {
					if (b & 1)
						*p = kTextColFG;
					b >>= 1;
					p += lsiz;
				}
				p = p - rsiz + psiz;
				if (p >= q)
					p = p - size + 1;
			}
			text++;
		}
	}
}


InfoLine::InfoLine(CGEEngine *vm, uint16 w) : Talk(vm), _oldText(NULL), _vm(vm) {
	if (!_ts) {
		_ts = new BitmapPtr[2];
		_ts[1] = NULL;
	}

	_ts[0] = new Bitmap(w, kFontHigh, kTextColBG);
	setShapeList(_ts);
}


void InfoLine::update(const char *text) {
	if (text != _oldText) {
		uint16 w = _ts[0]->_w;
		uint16 h = _ts[0]->_h;
		uint8 *v = (uint8 *) _ts[0]->_v;
		uint16 dsiz = w >> 2;                           // data size (1 plane line size)
		uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
		uint16 psiz = h * lsiz;                         // - last gape, but + plane trailer
		uint16 size = 4 * psiz;                         // whole map size

		// clear whole rectangle
		byte *pDest;
		memset(v + 2, kTextColBG, dsiz);                // data bytes
		for (pDest = v + lsiz; pDest < (v + psiz); pDest += lsiz) {
			Common::copy(v, v + lsiz, pDest);
		}
		*(uint16 *)(v + psiz - 2) = TO_LE_16(kBmpEOI);              // plane trailer uint16
		for (pDest = v + psiz; pDest < (v + 4 * psiz); pDest += psiz) {
			Common::copy(v, v + psiz, pDest);
		}

		// paint text line
		if (text) {
			uint8 *p = v + 2, * q = p + size;

			while (*text) {
				uint16 cw = _font->_wid[(unsigned char)*text];
				uint8 *fp = _font->_map + _font->_pos[(unsigned char)*text];

				for (uint16 i = 0; i < cw; i++) {
					register uint16 b = fp[i];
					for (uint16 n = 0; n < kFontHigh; n++) {
						if (b & 1)
							*p = kTextColFG;
						b >>= 1;
						p += lsiz;
					}
					if (p >= q)
						p = p - size + 1;
				}
				text++;
			}
		}
		_oldText = text;
	}
}

} // End of namespace CGE
