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

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_menukeys.h"
#include "illusions/bbdou/menusystem_bbdou.h"
#include "illusions/input.h"
#include "illusions/screen.h"

namespace Illusions {

// BBDOUMenuKeys

BBDOUMenuKeys::BBDOUMenuKeys(IllusionsEngine_BBDOU *vm)
	: _vm(vm) {
}

BBDOUMenuKeys::~BBDOUMenuKeys() {

}

void BBDOUMenuKeys::addMenuKey(uint bitMask, uint32 threadId) {
	MenuKey menuKey;
	menuKey.bitMask = bitMask;
	menuKey.threadId = threadId;
	_menuKeys.push_back(menuKey);
}

void BBDOUMenuKeys::update() {
	if (_vm->_screen->isDisplayOn() && !_vm->_menuSystem->isActive()) {
		for (MenuKeys::iterator it = _menuKeys.begin(); it != _menuKeys.end(); ++it) {
			const MenuKey &menuKey = *it;
			if (_vm->_input->pollButton(menuKey.bitMask)) {
				_vm->startScriptThread(menuKey.threadId, 0, 0, 0, 0);
				break;
			}
		}
	}
}

} // End of namespace Illusions
