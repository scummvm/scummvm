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
#include "neverhood/module1200.h"
#include "neverhood/module1400.h"
#include "neverhood/module1500.h"
#include "neverhood/module1700.h"
#include "neverhood/module1800.h"
#include "neverhood/module2300.h"

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

void GameModule::handleMouseMove(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseMove(%d, %d)", x, y);
		_childObject->sendPointMessage(0, mousePos, this);
	}				
}

void GameModule::handleMouseDown(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseDown(%d, %d)", x, y);
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
//	createModule1500(0); // Logos and intro video //Real
//	createModule1000(-1);
//	createModule2300(2);
	_vm->gameState().sceneNum = 6;
	//createModule1200(-1);
	//createModule1800(-1);
	//createModule1700(-1);
	//createModule1700(1);
	createModule1400(-1);
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
		createModule2300(0);
		_childObject->handleUpdate();
	}
}

void GameModule::createModule1200(int which) {
	setGlobalVar(0x91080831, 0x00478311);
	_childObject = new Module1200(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule1200);
}

void GameModule::updateModule1200() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			error("// TODO createModule2600(0);");
			// TODO createModule2600(0);
			// TODO _childObject->handleUpdate();
		} else {
			createModule2300(2);
			_childObject->handleUpdate();
		}
	}
}

void GameModule::createModule1400(int which) {
	setGlobalVar(0x91080831, 0x00AD0012);
	_childObject = new Module1400(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule1400);
}

void GameModule::updateModule1400() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			error("WEIRD!");
		} else {
			// TODO createModule1600(1);
			// TODO _childObject->handleUpdate();
		}
	}
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

void GameModule::createModule1700(int which) {
	setGlobalVar(0x91080831, 0x04212331);
	_childObject = new Module1700(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule1700);
}

void GameModule::updateModule1700() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		debug("Module1700 done; _field20 = %d", _field20);
		if (_field20 == 1) {
			// TODO createModule2900(3);
			// TODO _childObject->handleUpdate();
		} else {
			// TODO createModule1600(2);
			// TODO _childObject->handleUpdate();
		}
	}
}

void GameModule::createModule1800(int which) {
	setGlobalVar(0x91080831, 0x04A14718);
	_childObject = new Module1800(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule1800);
}

void GameModule::updateModule1800() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			// TODO GameState_clear();
			// TODO GameModule_handleKeyEscape();
		} else if (_field20 == 2) {
			// TODO createModule2700(0);
			// TODO _childObject->handleUpdate();
		} else if (_field20 == 3) {
			// TODO createModule3000(3);
			// TODO _childObject->handleUpdate();
		} else {
			// TODO createModule2800(0);
			// TODO _childObject->handleUpdate();
		}
	}
}

void GameModule::createModule2300(int which) {
	setGlobalVar(0x91080831, 0x1A214010);
	_childObject = new Module2300(_vm, this, which);
	SetUpdateHandler(&GameModule::updateModule2300);
}

void GameModule::updateModule2300() {
	if (!_childObject)
		return;
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			// TODO createModule2200(0);
			// TODO _childObject->handleUpdate();
		} else if (_field20 == 2) {
			createModule1200(0);
			_childObject->handleUpdate();
		} else if (_field20 == 3) {
			// TODO createModule2400(0);
			// TODO _childObject->handleUpdate();
		} else if (_field20 == 4) {
			// TODO createModule3000(0);
			// TODO _childObject->handleUpdate();
		} else {
			createModule1000(1);
			_childObject->handleUpdate();
		}
	}
	if (_field24 >= 0) {
		if (_field24 == 2) {
			// TODO _resourceTable4.load();
		} else if (_field24 == 0) {
			// TODO _resourceTable3.load();
		}
		_field24 = -1;
	}
	if (_field26 >= 0) {
		_field26 = -1;
	}
}

void GameModule::createModule2400(int which) {
	error("createModule2400");
}

void GameModule::createModule3000(int which) {
	error("createModule3000");
}

} // End of namespace Neverhood
