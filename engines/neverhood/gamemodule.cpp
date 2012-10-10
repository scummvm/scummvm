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
#include "neverhood/module1100.h"
#include "neverhood/module1200.h"
#include "neverhood/module1300.h"
#include "neverhood/module1400.h"
#include "neverhood/module1500.h"
#include "neverhood/module1600.h"
#include "neverhood/module1700.h"
#include "neverhood/module1800.h"
#include "neverhood/module1900.h"
#include "neverhood/module2000.h"
#include "neverhood/module2100.h"
#include "neverhood/module2200.h"
#include "neverhood/module2300.h"
#include "neverhood/module2400.h"
#include "neverhood/module2500.h"
#include "neverhood/module2600.h"
#include "neverhood/module2700.h"
#include "neverhood/module2800.h"
#include "neverhood/module3000.h"

namespace Neverhood {

GameModule::GameModule(NeverhoodEngine *vm)
	: Module(vm, NULL), _moduleNum(-1) {
	
	// Other initializations moved to actual engine class
	// TODO
	_vm->_soundMan->playSoundThree(0x002D0031, 0x8861079);
	SetMessageHandler(&GameModule::handleMessage);
}

GameModule::~GameModule() {

	_vm->_soundMan->deleteSoundGroup(0x002D0031);
	delete _childObject;
	_childObject = NULL;
	// TODO: Set palette to black but probably not neccessary
	// TODO SoundMan_deinit();
	// TODO Set debug vars (maybe)
}

void GameModule::handleMouseMove(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseMove(%d, %d)", x, y);
		sendPointMessage(_childObject, 0, mousePos);
	}				
}

void GameModule::handleMouseDown(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseDown(%d, %d)", x, y);
		sendPointMessage(_childObject, 1, mousePos);
	}				
}

void GameModule::initScene1307Vars() {

	// Exit if it's already initialized
	if (getSubVar(0x40050052, 0x25400B10))
		return;

	for (uint i = 0; i < 3; i++) {
		bool more;
		do {
			more = false;
			setSubVar(0x0C10A000, i, _vm->_rnd->getRandomNumber(16 - 1));
			if (i > 0) {
				for (uint j = 0; j < i && !more; j++) {
					more = getSubVar(0x0C10A000, j) == getSubVar(0x0C10A000, i);
				}
			}
		} while (more);
	}

	for (uint i = 0; i < 3; i++) {
		bool more;
		do {
			more = false;
			setSubVar(0xA010B810, i, _vm->_rnd->getRandomNumber(16 - 1));
			if (i > 0) {
				for (uint j = 0; j < i && !more; j++) {
					more = getSubVar(0xA010B810, j) == getSubVar(0xA010B810, i);
				}
			}
			if (getSubVar(0xA010B810, i) == getSubVar(0x0C10A000, i))
				more = true;
		} while (more);
	}

	setSubVar(0x40050052, 0x25400B10, 1);

}

