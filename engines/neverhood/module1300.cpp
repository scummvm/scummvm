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
#include "neverhood/module1200.h"
#include "neverhood/module1400.h"
#include "neverhood/module2200.h"
#include "neverhood/gamemodule.h"
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
	_vm->gameState().sceneNum = 5;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_play(0x203197, 0, 2, 1);
	_childObject = new Scene1306(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1306);
}
			
void Module1300::createScene1307(int which) {
	_vm->gameState().sceneNum = 6;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_play(0x203197, 0, 2, 1);
	_childObject = new Scene1307(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1307);
}
			
void Module1300::createScene1308(int which) {
	_vm->gameState().sceneNum = 7;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_play(0x203197, 0, 2, 1);
	_childObject = new Scene1308(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1308);
}
			
void Module1300::createScene1309(int which) {
	_vm->gameState().sceneNum = 8;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	_childObject = new DiskplayerScene(_vm, this, 1);
	SetUpdateHandler(&Module1300::updateScene1309);
}
			
void Module1300::createScene1310(int which) {
	_vm->gameState().sceneNum = 9;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	createSmackerScene(0x20082818, true, true, false);
	SetUpdateHandler(&Module1300::updateScene1310);
}
			
void Module1300::createScene1311(int which) {
	_vm->gameState().sceneNum = 10;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	createSmackerScene(0x20082828, true, true, false);
	SetUpdateHandler(&Module1300::updateScene1310);
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
	_vm->gameState().sceneNum = 16;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B2868, false, 0, 0, 0, 0);
	// TODO Music18hList_stop(0x203197, 0, 2);
	_childObject = new Scene1317(_vm, this, which);
	SetUpdateHandler(&Module1300::updateScene1317);
}
			
void Module1300::createScene1318(int which) {
	// TODO: Credits scene
}
			
