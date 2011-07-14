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

#include "neverhood/module1000.h"
#include "neverhood/navigationscene.h"//###

namespace Neverhood {

Module1000::Module1000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1000(%d)", which);

	_musicFileHash = getGlobalVar(0xD0A14D10) ? 0x81106480 : 0x00103144;		

	// TODO Music18hList_add(0x03294419, 0x061880C6);
	// TODO Music18hList_add(0x03294419, _musicFileHash);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1001(-1);
			break;
		case 1:
			createScene1002(-1);
			break;
		case 2:
			createScene1003(-1);
			break;
		case 3:
			createScene1004(-1);
			break;
		case 4:
			createScene1005(-1);
			break;
		}
	} else if (which == 0) {
		//createScene1001(0);
		// DEBUG: Jump to room
		createScene1002(0);
	} else if (which == 1) {
		createScene1002(1);
	}

}

Module1000::~Module1000() {
	// TODO Music18hList_deleteGroup(0x03294419);
}

void Module1000::createScene1001(int which) {
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1001(_vm, this, which);
	// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable2, &_resourceTable3);
	// TODO Music18hList_play(0x061880C6, 0, 0, 1);
	SetUpdateHandler(&Module1000::updateScene1001);
}
			
void Module1000::createScene1002(int which) {
	_vm->gameState().sceneNum = 1;
	_childObject = new Scene1002(_vm, this, which);
	// DEBUG _childObject = new NavigationScene(_vm, this, 0x004B67B8, 0, NULL);
	// TODO ResourceTable_multiLoad(&_resourceTable3, &_resourceTable4, &_resourceTable1);
	// TODO Music18hList_play(0x061880C6, 0, 0, 1);
	SetUpdateHandler(&Module1000::updateScene1002);
}

void Module1000::createScene1003(int which) {
	_vm->gameState().sceneNum = 2;
	_childObject = new Class152(_vm, this, 0xC084110C, 0x41108C00);
	SetUpdateHandler(&Module1000::updateScene1003);
	// TODO Music18hList_play(0x061880C6, 0, 0);
}

void Module1000::createScene1004(int which) {
}

void Module1000::createScene1005(int which) {
}

void Module1000::updateScene1001() {
	_childObject->handleUpdate();

	if (_done) {
	
		debug("SCENE 1001 DONE; _field20 = %d", _field20);
	
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 2) {
			createScene1003(0);
			_childObject->handleUpdate();
		} else {
			// TODO createScene1002();
			// TODO _childObject->handleUpdate();
		}
	}

	if (_field24 >= 0) {
		if (_field24 == 2) {
			// TODO ResourceTable_multiLoad(&_resourceTable2, &_resourceTable1, &_resourceTable3);
			_field24 = -1;
		} else {
			// TODO ResourceTable_multiLoad(&_resourceTable3, &_resourceTable1);
			_field24 = -1;
		}
	}

	if (_field26 >= 0) {
		// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable2, &_resourceTable3);
		_field26 = -1;
	}

}

void Module1000::updateScene1002() {

	_childObject->handleUpdate();

	if (_done) {
	
		debug("SCENE 1002 DONE; _field20 = %d", _field20);
	
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			_parentModule->sendMessage(0x1009, 0, this);
		} else if (_field20 == 2) {
			// TODO createScene1004(0);
			// TODO _childObject->handleUpdate();
		} else {
			createScene1001(1);
			_childObject->handleUpdate();
		}
	}

	if (_field24 >= 0) {
		if (_field24 == 1) {
			_parentModule->sendMessage(0x100A, 0, this);
		} else if (_field24 == 2) {
			// TODO ResourceTable_multiLoad(&_resourceTable4, &_resourceTable3, &_resourceTable1);
		} else {
			// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable3);
		}
		_field24 = -1;
	}

	if (_field26 >= 0) {
		if (_field26 == 1) {
			_parentModule->sendMessage(0x1023, 0, this);
		} else {
			// TODO ResourceTable_multiLoad(&_resourceTable3, &_resourceTable4, &_resourceTable1);
		}
		_field26 = -1;
	}

	if (_field28 >= 0) {
		_field28 = -1;
	}

}
			
void Module1000::updateScene1003() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1001(2);
		_childObject->handleUpdate();
	}
}
			
void Module1000::updateScene1004() {
}
			
void Module1000::updateScene1005() {
}
			
// Scene1001			

AsScene1001Door::AsScene1001Door(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm) {
	
	createSurface(800, 137, 242);
	_x = 726;
	_y = 440;
	callback1();
#if 0
	_soundResource2.set(0xED403E03);
	_soundResource2.load();
	_soundResource2.createSoundBuffer();
#endif
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Door::handleMessage);
}

uint32 AsScene1001Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		handleMessage2000h();
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return 0;
}

void AsScene1001Door::handleMessage2000h() {
	switch (getGlobalVar(0x52371C95)) {
	case 0:
	case 1:
		_soundResource1.play(0x65482F03);
		setFileHash(0x624C0498, 1, 3);
		SetAnimationCallback3(&AsScene1001Door::callback1);		
		break;
	case 2:
		_soundResource2.play();
		setFileHash(0x624C0498, 6, 6);
		SetAnimationCallback3(&AsScene1001Door::callback2);		
		break;
	default:
		// Nothing
		break;		
	}
	incGlobalVar(0x52371C95, 1);
}