void GameModule::initScene1405Vars() {

	// TODO: Give better names

	byte array44[3];
	byte array3C[10];
	byte array30[48];
	uint32 index3 = 48;
	uint32 index2 = 9;
	uint32 index1 = 2;
	uint32 rndIndex;

	// Exit if it's already initialized
	if (getSubVar(0x40050052, 0xC8606803))
		return;

	for (uint32 i = 0; i < 3; i++)
		setSubVar(0x61084036, i, 1);

	for (byte i = 0; i < 3; i++)
		array44[i] = i;

	for (byte i = 0; i < 10; i++)
		array3C[i] = i;

	for (byte i = 0; i < 48; i++)
		array30[i] = i;

	rndIndex = _vm->_rnd->getRandomNumber(3 - 1);

	setSubVar(0x13100631, array44[rndIndex], 5);

	for (byte i = 5; i < 9; i++)
		array3C[i] = array3C[i + 1];

	while (rndIndex < 2) {
		array44[rndIndex] = array44[rndIndex + 1];
		rndIndex++;
	}

	for (int i = 0; i < 2; i++) {
		uint32 rndIndex1 = _vm->_rnd->getRandomNumber(index2 - 1); // si
		uint32 rndIndex2 = _vm->_rnd->getRandomNumber(index1 - 1); // di
		setSubVar(0x13100631, array44[rndIndex2], array3C[rndIndex1]);
		index2--;
		while (rndIndex1 < index2) {
			array3C[rndIndex1] = array3C[rndIndex1 + 1];
			rndIndex1++;
		}
		index1--;
		while (rndIndex2 < index1) {
			array44[rndIndex2] = array44[rndIndex2 + 1];
			rndIndex2++;
		}
	}

	for (uint32 i = 0; i < 3; i++) {
		uint32 rndValue = _vm->_rnd->getRandomNumber(4 - 1) * 2 + 2;
		uint32 index4 = 0;
		setSubVar(0x7500993A, i, rndValue);
		while (index4 < rndValue) {
			uint32 rndIndex3 = _vm->_rnd->getRandomNumber(index3 - 1);
			setSubVar(0x0C65F80B, array30[rndIndex3], getSubVar(0x13100631, i));
			index3--;
			while (rndIndex3 < index3) {
				array30[rndIndex3] = array30[rndIndex3 + 1];
				rndIndex3++;
			}
			index4++;
		}
	}

	uint32 index5 = 0;
	while (index3 != 0) {
		uint32 rndIndex4 = _vm->_rnd->getRandomNumber(index3 - 1);
		index1 = array3C[index5];
		setSubVar(0x0C65F80B, array30[rndIndex4], index1);
		index3--;
		while (rndIndex4 < index3) {
			array30[rndIndex4] = array30[rndIndex4 + 1];
			rndIndex4++;
		}
		uint32 rndIndex5 = _vm->_rnd->getRandomNumber(index3 - 1);
		setSubVar(0x0C65F80B, array30[rndIndex5], index1);
		index3--;
		while (rndIndex5 < index3) {
			array30[rndIndex5] = array30[rndIndex5 + 1];
			rndIndex5++;
		}
		index5++;
		if (index5 >= index2)
			index5 = 0;

	}

	setSubVar(0x40050052, 0xC8606803, 1);
	
}

void GameModule::initScene2401Vars() {

	if (getSubVar(0x40050052, 0x40520234))
		return;

	setSubVar(0x90405038, 0, 3);
	setSubVar(0x90405038, 1, 1);
	setSubVar(0x90405038, 2, 2);
	setSubVar(0x90405038, 3, 0);
	setSubVar(0x90405038, 4, 4);
		
	setSubVar(0x40050052, 0x40520234, 1);

}

void GameModule::initScene2808Vars1() {

	// Exit if it's already initialized
	if (getSubVar(0x40050052, 0x20479010))
		return;

	for (uint i = 0; i < 3; i++)
		setSubVar(0x0C601058, i, _vm->_rnd->getRandomNumber(3 - 1) + 1);

	setSubVar(0x40050052, 0x20479010, 1);

}

void GameModule::initScene2808Vars2() {

	// Exit if it's already initialized
	if (getSubVar(0x40050052, 0x66059818))
		return;

	for (uint i = 0; i < 3; i++)
		setSubVar(0x40005834, i, _vm->_rnd->getRandomNumber(6 - 1) + 1);

	setSubVar(0x40050052, 0x66059818, 1);
}

void GameModule::initScene3009Vars() {
	if (!getSubVar(0x40050052, 0x8C9819C2)) {
		for (int i = 0; i < 3; i++) {
			setSubVar(0x00504B86, i, _vm->_rnd->getRandomNumber(12 - 1));
			setSubVar(0x0A4C0A9A, i, _vm->_rnd->getRandomNumber(12 - 1));
		}
		setSubVar(0x40050052, 0x8C9819C2, 1);
	}
}

uint32 GameModule::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0800:
		_someFlag1 = true;
		return messageResult;		
	case 0x1009:
		_moduleResult = param.asInteger();
		_done = true;
		return messageResult;
	case 0x100A:
		// Unused resource preloading message
		return messageResult;
	case 0x101F:
		_field2C = true;		
		return messageResult;
	case 0x1023:
		// Unused resource preloading message
		return messageResult;
	}
	return messageResult;
}