void Module1300::updateScene1302() {
	if (!updateChild()) {
		if (_moduleResult == 1) {
			createScene1305(0);
		} else {
			createScene1308(1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1303() {
	if (!updateChild()) {
		createScene1306(3);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1304() {
	if (!updateChild()) {
		createScene1316(0);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1305() {
	if (!updateChild()) {
		createScene1317(-1);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1306() {
	if (!updateChild()) {
		if (_moduleResult == 2) {
			createScene1309(0);
			_childObject->handleUpdate();
		} else if (_moduleResult == 3) {
			createScene1303(0);
			_childObject->handleUpdate();
		} else if (_moduleResult == 0) {
			sendMessage(_parentModule, 0x1009, 0);
		} else if (_moduleResult == 1) {
			createScene1311(-1);
			_childObject->handleUpdate();
		}
	}
}

void Module1300::updateScene1307() {
	if (!updateChild()) {
		createScene1308(2);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1308() {
	if (!updateChild()) {
		if (_moduleResult == 0) {
			createScene1314(0);
		} else if (_moduleResult == 1) {
			createScene1302(0);
		} else if (_moduleResult == 2) {
			createScene1307(0);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1309() {
	if (!updateChild()) {
		createScene1306(2);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1310() {
	if (!updateChild()) {
		if (_vm->gameState().sceneNum == 9)
			createScene1306(0);
		else
			createScene1315(0);
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1311() {
}

void Module1300::updateScene1312() {
	if (!updateChild()) {
		if (_moduleResult == 0) {
			createScene1313(0);
		} else if (_moduleResult == 1) {
			createScene1312(1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1313() {
	if (!updateChild()) {
		switch (_moduleResult) {
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
	if (!updateChild()) {
		if (_moduleResult == 0) {
			createScene1313(2);
		} else if (_moduleResult == 1) {
			createScene1308(0);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1315() {
	if (!updateChild()) {
		if (_moduleResult == 0) {
			createScene1313(3);
		} else if (_moduleResult == 1) {
			createScene1310(-1);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1316() {
	if (!updateChild()) {
		if (_moduleResult == 0) {
			createScene1313(4);
		} else if (_moduleResult == 1) {
			createScene1304(0);
		}
		_childObject->handleUpdate();
	}
}

void Module1300::updateScene1317() {
	if (!updateChild()) {
		createScene1318(-1);
		_childObject->handleUpdate();
	}
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
	sendMessage(_parentScene, 0x2032, 0);
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
		sendMessage(_parentScene, 0x1022, 995);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1015);
		break;
	}
	return messageResult;
}

Scene1302::Scene1302(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm) {

	SetMessageHandler(&Scene1302::handleMessage);
	_vm->_collisionMan->setHitRects(0x004B0858);
	setRectList(0x004B0A38);

	setBackground(0x420643C4);
	_palette = new Palette(_vm, 0x420643C4);
	_palette->usePalette();
	insertMouse433(0x643C0428);

	_class595 = addSprite(new Class595(_vm, this));
	_sprite1 = insertStaticSprite(0x942FC224, 300);
	_sprite2 = insertStaticSprite(0x70430830, 1200);
	_sprite2->getSurface()->setVisible(false);
	_sprite3 = insertStaticSprite(0x16E01E20, 1100);

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
		InsertKlaymanInitArgs(KmScene1002, 380, 364, (_class595, NULL));
		setMessageList(0x004B0868);
	} else {
		InsertKlaymanInitArgs(KmScene1002, 293, 330, (_class595, NULL));
		setMessageList(0x004B0870);
	}

	_klayman->getSurface()->getClipRect().x1 = 0;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = _sprite3->getSurface()->getDrawRect().x + _sprite3->getSurface()->getDrawRect().width;
	_klayman->getSurface()->getClipRect().y2 = 480;

	_asVenusFlyTrap = addSprite(new AsScene1002VenusFlyTrap(_vm, this, _klayman, true));
	_vm->_collisionMan->addSprite(_asVenusFlyTrap);

	sendEntityMessage(_klayman, 0x2007, _asVenusFlyTrap);
	
}

uint32 Scene1302::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4A845A00) {
			sendEntityMessage(_klayman, 0x1014, _asRing1);
		} else if (param.asInteger() == 0x43807801) {
			if (!getGlobalVar(0x13206309)) {
				sendEntityMessage(_klayman, 0x1014, _asRing2);
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
			sendEntityMessage(_klayman, 0x1014, _asRing3);
		} else if (param.asInteger() == 0x468C7B11) {
			if (!getGlobalVar(0x80101B1E)) {
				sendEntityMessage(_klayman, 0x1014, _asRing4);
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
			sendEntityMessage(_klayman, 0x1014, _asRing5);
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
			sendEntityMessage(_klayman, 0x1014, _asVenusFlyTrap);
			setMessageList2(0x004B08F0);	
		} else {
			setMessageList2(0x004B0920);
		}
		break;
	case 0x2002:
		if (_klayman->getX() > 545) {
			sendMessage(_parentModule, 0x1009, 1);
		}
		break;
	case 0x2032:
		_sprite2->getSurface()->setVisible(true);
		break;
	case 0x4806:
		sendMessage(_parentModule, 0x1024, 2);
		if (sender == _asRing1) {
			_soundResource.play(0x665198C0);
		} else if (sender == _asRing2) {
			sendMessage(_asBridge, 0x4808, 0);
			setGlobalVar(0x13206309, 1);
		} else if (sender == _asRing3) {
			_soundResource.play(0xE2D389C0);
		} else if (sender == _asRing4) {
			sendMessage(_ssFence, 0x4808, 0);
			setGlobalVar(0x80101B1E, 1);
		} else if (sender == _asRing5) {
			_soundResource.play(0x40428A09);
		}
		break;
	case 0x4807:
		if (sender == _asRing2) {
			sendMessage(_asBridge, 0x4809, 0);
			setGlobalVar(0x13206309, 0);
			_sprite2->getSurface()->setVisible(false);
		} else if (sender == _asRing4) {
			sendMessage(_ssFence, 0x4809, 0);
			setGlobalVar(0x80101B1E, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(0x13206309)) {
				sendMessage(_asRing2, 0x4807, 0);
			} else {
				sendMessage(_asRing4, 0x4807, 0);
			}
		}
		break;
	case 0x480F:
		if (sender == _asRing2) {
			_soundResource.play(0x60755842);
			sendMessage(_asBridge, 0x4808, 0);
			setGlobalVar(0x13206309, 1);
		} else if (sender == _asRing4) {
			_soundResource.play(0x60755842);
			sendMessage(_ssFence, 0x4808, 0);
			setGlobalVar(0x80101B1E, 1);
		}
		break;
	case 0x482A:
		sendMessage(_asVenusFlyTrap, 0x482B, 0);
		break;
	case 0x482B:
		sendMessage(_asVenusFlyTrap, 0x482A, 0);
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
		sendMessage(_parentScene, 0x4826, 0);
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
	
	setBackground(0x01581A9C);
	_palette = new Palette(_vm, 0x01581A9C);
	_palette->usePalette();
	insertMouse433(0x81A9801D);

	if (!getGlobalVar(0xAC00C0D0)) {
		_asBalloon = addSprite(new AsScene1303Balloon(_vm, this));
		_vm->_collisionMan->addSprite(_asBalloon);
	}
	
	_sprite1 = insertStaticSprite(0xA014216B, 1100);

	InsertKlayman(KmScene1303, 207, 332);
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
		sendMessage(_asBalloon, 0x2000, 0);
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
		sendMessage(_parentScene, 0x4826, 0);
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

	setBackground(0x062C0214);
	_palette = new Palette(_vm, 0x062C0214);
	_palette->usePalette();
	insertMouse433(0xC021006A);
	
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

	_sprite1 = insertStaticSprite(0x0562E621, 1100);
	insertStaticSprite(0x012AE033, 1100);
	insertStaticSprite(0x090AF033, 1100);

	if (which < 0) {
		InsertKlayman(KmScene1304, 217, 347);
		setMessageList(0x004B90E8);
	} else {
		InsertKlayman(KmScene1304, 100, 347);
		setMessageList(0x004B90F0);
	}

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
			sendEntityMessage(_klayman, 0x1014, _class544);
			setMessageList(0x004B9130);
		} else if (sender == _class545) {
			sendEntityMessage(_klayman, 0x1014, _class545);
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

	setBackground(0x28801B64);
	_palette = new Palette(_vm, 0x28801B64);
	_palette->usePalette();
	insertMouse433(0x01B60280);

	if (which < 0) {
		InsertKlayman(KmScene1305, 212, 441);
		setMessageList(0x004B6E40);
	} else {
		InsertKlayman(KmScene1305, 212, 441);
		setMessageList(0x004B6E48);
	}
	
}

uint32 Scene1305::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	return Scene::handleMessage(messageNum, param, sender);
}

AsScene1306Elevator::AsScene1306Elevator(NeverhoodEngine *vm, Scene *parentScene, AnimatedSprite *asElevatorDoor)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm), 
	_parentScene(parentScene), _asElevatorDoor(asElevatorDoor), _isUp(false), _isDown(true),
	_countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x043B0270, 100);
	setFileHash(0x043B0270, 0, -1);
	_newHashListIndex = 0;
	SetMessageHandler(&AsScene1306Elevator::handleMessage);
	_soundResource1.load(0x1C100E83);
	_soundResource2.load(0x1C08CEC5);
	_soundResource3.load(0x5D011E87);
}

void AsScene1306Elevator::update() {
	if (_isUp && _countdown != 0 && (--_countdown == 0)) {
		stGoingDown();
	}
	AnimatedSprite::update();
	if (_frameIndex == 7) {
		_soundResource3.play();
		_asElevatorDoor->getSurface()->setVisible(false);
	}
}

void AsScene1306Elevator::upGoingDown() {
	AnimatedSprite::update();
	if (_frameIndex == 5) {
		_asElevatorDoor->getSurface()->setVisible(true);
	}
}

uint32 AsScene1306Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		if (_isUp)
			_countdown = 144;
		messageResult = _isUp ? 1 : 0;
		break;
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4808:
		if (_isDown)
			stGoingUp();
		break;
	}
	return messageResult;
}

void AsScene1306Elevator::stGoingUp() {
	_surface->setVisible(true);
	_isDown = false;
	SetUpdateHandler(&AsScene1306Elevator::update);
	setFileHash(0x043B0270, 0, -1);
	SetAnimationCallback3(&AsScene1306Elevator::cbGoingUpEvent);
	_soundResource1.play();
}

void AsScene1306Elevator::cbGoingUpEvent() {
	SetUpdateHandler(&AsScene1306Elevator::update);
	sendMessage(_parentScene, 0x4808, 0);
	_isUp = true;
	_countdown = 144;
	setFileHash1();
	_surface->setVisible(false);
}

void AsScene1306Elevator::stGoingDown() {
	SetUpdateHandler(&AsScene1306Elevator::upGoingDown);
	_isUp = false;
	_surface->setVisible(true);
	setFileHash(0x043B0270, -1, -1);
	_playBackwards = true;
	SetAnimationCallback3(&AsScene1306Elevator::cbGoingDownEvent);
	_soundResource2.play();
}

void AsScene1306Elevator::cbGoingDownEvent() {
	_isDown = true;
	sendMessage(_parentScene, 0x4809, 0);
	SetUpdateHandler(&AsScene1306Elevator::update);
	setFileHash1();
}

Scene1306::Scene1306(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	if (getGlobalVar(0xC0780812) && !getGlobalVar(0x13382860))
		setGlobalVar(0x13382860, 4);
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene1306::handleMessage);

	setBackground(0x05303114);
	_palette = new Palette(_vm, 0x05303114);
	_palette->usePalette();
	insertMouse433(0x0311005B);

	if (!getGlobalVar(0x13382860)) {
		_class545 = addSprite(new Class545(_vm, this, 2, 1100, 435, 445));
		_vm->_collisionMan->addSprite(_class545);
	}

	_ssButton = addSprite(new SsCommonButtonSprite(_vm, this, 0x404A36A0, 100, 0x440C1000));
	
	_asTape = addSprite(new AsScene1201Tape(_vm, this, 19, 1100, 359, 445, 0x9148A011));

	_asElevatorDoor = new AnimatedSprite(_vm, 0x043B0270, 90, 320, 240);
	_asElevatorDoor->setFileHash(0x043B0270, 6, -1);
	_asElevatorDoor->setNewHashListIndex(6);
	addSprite(_asElevatorDoor);

	_asElevator = addSprite(new AsScene1306Elevator(_vm, this, _asElevatorDoor));
	
	_sprite1 = insertStaticSprite(0x036A1EE0, 80);
	
	insertStaticSprite(0x00042313, 1100);

	if (which < 0) {
		InsertKlayman(KmScene1306, 380, 440);
		setMessageList(0x004AFAD0);
		sendMessage(this, 0x2000, 0);
		_vm->_collisionMan->addSprite(_asTape);
	} else if (which == 1) {
		InsertKlayman(KmScene1306, 136, 440);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AFAF0);
		sendMessage(this, 0x2000, 1);
		_vm->_collisionMan->addSprite(_asTape);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			InsertKlayman(KmScene1306, 515, 440);
			_klayman->setDoDeltaX(1);
		} else {
			InsertKlayman(KmScene1306, 355, 440);
		}
		setMessageList(0x004AFBC8);
		sendMessage(this, 0x2000, 0);
		_vm->_collisionMan->addSprite(_asTape);
	} else if (which == 3) {
		InsertKlayman(KmScene1306, 534, 440);
		setMessageList(0x004AFC30);
		sendMessage(this, 0x2000, 0);
		_vm->_collisionMan->addSprite(_asTape);
	} else if (which == 4) {
		InsertKlayman(KmScene1306, 136, 440);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AFC38);
		sendMessage(this, 0x2000, 1);
		_vm->_collisionMan->addSprite(_asTape);
	} else if (which == 5) {
		InsertKlayman(KmScene1306, 136, 440);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AFB00);
		sendMessage(this, 0x2000, 1);
		_vm->_collisionMan->addSprite(_asTape);
	} else {
		InsertKlayman(KmScene1306, 286, 408);
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		sendMessage(this, 0x2000, 0);
		SetMessageHandler(&Scene1306::handleMessage416EB0);
		clearRectList();
		sendMessage(_asElevator, 0x4808, 0);
	}

}
	
Scene1306::~Scene1306() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1306::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_asElevator, 0x2001, 0) != 0) {
				setMessageList(0x004AFBD8);
			} else {
				setMessageList(0x004AFAE0);
			}
		} else if (param.asInteger() == 0x8E646E00) {
			setMessageList(0x004AFAD8);
			clearRectList();
			SetMessageHandler(&Scene1306::handleMessage416EB0);
		}
		break;
	case 0x2000:
		if (param.asInteger() != 0) {
			setRectList(0x004AFD28);
			_klayman->setKlaymanTable3();
		} else {
			setRectList(0x004AFD18);
			_klayman->setKlaymanTable1();
		}
		break;
	case 0x480B:
		if (sender == _ssButton) {
			sendMessage(_asElevator, 0x4808, 0);
		}
		break;
	case 0x4826:
		if (sender == _class545) {
			if (_klayman->getX() >= 249) {
				sendEntityMessage(_klayman, 0x1014, _class545);
				setMessageList(0x004AFC58);
			}
		} else if (sender == _asTape) {
			if (_klayman->getX() >= 249) {
				sendEntityMessage(_klayman, 0x1014, _class545);
				setMessageList(0x004AFC68);
			}
		}
		break;
	case 0x482A:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case 0x482B:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		_vm->_collisionMan->addSprite(_asTape);
		break;
	}
	return 0;
}

uint32 Scene1306::handleMessage416EB0(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		setMessageList(0x004AFBD0);
		SetMessageHandler(&Scene1306::handleMessage);
		break;
	case 0x4809:
		sendMessage(_parentModule, 0x1009, 1);
		break;
	case 0x482A:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case 0x482B:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		_vm->_collisionMan->addSprite(_asTape);
		break;
	}
	return 0;
}

static const uint32 kAsScene1307KeyResourceList1[] = {
	0x0438069C,
	0x45B0023C,
	0x05700217
};

static const uint32 kAsScene1307KeyResourceList2[] = {
	0x04441334,
	0x061433F0,
	0x06019390
};

static const uint32 kAsScene1307KeyResourceList3[] = {
	0x11A80030,
	0x178812B1,
	0x1488121C
};

static const uint32 *kAsScene1307KeyResourceLists[] = {
	kAsScene1307KeyResourceList1,
	kAsScene1307KeyResourceList2,
	kAsScene1307KeyResourceList3
};

static const int kAsScene1307KeySurfacePriorities[] = {
	700, 
	500, 
	300, 
	100
};

const uint kAsScene1307KeyPointsCount = 12;

static const NPoint kAsScene1307KeyPoints[] = {
	{-2,  0},
	{-5,  0},
	{ 5,  0},
	{12,  0},
	{17,  0},
	{25,  0},
	{16, -2},
	{10, -6},
	{ 0, -7},
	{-7, -3},
	{-3,  4},
	{ 2,  2}
};

const uint kAsScene1307KeyFrameIndicesCount = 20;

static const int16 kAsScene1307KeyFrameIndices[] = {
	 1,  4,  8, 11, 15, 16, 17, 17, 17, 16, 
	15, 14, 12, 10,  9,  7,  5,  3,  2,  1
};

const int kAsScene1307KeyDivValue = 200;

const int16 kAsScene1307KeyXDelta = 70;
const int16 kAsScene1307KeyYDelta = -12;

AsScene1307Key::AsScene1307Key(NeverhoodEngine *vm, Scene *parentScene, uint index, NRect *clipRects)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_soundResource4(vm), _parentScene(parentScene), _index(index), _clipRects(clipRects),
	_isClickable(true) {
	
	NPoint pt;
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_index]; 
	
	_dataResource.load(0x22102142);
	_pointList = _dataResource.getPointArray(0xAC849240);
	
	pt = (*_pointList)[getSubVar(0xA010B810, _index)];
	_x = pt.x;
	_y = pt.y;
	
	// TODO createSurface3(kAsScene1307KeySurfacePriorities[getSubVar(0xA010B810, _index) % 4], fileHashes);
	createSurface(kAsScene1307KeySurfacePriorities[getSubVar(0xA010B810, _index) % 4], 640, 480); //TODO: Remeove once the line above is done
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1307Key::handleMessage);
	
	setFileHash(fileHashes[0], 0, -1);
	
	_soundResource1.load(0xDC4A1280);
	_soundResource2.load(0xCC021233);
	_soundResource3.load(0xC4C23844);
	_soundResource3.load(0xC4523208);

}

uint32 AsScene1307Key::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_isClickable) {
			sendMessage(_parentScene, 0x4826, 0);
			stRemoveKey();
			messageResult = 1;
		}
		break;
	case 0x2000:
		_isClickable = param.asInteger() != 0;
		break;
	case 0x2001:
		setSubVar(0xA010B810, _index, param.asInteger());
		stMoveKey();
		break;
	case 0x2003:
		_soundResource4.play();
		stUnlock();
		break;
	case 0x2004:
		_soundResource3.play();
		stInsert();
		break;
	}
	return messageResult;
}

void AsScene1307Key::suRemoveKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x += kAsScene1307KeyPoints[_pointIndex].x;
		_y += kAsScene1307KeyPoints[_pointIndex].y;
		processDelta();
		_pointIndex++;
	} else {
		SetSpriteCallback(NULL);
	}
}

void AsScene1307Key::suInsertKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].x;
		_y -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].y;
		processDelta();
		_pointIndex++;
		if (_pointIndex == 7)
			_soundResource1.play();
	} else {
		SetSpriteCallback(NULL);
		sendMessage(_parentScene, 0x2002, 0);
	}
}