void AsScene1001Door::callback1() {
	switch (getGlobalVar(0x52371C95)) {
	case 1:
		setFileHash(0x624C0498, 4, -1);
		_newHashListIndex = 4;
		break;
	case 2:
		setFileHash(0x624C0498, 1, -1);
		_newHashListIndex = 1;
		break;
	case 3:
		setFileHash1();
		_surface->setVisible(false);
		break;
	default:
		setFileHash(0x624C0498, 0, -1);
		_newHashListIndex = 0;
		break;
	}
}

void AsScene1001Door::callback2() {
	setGlobalVar(0xD217189D, 1);
	setFileHash(0x624C0498, 6, 6);
	SetAnimationCallback3(&AsScene1001Door::callback3);
	_x = 30;
}

void AsScene1001Door::callback3() {
	_soundResource1.play();
	setFileHash1();
	_surface->setVisible(false);	
}
	
AsScene1001Hammer::AsScene1001Hammer(NeverhoodEngine *vm, Sprite *asDoor)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _asDoor(asDoor) {

	_x = 547;
	_y = 206;
	createSurface(900, 177, 192);
	setFileHash(0x022C90D4, -1, -1);
	_newHashListIndex = -2;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Hammer::handleMessage);
}

uint32 AsScene1001Hammer::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00352100) {
			if (_asDoor) {
				_asDoor->sendMessage(0x2000, 0, this);
			}
		} else if (param.asInteger() == 0x0A1A0109) {
			_soundResource.play(0x66410886);
		}
		break;
	case 0x2000:
		setFileHash(0x022C90D4, 1, -1);
		_soundResource.play(0xE741020A);
		_newHashListIndex = -2;
		break;
	}
	return 0;
}

AsScene1001Window::AsScene1001Window(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {

	_x = 320;
	_y = 240;
	createSurface(100, 66, 129);
	setFileHash(0xC68C2299, 0, -1);
	_newHashListIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Window::handleMessage);
}

uint32 AsScene1001Window::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0E0A1410) {
			_soundResource.play(0x60803F10);
		}
		break;
	case 0x2001:
		setFileHash(0xC68C2299, 0, -1);
		break;
	case 0x3002:
		SetMessageHandler(NULL);
		setGlobalVar(0x03C698DA, 1);
		_surface->setVisible(false);
		break;
	}
	return 0;
}

AsScene1001Lever::AsScene1001Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int deltaXType)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene) {
	
	createSurface(1010, 71, 73);
	setDoDeltaX(deltaXType);
	setFileHash(0x04A98C36, 0, -1);
	_newHashListIndex = 0;
	_x = x;
	_y = y;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Lever::handleMessage);
}

uint32 AsScene1001Lever::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00C0C444) {
			_parentScene->sendMessage(0x480F, 0, this);
		} else if (param.asInteger() == 0xC41A02C0) {
			_soundResource.play(0x40581882);
		}
		break;
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
		messageResult = 1;
		break;
	case 0x3002:
		setFileHash(0x04A98C36, 0, -1);
		_newHashListIndex = 0;
		break;
	case 0x480F:
		setFileHash(0x04A98C36, 0, -1);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}
	
SsCommonButtonSprite::SsCommonButtonSprite(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, int surfacePriority, uint32 soundFileHash)
	: StaticSprite(vm, fileHash, surfacePriority), _parentScene(parentScene), _soundResource(vm), _countdown(0) {

	_priority = 1100;
	_soundFileHash = soundFileHash != 0 ? soundFileHash : 0x44141000; 
	_surface->setVisible(false);
	SetUpdateHandler(&SsCommonButtonSprite::update);
	SetMessageHandler(&SsCommonButtonSprite::handleMessage);
}
	
void SsCommonButtonSprite::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		_surface->setVisible(false);
	}
}
	
uint32 SsCommonButtonSprite::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		_parentScene->sendMessage(0x480B, 0, this);
		_surface->setVisible(true);
		_countdown = 8;
		_soundResource.play(_soundFileHash);
		break;
	}
	return messageResult;
}
		
