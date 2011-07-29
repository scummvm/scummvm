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

#include "neverhood/module3000.h"
#include "neverhood/navigationscene.h"

namespace Neverhood {

Module3000::Module3000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _moduleDone(false), _soundVolume(0) {
	
	debug("Create Module3000(%d)", which);

	// TODO Sound1ChList_addSoundResources(0x81293110, dword_4B7FC8, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 1, 50, 600, 5, 150);
	// TODO Sound1ChList_setSoundValues(0x90F0D1C3, false, 20000, 30000, 20000, 30000);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);

    _flag = getGlobalVar(0x10938830) != 0;

	if (_flag) {
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
		// TODO Sound1ChList_playLooping(0x90F0D1C3);
	}

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		default:
		case 1:
			createScene3002(-1);
			break;
		case 2:
			createScene3003(-1);
			break;
		case 3:
			createScene3004(-1);
			break;
		case 4:
			createScene3005(-1);
			break;
		case 5:
			createScene3006(-1);
			break;
		case 6:
			createScene3007(-1);
			break;
		case 7:
			createScene3008(-1);
			break;
		case 8:
			createScene3009(-1);
			break;
		case 9:
			createScene3010(-1);
			break;
		case 10:
			createScene3011(-1);
			break;
		case 11:
			createScene3012(-1);
			break;
		case 12:
			createScene3013(-1);
			break;
		}
	} else if (which == 0) {
		createScene3002(0);
	} else if (which == 1) {
		createScene3005(2);
	} else if (which == 2) {
		createScene3005(1);
	} else if (which == 3) {
		createScene3006(1);
	}

	SetMessageHandler(&Module3000::handleMessage);

}

Module3000::~Module3000() {
	// TODO Sound1ChList_sub_407A50(0x81293110);
}

uint32 Module3000::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1009:
		_moduleDone = true;
		_moduleDoneStatus = param.asInteger();
		break;
	}
	return messageResult;
}

void Module3000::createScene3002(int which) {
	_vm->gameState().sceneNum = 1;
	if (!getGlobalVar(0x01BA1A52)) {
		createNavigationScene(0x004B7C80, which);
	} else if (getGlobalVar(0x10938830)) {
		createNavigationScene(0x004B7CE0, which);
	} else {
		createNavigationScene(0x004B7CB0, which);
	}
	SetUpdateHandler(&Module3000::updateScene3002);
}

void Module3000::createScene3002b(int which) {
	_vm->gameState().sceneNum = 1;
	if (!getGlobalVar(0x01BA1A52)) {
		if (getGlobalVar(0x10938830)) {
			createSmackerScene(0x00940021, true, true, false);
		} else {
			createSmackerScene(0x01140021, true, true, false);
		}
	} else {
		if (getGlobalVar(0x10938830)) {
			createSmackerScene(0x001011B1, true, true, false);
		} else {
			createSmackerScene(0x001021B1, true, true, false);
		}
	}
	SetUpdateHandler(&Module3000::updateScene3002b);
	setGlobalVar(0x01BA1A52, getGlobalVar(0x01BA1A52) ? 0 : 1);
}

void Module3000::createScene3003(int which) {
	_vm->gameState().sceneNum = 2;
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
	if (_flag) {
		_soundVolume = 90;
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 90);
	}
	if (getGlobalVar(0x10938830)) {
		createNavigationScene(0x004B7D58, which);
	} else {
		createNavigationScene(0x004B7D10, which);
	}
	SetUpdateHandler(&Module3000::updateScene3003);
}

void Module3000::createScene3004(int which) {
	_vm->gameState().sceneNum = 3;
	if (getGlobalVar(0x09221A62)) {
		createNavigationScene(0x004B7E60, which);
	} else if (getGlobalVar(0x10938830)) {
		createNavigationScene(0x004B7DA0, which);
	} else {
		createNavigationScene(0x004B7E00, which);
	}
	SetUpdateHandler(&Module3000::updateScene3004);
}

void Module3000::createScene3005(int which) {
	_vm->gameState().sceneNum = 4;
	if (getGlobalVar(0x09221A62)) {
		createNavigationScene(0x004B7F20, which);
	} else {
		createNavigationScene(0x004B7EC0, which);
	}
	SetUpdateHandler(&Module3000::updateScene3005);
}

