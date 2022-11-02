/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janusz B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/talk.h"
#include "cge/game.h"
#include "cge/events.h"
#include "cge/cge_main.h"
#include "common/config-manager.h"
#include "common/text-to-speech.h"

namespace CGE {

Font::Font(CGEEngine *vm, const char *name) : _vm(vm) {
	_map = (uint8 *)malloc(kMapSize);
	_pos = (uint16 *)malloc(kPosSize * sizeof(uint16));
	_widthArr = (uint8 *)malloc(kWidSize);

	assert((_map != nullptr) && (_pos != nullptr) && (_widthArr != nullptr));
	_vm->mergeExt(_path, name, kFontExt);
	load();
}

Font::~Font() {
	free(_map);
	free(_pos);
	free(_widthArr);
}

void Font::load() {
	EncryptedStream f(_vm->_resman, _path);
	assert(!f.err());

	f.read(_widthArr, kWidSize);
	assert(!f.err());

	uint16 p = 0;
	for (uint16 i = 0; i < kPosSize; i++) {
		_pos[i] = p;
		p += _widthArr[i];
	}
	f.read(_map, p);
}

uint16 Font::width(const char *text) {
	uint16 w = 0;
	if (!text)
		return 0;
	while (*text)
		w += _widthArr[(unsigned char)*(text++)];
	return w;
}

Talk::Talk(CGEEngine *vm, const char *text, TextBoxStyle mode, bool wideSpace)
	: Sprite(vm, nullptr), _mode(mode), _wideSpace(wideSpace), _vm(vm) {
	_ts = nullptr;
	_flags._syst = true;
	update(text);
}


Talk::Talk(CGEEngine *vm)
	: Sprite(vm, nullptr), _mode(kTBPure), _vm(vm) {
	_ts = nullptr;
	_flags._syst = true;
	_wideSpace = false;
}

void Talk::textToSpeech(const char *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (text != nullptr && ttsMan != nullptr && ConfMan.getBool("tts_enabled"))
		ttsMan->say(text);
}

void Talk::update(const char *text) {
	textToSpeech(text);

	const uint16 vmarg = (_mode) ? kTextVMargin : 0;
	const uint16 hmarg = (_mode) ? kTextHMargin : 0;
	uint16 mw = 0;
	uint16 ln = vmarg;
	uint8 *m;

	if (!_ts) {
		uint16 k = 2 * hmarg;
		uint16 mh = 2 * vmarg + kFontHigh;
		for (const char *p = text; *p; p++) {
			if (*p == '|' || *p == '\n') {
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

		_ts = new BitmapPtr[2];
		_ts[0] = box(mw, mh);
		_ts[1] = nullptr;
	}

	m = _ts[0]->_m + ln * mw + hmarg;

	while (*text) {
		if (*text == '|' || *text == '\n') {
			m = _ts[0]->_m + (ln += kFontHigh + kTextLineSpace) * mw + hmarg;
		} else {
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
	if (w < 8)
		w = 8;
	if (h < 8)
		h = 8;
	uint16 n = w * h;
	uint8 *b = (uint8 *)malloc(n);
	assert(b != nullptr);
	memset(b, kTextColBG, n);

	if (_mode) {
		uint8 *p = b;
		uint8 *q = b + n - w;
		memset(p, kVgaColLightGray, w);
		memset(q, kVgaColDarkGray, w);
		while (p < q) {
			p += w;
			*(p - 1) = kVgaColDarkGray;
			*p = kVgaColLightGray;
		}
		p = b;
		const uint16 r = (_mode == kTBRound) ? kTextRoundCorner : 0;
		for (int i = 0; i < r; i++) {
			int j;
			for (j = 0; j < r - i; j++) {
				p[j] = kPixelTransp;
				p[w - j - 1] = kPixelTransp;
				q[j] = kPixelTransp;
				q[w - j - 1] = kPixelTransp;
			}
			p[j] = kVgaColLightGray;
			p[w - j - 1] = kVgaColDarkGray;
			q[j] = kVgaColLightGray;
			q[w - j - 1] = kVgaColDarkGray;
			p += w;
			q -= w;
		}
	}
	return new Bitmap(_vm, w, h, b);
}

InfoLine::InfoLine(CGEEngine *vm, uint16 w) : Talk(vm), _oldText(nullptr), _vm(vm) {
	if (!_ts) {
		_ts = new BitmapPtr[2];
		_ts[1] = nullptr;
	}

	_ts[0] = new Bitmap(_vm, w, kFontHigh, kTextColBG);
	setShapeList(_ts);
}

void InfoLine::update(const char *text) {
	if (text == _oldText)
		return;

	_oldText = text;

	textToSpeech(text);

	uint16 w = _ts[0]->_w;
	uint16 h = _ts[0]->_h;
	uint8 *v = (uint8 *)_ts[0]->_v;
	uint16 dsiz = w >> 2;                           // data size (1 plane line size)
	uint16 lsiz = 2 + dsiz + 2;                     // uint16 for line header, uint16 for gap
	uint16 psiz = h * lsiz;                         // - last gape, but + plane trailer
	uint16 size = 4 * psiz;                         // whole map size

	// clear whole rectangle
	memset(v + 2, kTextColBG, dsiz);                // data bytes
	for (byte *pDest = v + lsiz; pDest < (v + psiz); pDest += lsiz) {
		Common::copy(v, v + lsiz, pDest);
	}
	*(uint16 *)(v + psiz - 2) = TO_LE_16(kBmpEOI);  // plane trailer uint16
	for (byte *pDest = v + psiz; pDest < (v + 4 * psiz); pDest += psiz) {
		Common::copy(v, v + psiz, pDest);
	}

	// paint text line
	if (text) {
		uint8 *p = v + 2, * q = p + size;

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
}

} // End of namespace CGE
