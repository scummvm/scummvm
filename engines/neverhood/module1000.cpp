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

	_musicFileHash = _vm->getGlobalVar(0xD0A14D10) ? 0x81106480 : 0x00103144;		

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
		createScene1001(0);
	} else if (which == 1) {
		createScene1002(1);
	}

}

Module1000::~Module1000() {
	// TODO Music18hList_deleteGroup(0x03294419);
}

void Module1000::createScene1001(int which) {
	debug("createScene1501");
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1001(_vm, this, which);
	// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable2, &_resourceTable3);
	// TODO Music18hList_play(0x061880C6, 0, 0, 1);
	SetUpdateHandler(&Module1000::updateScene1001);
}
			
void Module1000::createScene1002(int which) {
}

void Module1000::createScene1003(int which) {
}

void Module1000::createScene1004(int which) {
}

void Module1000::createScene1005(int which) {
}

void Module1000::updateScene1001() {
	_childObject->handleUpdate();

	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 2) {
			// TODO createScene1003();
			// TODO _childObject->handleUpdate();
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
}
			
void Module1000::updateScene1003() {
}
			
void Module1000::updateScene1004() {
}
			
void Module1000::updateScene1005() {
}
			
// Scene1001			

KmScene1001::KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
}

uint32 KmScene1001::xHandleMessage(int messageNum, const MessageParam &param) {
	debug("KmScene1001::xHandleMessage() messageNum = %04X", messageNum);
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		setCallback2(AnimationCallback(&Klayman::sub41FC80));
		break;		
	case 0x4804:
		if (param.asInteger() == 2) {
			setCallback2(AnimationCallback(&Klayman::sub4211B0));
		}
		break;
	case 0x480D:
		setCallback2(AnimationCallback(&KmScene1001::sub44FA50));
		break;
	case 0x4812:
		setCallback2(AnimationCallback(&Klayman::sub41FF80));
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			setCallback2(AnimationCallback(&Klayman::sub420120));
		} else if (param.asInteger() == 2) {
			setCallback2(AnimationCallback(&Klayman::sub420170));
		}else {
			setCallback2(AnimationCallback(&Klayman::sub4200D0));
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		

	case 0x481B:
		// TODO: It's not really a point but an x1/x2 pair
		if (param.asPoint().x != 0) {
			sub41CC40(param.asPoint().x, param.asPoint().y);
		} else {
			error("// TODO sub41CCE0(param.asPoint().y);");
			// TODO sub41CCE0(param.asPoint().y);
		}
		break;

	case 0x4836:
		if (param.asInteger() == 1) {
			_parentScene->sendMessage(0x2002, 0, this);
			setCallback2(AnimationCallback(&Klayman::sub4211F0));
		}
		break;		

	case 0x483F:
		sub41CD00(param.asInteger());
		break;		

	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}

	// TODO

	return 0;
}

void KmScene1001::sub44FA50() {
	if (!sub41CEB0(AnimationCallback(&KmScene1001::sub44FA50))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x00648953, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&KmScene1001::handleMessage44FA00);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	}
}

uint32 KmScene1001::handleMessage44FA00(int messageNum, const MessageParam &param, Entity *sender) {
	debug("KmScene1001::handleMessage44FA00(%04X)", messageNum);
	uint32 messageResult = Klayman::handleMessage41E210(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			_attachedSprite->sendMessage(0x480F, 0, this);
		}
		break;
	}
	return messageResult;
}

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
	switch (_vm->getGlobalVar(0x52371C95)) {
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
	_vm->incGlobalVar(0x52371C95, 1);
}

void AsScene1001Door::callback1() {
	switch (_vm->getGlobalVar(0x52371C95)) {
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
	_vm->setGlobalVar(0xD217189D, 1);
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
		_vm->setGlobalVar(0x03C698DA, 1);
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

	Sprite *staticSprite1;

	SetMessageHandler(&Scene1001::handleMessage);
	
	_vm->_collisionMan->setHitRects(0x004B4858);
	_surfaceFlag = false;
	_background = addBackground(new DirtyBackground(_vm, 0x4086520E, 0, 0));
	_palette = new Palette(_vm, 0x4086520E);
	_palette->usePalette();
	
	// TODO Mouse

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
		if (_vm->getGlobalVar(0xC0418A02)) {
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

	staticSprite1 = addSprite(new StaticSprite(_vm, 0x2080A3A8, 1300));

	// TODO: This sucks somehow, find a better way
	_klayman->getSurface()->getClipRect().x1 = 0;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
	_klayman->getSurface()->getClipRect().y2 = 480;
	
	if (_vm->getGlobalVar(0xD217189D) == 0) {
		_asDoor = addSprite(new AsScene1001Door(_vm));
		_asDoor->getSurface()->getClipRect().x1 = 0;
		_asDoor->getSurface()->getClipRect().y1 = 0;
		_asDoor->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
		_asDoor->getSurface()->getClipRect().y2 = 480;
	} else {
		_asDoor = NULL;
	}

	_asLever = addSprite(new AsScene1001Lever(_vm, this, 150, 433, 1));

	addSprite(new StaticSprite(_vm, 0x809861A6, 950));
	addSprite(new StaticSprite(_vm, 0x89C03848, 1100));

	_ssButton = addSprite(new SsCommonButtonSprite(_vm, this, 0x15288120, 100, 0));

	if (_vm->getGlobalVar(0x03C698DA) == 0) {
		staticSprite1 = addSprite(new StaticSprite(_vm, 0x8C066150, 200));
		_asWindow = addSprite(new AsScene1001Window(_vm));
		_asWindow->getSurface()->getClipRect().x1 = staticSprite1->getSurface()->getDrawRect().x;
		_asWindow->getSurface()->getClipRect().y1 = staticSprite1->getSurface()->getDrawRect().y;
		_asWindow->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
		_asWindow->getSurface()->getClipRect().y2 = staticSprite1->getSurface()->getDrawRect().y + staticSprite1->getSurface()->getDrawRect().height;
	} else {
		_asWindow = NULL;
	}

	_asHammer = addSprite(new AsScene1001Hammer(_vm, _asDoor));

}

Scene1001::~Scene1001() {
	// TODO _vm->setGlobalVar(0xC0418A02, _klayman->_doDeltaX);
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1001::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x == 0 && _vm->getGlobalVar(0xA4014072)) {
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
			if (_vm->getGlobalVar(0xD217189D)) {
				setMessageList(0x004B48A8);
			} else {
				setMessageList(0x004B48C8);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x040424D0) {
			_klayman->sendMessage(0x1014, _ssButton, this);
		} else if (param.asInteger() == 0x80006358) {
			if (_vm->getGlobalVar(0x03C698DA)) {
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

} // End of namespace Neverhood