void Module3000::createScene3006(int which) {
	static const byte kNavigationTypes[] = {3, 0};
	_vm->gameState().sceneNum = 5;
	createNavigationScene(0x004B7F80, which, kNavigationTypes);
	SetUpdateHandler(&Module3000::updateScene3006);
}

void Module3000::createScene3007(int which) {
	static const byte kNavigationTypes[] = {5};
	_vm->gameState().sceneNum = 6;
	createNavigationScene(0x004B7FB0, which, kNavigationTypes);
	SetUpdateHandler(&Module3000::updateScene3007);
}

void Module3000::createScene3008(int which) {
	_vm->gameState().sceneNum = 6;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 0, 0, 0, 0, 0);
	if (!getSubVar(0x40050052, 0x089809C2)) {
		setSubVar(0x40050052, 0x089809C2, 1);
		createSmackerScene(0x90022001, true, true, false);
	} else {
		createSmackerScene(0x98022001, true, true, false);
	}
	SetUpdateHandler(&Module3000::updateScene3002b);
}

void Module3000::createScene3009(int which) {
	_vm->gameState().sceneNum = 7;
	_childObject = new Scene3009(_vm, this, which);
	SetUpdateHandler(&Module3000::updateScene3009);
}

void Module3000::createScene3010(int which) {
	_vm->gameState().sceneNum = 9;
	_childObject = new Scene3010(_vm, this, 0);
	SetUpdateHandler(&Module3000::updateScene3010);
}

void Module3000::createScene3011(int which) {
	// TODO
}

void Module3000::createScene3012(int which) {
	_vm->gameState().sceneNum = 11;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 0, 0, 0, 0, 0);
	if (!getSubVar(0x40050052, 0x10130993)) {
		setSubVar(0x40050052, 0x10130993, 1);
		createSmackerScene(0x31093019, true, true, false);
	} else {
		createSmackerScene(0x20093019, true, true, false);
	}
	SetUpdateHandler(&Module3000::updateScene3002b);
}

void Module3000::createScene3013(int which) {
	_vm->gameState().sceneNum = 12;
	_childObject = new Scene3010(_vm, this, 1);
	SetUpdateHandler(&Module3000::updateScene3002b);
}

void Module3000::updateScene3002() {
	_childObject->handleUpdate();
#if 0 // ALL TODO
	if (navigationScene()->getSoundFlag1()) {
		uint32 frameNumber = navigationScene()->getFrameNumber();
		int navigationIndex = navigationScene()->getIndex();
		if (navigationIndex == 1) {
			if (frameNumber == 0) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
				// TODO Sound1ChList_setVolume(0x48498E46, 70);
				// TODO Sound1ChList_setVolume(0x50399F64, 70);
			} else if (frameNumber == 100) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
			}
		} else if (navigationIndex == 0) {
			if (frameNumber == 0) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
				// TODO Sound1ChList_setVolume(0x48498E46, 70);
				// TODO Sound1ChList_setVolume(0x50399F64, 70);
			} else if (frameNumber == 10) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
			}
			if (_flag && _soundVolume < 90 && frameNumber % 2) {
				if (frameNumber == 0)
					_soundVolume = 40;
				else
					_soundVolume++;
				// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
			}
		}
	}
#endif
	if (_moduleDone) {
		int areaType = navigationScene()->getNavigationAreaType();
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (!getGlobalVar(0x01BA1A52)) {
			if (_moduleDoneStatus == 0) {
				createScene3010(-1);
				_childObject->handleUpdate();
			} else if (_moduleDoneStatus == 1) {
				_parentModule->sendMessage(0x1009, 0, this);
			}
		} else {
			if (_moduleDoneStatus == 0) {
				if (areaType == 2) {
					createScene3003(0);
					_childObject->handleUpdate();
				} else {
					//createScene3002b(-1);
					_childObject->handleUpdate();
				}
			} else if (_moduleDoneStatus == 1) {
				_parentModule->sendMessage(0x1009, 0, this);
			}
		}
	}
}

void Module3000::updateScene3002b() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		switch (_vm->gameState().sceneNum) {
		case 1:
			if (getGlobalVar(0x01BA1A52)) {
				createScene3002(0);
				_childObject->handleUpdate();
			} else {
				createScene3013(-1);
				_childObject->handleUpdate();
			}
			break;
		case 7:
		case 8:
			createScene3009(-1);
			break;
		case 11:
			_parentModule->sendMessage(0x1009, 3, this);
			break;
		case 12:
			createScene3002(0);
			_childObject->handleUpdate();
			break;
		default:
			createScene3006(0);
			break;
		}
	}
}