void AsScene1307Key::suMoveKey() {
	if (_pointIndex < kAsScene1307KeyFrameIndicesCount) {
		_frameIndex += kAsScene1307KeyFrameIndices[_pointIndex];
		_x = _prevX + (_deltaX * _frameIndex) / kAsScene1307KeyDivValue;
		_y = _prevY + (_deltaY * _frameIndex) / kAsScene1307KeyDivValue;
		processDelta();
		_pointIndex++;
	} else {
		NPoint pt = (*_pointList)[getSubVar(0xA010B810, _index)];
		_x = pt.x + kAsScene1307KeyXDelta;
		_y = pt.y + kAsScene1307KeyYDelta;
		stInsertKey();
	}
}

void AsScene1307Key::stRemoveKey() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_index]; 
	_pointIndex = 0;
	SetSpriteCallback(&AsScene1307Key::suRemoveKey);
	setFileHash(fileHashes[0], 0, -1);
	_soundResource2.play();
}

void AsScene1307Key::stInsertKey() {
	_pointIndex = 0;
	sendMessage(_parentScene, 0x1022, kAsScene1307KeySurfacePriorities[getSubVar(0xA010B810, _index) % 4]);
	_surface->getClipRect() = _clipRects[getSubVar(0xA010B810, _index) % 4];
	SetSpriteCallback(&AsScene1307Key::suInsertKey);
	_newHashListIndex = -2;
}

