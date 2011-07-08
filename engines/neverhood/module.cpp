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

#include "neverhood/module.h"

namespace Neverhood {

Module::Module(NeverhoodEngine *vm, Module *parentModule)
	: Entity(vm, 0), _parentModule(parentModule), _childObject(NULL),
	_done(false), _field24(-1), _field26(-1), _field28(-1) {
	
	SetMessageHandler(&Module::handleMessage);
	
}

Module::~Module() {
	delete _childObject;
}

void Module::draw() {
	if (_childObject)
		_childObject->draw();
}

uint32 Module::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {

	switch (messageNum) {
	case 0x0008:
		if (_parentModule)
			_parentModule->sendMessage(8, 0, this);
		return 0;
	case 0x1009:
		_field24 = -1;
		_field26 = -1;
		_field28 = -1;
		_field20 = param.asInteger();
		_done = true;
		return 0;
	case 0x100A:
		_field24 = (int16)param.asInteger();
		return 0;
	case 0x1023:
		_field26 = (int16)param.asInteger();
		return 0;
	case 0x1024:
		_field28 = (int16)param.asInteger();
		return 0;
	default:
		if (_childObject && sender == _parentModule)
			return _childObject->sendMessage(messageNum, param, sender);
	}

	return 0;
}

} // End of namespace Neverhood
