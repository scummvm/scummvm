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

#include "cge2/console.h"

#include "cge2/vga13h.h"

namespace CGE2 {

CGE2Console::CGE2Console(CGE2Engine *vm) : _vm(vm), GUI::Debugger() {
	registerCmd("do_carpet_workaround", WRAP_METHOD(CGE2Console, doCarpetWorkaround));
}

CGE2Console::~CGE2Console() {
}

bool CGE2Console::doCarpetWorkaround(int argc, const char **argv) {
	Sprite *spr = _vm->_vga->_showQ->locate(1537); // 1537 is Carpet

	if (spr) {
		if (spr->_actionCtrl[1]._ptr == 26) {
			spr->_actionCtrl[1]._ptr = 8;
		}
	}

	return true;
}

} // End of namespace CGE