void AsScene1307Key::stMoveKey() {
	NPoint pt = (*_pointList)[getSubVar(0xA010B810, _index)];
	int16 newX = pt.x + kAsScene1307KeyXDelta;
	int16 newY = pt.y + kAsScene1307KeyYDelta;
	sendMessage(_parentScene, 0x1022, 1000);
	_surface->getClipRect().x1 = 0;
	_surface->getClipRect().y1 = 0;
	_surface->getClipRect().x2 = 640;
	_surface->getClipRect().y2 = 480;
	_prevX = _x;
	_prevY = _y;
	if (newX == _x && newY == _y) {
		stInsertKey();
	} else {
		const uint32 *fileHashes = kAsScene1307KeyResourceLists[_index]; 
		_pointIndex = 0;
		_frameIndex = 0;
		_deltaX = newX - _x;
		_deltaY = newY - _y;
		SetSpriteCallback(&AsScene1307Key::suMoveKey);
		setFileHash(fileHashes[0], 0, -1);
	}
}

void AsScene1307Key::stUnlock() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_index]; 
	setFileHash(fileHashes[1], 0, -1);
	_newHashListIndex = -2;
}

void AsScene1307Key::stInsert() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_index]; 
	setFileHash(fileHashes[2], 0, -1);
	_newHashListIndex = -2;
}