Scene1001::Scene1001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _fieldE4(-1), _fieldE6(-1) {

	_name = "Scene1001";

	Sprite *tempSprite;

	SetMessageHandler(&Scene1001::handleMessage);
	
	_vm->_collisionMan->setHitRects(0x004B4860);
	_surfaceFlag = false;
	_background = addBackground(new DirtyBackground(_vm, 0x4086520E, 0, 0));
	_palette = new Palette(_vm, 0x4086520E);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x6520A400, NULL));
	
	if (which < 0) {
		setRectList(0x004B49F0);
		_klayman = new KmScene1001(_vm, this, 200, 433);
		setMessageList(0x004B4888);
	} else if (which == 1) {
		setRectList(0x004B49F0);
		_klayman = new KmScene1001(_vm, this, 640, 433);
		setMessageList(0x004B4898);
	} else if (which == 2) {
		setRectList(0x004B49F0);
		if (getGlobalVar(0xC0418A02)) {
			_klayman = new KmScene1001(_vm, this, 390, 433);
			_klayman->setDoDeltaX(1);
		} else {
			_klayman = new KmScene1001(_vm, this, 300, 433);
		}
		setMessageList(0x004B4970);
	} else {
		setRectList(0x004B4A00);
		_klayman = new KmScene1001(_vm, this, 200, 433);
		setMessageList(0x004B4890);
	}
	addSprite(_klayman);

	tempSprite = addSprite(new StaticSprite(_vm, 0x2080A3A8, 1300));

	// TODO: This sucks somehow, find a better way
	_klayman->getSurface()->getClipRect().x1 = 0;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = tempSprite->getSurface()->getDrawRect().x + tempSprite->getSurface()->getDrawRect().width;
	_klayman->getSurface()->getClipRect().y2 = 480;
	
	if (getGlobalVar(0xD217189D) == 0) {
		_asDoor = addSprite(new AsScene1001Door(_vm));
		_asDoor->getSurface()->getClipRect().x1 = 0;
		_asDoor->getSurface()->getClipRect().y1 = 0;
		_asDoor->getSurface()->getClipRect().x2 = tempSprite->getSurface()->getDrawRect().x + tempSprite->getSurface()->getDrawRect().width;
		_asDoor->getSurface()->getClipRect().y2 = 480;
	} else {
		_asDoor = NULL;
	}

	_asLever = addSprite(new AsScene1001Lever(_vm, this, 150, 433, 1));

	addSprite(new StaticSprite(_vm, 0x809861A6, 950));
	addSprite(new StaticSprite(_vm, 0x89C03848, 1100));

	_ssButton = addSprite(new SsCommonButtonSprite(_vm, this, 0x15288120, 100, 0));

	if (getGlobalVar(0x03C698DA) == 0) {
		tempSprite = addSprite(new StaticSprite(_vm, 0x8C066150, 200));
		_asWindow = addSprite(new AsScene1001Window(_vm));
		_asWindow->getSurface()->getClipRect().x1 = tempSprite->getSurface()->getDrawRect().x;
		_asWindow->getSurface()->getClipRect().y1 = tempSprite->getSurface()->getDrawRect().y;
		_asWindow->getSurface()->getClipRect().x2 = tempSprite->getSurface()->getDrawRect().x + tempSprite->getSurface()->getDrawRect().width;
		_asWindow->getSurface()->getClipRect().y2 = tempSprite->getSurface()->getDrawRect().y + tempSprite->getSurface()->getDrawRect().height;
	} else {
		_asWindow = NULL;
	}

	_asHammer = addSprite(new AsScene1001Hammer(_vm, _asDoor));

}

Scene1001::~Scene1001() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX());
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1001::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x == 0 && getGlobalVar(0xA4014072)) {
			_parentModule->sendMessage(0x1009, 0, this);
		}
		break;
	case 0x000D:
		if (param.asInteger() == 0x188B2105) {
			_parentModule->sendMessage(0x1009, 0, this);
			messageResult = 1;
		}
		break;
	case 0x100D:
		if (param.asInteger() == 0x00342624) {
			_klayman->sendMessage(0x1014, _asLever, this);
			setMessageList2(0x004B4910);
			messageResult = 1;
		} else if (param.asInteger() == 0x21E64A00) {
			if (getGlobalVar(0xD217189D)) {
				setMessageList(0x004B48A8);
			} else {
				setMessageList(0x004B48C8);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x040424D0) {
			_klayman->sendMessage(0x1014, _ssButton, this);
		} else if (param.asInteger() == 0x80006358) {
			if (getGlobalVar(0x03C698DA)) {
				setMessageList(0x004B4938);
			} else {
				setMessageList(0x004B4960);
			}
		}
		break;
	case 0x2002:
		setRectList(0x004B49F0);
		break;
	case 0x480B:
		if (_asWindow) {
			_asWindow->sendMessage(0x2001, 0, this);
		}
		break;
	case 0x480F:
		if (_asHammer) {
			_asHammer->sendMessage(0x2000, 0, this);
		}
		break;
	}
	return messageResult;
}

// Scene1002

SsScene1002LadderArch::SsScene1002LadderArch(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 0x152C1313, 1015), _parentScene(parentScene) {

	SetMessageHandler(&SsScene1002LadderArch::handleMessage);
}

uint32 SsScene1002LadderArch::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x482A:
		_parentScene->sendMessage(0x1022, 995, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1015, this);
		break;
	}
	return messageResult;
}

Class599::Class599(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 0x316C4BB4, 1015), _parentScene(parentScene) {

	SetMessageHandler(&Class599::handleMessage);
}

uint32 Class599::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x482A:
		_parentScene->sendMessage(0x1022, 995, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1015, this);
		break;
	}
	return messageResult;
}

AsScene1002Ring::AsScene1002Ring(NeverhoodEngine *vm, Scene *parentScene, bool flag1, int16 x, int16 y, int16 clipY1, bool flag2)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _flag1(flag1) {

	SetUpdateHandler(&AsScene1002Ring::update);
	
	if (flag1) {
		createSurface(990, 68, 314);
		if (flag2) {
			setFileHash(0x04103090, 0, -1);
			SetMessageHandler(&AsScene1002Ring::handleMessage447930);
		} else {
			setFileHash(0xA85C4011, _vm->_rnd->getRandomNumber(15), -1);
			SetMessageHandler(&AsScene1002Ring::handleMessage4475E0);
		}
	} else {
		createSurface(990, 68, 138);
		setFileHash(0xA85C4011, _vm->_rnd->getRandomNumber(15), -1);
		SetMessageHandler(&AsScene1002Ring::handleMessage4475E0);
	}

	debug("clipY1 = %d", clipY1);

	_surface->getClipRect().x1 = 0;
	_surface->getClipRect().y1 = clipY1;
	_surface->getClipRect().x2 = 640;
	_surface->getClipRect().y2 = 480;

	_x = x;
	_y = y;

	setDoDeltaX(_vm->_rnd->getRandomNumber(1));

}

void AsScene1002Ring::update() {
	AnimatedSprite::updateAnim();
	AnimatedSprite::updatePosition();
}

