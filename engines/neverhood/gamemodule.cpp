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

#include "neverhood/gamemodule.h"

#include "neverhood/graphics.h"
#include "neverhood/module1000.h"
#include "neverhood/module1500.h"

namespace Neverhood {

GameModule::GameModule(NeverhoodEngine *vm)
	: Module(vm, NULL) {
	
	// Other initializations moved to actual engine class
	
	// TODO

	// TODO Sound1ChList_sub_407F70(0x2D0031, 0x8861079);
	
	SetMessageHandler(&GameModule::handleMessage);

	startup();
	
}

GameModule::~GameModule() {

	// TODO Sound1ChList_sub_407AF0(0x2D0031);

	delete _childObject;
	_childObject = NULL;

	// TODO: Set palette to black but probably not neccessary
	
	// TODO Sound1ChList_sub_408480();
	
	// TODO Set debug vars (maybe)

}

void GameModule::handleMouseDown(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug("GameModule::handleMouseDown(%d, %d)", x, y);
		_childObject->sendPointMessage(1, mousePos, this);
	}				
}

uint32 GameModule::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0800:
		_someFlag1 = true;
		return messageResult;		
	case 0x1009:
		_field24 = -1;
		_field26 = -1;
		_field28 = -1;
		_field20 = param.asInteger();
		_done = true;
		return messageResult;
	case 0x100A:
		_field24 = (int16)param.asInteger();
		return messageResult;
	case 0x101F:
		_field2C = true;		
		return messageResult;
	case 0x1023:
		_field26 = (int16)param.asInteger();
		return messageResult;
	}
	return messageResult;
}

void GameModule::startup() {
	// TODO: Displaying of error text probably not needed in ScummVM
//	createModule1500(0); // Logos and intro video

	createModule1000(0);
}

void GameModule::createModule1500(int which) {
	_someFlag1 = false;
	setGlobalVar(0x91080831, 0x00F10114);
	_childObject = new Module1500(_vm, this, which, true);
	SetUpdateHandler(&GameModule::updateModule1500);
}

void GameModule::updateModule1500() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createModule1000(0);
		_childObject->handleUpdate();
	}
}

void GameModule::createModule1000(int which) {
	setGlobalVar(0x91080831, 0x03294419);
	_childObject = new Module1000(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule1000);
}

void GameModule::updateModule1000() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		// TODO _resourceTable3.load();
		delete _childObject;
		_childObject = NULL;
		error("Done...");
		// TODO createModule2300();
		// TODO _childObject->handleUpdate();
	}
}

} // End of namespace Neverhood