Scene1307::Scene1307(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _countdown(0),
	_asCurrKey(NULL), _isInsertingKey(false), _doLeaveScene(false), _isPuzzleSolved(false) {

	//DEBUG
	setSubVar(0x08D0AB11, 0, 1);
	setSubVar(0x08D0AB11, 1, 1);
	setSubVar(0x08D0AB11, 2, 1);

	Sprite *tempSprite;
	
	_vm->gameModule()->initScene1307Vars();
	
	_dataResource.load(0x22102142);
	_keyHolePoints = _dataResource.getPointArray(0xAC849240);

	for (uint i = 0; i < 16; i++) {
		NPoint pt = (*_keyHolePoints)[i];
		_keyHoleRects[i].x1 = pt.x - 15;
		_keyHoleRects[i].y1 = pt.y - 15;
		_keyHoleRects[i].x2 = pt.x + 15;
		_keyHoleRects[i].y2 = pt.y + 15;
	}

	_surfaceFlag = true;
	SetMessageHandler(&Scene1307::handleMessage);
	SetUpdateHandler(&Scene1307::update);

	setBackground(0xA8006200);
	_palette = new Palette(_vm, 0xA8006200);
	_palette->usePalette();
	addEntity(_palette);
	insertMouse435(0x06204A88, 20, 620);

	tempSprite = insertStaticSprite(0x00A3621C, 800);
	_clipRects[0].x1 = tempSprite->getSurface()->getDrawRect().x;
	_clipRects[0].y1 = 0;
	_clipRects[0].x2 = 640;
	_clipRects[0].y2 = 480;

	tempSprite = insertStaticSprite(0x00A3641C, 600);
	_clipRects[1].x1 = tempSprite->getSurface()->getDrawRect().x;
	_clipRects[1].y1 = 0;
	_clipRects[1].x2 = 640;
	_clipRects[1].y2 = 480;

	tempSprite = insertStaticSprite(0x00A3681C, 400);
	_clipRects[2].x1 = tempSprite->getSurface()->getDrawRect().x;
	_clipRects[2].y1 = 0;
	_clipRects[2].x2 = 640;
	_clipRects[2].y2 = 480;

	tempSprite = insertStaticSprite(0x00A3701C, 200);
	_clipRects[3].x1 = tempSprite->getSurface()->getDrawRect().x;
	_clipRects[3].y1 = 0;
	_clipRects[3].x2 = 640;
	_clipRects[3].y2 = 480;

	for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
		if (getSubVar(0x08D0AB11, keyIndex)) {
			_asKeys[keyIndex] = addSprite(new AsScene1307Key(_vm, this, keyIndex, _clipRects));
			_vm->_collisionMan->addSprite(_asKeys[keyIndex]);
		} else {
			_asKeys[keyIndex] = NULL;
		}
	}

	_soundResource.load(0x68E25540);

}

void Scene1307::update() {
	Scene::update();
	if (_countdown && (--_countdown == 0)) {
		_doLeaveScene = true;
	} else if (_countdown == 20) {
		_palette->startFadeToWhite(40);
	}
	if (_doLeaveScene && !_soundResource.isPlaying()) {
		sendMessage(_parentModule, 0x1009, 1);
		setGlobalVar(0x80455A41, 1);
	} 
}

