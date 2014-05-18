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
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/spare.h"

namespace CGE2 {

void Spare::synchronize() {
	warning("STUB: Spare::Load");
}

void Spare::clear() {
	for (int i = 0; i < _container.size(); i++)
		delete _container[i];
	_container.clear();
}

Sprite *Spare::locate(int ref) {
	for (int i = 0; i < _container.size(); i++) {
		if (_container[i]->_ref == ref) {
			return _container[i];
		}
	}
	return nullptr;
}

void Spare::takeCave(int ref) {
	Sprite *spr = locate(ref);
	if (spr != nullptr)
		_vm->_vga->_showQ->insert(spr);
}

void Spare::store(Sprite *spr) {
	_container.insert_at(_container.size(), spr);
}

void Spare::dispose(Sprite *spr) {
	warning("STUB: Spare::Dispose()");

	if (spr) {
		_vm->_vga->_showQ->remove(spr);

		for (int i = 0; i < _container.size(); i++) {
			if (spr == _container[i]) {
				_container.remove_at(i);
			}
		}
	}
}

void Spare::dispose(int ref) {
	dispose(_vm->_vga->_showQ->locate(ref));
}

void Spare::dispose() {
	for (int i = 0; i < _container.size(); i++) {
		if (_container[i]->_ref > 255) {
			dispose(_container[i]);
		}
	}
}

} // End of namespace CGE2