void Module3000::updateScene3003() {
	_childObject->handleUpdate();
#if 0 // ALL TODO
	if (navigationScene()->getSoundFlag1()) {
		uint32 frameNumber = navigationScene()->getFrameNumber();
		int navigationIndex = navigationScene()->getIndex();
		if (_flag && _soundVolume > 1 && frameNumber % 2) {
			_soundVolume--;
			// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
		}
		if (navigationIndex == 0) {
			if (frameNumber == 35) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
			}
		} else if (navigationIndex == 1) {
			if (frameNumber == 55) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
				// TODO Sound1ChList_setVolume(0x48498E46, 70);
				// TODO Sound1ChList_setVolume(0x50399F64, 70);
			}
		}
	}
#endif
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
		if (_flag) {
			_soundVolume = 0;
			// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
		}
		if (_moduleDoneStatus == 0) {
			createScene3004(0);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 1) {
			setGlobalVar(0x01BA1A52, 0);
			createScene3002(1);
			_childObject->handleUpdate();
		}
	}
}

void Module3000::updateScene3004() {
	_childObject->handleUpdate();
#if 0 // ALL TODO
	if (navigationScene()->getSoundFlag1()) {
		uint32 frameNumber = navigationScene()->getFrameNumber();
		int navigationIndex = navigationScene()->getIndex();
		if (navigationIndex == 2) {
			if (frameNumber == 40) {
				// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
			}
			if (_flag && _soundVolume < 90 && frameNumber % 2) {
				if (frameNumber == 0)
					_soundVolume = 40;
				else
					_soundVolume++;
				// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
			}
		}
	}
#endif
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (_moduleDoneStatus == 1) {
			createScene3005(0);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 3) {
			createScene3011(-1);
			_childObject->handleUpdate();
		} else if (getGlobalVar(0x09221A62)) {
			createScene3006(0);
			_childObject->handleUpdate();
		} else {
			createScene3003(1);
			_childObject->handleUpdate();
		}
	}
}

void Module3000::updateScene3005() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (_moduleDoneStatus == 0) {
			_parentModule->sendMessage(0x1009, 1, this);
		} else if (_moduleDoneStatus == 1) {
			createScene3008(-1);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 2) {
			createScene3004(3);
			_childObject->handleUpdate();
		}
	}
	// NOTE: Skipped resource preloading stuff
}

void Module3000::updateScene3006() {
	_childObject->handleUpdate();
#if 0 // ALL TODO
	if (navigationScene()->getSoundFlag1() && navigationScene()->getIndex() == 0) {
		// TODO Sound1ChList_sub_4080B0(false);
	}
#endif
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (_moduleDoneStatus == 0) {
			createScene3007(0);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 1) {
			createScene3004(0);
			_childObject->handleUpdate();
		}
	}
}

void Module3000::updateScene3007() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		int areaType = navigationScene()->getNavigationAreaType();
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (areaType == 4) {
			createScene3012(-1);
			_childObject->handleUpdate();
		} else {
			createSmackerScene(0x080810C5, true, true, false);
			SetUpdateHandler(&Module3000::updateScene3002b);
		}
	}
}

void Module3000::updateScene3009() {
	_childObject->handleUpdate();
	// TODO...
}

void Module3000::updateScene3010() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_moduleDoneStatus == 0 || _moduleDoneStatus == 2) {
			createScene3002(0);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 1) {
			createScene3002b(-1);
			_childObject->handleUpdate();
		}
	}
}

// Scene3009

static const uint32 kScene3009SmackerFileHashes[] = {
	0x1010000D,
	0x340A0049,
	0x340A0049,
	0x0282081D,
	0x0082080D,
	0x0882080D,
	0x0882080D,
	0x0282081D,
	0x004B000B,
	0x014B000B,
	0x044B000B,
	0x0282081D,
	0x0282081D,
	0x0282081D,
	0x340A0049
};

static const uint32 kScene3009VarValues[] = {
	0x00000000,
	0x8004001B,
	0x0004001A,
	0x1048404B,
	0x50200109,
	0x12032109,
	0x10201109,
	0x000A2030,
	0x000A0028,
	0x000A0028,
	0x000A0028,
	0x040A1069,
	0x040A1069,
	0x040A1069,
	0x240A1101
};