void GameModule::startup() {
	// TODO: Displaying of error text probably not needed in ScummVM
//	createModule(1500, 0); // Logos and intro video //Real

	// DEBUG>>>
	/*
	setGlobalVar(V_SEEN_MUSIC_BOX, 1);
	setGlobalVar(0x0A18CA33, 0);
	setGlobalVar(0x0112090A, 0);
	//setGlobalVar(0x000CF819, 1);
	setGlobalVar(V_PROJECTOR_LOCATION, 2);
	*/
	//setGlobalVar(V_ENTRANCE_OPEN, 0);
	//setGlobalVar(V_DOOR_SPIKES_OPEN, 1);
	// <<<DEBUG

#if 1
	_vm->gameState().which = 0;
	_vm->gameState().sceneNum = 5;
	createModule(2200, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 0;
	createModule(1800, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 0;
	createModule(2000, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 5;
	createModule(2200, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 1;
	createModule(1000, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 1;
	createModule(1000, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 8;
	_vm->gameState().which = 1;
	createModule(1600, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 6;
	createModule(1900, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 0;
	createModule(2100, 3);
#endif
#if 0
	_vm->gameState().sceneNum = 8;
	createModule(2600, -1);
#endif
#if 0
	_vm->gameState().which = 0;
	_vm->gameState().sceneNum = 1;
	createModule(2700, -1);
#endif
#if 0
	setGlobalVar(V_KLAYMAN_SMALL, 1); // DEBUG Make Klayman small
	_vm->gameState().sceneNum = 2;
	createModule(2800, -1);
#endif
#if 0
	_vm->gameState().which = 0;
	_vm->gameState().sceneNum = 0;
	createModule(2500, -1);
#endif
#if 0
	_vm->gameState().sceneNum = 1;
	createModule(2400, -1);
#endif
}

void GameModule::createModule(int moduleNum, int which) {
	debug("GameModule::createModule(%d, %d)", moduleNum, which);
	_moduleNum = moduleNum;
	switch (_moduleNum) {
	case 1000:
		setGlobalVar(V_MODULE_NAME, 0x03294419);
		_childObject = new Module1000(_vm, this, which);
		break;
	case 1100:
		setGlobalVar(V_MODULE_NAME, 0x0002C818);
		_childObject = new Module1100(_vm, this, which);
		break;
	case 1200:
		setGlobalVar(V_MODULE_NAME, 0x00478311);
		_childObject = new Module1200(_vm, this, which);
		break;
	case 1300:
		setGlobalVar(V_MODULE_NAME, 0x0061C090);
		_childObject = new Module1300(_vm, this, which);
		break;
	case 1400:
		setGlobalVar(V_MODULE_NAME, 0x00AD0012);
		_childObject = new Module1400(_vm, this, which);
		break;
	case 1500:
		_someFlag1 = false;
		setGlobalVar(V_MODULE_NAME, 0x00F10114);
		_childObject = new Module1500(_vm, this, which, true);
		break;
	case 1600:
		setGlobalVar(V_MODULE_NAME, 0x01A008D8);
		_childObject = new Module1600(_vm, this, which);
		break;
	case 1700:
		setGlobalVar(V_MODULE_NAME, 0x04212331);
		_childObject = new Module1700(_vm, this, which);
		break;
	case 1800:
		setGlobalVar(V_MODULE_NAME, 0x04A14718);
		_childObject = new Module1800(_vm, this, which);
		break;
	case 1900:
		setGlobalVar(V_MODULE_NAME, 0x04E1C09C);
		_childObject = new Module1900(_vm, this, which);
		break;
	case 2000:
		setGlobalVar(V_MODULE_NAME, 0x08250000);
		_childObject = new Module2000(_vm, this, which);
		break;
	case 2100:
		setGlobalVar(V_MODULE_NAME, 0x10A10C14);
		_childObject = new Module2100(_vm, this, which);
		break;
	case 2200:
		setGlobalVar(V_MODULE_NAME, 0x11391412);
		_childObject = new Module2200(_vm, this, which);
		break;
	case 2300:
		setGlobalVar(V_MODULE_NAME, 0x1A214010);
		_childObject = new Module2300(_vm, this, which);
		break;
	case 2400:
		setGlobalVar(V_MODULE_NAME, 0x202D1010);
		_childObject = new Module2400(_vm, this, which);
		break;
	case 2500:
		setGlobalVar(V_MODULE_NAME, 0x29220120);
		_childObject = new Module2500(_vm, this, which);
		break;
	case 2600:
		setGlobalVar(V_MODULE_NAME, 0x40271018);
		_childObject = new Module2600(_vm, this, which);
		break;
	case 2700:
		setGlobalVar(V_MODULE_NAME, 0x42212411);
		_childObject = new Module2700(_vm, this, which);
		break;
	case 2800:
		setGlobalVar(V_MODULE_NAME, 0x64210814);
		_childObject = new Module2800(_vm, this, which);
		break;
	case 3000:
		setGlobalVar(V_MODULE_NAME, 0x81293110);
		_childObject = new Module3000(_vm, this, which);
		break;
	default:
		error("GameModule::createModule() Could not create module %d", moduleNum);
	}
	SetUpdateHandler(&GameModule::updateModule);
	_childObject->handleUpdate();
}

void GameModule::updateModule() {
	if (!updateChild()) {
		switch (_moduleNum) {
		case 1000:
			createModule(2300, 0);
			break;
		case 1200:
			if (_moduleResult == 1) {
				createModule(2600, 0);
			} else {
				createModule(2300, 2);
			}
			break;
		case 1100:
			if (_moduleResult == 0) {
				createModule(2900, 2);
			} else {
				setGlobalVar(V_ENTRANCE_OPEN, 1);
				createModule(1300, 0);
			}
			break;
		case 1300:
			if (_moduleResult == 1) {
				// TODO _gameState.clear();
				// TODO GameModule_handleKeyEscape
			} else {
				createModule(2900, 0);
			}
			break;
		case 1400:
			if (_moduleResult == 1) {
				error("WEIRD!");
			} else {
				createModule(1600, 1);
			}
			break;
		case 1500:
			createModule(1000, 0);
			break;
		case 1600:
			if (_moduleResult == 1) {
				createModule(1400, 0);
			} else if (_moduleResult == 2) {
				createModule(1700, 0);
			} else {
				createModule(2100, 0);
			}
			break;
		case 1700:
			if (_moduleResult == 1) {
				createModule(2900, 3);
			} else {
				createModule(1600, 2);
			}
			break;
		case 1800:
			if (_moduleResult == 1) {
				// TODO GameState_clear();
				// TODO GameModule_handleKeyEscape();
			} else if (_moduleResult == 2) {
				createModule(2700, 0);
			} else if (_moduleResult == 3) {
				createModule(3000, 3);
			} else {
				createModule(2800, 0);
			}
			break;
		case 1900:
			createModule(3000, 1);
			break;
		case 2000:
			createModule(2900, 4);
			break;
		case 2100:
			if (_moduleResult == 1) {
				createModule(2900, 1);
			} else {
				createModule(1600, 0);
			}
			break;
		case 2200:
			createModule(2300, 1);
			break;
		case 2300:
			if (_moduleResult == 1) {
				createModule(2200, 0);
			} else if (_moduleResult == 2) {
				createModule(1200, 0);
			} else if (_moduleResult == 3) {
				createModule(2400, 0);
			} else if (_moduleResult == 4) {
				createModule(3000, 0);
			} else {
				createModule(1000, 1);
			}
			break;
		case 2400:
			createModule(2300, 3);
			break;
		case 2500:
			createModule(2600, 1);
			break;
		case 2600:
			if (_moduleResult == 1) {
				createModule(2500, 0);
			} else {
				createModule(1200, 1);
			}
			break;
		case 2700:
			createModule(1800, 2);
			break;
		case 2800:
			if (_moduleResult == 1) {
				createModule(2900, 5);
			} else {
				createModule(1800, 0);
			}
			break;
		case 3000:
			if (_moduleResult == 1) {
				createModule(1900, 0);
			} else if (_moduleResult == 2) {
				// WEIRD: Sets the errorFlag
			} else if (_moduleResult == 3) {
				createModule(1800, 3);
			} else if (_moduleResult == 4) {
				createModule(3000, 0);
			} else {
				createModule(2300, 4);
			}
			break;
		}
	}
}

} // End of namespace Neverhood