uint32 Scene1307::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO Debug stuff
		if (!_isPuzzleSolved) {
			if (param.asPoint().x > 20 && param.asPoint().x < 620) {
				if (_asCurrKey && !_isInsertingKey) {
					int16 mouseX = param.asPoint().x;
					int16 mouseY = param.asPoint().y;
					uint clickedKeyHoleIndex;
					for (clickedKeyHoleIndex = 0; clickedKeyHoleIndex < 16; clickedKeyHoleIndex++) {
						if (mouseX >= _keyHoleRects[clickedKeyHoleIndex].x1 && mouseX <= _keyHoleRects[clickedKeyHoleIndex].x2 && 
							mouseY >= _keyHoleRects[clickedKeyHoleIndex].y1 && mouseY <= _keyHoleRects[clickedKeyHoleIndex].y2)
							break;
					}
					if (clickedKeyHoleIndex < 16) {
						// Check if the clicked keyhole is already occupied with a key
						bool occupied = false;
						for (uint keyIndex = 0; keyIndex < 3 && !occupied; keyIndex++) {
							if (getSubVar(0x08D0AB11, keyIndex) && _asKeys[keyIndex] != _asCurrKey) {
								if (getSubVar(0xA010B810, keyIndex) == clickedKeyHoleIndex)
									occupied = true;
							}
						}
						if (!occupied) {
							// If the keyhole is free, insert the current key
							sendMessage(_asCurrKey, 0x2001, clickedKeyHoleIndex);
							_isInsertingKey = true;
							_mouseClicked = false;
						}
					}
				}
			} else if (_countdown == 0 && !_asCurrKey && !_isInsertingKey) {
				sendMessage(_parentModule, 0x1009, 0);
			}
		}
		break;
	// TODO Debug stuff
	case 0x2002:
		// Check if all keys are in the correct keyholes
		if (getSubVar(0x08D0AB11, 0) && getSubVar(0xA010B810, 0) == getSubVar(0x0C10A000, 0) &&
			getSubVar(0x08D0AB11, 1) && getSubVar(0xA010B810, 1) == getSubVar(0x0C10A000, 1) &&
			getSubVar(0x08D0AB11, 2) && getSubVar(0xA010B810, 2) == getSubVar(0x0C10A000, 2)) {
			// Play unlock animations for all keys
			for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
				if (_asKeys[keyIndex])
					sendMessage(_asKeys[keyIndex], 0x2003, 1);
			}
			_soundResource.play();
			_isPuzzleSolved = true;
			_countdown = 47;
		} else {
			for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
				if (getSubVar(0x08D0AB11, keyIndex) && _asKeys[keyIndex]) {
					sendMessage(_asKeys[keyIndex], 0x2000, 1);
				}
			}
			sendMessage(_asCurrKey, 0x2004, 1);
		}
		_asCurrKey = NULL;
		_isInsertingKey = false;
		break;
	case 0x4826:
		_asCurrKey = (Sprite*)sender;
		for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
			if (getSubVar(0x08D0AB11, keyIndex) && _asKeys[keyIndex]) {
				sendMessage(_asKeys[keyIndex], 0x2000, 0);
			}
		}
		break;
	}
	return messageResult;
}

static const uint32 kScene1308FileHashes[] = {
	0x08006320,
	0x10006320,
	0x20006320,
	0x40006320,
	0x80006320,
	0x00006321,
	0x00006322,
	0x00006324,
	0x00006328,
	0x08306320,
	0x10306320,
	0x20306320,
	0x40306320,
	0x80306320,
	0x00306321,
	0x00306322
};

Class549::Class549(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xBA0AE050, 1100, 320, 240), _soundResource(vm), 
	_parentScene(parentScene) {
	
	SetMessageHandler(&Class549::handleMessage);
	_surface->setVisible(false);
	setFileHash1();
}

uint32 Class549::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4808:
		sub455470();
		break;
	case 0x4809:
		sub4554F0();
		break;
	}
	return messageResult;
}

void Class549::sub455470() {
	setFileHash(0xBA0AE050, 0, -1);
	_surface->setVisible(true);
	SetAnimationCallback3(&Class549::hide);
	_soundResource.play(calcHash("fxDoorOpen38"));
}

void Class549::hide() {
	sendMessage(_parentScene, 0x2000, 0);
	setFileHash1();
	_surface->setVisible(false);
}

void Class549::sub4554F0() {
	setFileHash(0xBA0AE050, -1, -1);
	_playBackwards = true;
	_surface->setVisible(true);
	SetAnimationCallback3(&Class549::sub455550);
	_soundResource.play(calcHash("fxDoorClose38"));
}

void Class549::sub455550() {
	sendMessage(_parentScene, 0x2001, 0);
	setFileHash1();
}

Class592::Class592(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xA08A0851, 1100, 320, 240), _soundResource(vm), 
	_parentScene(parentScene) {
	
	SetMessageHandler(&Class592::handleMessage);
	SetAnimationCallback3(&Class592::sub455710);
	_soundResource.play(0x51456049);
}
 
uint32 Class592::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class592::sub455710() {
	setFileHash(0x6238B191, 0, -1);
	SetAnimationCallback3(&Class592::sub455740);
	_x = 580;
	_y = 383;
}

void Class592::sub455740() {
	sendMessage(_parentScene, 0x2004, 0);
	setFileHash1();
	_surface->setVisible(false);
}

Class593::Class593(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0x80180A10, 100, 320, 240), _parentScene(parentScene) {
	
	SetMessageHandler(&Class593::handleMessage);
	_surface->setVisible(false);
	setFileHash1();
	Entity::_priority = 1200;
}

uint32 Class593::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		sub4558F0();
		break;
	case 0x2003:
		sub455920();
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class593::sub4558F0() {
	setFileHash(0x80180A10, 0, -1);
	_surface->setVisible(false);
	_newHashListIndex = -2;
}

void Class593::sub455920() {
	setFileHash(0x80180A10, -1, -1);
	_playBackwards = true;
	SetAnimationCallback3(&Class593::sub455950);
}

