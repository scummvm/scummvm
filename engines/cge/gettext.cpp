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

#include "cge/gettext.h"
#include "cge/events.h"
#include "cge/cge_main.h"

namespace CGE {

GetText *GetText::_ptr = NULL;


GetText::GetText(CGEEngine *vm, const char *info, char *text, int size)
	: Talk(vm), _text(text), _size(min<int>(size, kGetTextMax)), _len(min<int>(_size, strlen(text))),
	_cntr(kGetTextBlink), _oldKeybClient(_keyboard->setClient(this)), _vm(vm) {
	int i = 2 * kTextHMargin + _font->width(info);
	_ptr = this;
	_mode = kTBRect;

	_ts = new BitmapPtr[2];
	_ts[0] = box((i + 3) & ~3, 2 * kTextVMargin + 2 * kFontHigh + kTextLineSpace);
	_ts[1] = NULL;
	setShapeList(_ts);

	_flags._bDel = true;
	_flags._kill = true;
	memcpy(_buff, text, _len);
	_buff[_len] = ' ';
	_buff[_len + 1] = '\0';
	putLine(0, info);
	tick();
}


GetText::~GetText() {
	_keyboard->setClient(_oldKeybClient);
	_ptr = NULL;
}


void GetText::tick() {
	if (++_cntr >= kGetTextBlink) {
		_buff[_len] ^= (' ' ^ '_');
		_cntr = 0;
	}
	putLine(1, _buff);
	_time = kGetTextTime;
}


void GetText::touch(uint16 mask, int x, int y) {
	static char ogon[] = "•œ¥£˜ ¡";
	static char bezo[] = "ACELNOSXZ";
	char *p;

	if (mask & KEYB) {
		_vm->keyClick();
		switch (x) {
		case Enter :
			_buff[_len] = '\0';
			strcpy(_text, _buff);
			for (p = _text; *p; p++) {
				char *q = strchr(ogon, *p);
				if (q)
					*p = bezo[q - ogon];
			}
		case Esc   :
			_snail_->addCom(kSnKill, -1, 0, this);
			break;
		case BSp   :
			if (_len) {
				_len--;
				_buff[_len] = _buff[_len + 1];
				_buff[_len + 1] = _buff[_len + 2];
			}
			break;
		default    :
			if (x < 'A' || x > 'Z') {
				if (_oldKeybClient)
					_oldKeybClient->touch(mask, x, y);
			} else {
				if (_keyboard->_key[ALT]) {
					p = strchr(bezo, x);
					if (p)
						x = ogon[p - bezo];
				}
				if (_len < _size && 2 * kTextHMargin + _font->width(_buff) + _font->_wid[x] <= _w) {
					_buff[_len + 2] = _buff[_len + 1];
					_buff[_len + 1] = _buff[_len];
					_buff[_len++] = x;
				}
			}
			break;
		}
	} else
		Sprite::touch(mask, x, y);
}

} // End of namespace CGE
