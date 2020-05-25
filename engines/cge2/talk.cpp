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

#include "cge2/general.h"
#include "cge2/talk.h"

namespace CGE2 {

void CGE2Engine::setAutoColors() {
	Dac def[4] = {
		{ 0, 0, 0 },
		{ 220 >> 2, 220 >> 2, 220 >> 2 },
		{ 190 >> 2, 190 >> 2, 190 >> 2 },
		{ 160 >> 2, 160 >> 2, 160 >> 2 },
	};
	Dac pal[kPalCount];
	_vga->getColors(pal);
	for (int i = 0; i < 4; i++)
		_font->_colorSet[kCBRel][i] = _vga->closest(pal, def[i]);
}

Font::Font(CGE2Engine *vm) : _vm(vm) {
	_map = new uint8[kMapSize];
	_pos = new uint16[kPosSize];
	_widthArr = new uint8[kWidSize];

	load();
}

Font::~Font() {
	delete[] _map;
	delete[] _pos;
	delete[] _widthArr;
}

void Font::load() {
	char path[10];
	strcpy(path, "CGE.CFT");
	if (!_vm->_resman->exist(path))
		error("Missing Font file! %s", path);

	EncryptedStream fontFile(_vm, path);
	assert(!fontFile.err());

	fontFile.read(_widthArr, kWidSize);
	assert(!fontFile.err());

	uint16 p = 0;
	for (uint16 i = 0; i < kPosSize; i++) {
		_pos[i] = p;
		p += _widthArr[i];
	}
	fontFile.read(_map, p);

	strcpy(path, "CGE.TXC");
	if (!_vm->_resman->exist(path))
		error("Missing Color file! %s", path);

	// Reading in _colorSet:
	EncryptedStream colorFile(_vm, path);
	assert(!colorFile.err());

	char tmpStr[kLineMax + 1];
	int n = 0;

	for (Common::String line = colorFile.readLine(); !colorFile.eos(); line = colorFile.readLine()){
		if (line.empty())
			continue;
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
		_colorSet[n][0] = _vm->number(tmpStr);

		for (int i = 1; i < 4; i++)
			_colorSet[n][i] = _vm->number(nullptr);

		n++;
	}
}

uint16 Font::width(const char *text) {
	uint16 w = 0;
	if (!text)
		return 0;
	while (*text)
		w += _widthArr[(unsigned char)*(text++)];
	return w;
}

Talk::Talk(CGE2Engine *vm, const char *text, TextBoxStyle mode, ColorBank color, bool wideSpace)
	: Sprite(vm), _mode(mode), _created(false), _wideSpace(wideSpace), _vm(vm) {
	_color = _vm->_font->_colorSet[color];

	if (color == kCBRel)
		_vm->setAutoColors();
	update(text);
}

Talk::Talk(CGE2Engine *vm, ColorBank color)
	: Sprite(vm), _mode(kTBPure), _created(false), _wideSpace(false), _vm(vm) {
	_color = _vm->_font->_colorSet[color];

	if (color == kCBRel)
		_vm->setAutoColors();
}

uint8 *Talk::box(V2D siz) {
	uint16 n, r = (_mode == kTBRound) ? kTextRoundCorner : 0;
	const byte lt = _color[1], bg = _color[2], dk = _color[3];

	if (siz.x < 8)
		siz.x = 8;
	if (siz.y < 8)
		siz.y = 8;
	uint8 *b = new uint8[n = siz.area()];
	memset(b, bg, n);

	if (_mode) {
		uint8 *p = b;
		uint8 *q = b + n - siz.x;
		memset(p, lt, siz.x);
		memset(q, dk, siz.x);
		while (p < q) {
			p += siz.x;
			*(p - 1) = dk;
			*p = lt;
		}
		p = b;
		for (int i = 0; i < r; i++) {
			int j = 0;
			for (; j < r - i; j++) {
				p[j] = kPixelTransp;
				p[siz.x - j - 1] = kPixelTransp;
				q[j] = kPixelTransp;
				q[siz.x - j - 1] = kPixelTransp;
			}
			p[j] = lt;
			p[siz.x - j - 1] = dk;
			q[j] = lt;
			q[siz.x - j - 1] = dk;
			p += siz.x;
			q -= siz.x;
		}
	}
	return b;
}

void Talk::update(const char *text) {
	const uint16 vmarg = (_mode) ? kTextVMargin : 0;
	const uint16 hmarg = (_mode) ? kTextHMargin : 0;
	uint16 mw;
	uint16 mh;
	uint16 ln = vmarg;
	uint8 *m;
	uint8 *map;
	uint8 fg = _color[0];

	if (_created) {
		mw = _ext->_shpList->_w;
		mh = _ext->_shpList->_h;
		delete _ext->_shpList;
	} else {
		uint16 k = 2 * hmarg;
		mh = 2 * vmarg + kFontHigh;
		mw = 0;
		for (const char *p = text; *p; p++) {
			if ((*p == '|') || (*p == '\n')) {
				mh += kFontHigh + kTextLineSpace;
				if (k > mw)
					mw = k;
				k = 2 * hmarg;
			} else if ((*p == 0x20) && (_vm->_font->_widthArr[(unsigned char)*p] > 4) && (!_wideSpace))
				k += _vm->_font->_widthArr[(unsigned char)*p] - 2;
			else
				k += _vm->_font->_widthArr[(unsigned char)*p];
		}
		if (k > mw)
			mw = k;

		_created = true;
	}

	V2D sz(_vm, mw, mh);
	map = box(sz);

	m = map + ln * mw + hmarg;

	while (*text) {
		if ((*text == '|') || (*text == '\n'))
			m = map + (ln += kFontHigh + kTextLineSpace) * mw + hmarg;
		else {
			int cw = _vm->_font->_widthArr[(unsigned char)*text];
			uint8 *f = _vm->_font->_map + _vm->_font->_pos[(unsigned char)*text];

			// Handle properly space size, after it was enlarged to display properly
			// 'F1' text.
			int8 fontStart = 0;
			if ((*text == 0x20) && (cw > 4) && (!_wideSpace))
				fontStart = 2;

			for (int i = fontStart; i < cw; i++) {
				uint8 *pp = m;
				uint16 n;
				uint16 b = *(f++);
				for (n = 0; n < kFontHigh; n++) {
					if (b & 1)
						*pp = fg;
					b >>= 1;
					pp += mw;
				}
				m++;
			}
		}
		text++;
	}
	BitmapPtr b = new Bitmap[1];
	b[0] = Bitmap(_vm, sz.x, sz.y, map);
	delete[] map;
	setShapeList(b, 1);
}

InfoLine::InfoLine(CGE2Engine *vm, uint16 w, ColorBank color)
: Talk(vm), _oldText(nullptr), _newText(nullptr), _realTime(false), _vm(vm) {
	_wideSpace = false;
	BitmapPtr b = new Bitmap[1];
	if (color == kCBRel)
		_vm->setAutoColors();
	_color = _vm->_font->_colorSet[color];
	V2D siz = V2D(_vm, w, kFontHigh);
	b[0] = Bitmap(_vm, siz.x, siz.y, _color[2]);
	setShapeList(b, 1);
}

void InfoLine::update(const char *text) {
	if (!_realTime && (text == _oldText))
		return;

	_oldText = text;

	uint16 w = _ext->_shpList->_w;
	uint16 h = _ext->_shpList->_h;
	uint8 *v = _ext->_shpList->_v;
	uint16 dsiz = w >> 2;                           // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
	uint16 psiz = h * lsiz;                         // - last gape, but + plane trailer
	uint16 size = 4 * psiz;                         // whole map size
	uint8 fg = _color[0];
	uint8 bg = _color[2];

	// clear whole rectangle
	memset(v + 2, bg, dsiz);                // data bytes
	for (byte *pDest = v + lsiz; pDest < (v + psiz); pDest += lsiz) {
		Common::copy(v, v + lsiz, pDest);
	}
	*(uint16 *)(v + psiz - 2) = TO_LE_16(kBmpEOI);  // plane trailer uint16
	for (byte *pDest = v + psiz; pDest < (v + 4 * psiz); pDest += psiz) {
		Common::copy(v, v + psiz, pDest);
	}

	// paint text line
	if (_newText) {
		uint8 *p = v + 2, *q = p + size;

		while (*text) {
			uint16 cw = _vm->_font->_widthArr[(unsigned char)*text];
			uint8 *fp = _vm->_font->_map + _vm->_font->_pos[(unsigned char)*text];

			// Handle properly space size, after it was enlarged to display properly
			// 'F1' text.
			int8 fontStart = 0;
			if ((*text == 0x20) && (cw > 4) && (!_wideSpace))
				fontStart = 2;

			for (int i = fontStart; i < cw; i++) {
				uint16 b = fp[i];
				for (uint16 n = 0; n < kFontHigh; n++) {
					if (b & 1)
						*p = fg;
					b >>= 1;
					p += lsiz;
				}
				if (p >= q)
					p = p - size + 1;
			}
			text++;
		}
	}
}

} // End of namespace CGE2
