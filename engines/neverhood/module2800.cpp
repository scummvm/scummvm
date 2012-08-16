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

#include "neverhood/module2800.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1000.h"
#include "neverhood/module1200.h"
#include "neverhood/module1700.h"

namespace Neverhood {

Module2800::Module2800(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_fileHash = 0;
	// TODO music stuff
	// TODO Music18hList_add(0x64210814, 0xD2FA4D14);
	setGlobalVar(0x28D8C940, 1);
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, which);
	} else if (which == 2) {
		createScene(4, 3);
	} else if (which == 1) {
		createScene(4, 1);
	} else {
		createScene(0, 0);
	}

}

Module2800::~Module2800() {
	// TODO music stuff
	// TODO Sound1ChList_sub_407A50(0x64210814);
	// TODO Module2800_sub471DF0();
}

void Module2800::createScene(int sceneNum, int which) {
	debug("Module2800::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 0);
		_childObject = new Scene2801(_vm, this, which);
		break;
	case 1:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 0);
#if 0		
		_flag = true; // DEBUG!
		if (_flag) {
			_childObject = new Scene2802(_vm, this, which);
		} else {
			_childObject = new Class152(_vm, this, 0x000C6444, 0xC6440008);
		}
#endif		
		break;
	//		
	case 1001:
		break;
	// TODO ...		
	}
	SetUpdateHandler(&Module2800::updateScene);
	_childObject->handleUpdate();
}

void Module2800::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult != 2) {
				// TODO music stuff
			}			
			if (_moduleResult == 1) {
				createScene(2, 0);
			} else if (_moduleResult == 2) {
				createScene(1, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 0) {
				createScene(0, 2);
			} else {
				createScene(1001, -1);
			}
			break;
		//		
		case 1001:
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 0:
			// TODO Module2800_sub4731E0(true);
			break;
		case 1:
			// TODO Module2800_sub4731E0(false);
			break;
		}
	}
}

Scene2801::Scene2801(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	// TODO Weird palette glitches in the mouse cursor and sprite, check this later

	// TODO _vm->gameModule()->initScene2801Vars();

	_surfaceFlag = true;
	SetMessageHandler(&Scene2801::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (getGlobalVar(0x4DE80AC0) == 0) {
		insertStaticSprite(0x0001264C, 100);
	}

	if (which < 0) {
		insertKlayman<KmScene2801>(194, 430);
		setMessageList(0x004B6BB8);
	} else if (which == 1) {
		insertKlayman<KmScene2801>(443, 398);
		setMessageList(0x004B6BC0);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene2801>(312, 432);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene2801>(194, 432);
		}
		setMessageList(0x004B6C10);
	} else {
		insertKlayman<KmScene2801>(0, 432);
		setMessageList(0x004B6BB0);
	}

	if (getGlobalVar(0x09880D40)) {
		setRectList(0x004B6CE0);
		setBackground(0x01400666);
		setPalette(0x01400666);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x01400666, 0, 256, 0);
		_sprite1 = insertStaticSprite(0x100CA0A8, 1100);
		_sprite2 = insertStaticSprite(0x287C21A4, 1100);
		_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x0066201C);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape); 
	} else if (getGlobalVar(0x08180ABC)) {
		setRectList(0x004B6CD0);
		setBackground(0x11E00684);
		setPalette(0x11E00684);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x11E00684, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x061601C8, 1100);
		_klayman->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x00680116);
		_asTape = insertSprite<Class606>(this, 8, 1100, 302, 437, 0x01142428);
		_vm->_collisionMan->addSprite(_asTape); 
	} else {
		setRectList(0x004B6CF0);
		setBackground(0x030006E6);
		setPalette(0x030006E6);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x030006E6, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x273801CE, 1100);
		_klayman->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x006E2038);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape); 
	}
	
	addEntity(_palette);

	if (which == 1) {
		_palette->addPalette(0xB103B604, 0, 65, 0);
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
	} else {
		_palette->addPalette(_paletteHash, 0, 65, 0);
		_palette->addBasePalette(_paletteHash, 0, 65, 0);
	}
	
}

Scene2801::~Scene2801() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2801::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	// TODO: case 0x000D:
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B6C40);
		}
		break;
	case 0x482A:
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	case 0x482B:
		_palette->addBasePalette(_paletteHash, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