void Class593::sub455950() {
	sendMessage(_parentScene, 0x2003, 0);
	setFileHash1();
	_surface->setVisible(false);
}

Class601::Class601(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {
	
	_surface->setVisible(false);
	_x = _spriteResource.getPosition().x + index * 20;
	StaticSprite::update();
}

Class513::Class513(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _soundResource(vm) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class513::handleMessage);
	_x = 286;
	_y = 429;
	createSurface1(0xA282C472, 100);
	setFileHash(0xA282C472, 0, -1);
}

uint32 Class513::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x66382026) {
			_soundResource.play(0x0CD84468);
		} else if (param.asInteger() == 0x6E28061C) {
			_soundResource.play(0x78C8402C);
		} else if (param.asInteger() == 0x462F0410) {
			_soundResource.play(0x60984E28);
		}
		break;
	}
	return messageResult;
}

Scene1308::Scene1308(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag1(false) {
	
	_vm->gameModule()->initScene1307Vars();

	_surfaceFlag = true;		
	SetMessageHandler(&Scene1308::handleMessage);
	
	setBackground(0x41024202);
	_palette = new Palette(_vm, 0x41024202);
	_palette->usePalette();
	insertMouse433(0x24206418);

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 17, 1100, 502, 445, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape);

	if (getGlobalVar(0x01023818)) {
		addSprite(new Class513(_vm));
		addSprite(new AnimatedSprite(_vm, 0x461A1490, 200, 235, 429));
	}
	
	_sprite1 = insertStaticSprite(0x0A042060, 1100);
	_class549 = addSprite(new Class549(_vm, this));
	_class593 = addSprite(new Class593(_vm, this));

	_class601_1 = addSprite(new Class601(_vm, kScene1308FileHashes[getSubVar(0x0C10A000, 1)], 0));
	_class601_2 = addSprite(new Class601(_vm, kScene1308FileHashes[getSubVar(0x0C10A000, 0)], 1));
	_class601_2 = addSprite(new Class601(_vm, kScene1308FileHashes[getSubVar(0x0C10A000, 2)], 2));

	_sprite2 = insertStaticSprite(0x40043120, 995);
	_sprite3 = insertStaticSprite(0x43003100, 995);
	_sprite4 = NULL;

	if (which < 0) {
		InsertKlayman(KmScene1308, 380, 440);
		setMessageList(0x004B57C0);
		if (getGlobalVar(0x80455A41)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			setRectList(0x004B5990);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else if (which == 1) {
		InsertKlayman(KmScene1308, 640, 440);
		setMessageList(0x004B57C8);
		if (getGlobalVar(0x80455A41)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			setRectList(0x004B5990);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else if (which == 2) {
		InsertKlayman(KmScene1308, 475, 440);
		setMessageList(0x004B58B0);
		if (getGlobalVar(0x80455A41)) {
			_sprite5 = addSprite(new Class592(_vm, this));
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			_sprite4->getSurface()->setVisible(false);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else {
		InsertKlayman(KmScene1308, 41, 440);
		setMessageList(0x004B57D0);
		sendMessage(_class549, 0x4808, 0);
		_sprite1->getSurface()->setVisible(false);
		if (getGlobalVar(0x80455A41)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			_klayman->getSurface()->setVisible(false);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			_klayman->getSurface()->setVisible(false);
		}
	}

	if (_sprite4) {
		_klayman->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
		_klayman->getSurface()->getClipRect().y1 = 0;
		_klayman->getSurface()->getClipRect().x2 = _sprite4->getSurface()->getDrawRect().x + _sprite4->getSurface()->getDrawRect().width;
		_klayman->getSurface()->getClipRect().y2 = 480;
	} else {
		_klayman->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
		_klayman->getSurface()->getClipRect().y1 = 0;
		_klayman->getSurface()->getClipRect().x2 = 640;
		_klayman->getSurface()->getClipRect().y2 = 480;
	}

	if (getGlobalVar(0x04A105B3) == 4) {
		_class489 = new Class489(_vm, this, _klayman, 0);
		addSprite(_class489);
		_vm->_collisionMan->addSprite(_class489);
		_class489->getSurface()->getClipRect().x1 = 0;
		_class489->getSurface()->getClipRect().y1 = 0;
		_class489->getSurface()->getClipRect().x2 = 0;
		_class489->getSurface()->getClipRect().y2 = _sprite2->getSurface()->getDrawRect().y + _sprite2->getSurface()->getDrawRect().height;
		_class489->setRepl(64, 0);
	} else {
		_class489 = NULL;
	}

}

uint32 Scene1308::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x88C11390) {
			setRectList(0x004B59A0);
			_flag1 = true;
		} else if (param.asInteger() == 0x08821382) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			if (getGlobalVar(0x80455A41)) {
				setRectList(0x004B5990);
			} else {
				setRectList(0x004B5980);
			}
			_flag1 = false;
		} else if (param.asInteger() == 0x4AC68808) {
			clearRectList();
			sendMessage(_class549, 0x4809, 0);
			_sprite1->getSurface()->setVisible(false);
			_klayman->getSurface()->setVisible(false);
		}
		break;
	case 0x1022:
		if (sender == _class489) {
			if (param.asInteger() >= 1000)
				setSurfacePriority(_sprite3->getSurface(), 1100);
			else
				setSurfacePriority(_sprite3->getSurface(), 995);
		}
		break;
	case 0x2000:
		if (getGlobalVar(0x80455A41)) {
			setRectList(0x004B5990);
		} else {
			setRectList(0x004B5980);
		}
		setMessageList(0x004B57E8);
		_sprite1->getSurface()->setVisible(true);
		_klayman->getSurface()->setVisible(true);
		break;
	case 0x2001:
		sendMessage(_parentModule, 0x1009, 0);
		break;
	case 0x2003:
		_class601_1->getSurface()->setVisible(false);
		_class601_2->getSurface()->setVisible(false);
		_class601_3->getSurface()->setVisible(false);
		break;
	case 0x2004:
		_sprite4->getSurface()->setVisible(true);
		setRectList(0x004B5990);
		break;
	case 0x4807:
		sendMessage(_class593, 0x2003, 0);
		break;
	case 0x480F:
		sendMessage(_class593, 0x2002, 0);
		_class601_1->getSurface()->setVisible(true);
		_class601_2->getSurface()->setVisible(true);
		_class601_3->getSurface()->setVisible(true);
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_flag1) {
				setMessageList2(0x004B5868);
			} else {
				if (param.asInteger() == 1) {
					sendEntityMessage(_klayman, 0x1014, _class489);
					setMessageList2(0x004B5848);
				} else if (sendMessage(_class489, 0x480C, _klayman->getX() <= _class489->getX() ? 0 : 1) != 0) {
					sendEntityMessage(_klayman, 0x1014, _class489);
					setMessageList2(0x004B5830);
				} else {
					setMessageList2(0x004B5800);
				}
			}
		} else if (sender == _asTape) {
			if (_flag1) {
				setMessageList2(0x004B5868);
			} else if (_messageListStatus != 2) {
				sendEntityMessage(_klayman, 0x1014, _asTape);
				setMessageList2(0x004B58E0);
			}
		}
		break;
	}
	return 0;
}

