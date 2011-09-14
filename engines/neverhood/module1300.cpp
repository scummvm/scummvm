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

#include "neverhood/module1300.h"
#include "neverhood/module1000.h"
#include "neverhood/module2200.h"
#include "neverhood/diskplayerscene.h"
#include "neverhood/navigationscene.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

Module1300::Module1300(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1300(%d)", which);

	// TODO Music18hList_add(0x61C090, 0x203197);
	// TODO Sound1ChList_addSoundResources(0x61C090, dword_4B2868, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, 0, 50, 600, 20, 150);
	// TODO Sound1ChList_sub_407C70(0x61C090, 0x48498E46, 0x50399F64, 0);
	// TODO Sound1ChList_setVolume(0x48498E46, 70);
	// TODO Sound1ChList_setVolume(0x50399F64, 70);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			createScene1302(-1);
			break;
		case 2:
			createScene1303(-1);
			break;
		case 3:
			createScene1304(-1);
			break;
		case 4:
			createScene1305(-1);
			break;
		case 5:
			createScene1306(-1);
			break;
		case 6:
			createScene1307(-1);
			break;
		case 7:
			createScene1308(-1);
			break;
		case 8:
			createScene1309(-1);
			break;
		case 9:
			createScene1310(-1);
			break;
		case 10:
			createScene1311(-1);
			break;
		case 11:
			createScene1312(-1);
			break;
		case 12:
			createScene1313(-1);
			break;
		case 13:
			createScene1314(-1);
			break;
		case 14:
			createScene1315(-1);
			break;
		case 15:
			createScene1316(-1);
			break;
		case 16:
			createScene1317(-1);
			break;
		case 17:
			createScene1318(-1);
			break;
		default:
			createScene1312(0);
		}
	} else {
		switch (which) {
		case 0:
			createScene1312(0);
			break;
		case 1:
			createScene1314(0);
			break;
		case 2:
			createScene1315(0);
			break;
		case 3:
			createScene1316(0);
			break;
		case 4:
			createScene1308(0);
			break;
		case 5:
			createScene1306(1);
			break;
		case 6:
			createScene1306(5);
			break;
		case 7:
			createScene1304(0);
			break;
		case 8:
			createScene1302(0);
			break;
		case 9:
			createScene1303(0);
			break;
		case 10:
			createScene1307(0);
			break;
		case 11:
			createScene1305(0);
			break;
		default:
			createScene1313(0);
			break;
		}
	}
	
}

Module1300::~Module1300() {
	// TODO Sound1ChList_sub_407A50(0x61C090);
}

void Module1300::createScene1302(int which) {
	_vm->gameState().sceneNum = 1;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_play(0x203197, 0, 2, 1);
	_childObject = new Scene1302(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1302);
}
			
void Module1300::createScene1303(int which) {
	_vm->gameState().sceneNum = 2;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	_childObject = new Scene1303(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1303);
}
			
void Module1300::createScene1304(int which) {
	_vm->gameState().sceneNum = 3;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	_childObject = new Scene1304(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1304);
}
			
void Module1300::createScene1305(int which) {
	_vm->gameState().sceneNum = 4;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_play(0x203197, 0, 2, 1);
	_childObject = new Scene1305(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1305);
}
			
void Module1300::createScene1306(int which) {
}
			
void Module1300::createScene1307(int which) {
}
			
void Module1300::createScene1308(int which) {
}
			
void Module1300::createScene1309(int which) {
}
			
void Module1300::createScene1310(int which) {
}
			
void Module1300::createScene1311(int which) {
}
			
void Module1300::createScene1312(int which) {
	_vm->gameState().sceneNum = 11;
	createNavigationScene(0x004B27A8, which);
	SetUpdateHandler(&Module1300::updateScene1312);
	// TODO Sound1ChList_setSoundValuesMulti(0xdword_4B2868, true, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
}
			
void Module1300::createScene1313(int which) {
	_vm->gameState().sceneNum = 12;
	createNavigationScene(0x004B2718, which);
	SetUpdateHandler(&Module1300::updateScene1313);
	// TODO Sound1ChList_setSoundValuesMulti(0xdword_4B2868, true, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
}
			
