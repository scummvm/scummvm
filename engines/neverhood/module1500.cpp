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

#include "neverhood/module1500.h"

namespace Neverhood {

Module1500::Module1500(NeverhoodEngine *vm, Module *parentModule, int which, bool flag)
	: Module(vm, parentModule), _flag(flag) {

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			createScene1502();
			break;
		case 2:
			createScene1503();
			break;
		case 3:
			createScene1504();
			break;
		default:
			createScene1501();			
		}
	} else {
		createScene1504();
	}

}

void Module1500::update() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1502();
			break;
		case 1:
			if (_flag) {
				createScene1503();
			} else {
				_parentModule->sendMessage(0x1009, 0, this);
			}
			break;
		case 3:
			createScene1501();
			break;
		default:
			_parentModule->sendMessage(0x1009, 0, this);
			break;
		}
	}
}

void Module1500::createScene1501() {
	// TODO
}
			
void Module1500::createScene1502() {
	// TODO
}

void Module1500::createScene1503() {
	// TODO
}

void Module1500::createScene1504() {
	// TODO
}

} // End of namespace Neverhood
