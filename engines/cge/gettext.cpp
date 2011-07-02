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
#include <string.h>

namespace CGE {

GetText *GetText::_ptr = NULL;


GetText::GetText(CGEEngine *vm, const char *info, char *text, int size, void (*click)())
	: TALK(vm), _text(text), _size(min<int>(size, GTMAX)), _len(min<int>(_size, strlen(text))),
	  _cntr(GTBLINK), _click(click), _oldKeybClient(_keyboard->setClient(this)), _vm(vm) {
	int i = 2 * TEXT_HM + _Font->Width(info);
	_ptr = this;
	Mode = RECT;
	TS[0] = Box((i + 3) & ~3, 2 * TEXT_VM + 2 * FONT_HIG + TEXT_LS);
	setShapeList(TS);
	_flags._bDel = true;
	_flags._kill = true;
	memcpy(_buff, text, _len);
	_buff[_len] = ' ';
	_buff[_len + 1] = '\0';
	PutLine(0, info);
	tick();
}


GetText::~GetText() {
	_keyboard->setClient(_oldKeybClient);
	_ptr = NULL;
}


void GetText::tick() {
	if (++_cntr >= GTBLINK) {
		_buff[_len] ^= (' ' ^ '_');
		_cntr = 0;
	}
	PutLine(1, _buff);
	_time = GTTIME;
}


void GetText::touch(uint16 mask, int x, int y) {
	static char ogon[] = "èïêú•£ò†°";
	static char bezo[] = "ACELNOSXZ";
	char *p;

	if (mask & KEYB) {
		if (_click)
			_click();
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
			SNPOST_(SNKILL, -1, 0, this);
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
				if (_len < _size && 2 * TEXT_HM + _Font->Width(_buff) + _Font->Wid[x] <= _w) {
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