void Module1300::createScene1314(int which) {
	_vm->gameState().sceneNum = 13;
	createNavigationScene(0x004B27D8, which);
	SetUpdateHandler(&Module1300::updateScene1314);
	// TODO Sound1ChList_setSoundValuesMulti(0xdword_4B2868, true, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
}
			
void Module1300::createScene1315(int which) {
	_vm->gameState().sceneNum = 14;
	createNavigationScene(0x004B2808, which);
	SetUpdateHandler(&Module1300::updateScene1315);
	// TODO Sound1ChList_setSoundValuesMulti(0xdword_4B2868, true, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
}
			
void Module1300::createScene1316(int which) {
	_vm->gameState().sceneNum = 15;
	createNavigationScene(0x004B2838, which);
	SetUpdateHandler(&Module1300::updateScene1316);
	// TODO Sound1ChList_setSoundValuesMulti(0xdword_4B2868, true, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
}
			
void Module1300::createScene1317(int which) {
}
			
void Module1300::createScene1318(int which) {
}
			
void Module1300::updateScene1302() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene1305(0);
		} else {
			createScene1308(1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1303() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1306(3);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1304() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1316(0);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1305() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1317(-1);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1306() {
}

void Module1300::updateScene1307() {
}

void Module1300::updateScene1308() {
}

void Module1300::updateScene1309() {
}

void Module1300::updateScene1310() {
}

void Module1300::updateScene1311() {
}

void Module1300::updateScene1312() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1313(0);
		} else if (_field20 == 1) {
			createScene1312(1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1313() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		switch (_field20) {
		case 0:
			createScene1315(1);
			break;
		case 1:
			createScene1316(1);
			break;
		case 3:
			createScene1312(1);
			break;
		case 5:
			createScene1314(1);
			break;
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1314() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1313(2);
		} else if (_field20 == 1) {
			createScene1308(0);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1315() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1313(3);
		} else if (_field20 == 1) {
			createScene1310(-1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1316() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1313(4);
		} else if (_field20 == 1) {
			createScene1304(0);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1317() {
}

void Module1300::updateScene1318() {
}

AsScene1302Bridge::AsScene1302Bridge(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _parentScene(parentScene) {

	_x = 320;
	_y = 240;
	createSurface1(0x88148150, 500);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1302Bridge::handleMessage);
	if (!getGlobalVar(0x13206309)) {
		setFileHash(0x88148150, 0, -1);
		_newHashListIndex = 0;
	} else {
		setFileHash(0x88148150, -1, -1);
		_newHashListIndex = -2;
	}
	_soundResource1.load(0x68895082);
	_soundResource2.load(0x689BD0C1);
}

uint32 AsScene1302Bridge::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4808:
		stLowerBridge();
		break;
	case 0x4809:
		stRaiseBridge();
		break;
	}
	return messageResult;
}

void AsScene1302Bridge::stLowerBridge() {
	setFileHash(0x88148150, 0, -1);
	SetAnimationCallback3(&AsScene1302Bridge::cbLowerBridgeEvent);
	_soundResource2.play();
}

void AsScene1302Bridge::stRaiseBridge() {
	setFileHash(0x88148150, 7, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
	_soundResource1.play();
}

void AsScene1302Bridge::cbLowerBridgeEvent() {
	_parentScene->sendMessage(0x2032, 0, this);
	setFileHash(0x88148150, -1, -1);
	_newHashListIndex = -2;
}

SsScene1302Fence::SsScene1302Fence(NeverhoodEngine *vm)
	: StaticSprite(vm, 0x11122122, 200), _soundResource1(vm), _soundResource2(vm) {
	
	SetUpdateHandler(&SsScene1302Fence::update);
	SetMessageHandler(&SsScene1302Fence::handleMessage);
	SetSpriteCallback(NULL);
	_firstY = _y;
	if (getGlobalVar(0x80101B1E))
		_y += 152;
	_soundResource1.load(0x7A00400C);
	_soundResource2.load(0x78184098);
}

void SsScene1302Fence::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 SsScene1302Fence::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		_soundResource1.play();
		SetSpriteCallback(&SsScene1302Fence::suMoveDown);
		SetMessageHandler(NULL);
		break;
	case 0x4809:
		_soundResource2.play();
		SetSpriteCallback(&SsScene1302Fence::suMoveUp);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

void SsScene1302Fence::suMoveDown() {
	if (_y < _firstY + 152)
		_y += 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteCallback(NULL);
	}
}

void SsScene1302Fence::suMoveUp() {
	if (_y > _firstY)
		_y -= 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteCallback(NULL);
	}
}

Class595::Class595(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 0xB0420130, 1015), _parentScene(parentScene) {

	SetMessageHandler(&Class595::handleMessage);
}

uint32 Class595::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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

Scene1302::Scene1302(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm) {

	SetMessageHandler(&Scene1302::handleMessage);
	_vm->_collisionMan->setHitRects(0x004B0858);
	setRectList(0x004B0A38);

	_background = addBackground(new DirtyBackground(_vm, 0x420643C4, 0, 0));
	_palette = new Palette(_vm, 0x420643C4);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x643C0428, NULL));

	_class595 = addSprite(new Class595(_vm, this));
	_sprite1 = addSprite(new StaticSprite(_vm, 0x942FC224, 300));
	_sprite2 = addSprite(new StaticSprite(_vm, 0x70430830, 1200));
	_sprite2->getSurface()->setVisible(false);
	_sprite3 = addSprite(new StaticSprite(_vm, 0x16E01E20, 1100));

	_asRing1 = addSprite(new AsScene1002Ring(_vm, this, false, 218, 122, _class595->getSurface()->getDrawRect().y, false));
	_asRing2 = addSprite(new AsScene1002Ring(_vm, this, true, 218 + 32, 132, _class595->getSurface()->getDrawRect().y, getGlobalVar(0x13206309)));
	_asRing3 = addSprite(new AsScene1002Ring(_vm, this, false, 218 + 32 + 32, 122, _class595->getSurface()->getDrawRect().y, false));
	_asRing4 = addSprite(new AsScene1002Ring(_vm, this, true, 218 + 32 + 32 + 32, 132, _class595->getSurface()->getDrawRect().y, getGlobalVar(0x80101B1E)));
	_asRing5 = addSprite(new AsScene1002Ring(_vm, this, false, 218 + 32 + 32 + 32 + 32, 115, _class595->getSurface()->getDrawRect().y, false));

	_asBridge = addSprite(new AsScene1302Bridge(_vm, this));
	_ssFence = addSprite(new SsScene1302Fence(_vm));
	_ssFence->getSurface()->getClipRect().x1 = 0;
	_ssFence->getSurface()->getClipRect().y1 = 0;
	_ssFence->getSurface()->getClipRect().x2 = 640;
	_ssFence->getSurface()->getClipRect().y2 = _sprite1->getSurface()->getDrawRect().y + _sprite1->getSurface()->getDrawRect().height;

	if (which < 0) {
		_klayman = new KmScene1002(_vm, this, _class595, NULL, 380, 364);
		setMessageList(0x004B0868);
	} else {
		_klayman = new KmScene1002(_vm, this, _class595, NULL, 293, 330);
		setMessageList(0x004B0870);
	}
	addSprite(_klayman);

	_klayman->getSurface()->getClipRect().x1 = 0;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = _sprite3->getSurface()->getDrawRect().x + _sprite3->getSurface()->getDrawRect().width;
	_klayman->getSurface()->getClipRect().y2 = 480;

	_asVenusFlyTrap = addSprite(new AsScene1002VenusFlyTrap(_vm, this, _klayman, true));
	_vm->_collisionMan->addSprite(_asVenusFlyTrap);

	_klayman->sendEntityMessage(0x2007, _asVenusFlyTrap, this);
	
}

