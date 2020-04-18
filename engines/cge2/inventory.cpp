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

#include "cge2/cge2.h"
#include "cge2/hero.h"

namespace CGE2 {

int CGE2Engine::findActivePocket(int ref) {
	for (int i = 0; i < kPocketMax; i++) {
		Sprite *spr = _heroTab[_sex]->_pocket[i];
		if (ref >= 0) {
			if (spr && (spr->_ref == ref))
				return i;
		} else if (!spr)
			return i;
	}
	return -1;
}

void CGE2Engine::selectPocket(int n) {
	Sprite **p = _heroTab[_sex]->_pocket;
	int &pp = _heroTab[_sex]->_pocPtr;
	if ((n < 0) || (pp == n)) {
		n = findActivePocket(-1);
		if (n >= 0)
			pp = n;
	} else if (p[n])
		pp = n;
}

void CGE2Engine::pocFul() {
	Hero *h = _heroTab[_sex]->_ptr;
	h->park();
	_commandHandler->addCommand(kCmdWait, -1, -1, h);
	_commandHandler->addCommand(kCmdSound, -1, 2, h);
	_commandHandler->addCommand(kCmdSay, -1, kPocketFull + _sex, h);
}

void CGE2Engine::releasePocket(Sprite *spr) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kPocketMax; j++) {
			Sprite *&poc = _heroTab[i]->_pocket[j];
			if (poc == spr) {
				spr->_flags._kept = false;
				poc = nullptr;
				return;
			}
		}
	}
}

int CGE2Engine::freePockets(int sx) {
	int n = 0;
	for (int i = 0; i < kPocketMax; i++){
		if (_heroTab[sx]->_pocket[i] == nullptr)
			++n;
	}
	return n;
}

void CGE2Engine::openPocket() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kPocketMax + 1; j++) {
			int ref = (int)_heroTab[i]->_downPocketId[j];
			_heroTab[i]->_pocket[j] = (ref == -1) ? nullptr : _vga->_showQ->locate(ref);
		}
	}
}

void CGE2Engine::closePocket() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kPocketMax + 1; j++) {
			Sprite *spr = _heroTab[i]->_pocket[j];
			_heroTab[i]->_downPocketId[j] = (spr) ? spr->_ref : -1;
		}
	}
}

} // End of namespace CGE2