static const uint32 kClass439FileHashes[] = {
	0x618827A0,
	0xB1A92322
};

static const uint32 kClass440FileHashes[] = {
	0x4011018C,
	0x15086623
};

static const NPoint kClass524Points[] = {
	{289, 338},
	{285, 375},
	{284, 419},
	{456, 372},
	{498, 372},
	{541, 372}
};

static const uint32 kClass524FileHashes[] = {
	0x24542582,
	0x1CD61D96
};

static const uint32 kClass441FileHashes1[] = {
	0x24016060,  
	0x21216221,
	0x486160A0,
	0x42216422,
	0x90A16120,
	0x84216824,
	0x08017029,
	0x08217029,
	0x10014032,
	0x10214032,
	0x20012004,
	0x20212004
};

static const uint32 kClass441FileHashes2[] = {
	0x40092024, 
	0x01636002,
	0x8071E028,
	0x02A56064,
	0x00806031,
	0x052960A8,
	0x0A116130,
	0x0A316130,
	0x14216200,
	0x14016200,
	0x28416460,
	0x28616460
};

Class438::Class438(NeverhoodEngine *vm, Scene3009 *parentScene)
	: StaticSprite(vm, 1400), _soundResource(vm), _parentScene(parentScene),
	_flag1(false) {
	
	_spriteResource.load2(0x120B24B0);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = _spriteResource.getDimensions().width;
	_deltaRect.height = _spriteResource.getDimensions().height;
	_surface->setVisible(false);
	processDelta();
	_needRefresh = true;
	SetUpdateHandler(&Class438::update);
	SetMessageHandler(&Class438::handleMessage);
	_soundResource.load(0x3901B44F);
}

void Class438::update() {
	StaticSprite::update();
	if (_flag1 && !_soundResource.isPlaying()) {
		_parentScene->sendMessage(0x2000, 0, this);
		_surface->setVisible(false);
	}
}

uint32 Class438::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_flag1 && !_parentScene->sub462E90()) {
			_flag1 = true;
			_surface->setVisible(true);
			_soundResource.play();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

Class439::Class439(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400), _blinkCountdown(0) {

	_spriteResource.load2(kClass439FileHashes[index]);
	createSurface(600, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;
	if (getGlobalVar(0x0C0288F4)) {
		hide();
	} else {
		startBlinking();
	}
	SetUpdateHandler(&Class439::update);
}

void Class439::update() {
	if (_blinkCountdown != 0 && (--_blinkCountdown == 0)) {
		if (_blinkToggle) {
			_surface->setVisible(true);
		} else {
			_surface->setVisible(false);
		}
		StaticSprite::update();
		_blinkCountdown = 3;
		_blinkToggle = !_blinkToggle;
	}
}

void Class439::show() {
	_surface->setVisible(true);
	StaticSprite::update();
	_blinkCountdown = 0;
}

void Class439::hide() {
	_surface->setVisible(false);
	StaticSprite::update();
	_blinkCountdown = 0;
}

void Class439::startBlinking() {
	_surface->setVisible(true);
	StaticSprite::update();
	_blinkCountdown = 3;
	_blinkToggle = true;
}

Class440::Class440(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400) {

	_spriteResource.load2(kClass440FileHashes[index]);
	createSurface(600, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_surface->setVisible(false);
	_needRefresh = true;
}

Class522::Class522(NeverhoodEngine *vm, Scene3009 *parentScene, int index)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {

	_x = 300;
	_y = getGlobalVar(0x000809C2) ? 52 : 266;
	createSurface1(0xC2463913, 1200);
	_needRefresh = true;
	updatePosition();
	_surface->setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class522::handleMessage);
}

void Class522::show() {
	setFileHash(0xC2463913, 0, -1);
	_surface->setVisible(true);
	updatePosition();
	_enabled = true;
}

uint32 Class522::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled) {
			_parentScene->sendMessage(0x2002, 0, this);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

Scene3009::Scene3009(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag1(false), _flag2(false), 
	_flag3(false), _flag4(false), _countdown1(1), _countdown2(1) {
	
	_varValue = getGlobalVar(0x20580A86);
	
	// TODO _vm->gameModule()->initScene3009Vars();
	
	setGlobalVar(0xF0402B0A, 0);
	_surfaceFlag = true;
	
	_vm->_screen->clear();
	
	_background = addBackground(new DirtyBackground(_vm, 0xD000420C, 0, 0));
	_palette = new Palette(_vm, 0xD000420C);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse435(_vm, 0x04208D08, 20, 620));

    _class438 = addSprite(new Class438(_vm, this));
    _vm->_collisionMan->addSprite(_class438);

    _class522 = new Class522(_vm, this, _varValue);
    addSprite(_class522);
    _vm->_collisionMan->addSprite(_class522);

#if 0
    _class523 = new Class523(_vm, this, _varValue);
    addSprite(_class523);
    _vm->_collisionMan->addSprite(_class523);
#endif

	if (_varValue != 0 && _varValue != 8 && _varValue != 9 && _varValue != 9) {
		_flag1 = true;
	} else {
		_flag1 = false;
		if (_varValue == 0) {
#if 0		
			_class523->stMoveUp();
#endif			
			_flag4 = true;
		}
	}

	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, kScene3009SmackerFileHashes[_varValue], false, _flag1));
	_smackerPlayer->setDrawPos(89, 37);

	addSprite(new StaticSprite(_vm, 0x8540252C, 400));

	for (int i = 0; i < 2; i++) {
		_class439Array[i] = new Class439(_vm, i);
		addSprite(_class439Array[i]);
		_class440Array[i] = new Class440(_vm, i);
		addSprite(_class440Array[i]);
	}


#if 0
	for (int i = 0; i < 6; i++) {
		_class524Array[i] = new Class524(_vm, this, i);
		addSprite(_class524Array[i]);
		if (i < 3)
			_varValueArray[i] = getSubVar(0x00504B86, i);
		else
			_varValueArray[i] = getSubVar(0x0A4C0A9A, i - 3);
	}
#endif

	SetMessageHandler(&Scene3009::handleMessage);
	SetUpdateHandler(&Scene3009::update);
}