uint32 Scene1302::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4A845A00) {
			_klayman->sendEntityMessage(0x1014, _asRing1, this);
		} else if (param.asInteger() == 0x43807801) {
			if (!getGlobalVar(0x13206309)) {
				_klayman->sendEntityMessage(0x1014, _asRing2, this);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32) {
					setMessageList(0x004B0940);
				} else {
					setMessageList(0x004B0938);
				}
			} else {
				setMessageList(0x004B0950);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x46C26A01) {
			_klayman->sendEntityMessage(0x1014, _asRing3, this);
		} else if (param.asInteger() == 0x468C7B11) {
			if (!getGlobalVar(0x80101B1E)) {
				_klayman->sendEntityMessage(0x1014, _asRing4, this);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 + 32 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32 + 32 + 32) {
					setMessageList(0x004B0940);
				} else {
					setMessageList(0x004B0938);
				}
			} else {
				setMessageList(0x004B0950);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x42845B19) {
			_klayman->sendEntityMessage(0x1014, _asRing5, this);
		} else if (param.asInteger() == 0x430A6060) {
			if (getGlobalVar(0x13206309)) {
				setMessageList2(0x004B0910);
			} else {
				messageList402220();
			}
		} else if (param.asInteger() == 0x012E2070) {
			if (getGlobalVar(0x13206309)) {
				setMessageList2(0x004B0968);
			} else {
				messageList402220();
			}
		} else if (param.asInteger() == 0x11C40840) {
			if (_asVenusFlyTrap->getX() >= 260 && _asVenusFlyTrap->getX() <= 342) {
				setMessageList(0x004B0878);
			} else {
				setMessageList(0x004B0978);
			}
		}
		break;
	case 0x2000:
		if (_klayman->getY() > 360) {
			_klayman->sendEntityMessage(0x1014, _asVenusFlyTrap, this);
			setMessageList2(0x004B08F0);	
		} else {
			setMessageList2(0x004B0920);
		}
		break;
	case 0x2002:
		if (_klayman->getX() > 545) {
			_parentModule->sendMessage(0x1009, 1, this);
		}
		break;
	case 0x2032:
		_sprite2->getSurface()->setVisible(true);
		break;
	case 0x4806:
		_parentModule->sendMessage(0x1024, 2, this);
		if (sender == _asRing1) {
			_soundResource.play(0x665198C0);
		} else if (sender == _asRing2) {
			_asBridge->sendMessage(0x4808, 0, this);
			setGlobalVar(0x13206309, 1);
		} else if (sender == _asRing3) {
			_soundResource.play(0xE2D389C0);
		} else if (sender == _asRing4) {
			_ssFence->sendMessage(0x4808, 0, this);
			setGlobalVar(0x80101B1E, 1);
		} else if (sender == _asRing5) {
			_soundResource.play(0x40428A09);
		}
		break;
	case 0x4807:
		if (sender == _asRing2) {
			_asBridge->sendMessage(0x4809, 0, this);
			setGlobalVar(0x13206309, 0);
			_sprite2->getSurface()->setVisible(false);
		} else if (sender == _asRing4) {
			_ssFence->sendMessage(0x4809, 0, this);
			setGlobalVar(0x80101B1E, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(0x13206309)) {
				_asRing2->sendMessage(0x4807, 0, this);
			} else {
				_asRing4->sendMessage(0x4807, 0, this);
			}
		}
		break;
	case 0x480F:
		if (sender == _asRing2) {
			_soundResource.play(0x60755842);
			_asBridge->sendMessage(0x4808, 0, this);
			setGlobalVar(0x13206309, 1);
		} else if (sender == _asRing4) {
			_soundResource.play(0x60755842);
			_ssFence->sendMessage(0x4808, 0, this);
			setGlobalVar(0x80101B1E, 1);
		}
		break;
	case 0x482A:
		_asVenusFlyTrap->sendMessage(0x482B, 0, this);
		break;
	case 0x482B:
		_asVenusFlyTrap->sendMessage(0x482A, 0, this);
		break;
	}
	return messageResult;
}

