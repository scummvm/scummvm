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

#define WID_SIZ     256
#define POS_SIZ     256
#define MAP_SIZ     (256*8)

//uint8 FONT::Wid[WID_SIZ];
//uint16    FONT::Pos[POS_SIZ];
//uint8 FONT::Map[MAP_SIZ];


Font::Font(const char *name) {
	_map = farnew(uint8, MAP_SIZ);
	_pos = farnew(uint16, POS_SIZ);
	_wid = farnew(uint8, WID_SIZ);
	if ((_map == NULL) || (_pos == NULL) || (_wid == NULL))
		error("No core");
	mergeExt(_path, name, FONT_EXT);
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
		f.read(_wid, WID_SIZ);
		if (!f._error) {
			uint16 i, p = 0;
			for (i = 0; i < POS_SIZ; i++) {
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


/*
void Font::save() {
	CFILE f((const char *) _path, WRI);
	if (!f._error) {
		f.Write(_wid, WID_SIZ);
		if (!f._error)
			f.Write(_map, _pos[POS_SIZ - 1] + _wid[WID_SIZ - 1]);
	}
}
*/


Talk::Talk(CGEEngine *vm, const char *tx, TBOX_STYLE mode)
	: Sprite(vm, NULL), _mode(mode), _vm(vm) {
	
	_ts = NULL;
	_flags._syst = true;
	update(tx);
}


Talk::Talk(CGEEngine *vm)
	: Sprite(vm, NULL), _mode(PURE), _vm(vm) {
	_ts = NULL;
	_flags._syst = true;
}


/*
Talk::~Talk() {
	for (uint16 i = 0; i < ShpCnt; i++) {
		if (FP_SEG(_shpList[i]) != _DS) { // small model: always false
			delete _shpList[i];
			ShpList[i] = NULL;
		}
	}
}
*/

Font *Talk::_font;

void Talk::init() {
	_font = new Font(progName());
}

void Talk::deinit() {
	delete _font;
}


void Talk::update(const char *tx) {
	uint16 vmarg = (_mode) ? TEXT_VM : 0;
	uint16 hmarg = (_mode) ? TEXT_HM : 0;
	uint16 mw = 0, mh, ln = vmarg;
	const char *p;
	uint8 *m;

	if (!_ts) {
		uint16 k = 2 * hmarg;
		mh = 2 * vmarg + FONT_HIG;
		for (p = tx; *p; p++) {
			if (*p == '|' || *p == '\n') {
				mh += FONT_HIG + TEXT_LS;
				if (k > mw)
					mw = k;
				k = 2 * hmarg;
			} else
				k += _font->_wid[(unsigned char)*p];
		}
		if (k > mw)
			mw = k;

		_ts = new BMP_PTR[2];
		_ts[0] = box(mw, mh);
		_ts[1] = NULL;
	}

	m = _ts[0]->_m + ln * mw + hmarg;

	while (* tx) {
		if (*tx == '|' || *tx == '\n')
			m = _ts[0]->_m + (ln += FONT_HIG + TEXT_LS) * mw + hmarg;
		else {
			int cw = _font->_wid[(unsigned char)*tx], i;
			uint8 *f = _font->_map + _font->_pos[(unsigned char)*tx];
			for (i = 0; i < cw; i++) {
				uint8 *pp = m;
				uint16 n;
				register uint16 b = *(f++);
				for (n = 0; n < FONT_HIG; n++) {
					if (b & 1)
						*pp = TEXT_FG;
					b >>= 1;
					pp += mw;
				}
				m++;
			}
		}
		tx++;
	}
	_ts[0]->code();
	setShapeList(_ts);
}




Bitmap *Talk::box(uint16 w, uint16 h) {
	uint8 *b, * p, * q;
	uint16 n, r = (_mode == ROUND) ? TEXT_RD : 0;

	if (w < 8)
		w = 8;
	if (h < 8)
		h = 8;
	b = farnew(uint8, n = w * h);
	if (! b)
		error("No core");
	memset(b, TEXT_BG, n);

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
				p[j] = TRANS;
				p[w - j - 1] = TRANS;
				q[j] = TRANS;
				q[w - j - 1] = TRANS;
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
	uint16 rsiz = FONT_HIG * lsiz;    // length of whole text row map

	// set desired line pointer
	v += (TEXT_VM + (FONT_HIG + TEXT_LS) * line) * lsiz;
	p = v;                // assume blanked line above text

	// clear whole rectangle
	assert((rsiz % lsiz) == 0);
	for (int planeCtr = 0; planeCtr < 4; ++planeCtr, p += psiz) {
		for (byte *pDest = p; pDest < (p + (rsiz - lsiz)); pDest += lsiz)
			Common::copy(p - lsiz, p, pDest);
	}

	// paint text line
	if (text) {
		uint8 *q;
		p = v + 2 + TEXT_HM / 4 + (TEXT_HM % 4) * psiz;
		q = v + size;

		while (* text) {
			uint16 cw = _font->_wid[(unsigned char)*text], i;
			uint8 *fp = _font->_map + _font->_pos[(unsigned char)*text];

			for (i = 0; i < cw; i++) {
				register uint16 b = fp[i];
				uint16 n;
				for (n = 0; n < FONT_HIG; n++) {
					if (b & 1)
						*p = TEXT_FG;
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


InfoLine::InfoLine(CGEEngine *vm, uint16 w) : Talk(vm), _oldTxt(NULL), _vm(vm) {
	if (!_ts) {
		_ts = new BMP_PTR[2];
		_ts[1] = NULL;
	}

	_ts[0] = new Bitmap(w, FONT_HIG, TEXT_BG);
	setShapeList(_ts);
}


void InfoLine::update(const char *tx) {
	if (tx != _oldTxt) {
		uint16 w = _ts[0]->_w;
		uint16 h = _ts[0]->_h;
		uint8 *v = (uint8 *) _ts[0]->_v;
		uint16 dsiz = w >> 2;                           // data size (1 plane line size)
		uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
		uint16 psiz = h * lsiz;                         // - last gape, but + plane trailer
		uint16 size = 4 * psiz;                         // whole map size

		// clear whole rectangle
		byte *pDest;
		memset(v + 2, TEXT_BG, dsiz);                   // data bytes
		for (pDest = v + lsiz; pDest < (v + psiz); pDest += lsiz) {
			Common::copy(v, v + lsiz, pDest);
		}
		*(uint16 *)(v + psiz - 2) = EOI;               // plane trailer uint16
		for (pDest = v + psiz; pDest < (v + 4 * psiz); pDest += psiz) {
			Common::copy(v, v + psiz, pDest);
		}

		// paint text line
		if (tx) {
			uint8 *p = v + 2, * q = p + size;

			while (*tx) {
				uint16 cw = _font->_wid[(unsigned char)*tx];
				uint8 *fp = _font->_map + _font->_pos[(unsigned char)*tx];

				for (uint16 i = 0; i < cw; i++) {
					register uint16 b = fp[i];
					for (uint16 n = 0; n < FONT_HIG; n++) {
						if (b & 1)
							*p = TEXT_FG;
						b >>= 1;
						p += lsiz;
					}
					if (p >= q)
						p = p - size + 1;
				}
				tx++;
			}
		}
		_oldTxt = tx;
	}
}

} // End of namespace CGE