uint32 AsScene1002Ring::handleMessage4475E0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4806:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		_parentScene->sendMessage(0x4806, 0, this);
		SetMessageHandler(&AsScene1002Ring::handleMessage447760);
		if (_flag1) {
			setFileHash(0x87502558, 0, -1);
		} else {
			setFileHash(0x80DD4010, 0, -1);
		}
		break;
	case 0x480F:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		_parentScene->sendMessage(0x480F, 0, this);
		SetMessageHandler(&AsScene1002Ring::handleMessage447890);
		setFileHash(0x861A2020, 0, -1);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

uint32 AsScene1002Ring::handleMessage447760(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		if (_flag1) {
			setFileHash(0x78D0A812, 0, -1);
			SetMessageHandler(&AsScene1002Ring::handleMessage447930);
		} else {
			setFileHash(0xB85D2A10, 0, -1);
			SetMessageHandler(&AsScene1002Ring::handleMessage447930);
		}
		break;
	case 0x4807:
		_parentScene->sendMessage(0x4807, 0, this);
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		setFileHash(0x8258A030, 0, -1);
		SetMessageHandler(&AsScene1002Ring::handleMessage447A00);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

uint32 AsScene1002Ring::handleMessage447890(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		setFileHash(0x04103090, 0, -1);
		SetMessageHandler(&AsScene1002Ring::handleMessage447930);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

uint32 AsScene1002Ring::handleMessage447930(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4807:
		_parentScene->sendMessage(0x4807, 0, this);
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		setFileHash(0x8258A030, 0, -1);
		SetMessageHandler(&AsScene1002Ring::handleMessage447A00);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

uint32 AsScene1002Ring::handleMessage447A00(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage4475E0(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x05410F72) {
			_soundResource.play(0x21EE40A9);
		}
		break;
	case 0x3002:
		setFileHash(0xA85C4011, 0, -1);
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

AsScene1002Door::AsScene1002Door(NeverhoodEngine *vm, NRect &clipRect)
	: StaticSprite(vm, 1200) {
	
	_spriteResource.load2(0x1052370F);
	createSurface(800, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_surface->getClipRect() = clipRect;

	_x = 526;
	
	if (getGlobalVar(0x8306F218)) {
		_y = 49; 
	} else {
		_y = 239; 
	}

	_surface->getDrawRect().x = 0;
	_surface->getDrawRect().y = 0;
	_surface->getDrawRect().width = _spriteResource.getDimensions().width;
	_surface->getDrawRect().height = _spriteResource.getDimensions().height;

	_needRefresh = true;
	
	SetUpdateHandler(&AsScene1002Door::update);
	SetMessageHandler(&AsScene1002Door::handleMessage);
	SetSpriteCallback(NULL);
	StaticSprite::update();
	
}

void AsScene1002Door::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 AsScene1002Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		setGlobalVar(0x8306F218, 1);
		SetSpriteCallback(&AsScene1002Door::suOpenDoor);
		break;
	case 0x4809:
		setGlobalVar(0x8306F218, 0);
		SetSpriteCallback(&AsScene1002Door::suCloseDoor);
		break;
	}
	return messageResult;
}

void AsScene1002Door::suOpenDoor() {
	if (_y > 49) {
		_y -= 8;
		if (_y < 49) {
			SetSpriteCallback(NULL);
			_y = 49;
		}
		_needRefresh = true;
	}
}

void AsScene1002Door::suCloseDoor() {
	if (_y < 239) {
		_y += 8;
		if (_y > 239) {
			SetSpriteCallback(NULL);
			_y = 239;
		}
		_needRefresh = true;
	}
}

Class505::Class505(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {

	createSurface(1025, 88, 165);
	_surface->setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class505::handleMessage);	
}

uint32 Class505::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		_x = ((Sprite*)sender)->getX() - 98;
		_y = ((Sprite*)sender)->getY() - 111;
		setFileHash(0x0422255A, 0, -1);
		_surface->setVisible(true);
		break;
	case 0x3002:
		setFileHash1();
		_surface->setVisible(false);
		break;
	}
	return messageResult;
}

AsScene1002DoorSpy::AsScene1002DoorSpy(NeverhoodEngine *vm, NRect &clipRect, Scene *parentScene, Sprite *asDoor, Sprite *class505)
	: AnimatedSprite(vm, 1300), _rect(clipRect), _parentScene(parentScene), _asDoor(asDoor), _class505(class505),
	_soundResource(vm) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1002DoorSpy::handleMessage4489D0);
	SetSpriteCallback(&AsScene1002DoorSpy::spriteUpdate448AA0);
	
	createSurface(800, 136, 147);
	_surface->getClipRect() = clipRect;
	
	spriteUpdate448AA0();

	// TODO _soundResource.load(0xC0C40298);
	
	setFileHash(0x586C1D48, 0, 0);
		
}

uint32 AsScene1002DoorSpy::handleMessage4489D0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xA61CA1C2) {
			_class505->sendMessage(0x2004, 0, this);
		} else if (param.asInteger() == 0x14CE0620) {
			// TODO _soundResource.play();
		}
		break;
	case 0x2003:
		sub448B10();
		break;
	}
	return messageResult;
}