AsScene1303Balloon::AsScene1303Balloon(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene) {
	
	// TODO createSurface3(200, dword_4AF9F8);
	createSurface(200, 640, 480); //TODO: Remeove once the line above is done
	_x = 289;
	_y = 390;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1303Balloon::handleMessage);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	setFileHash(0x800278D2, 0, -1);
}

uint32 AsScene1303Balloon::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
		messageResult = 1;
		break;
	case 0x2000:
		stPopBalloon();
		break;
	}
	return messageResult;
}

uint32 AsScene1303Balloon::hmBalloonPopped(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x020B0003) {
			_soundResource.play(0x742B0055);
		} 
		break;
	case 0x3002:
		_soundResource.play(0x470007EE);
		setFileHash1();
		SetMessageHandler(NULL);
		_surface->setVisible(false);
		break;
	}
	return messageResult;
}

void AsScene1303Balloon::stPopBalloon() {
	setFileHash(0xAC004CD0, 0, -1);
	SetMessageHandler(&AsScene1303Balloon::hmBalloonPopped);
}

Scene1303::Scene1303(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	_surfaceFlag = true;
	SetMessageHandler(&Scene1303::handleMessage);
	setRectList(0x004AF9E8);
	
	_background = addBackground(new DirtyBackground(_vm, 0x01581A9C, 0, 0));
	_palette = new Palette(_vm, 0x01581A9C);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x81A9801D, NULL));

	if (!getGlobalVar(0xAC00C0D0)) {
		_asBalloon = addSprite(new AsScene1303Balloon(_vm, this));
		_vm->_collisionMan->addSprite(_asBalloon);
	}
	
	_sprite1 = addSprite(new StaticSprite(_vm, 0xA014216B, 1100));

	_klayman = new KmScene1303(_vm, this, 207, 332);
	addSprite(_klayman);
	setMessageList(0x004AF9A0);

	_klayman->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

}