void Scene3009::update() {
	Scene::update();
	if (!_flag1 && _smackerPlayer->getFrameNumber() + 1 == _smackerPlayer->getFrameCount() && _varValue <= 14) {
		switch (_varValue) {
		case 0:
		case 14:
			_smackerPlayer->open(0x340A0049, true);
			_palette->usePalette();
			_flag1 = true;
			break;
		case 8:
			_smackerPlayer->open(0x0082080D, true);
			_palette->usePalette();
			_flag1 = true;
			_flag4 = false;
			break;
		case 9:
			_smackerPlayer->open(0x0282080D, true);
			_palette->usePalette();
			_flag1 = true;
			_flag4 = false;
			break;
		case 10:
			_smackerPlayer->open(0x0882080D, true);
			_palette->usePalette();
			_flag1 = true;
			_flag4 = false;
			break;
		case 11:
		case 12:
		case 13:
			if (_flag2) {
				if (_varValue == 11)
					_smackerPlayer->open(0x110A000F, false);
				else if (_varValue == 12)				
					_smackerPlayer->open(0x500B004F, false);
				else if (_varValue == 13)				
					_smackerPlayer->open(0x100B010E, false);
				_palette->usePalette();
				_flag2 = false;
#if 0				
				_class523->stMoveDown();
#endif				
			} else {
				sub462DC0();
			}
			break;
		}
	}

	if (_countdown1 != 0 && (--_countdown1 == 0) && sub462E10()) {
#if 0
		for (int i = 0; i < 3; i++)
			_class524Array[i]->hide();
#endif
		if (!getGlobalVar(0x0C0288F4) || getGlobalVar(0x000809C2) || getGlobalVar(0x9040018A)) {
			_class439Array[0]->show();
			_class440Array[0]->getSurface()->setVisible(true);
			// TODO _class440Array->StaticSprite_update
			_class522->show();
		}
	}

	if (_countdown2 != 0 && (--_countdown2 == 0) && sub462E50()) {
#if 0
		for (int i = 0; i < 6; i++)
			_class524Array[i]->hide();
#endif
		if (!getGlobalVar(0x0C0288F4) || getGlobalVar(0x000809C2) || getGlobalVar(0x9040018A)) {
			_class439Array[1]->show();
			_class440Array[1]->getSurface()->setVisible(true);
#if 0
			// TODO _class440Array[1]->StaticSprite_update
			_class523->show();
#endif
		}
	}

}