Scene1317::Scene1317(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	SetMessageHandler(&Scene1317::handleMessage);
	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, 0x08982841, true, false));
	insertMouse433(0x08284011);
	showMouse(false);
	_smackerFileHash = 0;
	_smackerFlag1 = false;
}

void Scene1317::update() {
	if (_smackerFileHash) {
		_smackerPlayer->open(_smackerFileHash, _smackerFlag1);
		_smackerFileHash = 0;
	}
	Scene::update();
}

void Scene1317::upChooseKing() {
	if (!_klaymanBlinks && _klaymanBlinkCountdown != 0 && (--_klaymanBlinkCountdown == 0))
		_klaymanBlinks = true;
		
	if (!_klaymanBlinks && _smackerPlayer->getFrameNumber() + 1 >= 2) {
		_smackerPlayer->rewind();
	} else if (_klaymanBlinks && _smackerPlayer->getFrameNumber() + 1 >= 6) {
		_smackerPlayer->rewind();
		_klaymanBlinks = false;
		_klaymanBlinkCountdown = _vm->_rnd->getRandomNumber(30 - 1) + 15;
	}

	if (!_klaymanBlinks && _decisionCountdown != 0 && (--_decisionCountdown == 0))
		stNoDecisionYet();
			
	if (_smackerFileHash) {
		_smackerPlayer->open(_smackerFileHash, _smackerFlag1);
		_smackerFileHash = 0;
	}

	Scene::update();
	
}

uint32 Scene1317::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		stChooseKing();
		break;
	}
	return messageResult;
}
	
uint32 Scene1317::hmChooseKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x >= 21 && param.asPoint().y >= 24 &&
			param.asPoint().x <= 261 && param.asPoint().y <= 280) {
			stHoborgAsKing();
		} else if (param.asPoint().x >= 313 && param.asPoint().y >= 184 &&
			param.asPoint().x <= 399 && param.asPoint().y <= 379) {
			stKlaymanAsKing();
		} else if (param.asPoint().x >= 347 && param.asPoint().y >= 380 &&
			param.asPoint().x <= 418 && param.asPoint().y <= 474) {
			stKlaymanAsKing();
		}
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmNoDecisionYet(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		stChooseKing();
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmHoborgAsKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		stEndMovie();
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmKlaymanAsKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		sendMessage(_parentModule, 0x1009, 0);
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmEndMovie(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		sendMessage(_parentModule, 0x1009, 0);
		break;
	}
	return messageResult;
}

void Scene1317::stChooseKing() {
	showMouse(true);
	SetMessageHandler(&Scene1317::hmChooseKing);
	SetUpdateHandler(&Scene1317::upChooseKing);
	_smackerFileHash = 0x10982841;
	_smackerFlag1 = true;
	_decisionCountdown = 450;
	_klaymanBlinks = false;
	_klaymanBlinkCountdown = _vm->_rnd->getRandomNumber(30 - 1) + 15;
}

void Scene1317::stNoDecisionYet() {
	showMouse(false);
	SetMessageHandler(&Scene1317::hmNoDecisionYet);
	SetUpdateHandler(&Scene1317::update);
	_smackerFileHash = 0x20982841;
	_smackerFlag1 = false;
}

void Scene1317::stHoborgAsKing() {
	showMouse(false);
	SetMessageHandler(&Scene1317::hmHoborgAsKing);
	SetUpdateHandler(&Scene1317::update);
	_smackerFileHash = 0x40982841;
	_smackerFlag1 = false;
}

void Scene1317::stKlaymanAsKing() {
	showMouse(false);
	SetMessageHandler(&Scene1317::hmKlaymanAsKing);
	SetUpdateHandler(&Scene1317::update);
	_smackerFileHash = 0x80982841;
	_smackerFlag1 = false;
}

void Scene1317::stEndMovie() {
	showMouse(false);
	SetMessageHandler(&Scene1317::hmEndMovie);
	SetUpdateHandler(&Scene1317::update);
	_smackerFileHash = 0x40800711;
	_smackerFlag1 = false;
}

} // End of namespace Neverhood
