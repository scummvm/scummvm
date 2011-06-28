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
#include "cge/mouse.h"

namespace CGE {

#define WID_SIZ     256
#define POS_SIZ     256
#define MAP_SIZ     (256*8)

//uint8 FONT::Wid[WID_SIZ];
//uint16    FONT::Pos[POS_SIZ];
//uint8 FONT::Map[MAP_SIZ];


FONT::FONT(const char *name) {
	Map = farnew(uint8, MAP_SIZ);
	Pos = farnew(uint16, POS_SIZ);
	Wid = farnew(uint8, WID_SIZ);
	if ((Map == NULL) || (Pos == NULL) || (Wid == NULL))
		error("No core");
	MergeExt(Path, name, FONT_EXT);
	Load();
}


FONT::~FONT(void) {
	free(Map);
	free(Pos);
	free(Wid);
}


void FONT::Load(void) {
	INI_FILE f(Path);
	if (! f.Error) {
		f.Read(Wid, WID_SIZ);
		if (! f.Error) {
			uint16 i, p = 0;
			for (i = 0; i < POS_SIZ; i++) {
				Pos[i] = p;
				p += Wid[i];
			}
			f.Read(Map, p);
		}
	}
}


uint16 FONT::Width(const char *text) {
	uint16 w = 0;
	if (text)
		while (* text)
			w += Wid[*(text++)];
	return w;
}


/*
void FONT::Save(void) {
	CFILE f((const char *) Path, WRI);
	if (! f.Error) {
		f.Write(Wid, WID_SIZ);
		if (! f.Error)
			f.Write(Map, Pos[POS_SIZ - 1] + Wid[WID_SIZ - 1]);
	}
}
*/


TALK::TALK(CGEEngine *vm, const char *tx, TBOX_STYLE mode)
	: SPRITE(vm, NULL), Mode(mode), _vm(vm) {
	TS[0] = TS[1] = NULL;
	Flags.Syst = true;
	Update(tx);
}


TALK::TALK(CGEEngine *vm)
	: SPRITE(vm, NULL), Mode(PURE), _vm(vm) {
	TS[0] = TS[1] = NULL;
	Flags.Syst = true;
}


/*
TALK::~TALK (void) {
	for (uint16 i = 0; i < ShpCnt; i++) {
		if (FP_SEG(ShpList[i]) != _DS) { // small model: always false
			delete ShpList[i];
			ShpList[i] = NULL;
		}
	}
}
*/

FONT *TALK::_Font;

void TALK::init() {
	_Font = new FONT(ProgName());
}

void TALK::deinit() {
	delete _Font;
}


void TALK::Update(const char *tx) {
	uint16 vmarg = (Mode) ? TEXT_VM : 0;
	uint16 hmarg = (Mode) ? TEXT_HM : 0;
	uint16 mw = 0, mh, ln = vmarg;
	const char *p;
	uint8 *m;

	if (!TS[0]) {
		uint16 k = 2 * hmarg;
		mh = 2 * vmarg + FONT_HIG;
		for (p = tx; *p; p++) {
			if (*p == '|' || *p == '\n') {
				mh += FONT_HIG + TEXT_LS;
				if (k > mw)
					mw = k;
				k = 2 * hmarg;
			} else
				k += _Font->Wid[*p];
		}
		if (k > mw)
			mw = k;
		TS[0] = Box(mw, mh);
	}

	m = TS[0]->M + ln * mw + hmarg;

	while (* tx) {
		if (*tx == '|' || *tx == '\n')
			m = TS[0]->M + (ln += FONT_HIG + TEXT_LS) * mw + hmarg;
		else {
			int cw = _Font->Wid[*tx], i;
			uint8 *f = _Font->Map + _Font->Pos[*tx];
			for (i = 0; i < cw; i++) {
				uint8 *p = m;
				uint16 n;
				register uint16 b = *(f++);
				for (n = 0; n < FONT_HIG; n++) {
					if (b & 1)
						*p = TEXT_FG;
					b >>= 1;
					p += mw;
				}
				++m;
			}
		}
		++tx;
	}
	TS[0]->Code();
	SetShapeList(TS);
}




BITMAP *TALK::Box(uint16 w, uint16 h) {
	uint8 *b, * p, * q;
	uint16 n, r = (Mode == ROUND) ? TEXT_RD : 0;

	if (w < 8)
		w = 8;
	if (h < 8)
		h = 8;
	b = farnew(uint8, n = w * h);
	if (! b)
		error("No core");
	memset(b, TEXT_BG, n);

	if (Mode) {
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
	return new BITMAP(w, h, b);
}


void TALK::PutLine(int line, const char *text) {
// Note: (TS[0].W % 4) have to be 0
	uint16 w = TS[0]->W, h = TS[0]->H;
	uint8 *v = TS[0]->V, * p;
	uint16 dsiz = w >> 2;     // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;   // uint16 for line header, uint16 for gap
	uint16 psiz = h * lsiz;       // - last gap, but + plane trailer
	uint16 size = 4 * psiz;       // whole map size
	uint16 rsiz = FONT_HIG * lsiz;    // length of whole text row map

	// set desired line pointer
	v += (TEXT_VM + (FONT_HIG + TEXT_LS) * line) * lsiz;

	// clear whole rectangle
	p = v;                // assume blanked line above text
	memcpy(p, p - lsiz, rsiz);
	p += psiz;   // tricky replicate lines for plane 0
	memcpy(p, p - lsiz, rsiz);
	p += psiz;   // same for plane 1
	memcpy(p, p - lsiz, rsiz);
	p += psiz;   // same for plane 2
	memcpy(p, p - lsiz, rsiz);    // same for plane 3

	// paint text line
	if (text) {
		uint8 *q;
		p = v + 2 + TEXT_HM / 4 + (TEXT_HM % 4) * psiz;
		q = v + size;

		while (* text) {
			uint16 cw = _Font->Wid[*text], i;
			uint8 *fp = _Font->Map + _Font->Pos[*text];

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
			++text;
		}
	}
}


INFO_LINE::INFO_LINE(CGEEngine *vm, uint16 w) : TALK(vm), OldTxt(NULL), _vm(vm) {
	TS[0] = new BITMAP(w, FONT_HIG, TEXT_BG);
	SetShapeList(TS);
}


void INFO_LINE::Update(const char *tx) {
	if (tx != OldTxt) {
		uint16 w = TS[0]->W, h = TS[0]->H;
		uint8 *v = (uint8 *) TS[0]->V;
		uint16 dsiz = w >> 2;                           // data size (1 plane line size)
		uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
		uint16 psiz = h * lsiz;                         // - last gape, but + plane trailer
		uint16 size = 4 * psiz;                         // whole map size

		// claer whole rectangle
		memset(v + 2, TEXT_BG, dsiz);                   // data bytes
		memcpy(v + lsiz, v, psiz - lsiz);               // tricky replicate lines
		*(uint16 *)(v + psiz - 2) = EOI;               // plane trailer uint16
		memcpy(v + psiz, v, 3 * psiz);                  // tricky replicate planes

		// paint text line
		if (tx) {
			uint8 *p = v + 2, * q = p + size;

			while (*tx) {
				uint16 cw = _Font->Wid[*tx];
				uint8 *fp = _Font->Map + _Font->Pos[*tx];

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
				++tx;
			}
		}
		OldTxt = tx;
	}
}

} // End of namespace CGE