uint32 Scene3009::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !getGlobalVar(0x000809C2)) {
			setGlobalVar(0x20580A86, 0);
			_parentModule->sendMessage(0x1009, 0, this);
		}
		break;
	case 0x000D:
		// TODO: Debug stuff
		break;
	case 0x2000:
		if (!getGlobalVar(0x000809C2)) {
			if (!getGlobalVar(0x10938830)) {
				_varValue = 1;
				setGlobalVar(0x10938830, 1);
			} else {
				_varValue = 2;
			}
		} else if (!getGlobalVar(0x9040018A)) {
			_varValue = 3;
		} else if (!getGlobalVar(0x610210B7)) {
			_varValue = 4;
		} else if (!getGlobalVar(0x0C0288F4)) {
			setGlobalVar(0x0C0288F4, 1);
			_varValue = 5;
		} else {
			_varValue = 6;
		}
		sub462DC0();
		break;
	case 0x2001:
		_countdown1 = 24;
		break;
	case 0x2002:
		if (!getGlobalVar(0x9040018A) && !_flag4) {
			if (getGlobalVar(0x000809C2)) {
				_varValue = 14;
				setGlobalVar(0x000809C2, 0);
			} else {
				_varValue = 7;
				setGlobalVar(0x000809C2, 1);
			}
			sub462DC0();
		}
		break;
	case 0x2003:
		_countdown2 = 24;
		break;
	case 0x2004:
		if (getGlobalVar(0x000809C2)) {
			if (!getGlobalVar(0x9040018A)) {
				if (!getGlobalVar(0x610210B7)) {
					_varValue = 8;
				} else {
					if (!getGlobalVar(0x0C0288F4)) {
						_varValue = 9;
					} else {
						_varValue = 10;
					}
				}
				setGlobalVar(0x9040018A, 1);
				_flag4 = true;
				sub462DC0();
			} else if (!getGlobalVar(0x610210B7)) {
				_varValue = 11;
				_smackerPlayer->open(0x108A000F, false);
			} else if (!getGlobalVar(0x0C0288F4)) {
				_varValue = 12;
				_smackerPlayer->open(0x500B002F, false);
			} else {
				_varValue = 13;
				_smackerPlayer->open(0x100B008E, false);
			}
			_palette->usePalette();
			_flag2 = true;
			_flag4 = true;
			_flag1 = false;
			setGlobalVar(0x9040018A, 0);
		}
		break;
	}
	return 0;
}

void Scene3009::sub462DC0() {
	setGlobalVar(0x20580A86, _varValue);
	setGlobalVar(0xF0402B0A, kScene3009VarValues[_varValue]);
	_parentModule->sendMessage(0x1009, 1, this);
}

bool Scene3009::sub462E10() {
	for (int i = 0; i < 3; i++)
		if (_varValueArray[i] != getSubVar(0x00000914, i))
			return false;
	return true;
}

bool Scene3009::sub462E50() {
	for (int i = 0; i < 6; i++)
		if (_varValueArray[i] != getSubVar(0x00000914, i))
			return false;
	return true;
}

bool Scene3009::sub462E90() {
	return _flag3 || _flag4;
}

// Scene3010

static const uint32 kScene3010VarNameHashes[] = {
	0x304008D2,
	0x40119852,
	0x01180951
};

static const uint32 kScene3010DeadBoltButtonFileHashes1[] = {
	0x301024C2,
	0x20280580,
	0x30200452
};

static const uint32 kScene3010DeadBoltButtonFileHashes2[] = {
	0x50C025A8,
	0x1020A0A0,
	0x5000A7E8
};

static const NPoint kAsScene3010DeadBoltPoints[] = {
	{550, 307},
	{564, 415},
	{560, 514}
};

static const uint32 kAsScene3010DeadBoltFileHashes2[] = {
	0x181A0042,
	0x580A08F2,
	0x18420076
};

static const uint32 kAsScene3010DeadBoltFileHashes1[] = {
	0x300E105A,
	0x804E0052,
	0x040E485A
};

