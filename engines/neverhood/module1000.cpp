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
		_parentScene->sendMessage(0x1022, 0x3DE, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 0x3F2, this);
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
	
	_vm->_collisionMan->setHitRects(0x004B4858);
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

Scene1002::Scene1002(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_flag1B4(false), _flag1BE(false) {

	static const uint32 kClass426FileHashList[] = {
		0x00412692,
		0x140B60BE
	};

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

	_vm->_collisionMan->setHitRects(0x004B4134);

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

//	_ssLadderArchPart3->getSurface()->getDrawRect().x = 200;
	
#if 0
	if (which < 0) {
		if (_vm->_gameState.field2 == 0) {
			_klayman = addSprite(new KmScene1002(_vm, this, _class599, _ssLadderArch, 90, 226));
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4270);
			// TODO
			// TODO _klayman->setRepl(64, 0);
		} else {
			_klayman = addSprite(new KmScene1002(_vm, this, _class599, _ssLadderArch, 379, 435));
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4270);
			// TODO
		}
	} else {
		if (which == 1) {
			_klayman = addSprite(new KmScene1002(_vm, this, _class599, _ssLadderArch, 650, 435));
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4478);
			// TODO
			_vm->_gameState.field2 = 1;
		} else if (which == 2) {
			_klayman = addSprite(new KmScene1002(_vm, this, _class599, _ssLadderArch, 68, 645));
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4298);
			// TODO
			_vm->_gameState.field2 = 1;
			_klayman->sendMessage(0x4820, 0, this);
		} else {
			_klayman = addSprite(new KmScene1002(_vm, this, _class599, _ssLadderArch, 90, 226));
			_class478 = addSprite(new Class478(_vm, _klayman));
			setMessageList(0x004B4470);
			// TODO
			_class479 = addSprite(new Class479(_vm, this, _klayman));
			// TODO
			// TODO _klayman->setRepl(64, 0);
			_vm->_gameState.field2 = 0;
		} 
	}
#endif

	_mouseCursor = addSprite(new Mouse433(_vm, 0x23303124, NULL));

	// TODO

	addSprite(new StaticSprite(_vm, 0xB3242310, 825));

	// TODO

#if 0

	_class502_1 = addSprite(new Class502(_vm, this, false, 258, 191, _class599->getSurface()->getDrawRect().y, false);
	_class502_2 = addSprite(new Class502(_vm, this, false, 297, 189, _class599->getSurface()->getDrawRect().y, false);
	_class502_3 = addSprite(new Class502(_vm, this, true, 201, 370, _class599->getSurface()->getDrawRect().y, getGlobalVar(0x8306F218) != 0);
	_class502_4 = addSprite(new Class502(_vm, this, false, 334, 191, _class599->getSurface()->getDrawRect().y, false);
	_class502_5 = addSprite(new Class502(_vm, this, false, 425, 184, _class599->getSurface()->getDrawRect().y, false);

	_class431 = addSprite(new Class431(_vm, tempClipRect));
	
	tempSprite = addSprite(new Class505(_vm, tempClipRect));

	_class504 = addSprite(new Class504(_vm, tempClipRect, this, _class431, tempSprite));
	_class426 = addSprite(new Class426(_vm, this, kClass426FileHashList, 800, 0));
	_class503 = addSprite(new Class503(_vm, this, _klayman, false));
	
	_vm->_collisionMan->addSprite(_class503);
	_klayman->sendEntityMessage(0x2007, _class503, this);

	_class506 = addSprite(new Class506(_vm));
								  
	setRectList(0x004B43A0);

#endif
								  
#if 0 // TODO
	_soundResource2.load(0x60755842);
	_soundResource3.load(0x616D5821);
#endif

}

Scene1002::~Scene1002() {
}

void Scene1002::update() {
	Scene::update();
#if 0 // TODO: Waiting for Klayman...	
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
	
#endif

}

uint32 Scene1002::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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
			_klayman->sendEntityMessage(0x1014, _class502_1, this);
		} else if (param.asInteger() == 0x43807801) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _class502_2, this);
		} else if (param.asInteger() == 0x46C26A01) {
			if (getGlobalVar(0x8306F218)) {
				setMessageList(0x004B44B8);
			} else {
				_klayman->sendEntityMessage(0x1014, _class502_3, this);
				if (_class503->getX() - 10 < 366 && _class503->getX() + 10 > 366) {
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
			_klayman->sendEntityMessage(0x1014, _class502_4, this);
		} else if (param.asInteger() == 0x42845B19) {
			// TODO _resourceTable4.load();
			_klayman->sendEntityMessage(0x1014, _class502_5, this);
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
				_klayman->sendEntityMessage(0x1014, _class503, this);
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
		if (sender == _class502_1) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0x665198C0);
		} else if (sender == _class502_2) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE2D389C0);
		} else if (sender == _class502_3) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource2.play();
			_class431->sendMessage(0x4808, 0, this);
			_class506->sendMessage(0x4808, 0, this);
		} else if (sender == _class502_4) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE0558848);
		} else if (sender == _class502_5) {
			setGlobalVar(0x4DE80AC0, 1);
			_soundResource1.play(0x44014282);
		}
		break;
	case 0x4807:
		if (sender == _class502_3) {
			_soundResource3.play();
			_class431->sendMessage(0x4809, 0, this);
			_class506->sendMessage(0x4809, 0, this);
		} else if (sender == _class503) {
			if (getGlobalVar(0x8306F218)) {
				_class502_3->sendMessage(0x4807, 0, this);
			}
		}	
		break;
	case 0x480B:
		_klayman->sendEntityMessage(0x1014, _class504, this);
		break;				
	case 0x480F:
		setGlobalVar(0x4DE80AC0, 0);
		_soundResource2.play();
		_class431->sendMessage(0x4808, 0, this);
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