uint32 AsScene1002DoorSpy::handleMessage448A60(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage4489D0(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene1002DoorSpy::spriteUpdate448AA0() {
	_x = _asDoor->getX() + 34;
	_y = _asDoor->getY() + 175;
}

void AsScene1002DoorSpy::sub448AC0() {
	_surface->getClipRect() = _rect;
	_parentScene->setSurfacePriority(getSurface(), 800);
	setFileHash(0x586C1D48, 0, 0);
	SetMessageHandler(&AsScene1002DoorSpy::handleMessage4489D0);
}

void AsScene1002DoorSpy::sub448B10() {
	_surface->getClipRect().x1 = 0;
	_surface->getClipRect().y1 = 0;
	_surface->getClipRect().x2 = 640;
	_surface->getClipRect().y2 = 480;
	_parentScene->setSurfacePriority(getSurface(), 1200);
	setFileHash(0x586C1D48, 1, -1);
	SetMessageHandler(&AsScene1002DoorSpy::handleMessage448A60);
	SetAnimationCallback3(&AsScene1002DoorSpy::sub448AC0);
}

Class426::Class426(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash) 
	: StaticSprite(vm, 1100), _parentScene(parentScene), _soundResource(vm), _status(0) {

	_soundFileHash = soundFileHash != 0 ? soundFileHash : 0x44141000;

	_fileHashes[0] = fileHash1;
	_fileHashes[1] = fileHash2;
	
	_spriteResource.load2(fileHash1);
	createSurface(surfacePriority, 40, 40);
	
	_surface->getDrawRect().x = 0;
	_surface->getDrawRect().y = 0;
	_surface->getDrawRect().width = _spriteResource.getDimensions().width;
	_surface->getDrawRect().height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;

	_surface->setVisible(false);
	_needRefresh = true;
	
	SetUpdateHandler(&Class426::update);
	SetMessageHandler(&Class426::handleMessage);

}

void Class426::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		if (_status == 1) {
			_status = 2;
			_spriteResource.load2(_fileHashes[1]);
			_surface->getDrawRect().x = 0;
			_surface->getDrawRect().y = 0;
			_surface->getDrawRect().width = _spriteResource.getDimensions().width;
			_surface->getDrawRect().height = _spriteResource.getDimensions().height;
			_x = _spriteResource.getPosition().x;
			_y = _spriteResource.getPosition().y;
			_needRefresh = true;
			StaticSprite::update();
			_countdown = 4;
		} else if (_status == 2) {
			_status = 3;
			_spriteResource.load2(_fileHashes[0]);
			_surface->getDrawRect().x = 0;
			_surface->getDrawRect().y = 0;
			_surface->getDrawRect().width = _spriteResource.getDimensions().width;
			_surface->getDrawRect().height = _spriteResource.getDimensions().height;
			_x = _spriteResource.getPosition().x;
			_y = _spriteResource.getPosition().y;
			_needRefresh = true;
			StaticSprite::update();
			_countdown = 4;
		} else if (_status == 3) {
			_status = 0;
			_surface->setVisible(false);
		}
	}
}

uint32 Class426::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		_parentScene->sendMessage(0x480B, 0, this);
		_status = 1;
		_countdown = 4;
		_surface->setVisible(true);
		_soundResource.play(_soundFileHash);
		break;
	}
	return messageResult;
}

AsScene1002VenusFlyTrap::AsScene1002VenusFlyTrap(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _klayman(klayman),
	_flag(flag), _countdown(0) {

	createSurface(995, 175, 195);

	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448000);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);

	if (!flag) {
		if (getGlobalVar(0x8306F218)) {
			setDoDeltaX(1);
			_x = 366;
			_y = 435;
			sub4485F0();
		} else {
			_x = 174 + getGlobalVar(0x1B144052) * 32;
			_y = 435;
			sub448660();
		}
	} else {
		_x = 186 + getGlobalVar(0x86341E88) * 32;
		_y = 364;
		if (getGlobalVar(0x13206309) || getGlobalVar(0x80101B1E)) {
			sub4485F0();
		} else {
			sub448660();
		} 
	}
	
	_flags = 4;
}

void AsScene1002VenusFlyTrap::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		removeCallbacks();
	}
	AnimatedSprite::update();
}

void AsScene1002VenusFlyTrap::update447FB0() {
	if (_countdown == 0 && _klayman->getX() - 20 > _x) {
		setDoDeltaX(1);
	} else if (_klayman->getX() + 20 < _x) {
		setDoDeltaX(0);
	}
	update();
}

uint32 AsScene1002VenusFlyTrap::handleMessage448000(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000890C4) {
			_soundResource.play(0xC21190D8);
		} else if (param.asInteger() == 0x522200A0) {
			_soundResource.play(0x931080C8);
		}
		break;
	case 0x1011:
		if (_flag) {
			if (_x >= 154 && _x <= 346) {
				_parentScene->sendMessage(0x2000, 0, this);
				messageResult = 1;
			}
		} else {
			if (_x >= 174 && _x <= 430) {
				_parentScene->sendMessage(0x2000, 0, this);
				messageResult = 1;
			}
		}
		break;
	case 0x480B:
		setDoDeltaX(param.asInteger() != 0 ? 1 : 0);
		if (_flag) {
			if (getGlobalVar(0x8306F218)) {
				sub448560();
			} else {
				sub448530();
			}
		} else {
			if (getGlobalVar(0x13206309) || getGlobalVar(0x80101B1E)) {
				sub448560();
			} else {
				sub448530();
			}
		}
		break;
	case 0x480C:
		if (_flag) {
			if (_x >= 154 && _x <= 346)
				messageResult = 1;
			else				
				messageResult = 0;
		} else {
			if (_x >= 174 && _x <= 430)
				messageResult = 1;
			else				
				messageResult = 0;
		}
		break;
	case 0x480E:
		if (param.asInteger() == 1) {
			sub4485B0();
		}
		break;
	case 0x4810:
	debug("trap collision");
		sub448780();
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 995, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1015, this);
		break;
	}
	return messageResult;
}