SsScene3010DeadBoltButton::SsScene3010DeadBoltButton(NeverhoodEngine *vm, Scene *parentScene, int buttonIndex, int initCountdown, bool initDisabled)
	: StaticSprite(vm, 900), _parentScene(parentScene), _soundResource1(vm),
	_soundResource2(vm), _soundResource3(vm), _buttonLocked(false), _countdown1(0), 
	_countdown2(0), _buttonIndex(buttonIndex) {

	NDimensions dimensions1, dimensions2;
	 
	_buttonEnabled = getSubVar(0x14800353, kScene3010VarNameHashes[_buttonIndex]) != 0;
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	dimensions1 = _spriteResource.getDimensions();
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	dimensions2 = _spriteResource.getDimensions();
	createSurface(400, 
		MAX(dimensions1.width, dimensions2.width),
		MAX(dimensions1.height, dimensions2.height));
	setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	if (initDisabled) {
		disableButton();
	} else if (_buttonEnabled) {
		_countdown1 = initCountdown * 12 + 1;
	}
	_soundResource1.load(0xF4217243);
	_soundResource2.load(0x44049000);
	_soundResource3.load(0x6408107E);
	SetUpdateHandler(&SsScene3010DeadBoltButton::update);
	SetMessageHandler(&SsScene3010DeadBoltButton::handleMessage);
}

void SsScene3010DeadBoltButton::update() {

	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		_soundResource1.play();
		_surface->setVisible(false);
		setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	}

	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		_surface->setVisible(true);
		setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	}

}

uint32 SsScene3010DeadBoltButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_buttonLocked && _countdown1 == 0) {
			if (_buttonEnabled) {
				_soundResource2.play();
				_soundResource3.play();
				_surface->setVisible(true);
				_buttonLocked = true;
				_parentScene->sendMessage(0x2000, _buttonIndex, this);
			} else {
				_parentScene->sendMessage(0x2002, _buttonIndex, this);
			}
			_needRefresh = true;
			StaticSprite::update();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void SsScene3010DeadBoltButton::disableButton() {
	_buttonLocked = true;
	setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	_surface->setVisible(true);
}

void SsScene3010DeadBoltButton::setSprite(uint32 fileHash) {
	_spriteResource.load(fileHash);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = _spriteResource.getDimensions().width;
	_deltaRect.height = _spriteResource.getDimensions().height;
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
}

void SsScene3010DeadBoltButton::setCountdown(int count) {
	_countdown2 = count * 18 + 1;
}

AsScene3010DeadBolt::AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_parentScene(parentScene), _boltIndex(boltIndex), _soundToggle(true), _unlocked(false), _locked(false),
	_countdown(0) {

    _x = kAsScene3010DeadBoltPoints[_boltIndex].x;
    _y = kAsScene3010DeadBoltPoints[_boltIndex].y;

	if (getSubVar(0x14800353, kScene3010VarNameHashes[_boltIndex])) {
		createSurface1(kAsScene3010DeadBoltFileHashes1[_boltIndex], 1200);
		setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
		_soundResource1.load(0x46005BC4);
	} else {
		createSurface1(kAsScene3010DeadBoltFileHashes2[_boltIndex], 1200);
		setFileHash(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		_soundResource1.load(0x420073DC);
		_soundResource2.load(0x420073DC);
	}
	
	_surface->setVisible(false);
	stIdle();
	if (initUnlocked)
		unlock(true);

	_needRefresh = true;
	AnimatedSprite::updatePosition();
	
}

void AsScene3010DeadBolt::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		stDisabled();
	}
}

uint32 AsScene3010DeadBolt::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene3010DeadBolt::stIdle() {
	setFileHash1();
	SetUpdateHandler(&AsScene3010DeadBolt::update);
	SetMessageHandler(&Sprite::handleMessage);
	_locked = false;
}

void AsScene3010DeadBolt::unlock(bool skipAnim) {
	if (!_unlocked) {
		_surface->setVisible(true);
		if (skipAnim) {
			setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], -1, 0);
			_newHashListIndex = -2;
		} else {
			setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
			SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
			setCallback1(AnimationCallback(&AsScene3010DeadBolt::stIdleMessage));
			SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
			_soundResource1.play();
		}
		_unlocked = true;
		_soundResource3.load(0x4010C345);
	}
}

void AsScene3010DeadBolt::stIdleMessage() {
	setFileHash1();
	SetMessageHandler(&Sprite::handleMessage);
	_parentScene->sendMessage(0x2001, _boltIndex, this);
}