uint32 Scene1303::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		setGlobalVar(0xAC00C0D0, 1);
		_asBalloon->sendMessage(0x2000, 0, this);
		break;
	case 0x4826:
		if (sender == _asBalloon && getGlobalVar(0x31C63C51)) {
			setMessageList(0x004AF9B8);
		}
		break;
	}
	return 0;
}

Class544::Class544(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, 0x548E9411, surfacePriority, x, y), _parentScene(parentScene) {
	
	if (getGlobalVar(0x31C63C51)) {
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	} else {
		SetMessageHandler(&Class544::handleMessage);
	}
}

uint32 Class544::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
		messageResult = 1;
		break;
	case 0x4806:
		setGlobalVar(0x31C63C51, 1);
		_surface->setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

Scene1304::Scene1304(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene1304::handleMessage);
	setRectList(0x004B91A8);

	_background = addBackground(new DirtyBackground(_vm, 0x062C0214, 0, 0));
	_palette = new Palette(_vm, 0x062C0214);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0xC021006A, NULL));
	
	if (getGlobalVar(0xAC00C0D0)) {
		_class545 = addSprite(new Class545(_vm, this, 0, 1100, 278, 347));
		_vm->_collisionMan->addSprite(_class545);
	} else {
		_class545 = addSprite(new AnimatedSprite(_vm, 0x80106018, 100, 279, 48));
		// TODO _class545->setUpdateDeltaXY();
	}

	if (!getGlobalVar(0x31C63C51)) {
		_class544 = addSprite(new Class544(_vm, this, 1100, 278, 347));
		_vm->_collisionMan->addSprite(_class544);
	} else {
		_class544 = NULL;
	}

	_sprite1 = addSprite(new StaticSprite(_vm, 0x0562E621, 1100));
	addSprite(new StaticSprite(_vm, 0x012AE033, 1100));
	addSprite(new StaticSprite(_vm, 0x090AF033, 1100));

	if (which < 0) {
		_klayman = new KmScene1304(_vm, this, 217, 347);
		setMessageList(0x004B90E8);
	} else {
		_klayman = new KmScene1304(_vm, this, 100, 347);
		setMessageList(0x004B90F0);
	}
	addSprite(_klayman);

	_klayman->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

}

uint32 Scene1304::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x415634A4) {
			if (getGlobalVar(0xAC00C0D0)) {
				messageList402220();
			} else {
				setMessageList(0x004B9158);
			}
		}
		break;
	case 0x4826:
		if (sender == _class544) {
			_klayman->sendEntityMessage(0x1014, _class544, this);
			setMessageList(0x004B9130);
		} else if (sender == _class545) {
			_klayman->sendEntityMessage(0x1014, _class545, this);
			setMessageList(0x004B9140);
		}
		break;
	}
	return 0;
}

Scene1305::Scene1305(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	_surfaceFlag = true;
	SetMessageHandler(&Scene1305::handleMessage);
	setRectList(0x004B6E98);

	_background = addBackground(new DirtyBackground(_vm, 0x28801B64, 0, 0));
	_palette = new Palette(_vm, 0x28801B64);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x01B60280, NULL));

	if (which < 0) {
		_klayman = new KmScene1305(_vm, this, 212, 441);
		setMessageList(0x004B6E40);
	} else {
		_klayman = new KmScene1305(_vm, this, 212, 441);
		setMessageList(0x004B6E48);
	}
	addSprite(_klayman);
	
}

uint32 Scene1305::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	return Scene::handleMessage(messageNum, param, sender);
}

} // End of namespace Neverhood