uint32 AsScene1002VenusFlyTrap::handleMessage4482E0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage448000(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

uint32 AsScene1002VenusFlyTrap::handleMessage448320(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000890C4) {
			_soundResource.play(0xC21190D8);
		} else if (param.asInteger() == 0x41881801) {
			if (_flag) {
				if (_x > 330) {
					_klayman->sendMessage(0x4811, 2, this);
				} else if (_x > 265) {
					_klayman->sendMessage(0x4811, 0, this);
				} else {
					_klayman->sendMessage(0x4811, 0, this);
				}
			} else {
				_klayman->sendMessage(0x4811, 0, this);
			}
		} else if (param.asInteger() == 0x522200A0) {
			_soundResource.play(0x931080C8);
		}
		break;
	case 0x3002:
		removeCallbacks();
		break;
	case 0x482A:
		_parentScene->sendMessage(0x1022, 995, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1015, this);
		break;
	}
	return messageResult;
}

void AsScene1002VenusFlyTrap::sub4484F0() {
	//ok
	setDoDeltaX(2);
	setFileHash(0xC4080034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448320);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448660);
}

void AsScene1002VenusFlyTrap::sub448530() {
	//ok
	setFileHash(0xC4080034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage4482E0);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448660);
}

void AsScene1002VenusFlyTrap::sub448560() {
	//ok
	_parentScene->sendMessage(0x4807, 0, this);
	setFileHash(0x82292851, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage4482E0);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448660);
}

void AsScene1002VenusFlyTrap::sub4485B0() {
	//ok
	setDoDeltaX(1);
	setFileHash(0x86A82A11, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage4482E0);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub4485F0);
}

void AsScene1002VenusFlyTrap::sub4485F0() {
	//ok
	setFileHash(0xB5A86034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448000);
}

void AsScene1002VenusFlyTrap::sub448620() {
	//ok
	setFileHash(0x31303094, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(NULL);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448720);
	_countdown = 24;
}

void AsScene1002VenusFlyTrap::sub448660() {
	setFileHash(0xC8204250, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update447FB0);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448000);
	if (_flag) {
		if (_x >= 154 && _x <= 346) {
			setGlobalVar(0x86341E88, (_x - 186) / 32);
		} else {
			SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub4484F0);
			_countdown = 12;
		}
	} else {
		if (_x >= 174 && _x <= 430) {
			setGlobalVar(0x1B144052, (_x - 174) / 32);
		} else {
			SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub4484F0);
			_countdown = 12;
		}
	}
}

void AsScene1002VenusFlyTrap::sub448720() {
	//ok
	setFileHash(0x152920C4, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448320);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448750);
}

void AsScene1002VenusFlyTrap::sub448750() {
	//ok
	setFileHash(0x84001117, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448320);
	SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448660);
}

void AsScene1002VenusFlyTrap::sub448780() {
debug("AsScene1002VenusFlyTrap::sub448780()");
	if (_x - 15 < _klayman->getX() && _x + 15 > _klayman->getX()) {
		if (_flag) {
			setDoDeltaX(_x > 265 && _x < 330 ? 1 : 0);
		} else {
			setDoDeltaX(_x > 320 ? 1 : 0);
		}
		_klayman->sendMessage(0x2001, 0, this);
		setFileHash(0x8C2C80D4, 0, -1);
		SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
		SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage448320);
		SetAnimationCallback3(&AsScene1002VenusFlyTrap::sub448620);
	}
}

Class506::Class506(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _countdown(0) {

	createSurface(850, 186, 212);
	_x = 320;
	_y = 240;
	if (getGlobalVar(0x8306F218)) {
		setFileHash(0x004A4495, -1, -1);
		_newHashListIndex = -2;
	} else {
		_surface->setVisible(false);
	}
	SetUpdateHandler(&Class506::update);
	SetMessageHandler(&Class506::handleMessage4491B0);	
}

void Class506::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		if (_flag) {
			sub449280();
		} else {
			sub449250();
		}
	}
	AnimatedSprite::update();
}

uint32 Class506::handleMessage4491B0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageResult) {
	case 0x4808:
		_flag = false;
		_countdown = 2;
		break;
	case 0x4809:
		_flag = true;
		_countdown = 2;
		break;
	}
	return messageResult;
}