void AsScene3010DeadBolt::lock() {
	if (!_locked) {
		_locked = true;
		_surface->setVisible(true);
		setFileHash(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
		setCallback1(AnimationCallback(&AsScene3010DeadBolt::stDisabledMessage));
		SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
		if (_soundToggle) {
			_soundResource1.play();
		} else {
			_soundResource2.play();
		}
		_soundToggle = !_soundToggle;
	}
}

void AsScene3010DeadBolt::setCountdown(int count) {
	_countdown = count * 18 + 1;
}

void AsScene3010DeadBolt::stDisabled() {
	_surface->setVisible(true);
	setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
	SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
	setCallback1(AnimationCallback(&AsScene3010DeadBolt::stDisabledMessage));
	SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
	_playBackwards = true;
	_soundResource3.play();
}

void AsScene3010DeadBolt::stDisabledMessage() {
	_surface->setVisible(false);
	_parentScene->sendMessage(0x2003, _boltIndex, this);
}

Scene3010::Scene3010(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _countdown(0),
	_doorUnlocked(false), _checkUnlocked(false) {
	
	int initCountdown = 0;

	// DEBUG: Enable all buttons
    setSubVar(0x14800353, kScene3010VarNameHashes[0], 1);
    setSubVar(0x14800353, kScene3010VarNameHashes[1], 1);
    setSubVar(0x14800353, kScene3010VarNameHashes[2], 1);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x80802626, 0, 0));
	_palette = new Palette(_vm, 0x80802626);
	_palette->usePalette();

	for (int i = 0; i < 3; i++) {
		_asDeadBolts[i] = new AsScene3010DeadBolt(_vm, this, i, which == 1);//CHECKME
		addSprite(_asDeadBolts[i]);
		_ssDeadBoltButtons[i] = new SsScene3010DeadBoltButton(_vm, this, i, initCountdown, which == 1);//CHECKME
		addSprite(_ssDeadBoltButtons[i]);
		_vm->_collisionMan->addSprite(_ssDeadBoltButtons[i]);
		if (getSubVar(0x14800353, kScene3010VarNameHashes[i]))
			initCountdown++;
		_boltUnlocking[i] = false;
		_boltUnlocked[i] = false;
	}

	if (which == 0) {
		_mouseCursor = addSprite(new Mouse435(_vm, 0x02622800, 20, 620));
	}

	_soundResource.load(0x68E25540);

	SetMessageHandler(&Scene3010::handleMessage);
	SetUpdateHandler(&Scene3010::update);

	if (which == 1) {
		_checkUnlocked = true;
		for (int i = 0; i < 3; i++) {
			_boltUnlocked[i] = true;
			_ssDeadBoltButtons[i]->setCountdown(i + 1);
			_asDeadBolts[i]->setCountdown(i + 1);
		}
	}

}

void Scene3010::update() {
	Scene::update();
	if (_checkUnlocked && !_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
		_countdown = 24;
		_checkUnlocked = false;
	}
	if (_countdown != 0 && (--_countdown == 0)) {
		_parentModule->sendMessage(0x1009, _doorUnlocked ? 1 : 0, this);
	}
}

uint32 Scene3010::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && _countdown == 0 && !_checkUnlocked) {
			if (!_boltUnlocking[0] && !_boltUnlocking[1] && !_boltUnlocking[2]) {
				_mouseCursor->getSurface()->setVisible(false);
				if (!_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
					_countdown = 1;
				} else {
					_checkUnlocked = true;
					for (int i = 0; i < 3; i++) {
						_ssDeadBoltButtons[i]->setCountdown(i);
						if (_boltUnlocked[i]) {
							_asDeadBolts[i]->setCountdown(i);
						}
					}
				}
			}
		}
		break;
	case 0x000D:
		// TODO: Debug stuff
		break;
	case 0x2000:
		if (!_boltUnlocked[param.asInteger()] && !_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->unlock(false);
			_boltUnlocking[param.asInteger()] = true;
		}
		break;
	case 0x2001:
		_boltUnlocked[param.asInteger()] = true;
		_boltUnlocking[param.asInteger()] = false;
		if (_boltUnlocked[0] && _boltUnlocked[1] && _boltUnlocked[2]) {
			if (!getGlobalVar(0x00040153)) {
				setGlobalVar(0x00040153, 1);
				_soundResource.play();
				_countdown = 60;
			} else {
				_countdown = 48;
			}
			_doorUnlocked = true;
		}
		break;
	case 0x2002:
		if (!_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->lock();
		}
		break;
	case 0x2003:
		_boltUnlocked[param.asInteger()] = false;
		break;
	}
	return 0;
}

} // End of namespace Neverhood
