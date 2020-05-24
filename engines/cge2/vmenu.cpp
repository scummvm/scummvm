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

#include "cge2/text.h"
#include "cge2/vmenu.h"
#include "cge2/events.h"

namespace CGE2 {

Choice::Choice(CGE2Engine *vm) : _vm(vm), _text(nullptr) {}

ExitGameChoice::ExitGameChoice(CGE2Engine *vm) : Choice(vm) {
	_text = _vm->_text->getText(kQuitText);
}

void ExitGameChoice::proc() {
	_vm->switchScene(-1);
}

ReturnToGameChoice::ReturnToGameChoice(CGE2Engine *vm) : Choice(vm) {
	_text = _vm->_text->getText(kNoQuitText);
}

void ReturnToGameChoice::proc() {
	_vm->_commandHandlerTurbo->addCommand(kCmdSeq, kPowerRef, 1, nullptr);
	_vm->keyClick();
}

MenuBar::MenuBar(CGE2Engine *vm, uint16 w, byte *c) : Talk(vm) {
	_color = c;
	int h = kFontHigh + 2 * kMenuBarVerticalMargin, i = (w += 2 * kMenuBarHorizontalMargin) * h;
	uint8 *p = new uint8[i];
	uint8 *p1;
	uint8 *p2;
	uint8 lt = _color[kLt];
	uint8 rb = _color[kRb];
	BitmapPtr b;

	memset(p + w, kPixelTransp, i - 2 * w);
	memset(p, lt, w);
	memset(p + i - w, rb, w);
	p1 = p;
	p2 = p + i - 1;
	for (i = 0; i < h; i++) {
		*p1 = lt;
		*p2 = rb;
		p1 += w;
		p2 -= w;
	}
	b = new Bitmap[1];
	b[0] = Bitmap(vm, w, h, p);
	delete[] p;
	setShapeList(b, 1);
	_flags._slav = true;
	_flags._tran = true;
	_flags._kill = true;
}

VMenu *VMenu::_addr = nullptr;

VMenu::VMenu(CGE2Engine *vm, Common::Array<Choice *> list, V2D pos, ColorBank col)
	: Talk(vm, vmGather(list), kTBRect, col), _menu(list), _bar(nullptr), _items(list.size()), _vm(vm) {
	delete[] _vmgt; // Lefotver of vmGather.

	_addr = this;
	_recent = -1;
	_flags._kill = true;

	if (pos.x < 0 || pos.y < 0)
		center();
	else
		gotoxyz(V2D(_vm, pos.x - _siz.x / 2, pos.y - (kTextVMargin + kFontHigh / 2)));

	_vm->_vga->_showQ->append(this);
	_bar = new MenuBar(_vm, _siz.x - 2 * kTextHMargin, _color);
	_bar->gotoxyz(V2D(_vm, _pos2D.x, _pos2D.y + kTextVMargin - kMenuBarVerticalMargin));
	_vm->_vga->_showQ->append(_bar);
}

char *VMenu::vmGather(Common::Array<Choice *> list) {
	int len = 0;
	int h = 0;

	for (uint i = 0; i < list.size(); i++) {
		len += strlen(list[i]->_text);
		++h;
	}
	_vmgt = new char[len + h];
	*_vmgt = '\0';
	for (uint i = 0; i < list.size(); i++) {
		if (*_vmgt)
			strcat(_vmgt, "|");
		strcat(_vmgt, list[i]->_text);
		++h;
	}

	return _vmgt;
}


VMenu::~VMenu() {
	_addr = nullptr;

	for (uint i = 0; i < _menu.size(); i++) {
		delete _menu[i];
	}
}

void VMenu::touch(uint16 mask, V2D pos, Common::KeyCode keyCode) {
	if (_items) {
		Sprite::touch(mask, pos, keyCode);

		int n = 0;
		bool ok = false;
		int h = kFontHigh + kTextLineSpace;
		pos.y -= kTextVMargin - 1;
		if (pos.y >= 0) {
			if (pos.x < 0)
				pos.x = -pos.x;
			n = pos.y / h;
			if (n < _items)
				ok = (pos.x <= (_siz.x >> 1) - kTextHMargin);
			else
				n = _items - 1;
		}

		_bar->gotoxyz(V2D(_vm, _pos2D.x, _pos2D.y + kTextVMargin + n * h - kMenuBarVerticalMargin));
		n = _items - 1 - n;

		if (ok && (mask & kMouseLeftUp)) {
			_items = 0;
			_vm->_commandHandlerTurbo->addCommand(kCmdKill, -1, 0, this);
			_menu[_recent = n]->proc();
		}
	}
}

} // End of namespace CGE2