uint32 Class506::handleMessage449210(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage4491B0(messageNum, param, sender);
	switch (messageResult) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class506::sub449250() {
	setFileHash(0x004A4495, 0, -1);
	SetMessageHandler(&Class506::handleMessage4491B0);
	_newHashListIndex = -2;
	_surface->setVisible(true);
}

void Class506::sub449280() {
	setFileHash(0x004A4495, -1, -1);
	_playBackwards = true;
	SetMessageHandler(&Class506::handleMessage449210);
	SetAnimationCallback3(&Class506::sub4492C0);
	_surface->setVisible(true);
}

void Class506::sub4492C0() {
	_surface->setVisible(false);
	setFileHash1();
}

Class478::Class478(NeverhoodEngine *vm, Klayman *klayman)
	: AnimatedSprite(vm, 1200), _klayman(klayman) {
	
	createSurface(1200, 40, 163);
	SetUpdateHandler(&Class478::update);
	SetMessageHandler(&Sprite::handleMessage);
	_surface->setVisible(false);
}

void Class478::update() {
	if (_klayman->getCurrAnimFileHash() == 0x3A292504) {
		setFileHash(0xBA280522, _frameIndex, -1);
		_newHashListIndex = _klayman->getFrameIndex();
		_surface->setVisible(true);
		_x = _klayman->getX(); 
		_y = _klayman->getY(); 
		setDoDeltaX(_klayman->isDoDeltaX() ? 1 : 0);
	} else if (_klayman->getCurrAnimFileHash() == 0x122D1505) {
		setFileHash(0x1319150C, _frameIndex, -1);
		_newHashListIndex = _klayman->getFrameIndex();
		_surface->setVisible(true);
		_x = _klayman->getX(); 
		_y = _klayman->getY(); 
		setDoDeltaX(_klayman->isDoDeltaX() ? 1 : 0);
	} else {
		_surface->setVisible(false);
	}
	AnimatedSprite::update();
}

Scene1002::Scene1002(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_flag1B4(false), _flag1BE(false) {

	NRect tempClipRect;
	Sprite *tempSprite;

	// TODO _field1B6 = -1;
	// TODO _resourceTable8.setResourceList(0x004B4110);
	// TODO _resourceTable7.setResourceList(0x004B4100);
	// TODO _resourceTable6.setResourceList(0x004B40E8);
	// TODO _resourceTable5.setResourceList(0x004B40C0);
	// TODO _resourceTable4.setResourceList(0x004B4080);
	// TODO _resourceTable3.setResourceList(0x004B4060);
	// TODO _resourceTable2.setResourceList(0x004B4000, true);
	// TODO _resourceTable1.setResourceList(0x004B3F90, true);
	// TODO _resourceTable1.loadSome(3000);

	SetUpdateHandler(&Scene1002::update);
	SetMessageHandler(&Scene1002::handleMessage);

	_vm->_collisionMan->setHitRects(0x004B4138);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x12C23307, 0, 0));
	_palette = new Palette(_vm, 0x12C23307);
	_palette->usePalette();

	_flag = false;

	addSprite(new StaticSprite(_vm, 0x06149428, 1100));
	addSprite(new StaticSprite(_vm, 0x312C8774, 1100));

	_ssLadderArch = addSprite(new SsScene1002LadderArch(_vm, this));
	_ssLadderArchPart1 = addSprite(new StaticSprite(_vm, 0x060000A0, 1200));
	_ssLadderArchPart2 = addSprite(new StaticSprite(_vm, 0xB2A423B0, 1100));
	_ssLadderArchPart3 = addSprite(new StaticSprite(_vm, 0x316E0772, 1100));

	_class599 = addSprite(new Class599(_vm, this));

	// DEBUG/HACK!!!!
	which = 1; setGlobalVar(0x8306F218, 1);

	if (which < 0) {
		if (_vm->_gameState.field2 == 0) {
			_klayman = new KmScene1002(_vm, this, _class599, _ssLadderArch, 90, 226);
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4270);
			// TODO			
			_klayman->setRepl(64, 0);
		} else {
			_klayman = new KmScene1002(_vm, this, _class599, _ssLadderArch, 379, 435);
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4270);
			// TODO			
		}
	} else {
		if (which == 1) {
			_klayman = new KmScene1002(_vm, this, _class599, _ssLadderArch, 650, 435);
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4478);
			// TODO			
			_vm->_gameState.field2 = 1;
		} else if (which == 2) {
			_klayman = new KmScene1002(_vm, this, _class599, _ssLadderArch, 68, 645);
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4298);
			// TODO
			_vm->_gameState.field2 = 1;
			_klayman->sendMessage(0x4820, 0, this);
		} else {
			_klayman = new KmScene1002(_vm, this, _class599, _ssLadderArch, 90, 226);
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4470);
			// TODO
			//_class479 = addSprite(new Class479(_vm, this, _klayman));
			// TODO
			_klayman->setRepl(64, 0);
			_vm->_gameState.field2 = 0;
		} 
	}

    addSprite(_klayman);

	_mouseCursor = addSprite(new Mouse433(_vm, 0x23303124, NULL));

	tempSprite = addSprite(new StaticSprite(_vm, 0xB3242310, 825));
	tempClipRect.x1 = tempSprite->getSurface()->getDrawRect().x;
	tempClipRect.y1 = tempSprite->getSurface()->getDrawRect().y;
	tempClipRect.x2 = _ssLadderArchPart2->getSurface()->getDrawRect().x + _ssLadderArchPart2->getSurface()->getDrawRect().width;
	tempClipRect.y2 = _ssLadderArchPart2->getSurface()->getDrawRect().y + _ssLadderArchPart2->getSurface()->getDrawRect().height;

	_asRing1 = addSprite(new AsScene1002Ring(_vm, this, false, 258, 191, _class599->getSurface()->getDrawRect().y, false));
	_asRing2 = addSprite(new AsScene1002Ring(_vm, this, false, 297, 189, _class599->getSurface()->getDrawRect().y, false));
	_asRing3 = addSprite(new AsScene1002Ring(_vm, this, true, 370, 201, _class599->getSurface()->getDrawRect().y, getGlobalVar(0x8306F218) != 0));
	_asRing4 = addSprite(new AsScene1002Ring(_vm, this, false, 334, 191, _class599->getSurface()->getDrawRect().y, false));
	_asRing5 = addSprite(new AsScene1002Ring(_vm, this, false, 425, 184, _class599->getSurface()->getDrawRect().y, false));

	_asDoor = addSprite(new AsScene1002Door(_vm, tempClipRect));
	
	tempSprite = addSprite(new Class505(_vm));

	_asDoorSpy = addSprite(new AsScene1002DoorSpy(_vm, tempClipRect, this, _asDoor, tempSprite));
	
	_class426 = addSprite(new Class426(_vm, this, 0x00412692, 0x140B60BE, 800, 0));
	_asVenusFlyTrap = addSprite(new AsScene1002VenusFlyTrap(_vm, this, _klayman, false));
	
	_vm->_collisionMan->addSprite(_asVenusFlyTrap);

	_klayman->sendEntityMessage(0x2007, _asVenusFlyTrap, this);

	_class506 = addSprite(new Class506(_vm));
								  
	setRectList(0x004B43A0);

								  
#if 0 // TODO
	_soundResource2.load(0x60755842);
	_soundResource3.load(0x616D5821);
#endif

}

Scene1002::~Scene1002() {
}

void Scene1002::update() {
	Scene::update();
	if (!_flag1B4 && _klayman->getY() > 230) {
		// TODO
		deleteSprite(&_ssLadderArchPart3);
		_klayman->clearRepl();
		_flag1B4 = true;
		_vm->_gameState.field2 = 1;
	}

	if (_flag1BE && _klayman->getY() > 422) {
		_parentModule->sendMessage(0x1024, 1, this);
		_flag1BE = false;
	}
	
}

uint32 Scene1002::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1002::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x == 0 && getGlobalVar(0xA4014072)) {
			setGlobalVar(0x8306F218, 1);
			setGlobalVar(0x1B144052, 3);
			_parentModule->sendMessage(0x1009, 1, this);
		}
		break;
	case 0x000D:
		if (param.asInteger() == 0x48848178) {
			setGlobalVar(0x8306F218, 1);
			setGlobalVar(0x1B144052, 3);
			_parentModule->sendMessage(0x1009, 1, this);
		}
		messageResult = 1;
		break;
	case 0x100D:
		if (param.asInteger() == 0xE6EE60E1) {
			if (getGlobalVar(0x8306F218)) {
				setMessageList(0x004B4428);
			} else {
				// TODO _resourceTable3.load();
				setMessageList(0x004B4448);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x4A845A00) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _asRing1, this);
		} else if (param.asInteger() == 0x43807801) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _asRing2, this);
		} else if (param.asInteger() == 0x46C26A01) {
			if (getGlobalVar(0x8306F218)) {
				setMessageList(0x004B44B8);
			} else {
				_klayman->sendEntityMessage(0x1014, _asRing3, this);
				if (_asVenusFlyTrap->getX() - 10 < 366 && _asVenusFlyTrap->getX() + 10 > 366) {
					setGlobalVar(0x2B514304, 1);
					setMessageList(0x004B44A8);
				} else {
					// TODO _resourceTable5.load();
					setMessageList(0x004B44A0);
				}
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x468C7B11) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _asRing4, this);
		} else if (param.asInteger() == 0x42845B19) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _asRing5, this);
		} else if (param.asInteger() == 0xC0A07458) {
			_klayman->sendEntityMessage(0x1014, _class426, this);
		}
		break;
	case 0x1024:
		if (param.asInteger() == 1) {
			// TODO _resourceTable3.load();
		} else if (param.asInteger() == 3) {
			// TODO _resourceTable2.load();
		}
		_parentModule->sendMessage(0x1024, param, this);
		break;
	case 0x2000:
		if (_flag) {
			setMessageList2(0x004B43D0);
		} else {
			if (_klayman->getY() > 420) {
				_klayman->sendEntityMessage(0x1014, _asVenusFlyTrap, this);
				setMessageList2(0x004B4480);
			} else if (_klayman->getY() > 227) {
				setMessageList2(0x004B41E0);
			} else {
				setMessageList2(0x004B4148);
			}
		}
		break;
	case 0x2002:
		_messageList = NULL;
		break;										
	case 0x2005:
		_flag = true;
		setRectList(0x004B4418);
		break;										
	case 0x2006:
		_flag = false;
		setRectList(0x004B43A0);
		break;
	case 0x4806:
		_parentModule->sendMessage(0x1024, 2, this);
		_flag1BE = true;
		if (sender == _asRing1) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0x665198C0);
		} else if (sender == _asRing2) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE2D389C0);
		} else if (sender == _asRing3) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource2.play();
			_asDoor->sendMessage(0x4808, 0, this);
			_class506->sendMessage(0x4808, 0, this);
		} else if (sender == _asRing4) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE0558848);
		} else if (sender == _asRing5) {
			setGlobalVar(0x4DE80AC0, 1);
			_soundResource1.play(0x44014282);
		}
		break;
	case 0x4807:
		if (sender == _asRing3) {
			_soundResource3.play();
			_asDoor->sendMessage(0x4809, 0, this);
			_class506->sendMessage(0x4809, 0, this);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(0x8306F218)) {
				_asRing3->sendMessage(0x4807, 0, this);
			}
		}	
		break;
	case 0x480B:
		_klayman->sendEntityMessage(0x1014, _asDoorSpy, this);
		break;				
	case 0x480F:
		setGlobalVar(0x4DE80AC0, 0);
		_soundResource2.play();
		_asDoor->sendMessage(0x4808, 0, this);
		_class506->sendMessage(0x4808, 0, this);
		break;				
	}	
	return messageResult;
}

// Class152

Class152::Class152(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 cursorFileHash)
	: Scene(vm, parentModule, true), _fieldD0(-1), _fieldD2(-1) {

	_surfaceFlag = false;

	SetMessageHandler(&Class152::handleMessage);
	
	_background = addBackground(new DirtyBackground(_vm, backgroundFileHash, 0, 0));
	_palette = new Palette(_vm, backgroundFileHash);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse435(_vm, cursorFileHash, 20, 620));
}

uint32 Class152::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			_parentModule->sendMessage(0x1009, 0, this);
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
