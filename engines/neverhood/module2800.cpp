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
	case 2:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		setGlobalVar(0x1860C990,1);//DEBUG
		if (getGlobalVar(0x1860C990))
			_childObject = new Scene2803b(_vm, this, which);
		else {
			// TODO _childObject = new Scene2803(_vm, this, which);
		}
		break;
	case 3:
		_childObject = new Scene2804(_vm, this, which);
		break;
	case 4:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 2);
		_childObject = new Scene2805(_vm, this, which);
		break;
	case 5:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2806(_vm, this, which);
		break;
	case 6:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2807(_vm, this, which);
		break;
	case 7:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2808(_vm, this, 0);
		break;
	case 8:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2809(_vm, this, which);
		break;
	case 10:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2808(_vm, this, 1);
		break;
	case 25:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		if (getGlobalVar(0x190A1D18))
			_childObject = new Class152(_vm, this, 0x01600204, 0x0020001E);
		else
			_childObject = new Class152(_vm, this, 0x08611204, 0x1120008E);
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
		case 2:
			if (_moduleResult == 1)
				createScene(3, 0);
			else if (_moduleResult == 2)
				createScene(5, 0);
			else if (_moduleResult == 3)
				createScene(6, 0);
			else if (_moduleResult == 4)
				createScene(9, 0);
			else if (_moduleResult == 5)
				createScene(25, 0);
			else 
				createScene(0, 1);
			break;
		case 3:
			createScene(2, 1);
			break;
		case 4:
			if (_moduleResult == 1) {
				leaveModule(1);
			} else {
				createScene(11, 1);
			}
			break;
		case 5:
			if (_moduleResult == 1) {
				createScene(7, 0);
			} else {
				createScene(2, 2);
			}
			break;
		case 6:
			createScene(2, 3);
			break;
		case 7:
			createScene(5, _moduleResult);
			break;
		case 8:
			if (_moduleResult == 1) {
				createScene(10, 0);
			} else {
				createScene(9, 4);
			}
			break;
		case 10:
			createScene(8, _moduleResult);
			break;
		case 25:
			createScene(2, 5);
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

Class488::Class488(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _fileHash1(fileHash1), _fileHash2(fileHash2),
	_flag1(false), _flag2(false), _soundResource(vm) {

	createSurface(1010, 640, 480); // TODO Use correct size	from the two hashes
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = y;
	sub4343C0();
}

uint32 Class488::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_flag2 && param.asInteger() == calcHash("ClickSwitch")) {
			sendMessage(_parentScene, 0x480F, 0);
			_soundResource.play(0x4E1CA4A0);
		}
		break;
	case 0x480F:
		sub434380();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

uint32 Class488::handleMessage434340(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void Class488::sub434380() {
	_flag2 = false;
	_flag1 = true;
	startAnimation(_fileHash2, 0, -1);
	SetMessageHandler(&Class488::handleMessage434340);
	NextState(&Class488::sub4343C0);
}

void Class488::sub4343C0() {
	_flag1 = false;
	startAnimation(_fileHash1, 0, -1);
	SetMessageHandler(&Class488::handleMessage);
}

void Class488::setFileHashes(uint32 fileHash1, uint32 fileHash2) {
	_fileHash1 = fileHash1;
	_fileHash2 = fileHash2;
	if (_flag1) {
		startAnimation(_fileHash2, _currFrameIndex, -1);
		_flag2 = true;
	} else {
		startAnimation(_fileHash1, 0, -1);
	}
}

Scene2803b::Scene2803b(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _palStatus(0) {

	static const uint32 kScene2803bFileHashes1[] = {
		0, 0x081000F1, 0x08100171, 0x08100271
	};

	static const uint32 kScene2803bFileHashes2[] = {
		0, 0x286800D4, 0x286806D4, 0x28680AD4
	};
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2803b::handleMessage);

	loadDataResource(0x81120132);
	insertMouse433(0x00A05290);

	insertSprite<Class488>(this, 0xAFAD591A, 0x276E321D, 578, 200);

	if (getGlobalVar(0x190A1D18)) {
		setBackground(0x412A423E);
		setPalette(0x412A423E);
		_palette->addBasePalette(0x412A423E, 0, 256, 0);
		addEntity(_palette);
		_sprite1 = insertStaticSprite(0x0C03AA23, 1100);
		_sprite2 = insertStaticSprite(0x24320220, 1100);
		_sprite3 = insertStaticSprite(0x1A032204, 1100);
		_sprite4 = insertStaticSprite(0x18032204, 1100);
		_sprite5 = insertStaticSprite(0x34422912, 1100);
		_sprite6 = insertStaticSprite(0x3C42022F, 1100);
		_sprite7 = insertStaticSprite(0x341A0237, 1100);

		if (getSubVar(0x0C601058, 0) == 0)
			insertStaticSprite(0x66121222, 100);
		else
			insertSprite<AnimatedSprite>(kScene2803bFileHashes1[getSubVar(0x0C601058, 0)], 100, 529, 326);

		if (getSubVar(0x0C601058, 1) == 3)
			insertStaticSprite(0x64330236, 100);

		if (getSubVar(0x0C601058, 2) == 3)
			insertStaticSprite(0x2E4A22A2, 100);

	} else {
		setBackground(0x29800A01);
		setPalette(0x29800A01);
		_palette->addBasePalette(0x29800A01, 0, 256, 0);
		addEntity(_palette);
		_sprite1 = insertStaticSprite(0x16202200, 1100);
		_sprite2 = insertStaticSprite(0xD0802EA0, 1100);
		_sprite3 = insertStaticSprite(0x780C2E30, 1100);
		_sprite4 = insertStaticSprite(0x700C2E30, 1100);
		_sprite5 = insertStaticSprite(0x102CE6E1, 900);
		_sprite6 = insertStaticSprite(0x108012C1, 1100);
		_sprite7 = insertStaticSprite(0x708072E0, 1100);
		insertStaticSprite(0x90582EA4, 100);

		setSubVar(0x0C601058, 0, 1);//DEBUG, FIXME crashes when not done?!
		if (getSubVar(0x0C601058, 0) == 0)
			insertStaticSprite(0x50C027A8, 100);
		else
			insertSprite<AnimatedSprite>(kScene2803bFileHashes2[getSubVar(0x0C601058, 0)], 100, 529, 326);

		if (getSubVar(0x0C601058, 1) == 3)
			insertStaticSprite(0xD48077A0, 100);

		if (getSubVar(0x0C601058, 2) == 3)
			insertStaticSprite(0x30022689, 100);

	}
	
	_sprite6->setVisible(false);
	_sprite7->setVisible(false);

	if (which < 0) {
		insertKlayman<KmScene2803b>(479, 435);
		sub460110();
		setMessageList(0x004B60D8);
	} else if (which == 3) {
		NPoint pt = _dataResource.getPoint(0x096520ED);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B6100);
		_klayman->setRepl(64, 0);
	} else if (which == 4) {
		NPoint pt = _dataResource.getPoint(0x20C6238D);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B60F8);
		_klayman->setRepl(64, 0);
	} else if (which == 5) {
		NPoint pt = _dataResource.getPoint(0x2146690D);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B6100);
		_klayman->setRepl(64, 0);
	} else if (which == 2) {
		NPoint pt = _dataResource.getPoint(0x104C03ED);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460110();
		setMessageList(0x004B6138);
	} else {
		insertKlayman<KmScene2803b>(135, 444);
		sub460110();
		setMessageList(0x004B60E0, false);
		_sprite6->setVisible(true);
		_sprite7->setVisible(true);
	}

}

uint32 Scene2803b::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xB4E4884C) {
			setMessageList(0x004B6180);
		} else if (param.asInteger() == 0xB1FDAB2E) {
			NPoint pt = _dataResource.getPoint(0x0D84A1AD);
			_klayman->setX(pt.x);
			_klayman->setY(pt.y);
			_klayman->processDelta();
			sub460110();
			_klayman->setClipRect(517, 401, 536, 480);
			setMessageList(0x004B6198);
		} else if (param.asInteger() == 0xB00C7C48) {
			setMessageList(0x004B6108);
		} else if (param.asInteger() == 0x61F64346) {
			setMessageList(0x004B6150);
		} else if (param.asInteger() == 0xAC69A28D) {
			setMessageList(0x004B6168);
		} else if (param.asInteger() == 0x00086212) {
			_klayman->setClipRect(0, 0, 560, 315);
			_klayman->setX(560);
			_klayman->setY(315);
			_klayman->processDelta();
			sub460090();
			setMessageList(0x004B61A0);
		} else if (param.asInteger() == 0x002CAA68) {
			setMessageList(0x004B61A8);
		}
		break;
	case 0x482A:
		if (_klayman->getX() < 200) {
			sub4601D0();
		} else if (_klayman->getX() < 500) {
			setSurfacePriority(_sprite5->getSurface(), 1100);
			sendMessage(_klayman, 0x482C, 0);
			sub4601B0();
		} else {
			_klayman->setClipRect(517, 401, 536, 480);
			sub4601B0();
		}
		break;
	case 0x482B:
		_sprite6->setVisible(false);
		_sprite7->setVisible(false);
		_klayman->setClipRect(0, 0, 640, 480);
		setSurfacePriority(_sprite5->getSurface(), 900);
		sendMessage(_klayman, 0x482C, 0x2086222D);
		break;
	}
	return 0;
}

void Scene2803b::update45FCB0() {
	if (_klayman->getX() < 388) {
		_klayman->setClipRect(_sprite3->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		sub460170();
	} else if (_klayman->getX() < 500) {
		_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
		sub460190();
	}
	Scene::update();
}

void Scene2803b::update45FD50() {
	if (_klayman->getX() > 194 && _klayman->getX() < 273)
		sub4601B0();
	else if (_klayman->getX() > 155 && _klayman->getX() < 300)
		sub460170();
	Scene::update();
}

void Scene2803b::sub460090() {
	SetUpdateHandler(&Scene2803b::update45FCB0);
	sendMessage(_klayman, 0x482C, 0x23C630D9);
	_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
	_klayman->setRepl(64, 0);
	_sprite1->setVisible(true);
}

void Scene2803b::sub460110() {
	SetUpdateHandler(&Scene2803b::update45FD50);
	sendMessage(_klayman, 0x482C, 0x2086222D);
	_klayman->setClipRect(0, 0, 640, 480);
	_klayman->clearRepl();
	_sprite1->setVisible(false);
}

void Scene2803b::sub460170() {
	if (_palStatus != 0) {
		_palStatus = 0;
		sub4601F0(false);
	}
}

void Scene2803b::sub460190() {
	if (_palStatus != 1) {
		_palStatus = 1;
		sub4601F0(false);
	}
}

void Scene2803b::sub4601B0() {
	if (_palStatus != 2) {
		_palStatus = 2;
		sub4601F0(false);
	}
}

void Scene2803b::sub4601D0() {
	if (_palStatus != 3) {
		_palStatus = 3;
		sub4601F0(true);
	}
}

void Scene2803b::sub4601F0(bool flag) {
	if (getGlobalVar(0x190A1D18)) {
		switch (_palStatus) {
		case 1:
			_palette->addBasePalette(0x0A938204, 0, 64, 0);
			break;
		case 2:
			_palette->addBasePalette(0xB103B604, 0, 64, 0);
			break;
		case 3:
			// TODO _palette->sub_47BFB0(0, 64);
			break;
		default:
			_palette->addBasePalette(0x412A423E, 0, 64, 0);
			break;
		}
	} else {
		switch (_palStatus) {
		case 2:
			_palette->addBasePalette(0x0263D144, 0, 64, 0);
			break;
		case 3:
			// TODO _palette->sub_47BFB0(0, 64);
			break;
		default:
			_palette->addBasePalette(0x29800A01, 0, 64, 0);
			break;
		}
	}
	if (flag) {
		_palette->startFadeToPalette(0);
	} else {
		_palette->startFadeToPalette(12);
	}
}

SsScene2804RedButton::SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene),
	_soundResource(vm) {
	
	if (getGlobalVar(0x190A1D18))
		_spriteResource.load2(0x51A10202);
	else
		_spriteResource.load2(0x11814A21);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&SsScene2804RedButton::update);
	SetMessageHandler(&SsScene2804RedButton::handleMessage);
	_soundResource.load(0x44241240);
}

void SsScene2804RedButton::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804RedButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			_soundResource.play();
			setVisible(true);
			_countdown = 4;
			sendMessage(_parentScene, 0x2000, 0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804LightCoil::SsScene2804LightCoil(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(0x8889B008);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	SetMessageHandler(&SsScene2804LightCoil::handleMessage);
}

uint32 SsScene2804LightCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		setVisible(true);
		StaticSprite::update();
		messageResult = 1;
		break;
	case 0x2003:
		setVisible(false);
		StaticSprite::update();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804LightTarget::SsScene2804LightTarget(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(0x06092132);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	SetMessageHandler(&SsScene2804LightTarget::handleMessage);
}

uint32 SsScene2804LightTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		StaticSprite::update();
		messageResult = 1;
		break;
	case 0x2005:
		setVisible(false);
		StaticSprite::update();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804Flash::SsScene2804Flash(NeverhoodEngine *vm)
	: StaticSprite(vm, 900), _soundResource(vm) {
	
	_spriteResource.load2(0x211003A0);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	_soundResource.load(0xCB36BA54);
}

void SsScene2804Flash::show() {
	setVisible(true);
	StaticSprite::update();
	_soundResource.play();
}

SsScene2804BeamCoilBody::SsScene2804BeamCoilBody(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(0x9A816000);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
}

AsScene2804CrystalWaves::AsScene2804CrystalWaves(NeverhoodEngine *vm, uint crystalIndex)
	: AnimatedSprite(vm, 1100), _crystalIndex(crystalIndex) {

	static const NPoint kAsScene2804CrystalWavesPoints[] = {
		{323, 245},
		{387, 76},
		{454, 260},
		{527, 70}
	};

	_x = kAsScene2804CrystalWavesPoints[crystalIndex].x;
	_y = kAsScene2804CrystalWavesPoints[crystalIndex].y;
	createSurface1(0x840C41F0, 1200);
	if (crystalIndex & 1)
		setDoDeltaX(1);
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Sprite::handleMessage);
}

void AsScene2804CrystalWaves::show() {
	setVisible(true);
	startAnimation(0x840C41F0, 0, -1);
}

void AsScene2804CrystalWaves::hide() {
	setVisible(false);
	stopAnimation();
}

static const int16 kAsScene2804CrystalFrameNums[] = {
	0, 6, 2, 8, 1, 10, 0, 0
};

static const uint32 kAsScene2804CrystalFileHashes[] = {
	0x000540B0,
	0x001280D0,
	0x003D0010,
	0x00620190,
	0x00DC0290
};

AsScene2804Crystal::AsScene2804Crystal(NeverhoodEngine *vm, AsScene2804CrystalWaves *asCrystalWaves, uint crystalIndex)
	: AnimatedSprite(vm, 1100), _asCrystalWaves(asCrystalWaves), _crystalIndex(crystalIndex),
	_isShowing(false), _soundResource(vm) {

	static const NPoint kAsScene2804CrystalPoints[] = {
		{204, 196},
		{272, 316},
		{334, 206},
		{410, 334},
		{470, 180}
	};

	_colorNum = (int16)getSubVar(0xE11A1929, crystalIndex);
	_isLightOn = getGlobalVar(0x190A1D18) != 0;
	if (_isLightOn) {
		_x = kAsScene2804CrystalPoints[crystalIndex].x;
		_y = kAsScene2804CrystalPoints[crystalIndex].y;
		createSurface1(0x108DFB12, 1200);
		startAnimation(0x108DFB12, kAsScene2804CrystalFrameNums[_colorNum], -1);
		_needRefresh = true;
		_newStickFrameIndex = kAsScene2804CrystalFrameNums[_colorNum];
	} else {
		_x = 320;
		_y = 240;
		createSurface1(kAsScene2804CrystalFileHashes[crystalIndex], 1200);
		startAnimation(kAsScene2804CrystalFileHashes[crystalIndex], _colorNum, -1);
		setVisible(false);
		_needRefresh = true;
		_newStickFrameIndex = _colorNum;
	}
	_soundResource.load(0x725294D4);
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene2804Crystal::show() {
	if (!_isLightOn) {
		setVisible(true);
		_isShowing = true;
		if (_asCrystalWaves)
			_asCrystalWaves->show();
		_soundResource.play();
	}
}

void AsScene2804Crystal::hide() {
	if (!_isLightOn) {
		setVisible(false);
		_isShowing = false;
		if (_asCrystalWaves)
			_asCrystalWaves->hide();
	}
}

void AsScene2804Crystal::activate() {
	if (!_isShowing) {
		int16 frameNum = kAsScene2804CrystalFrameNums[_colorNum];
		_colorNum++;
		if (_colorNum >= 6)
			_colorNum = 0;
		if (_isLightOn) {
			startAnimation(0x108DFB12, frameNum, kAsScene2804CrystalFrameNums[_colorNum]);
			_playBackwards = kAsScene2804CrystalFrameNums[_colorNum] < _colorNum;
			_newStickFrameIndex = kAsScene2804CrystalFrameNums[_colorNum];
		} else {
			startAnimation(kAsScene2804CrystalFileHashes[_crystalIndex], _colorNum, -1);
			_newStickFrameIndex = _colorNum;
		}
		setSubVar(0xE11A1929, _crystalIndex, _colorNum);
	}
}

SsScene2804CrystalButton::SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene), _asCrystal(asCrystal),
	_crystalIndex(crystalIndex), _soundResource(vm) {

	static const uint32 kSsScene2804CrystalButtonFileHashes1[] = {
		0x911101B0,
		0x22226001,
		0x4444A362,
		0x888925A4,
		0x11122829
	};

	static const uint32 kSsScene2804CrystalButtonFileHashes2[] = {
		0xB500A1A0,
		0x6A012021,
		0xD4022322,
		0xA8042525,
		0x5008292B
	};
	
	if (getGlobalVar(0x190A1D18))
		_spriteResource.load2(kSsScene2804CrystalButtonFileHashes1[crystalIndex]);
	else
		_spriteResource.load2(kSsScene2804CrystalButtonFileHashes2[crystalIndex]);

	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	setVisible(false);
	_soundResource.load(0x44045140);
	_needRefresh = true;
	SetUpdateHandler(&SsScene2804CrystalButton::update);
	SetMessageHandler(&SsScene2804CrystalButton::handleMessage);
}

void SsScene2804CrystalButton::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804CrystalButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			_soundResource.play();
			setVisible(true);
			_countdown = 4;
			_asCrystal->activate();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene2804BeamCoil::AsScene2804BeamCoil(NeverhoodEngine *vm, Scene *parentScene, SsScene2804BeamCoilBody *ssBeamCoilBody)
	: AnimatedSprite(vm, 1400), _parentScene(parentScene), _ssBeamCoilBody(ssBeamCoilBody),
	_countdown(0), _soundResource(vm) {
	
	createSurface1(0x00494891, 1000);
	_x = 125;
	_y = 184;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	_soundResource.load(0x6352F051);
	// TODO Sound1ChList_addSoundResource(0xC5EA0B28, 0xEF56B094, true);
	SetUpdateHandler(&AsScene2804BeamCoil::update);
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
}

void AsScene2804BeamCoil::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		sendMessage(_parentScene, 0x2001, 0);
	}
}

uint32 AsScene2804BeamCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		show();
		_countdown = 92;
		messageResult = 1;
		break;
	case 0x2003:
		hide();
		messageResult = 1;
		break;
	}
	return messageResult;
}
	
void AsScene2804BeamCoil::show() {
	_ssBeamCoilBody->setVisible(true);
	// TODO _ssBeamCoilBody->update(); -> show()
	setVisible(true);
	startAnimation(0x00494891, 0, -1);
	_soundResource.play();
	SetMessageHandler(&AsScene2804BeamCoil::hmBeaming);
	NextState(&AsScene2804BeamCoil::stBeaming);
}

void AsScene2804BeamCoil::hide() {
	stopAnimation();
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
	setVisible(false);
	_ssBeamCoilBody->setVisible(false);
	// TODO _ssBeamCoilBody->update(); -> hide()
	// TODO Sound1ChList_stop(0xEF56B094);
}

void AsScene2804BeamCoil::stBeaming() {
	startAnimation(0x00494891, 93, -1);
	NextState(&AsScene2804BeamCoil::stBeaming);
	// TODO Sound1ChList_playLooping(0xEF56B094);
}

uint32 AsScene2804BeamCoil::hmBeaming(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

AsScene2804BeamTarget::AsScene2804BeamTarget(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {
	
	createSurface1(0x03842000, 1000);
	_x = 475;
	_y = 278;
	setVisible(false);
	_needRefresh = true;
	updatePosition();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2804BeamTarget::handleMessage);
}

uint32 AsScene2804BeamTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		startAnimation(0x03842000, 0, -1);
		messageResult = 1;
		break;
	case 0x2005:
		setVisible(false);
		stopAnimation();
		messageResult = 1;
		break;
	}
	return messageResult;
}

Scene2804::Scene2804(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _countdown1(0), _countdown2(0), _countdown3(0),
	_beamStatus(0), _isSolved(false), _isWorking(false) {

	initCrystalColors();

	_surfaceFlag = true;
	SetMessageHandler(&Scene2804::handleMessage);
	SetUpdateHandler(&Scene2804::update);

	//setGlobalVar(0x190A1D18, 1); // DEBUG Set lights on

	if (getGlobalVar(0x190A1D18)) {
		setBackground(0xA1D03005);
		setPalette(0xA1D03005);
		addEntity(_palette);
		insertMouse435(0x03001A15, 20, 620);
		_asCoil = insertSprite<SsScene2804LightCoil>();
		_asTarget = insertSprite<SsScene2804LightTarget>();
	} else {
		SsScene2804BeamCoilBody *ssBeamCoilBody;
		setBackground(0x01C01414);
		setPalette(0x01C01414);
		addEntity(_palette);
		insertMouse435(0x01410014, 20, 620);
		ssBeamCoilBody = insertSprite<SsScene2804BeamCoilBody>();
		_asCoil = insertSprite<AsScene2804BeamCoil>(this, ssBeamCoilBody);
		_asTarget = insertSprite<AsScene2804BeamTarget>();
		_ssFlash = insertSprite<SsScene2804Flash>();
	}
	
	_ssRedButton = insertSprite<SsScene2804RedButton>(this);
	_vm->_collisionMan->addSprite(_ssRedButton);

	for (uint crystalIndex = 0; crystalIndex < 5; crystalIndex++) {
		AsScene2804CrystalWaves *asCrystalWaves = NULL;
		if (crystalIndex < 4 && getGlobalVar(0x190A1D18) == 0)
			asCrystalWaves = insertSprite<AsScene2804CrystalWaves>(crystalIndex);
		_asCrystals[crystalIndex] = insertSprite<AsScene2804Crystal>(asCrystalWaves, crystalIndex);
		_ssCrystalButtons[crystalIndex] = insertSprite<SsScene2804CrystalButton>(this, _asCrystals[crystalIndex], crystalIndex);
		_vm->_collisionMan->addSprite(_ssCrystalButtons[crystalIndex]);
	}

}

uint32 Scene2804::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO DEBUG
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		// TODO DEBUG
		break;
	case 0x2000:
		_isWorking = true;
		sendMessage(_asCoil, 0x2002, 0);
		if (getGlobalVar(0x190A1D18)) {
			sendMessage(_asTarget, 0x2004, 0);
			_countdown2 = 48;
		}
		break;
	case 0x2001:
		_countdown3 = 2;
		_isSolved = true;
		_beamStatus = 0;
		for (uint index = 0; index < 5; index++)
			if (_asCrystals[index]->getColorNum() != (int16)getSubVar(0xD4B2089C, index))
				_isSolved = false;
		_countdown2 = 48;
		break;
	}
	return 0;
}

void Scene2804::update() {

	Scene::update();
	
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		leaveScene(0);
	}

	if (_countdown2 != 0 && (--_countdown2) == 0) {
		_isWorking = false;
		sendMessage(_asCoil, 0x2003, 0);
		sendMessage(_asTarget, 0x2005, 0);
		for (uint index = 0; index < 5; index++)
			_asCrystals[index]->hide();
	}

	if (_countdown3 != 0 && (--_countdown3) == 0) {
		if (_beamStatus == 5) {
			sendMessage(_asTarget, 0x2004, 0);
			if (_isSolved) {
				_palette->fillBaseWhite(0, 256);
				_palette->startFadeToPalette(18);
				setGlobalVar(0x1860C990, 1);
				_countdown1 = 48;
			}
		} else if (_beamStatus == 6) {
			if (_isSolved)
				_ssFlash->show();
		} else {
			_asCrystals[_beamStatus]->show();
		}
		_beamStatus++;
		if (_beamStatus < 6)
			_countdown3 = 2;
		else if (_beamStatus < 7)
			_countdown3 = 4;
	}

}

void Scene2804::initCrystalColors() {
	// TODO Maybe move this into the GameModule so all puzzle init code is together
	if (getGlobalVar(0xDE2EC914) == 0) {
		TextResource textResource(_vm);
		const char *textStart, *textEnd;
		textResource.load(0x46691611);
		textStart = textResource.getString(0, textEnd);
		for (uint index = 0; index < 5; index++) {
			char colorLetter = (byte)textStart[index];
			byte correctColorNum = 0, misalignedColorNum;
			switch (colorLetter) {
			case 'B':
				correctColorNum = 4;
				break;
			case 'G':
				correctColorNum = 3;
				break;
			case 'O':
				correctColorNum = 1;
				break;
			case 'R':
				correctColorNum = 0;
				break;
			case 'V':
				correctColorNum = 5;
				break;
			case 'Y':
				correctColorNum = 2;
				break;
			}
			do {
				misalignedColorNum = _vm->_rnd->getRandomNumber(6 - 1);
			} while (misalignedColorNum == correctColorNum);
			setSubVar(0xD4B2089C, index, correctColorNum);
			setSubVar(0xE11A1929, index, misalignedColorNum);
		}
		setGlobalVar(0xDE2EC914, 1);
	}
}

Scene2805::Scene2805(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2805::handleMessage);

	setBackground(0x08021E04);
	setPalette(0x08021E04);
	_palette->addPalette(0x8A6B1F91, 0, 65, 0);
	insertMouse433(0x21E00088);

	_sprite1 = insertStaticSprite(0x008261E7, 1100);
	_sprite2 = insertStaticSprite(0x020CE421, 1100);

	if (which < 0) {
		insertKlayman<KmScene2805>(380, 338);
		setMessageList(0x004AE1C8);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE1D0, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE288, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE1E0, false);
		sendMessage(this, 0x2000, 1);
	} else {
		insertKlayman<KmScene2805>(340, 338);
		setMessageList(0x004AE1C0);
		sendMessage(this, 0x2000, 0);
	}

	_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);

}

uint32 Scene2805::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004AE318);
			_klayman->setKlaymanIdleTable3();
		} else {
			setRectList(0x004AE308);
			_klayman->setKlaymanIdleTable1();
		}
		break;
	}
	return 0;
}

AsScene2806Spew::AsScene2806Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {
	
	createSurface1(0x04211490, 1200);
	_x = 378;
	_y = 423;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2806Spew::handleMessage);
	setDoDeltaX(1);
	setVisible(false);
}

uint32 AsScene2806Spew::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		_soundResource.play(0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}
	
Scene2806::Scene2806(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	Sprite *tempSprite;

	_surfaceFlag = true;
	SetMessageHandler(&Scene2806::handleMessage);
	SetUpdateHandler(&Scene2806::update);
	
	loadDataResource(0x98182003);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x3606A422);

	insertMouse433(0x22114C13);	
	setBackground(0xC1B22110);
	setPalette(0xC1B22110);
	
	_sprite1 = insertStaticSprite(0xA21F82CB, 1100);
	_clipRects[0].x1 = _sprite1->getDrawRect().x;
	_clipRects[0].y1 = _sprite1->getDrawRect().y;
	_clipRects[0].x2 = _sprite1->getDrawRect().x2();
	_clipRects[0].y2 = _sprite1->getDrawRect().y2();

	_sprite2 = insertStaticSprite(0x92035301, 1100);
	_clipRects[1].y2 = _sprite2->getDrawRect().y2();

	_sprite3 = insertStaticSprite(0x3182220E, 1100);

	_sprite4 = insertStaticSprite(0x72090342, 1100);
	_clipRects[1].x1 = _sprite4->getDrawRect().x;
	_clipRects[1].y1 = _sprite4->getDrawRect().y;
	
	_fieldEC = true;

	tempSprite = insertStaticSprite(0xD2012C02, 1100);
	_clipRects[2].x1 = tempSprite->getDrawRect().x;
	_clipRects[2].y2 = tempSprite->getDrawRect().y2();
	_clipRects[3].y1 = tempSprite->getDrawRect().y2();
	_clipRects[1].x2 = tempSprite->getDrawRect().x2();

	tempSprite = insertStaticSprite(0x72875F42, 1100);
	_clipRects[3].x1 = tempSprite->getDrawRect().x;

	insertStaticSprite(0x0201410A, 1100);
	insertStaticSprite(0x72875F42, 1100);

	_asSpew = insertSprite<AsScene2806Spew>();

	_clipRects[2].y1 = 0;
	_clipRects[3].y2 = 480;
	_clipRects[2].x2 = 640;
	_clipRects[3].x2 = 640;

	if (which < 0) {
		insertKlayman<KmScene2806>(441, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 1) {
		insertKlayman<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 2) {
		insertKlayman<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF0C8, false);
	} else if (which == 3) {
		insertKlayman<KmScene2806>(378, 423, true, _clipRects, 4);
		setMessageList(0x004AF0A0, false);
		setGlobalVar(0x1860C990, 0);
	} else {
		insertKlayman<KmScene2806>(670, 423, false, _clipRects, 4);
		setMessageList(0x004AF090);
	}

	_pointIndex = -1;
	findClosestPoint();

}

uint32 Scene2806::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x44262B12) {
			setMessageList(0x004AF0E0);
		}
		break;
	case 0x2000:
		sendMessage(_asSpew, 0x2000, 0);
		break;
	}
	return 0;
}

void Scene2806::update() {
	Scene::update();
	findClosestPoint();
}

void Scene2806::findClosestPoint() {

	static const uint32 kScene2806PaletteFileHashes[] = {
		0x48052508,
		0x01139404,
		0x01138C04,
		0x01138004,
		0x01138604,
		0x086B8890
	};

	int16 x = MIN<int16>(_klayman->getX(), 639);
	int index = 1;
	
	while (index < (int)_pointList->size() && (*_pointList)[index].x < x)
		++index;
	--index;

	if (_pointIndex != index) {
		_pointIndex = index;
		_palette->addPalette(kScene2806PaletteFileHashes[index], 0, 64, 0);
	}
	
}

Scene2807::Scene2807(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2807::handleMessage);

	if (getSubVar(0x0C601058, 0) == 1) {
		insertStaticSprite(0x103021E2, 300);
	} else if (getSubVar(0x0C601058, 0) == 2) {
		insertStaticSprite(0x103022E2, 300);
	} else if (getSubVar(0x0C601058, 0) == 3) {
		insertStaticSprite(0x103024E2, 300);
	}

	if (getSubVar(0x0C601058, 1) == 1) {
		insertStaticSprite(0x4800A52A, 200);
	} else if (getSubVar(0x0C601058, 1) == 2) {
		insertStaticSprite(0x4800A62A, 200);
	} else if (getSubVar(0x0C601058, 1) == 3) {
		insertStaticSprite(0x4800A02A, 200);
	}

	if (getSubVar(0x0C601058, 2) == 1) {
		insertStaticSprite(0x31203430, 100);
	} else if (getSubVar(0x0C601058, 2) == 2) {
		insertStaticSprite(0x31203400, 100);
	} else if (getSubVar(0x0C601058, 2) == 3) {
		insertStaticSprite(0x31203460, 100);
	}

	_surfaceFlag = true;
	setBackground(0x3E049A95);
	setPalette(0x3E049A95);
	insertMouse435(0x49A913E8, 20, 620);

}

uint32 Scene2807::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	}
	return 0;
}

static const uint32 kScene2808FileHashes1[] = {
	0x90B0392,
	0x90B0192
};

static const uint32 kScene2808FileHashes2[] = {
	0xB0396098,
	0xB0196098
};

static const uint32 kClass428FileHashes[] = {
	0x140022CA,
	0x4C30A602,
	0xB1633402,
	0x12982135,
	0x0540B728,
	0x002A81E3,
	0x08982841,
	0x10982841,
	0x20982841,
	0x40982841,
	0x80982841,
	0x40800711
};

static const int kClass428Countdowns1[] = {
	18, 16, 10, 0
};

static const int kClass428Countdowns2[] = {
	9, 9, 8, 8, 5, 5, 0, 0
};

static const uint32 kClass490FileHashes[] = {
	0x08100071,
	0x24084215,
	0x18980A10
};

static const int16 kClass490FrameIndices1[] = {
	0, 8, 15, 19
};

static const int16 kClass490FrameIndices2[] = {
	0, 4, 8, 11, 15, 17, 19, 0
};

SsScene2808Dispenser::SsScene2808Dispenser(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum, int testTubeIndex)
	: StaticSprite(vm, 900), _parentScene(parentScene), _countdown(0), _testTubeSetNum(testTubeSetNum),
	_testTubeIndex(testTubeIndex) {
	
	_spriteResource.load2(kClass428FileHashes[testTubeSetNum * 3 + testTubeIndex]);
	createSurface(1500, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	SetUpdateHandler(&SsScene2808Dispenser::update);
	SetMessageHandler(&SsScene2808Dispenser::handleMessage);
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
}

void SsScene2808Dispenser::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}
	
uint32 SsScene2808Dispenser::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2000, _testTubeIndex);
		messageResult = 1;
		break;
	}
	return messageResult;
}

void SsScene2808Dispenser::startCountdown(int index) {
	setVisible(true);
	StaticSprite::update();
	if (_testTubeSetNum == 0) {
		_countdown = kClass428Countdowns1[index];
	} else {
		_countdown = kClass428Countdowns2[index];
	}
}

AsScene2808TestTube::AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser)
	: AnimatedSprite(vm, 1100), _testTubeSetNum(testTubeSetNum), _testTubeIndex(testTubeIndex), _ssDispenser(ssDispenser),
	_soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_soundResource4(vm), _soundResource5(vm), _soundResource6(vm),
	_soundResource7(vm), _soundResource8(vm), _soundResource9(vm), _fillLevel(0) {

	if (testTubeSetNum == 0) {
		_x = 504;
		_y = 278;
	} else {
		setDoDeltaX(1);
		_x = 136;
		_y = 278;
	}

	createSurface1(kClass490FileHashes[testTubeIndex], 1100);

	if (testTubeSetNum == 0) {
		_soundResource1.load(0x30809E2D);
		_soundResource2.load(0x72811E2D);
		_soundResource3.load(0x78B01625);
	} else {
		_soundResource4.load(0x70A41E0C);
		_soundResource5.load(0x50205E2D);
		_soundResource6.load(0xF8621E2D);
		_soundResource7.load(0xF1A03C2D);
		_soundResource8.load(0x70A43D2D);
		_soundResource9.load(0xF0601E2D);
	}
	
	startAnimation(kClass490FileHashes[testTubeIndex], 0, -1);
	_newStickFrameIndex = 0;
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2808TestTube::handleMessage);
	
	if (_fillLevel == 0)
		setVisible(false);
	
}

uint32 AsScene2808TestTube::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		fill();
		messageResult = 1;
		break;
	}
	return messageResult;
}

void AsScene2808TestTube::fill() {


	if ((int)_fillLevel >= _testTubeSetNum * 3 + 3)
		return;
		
	if (_testTubeSetNum == 0) {
		switch (_fillLevel) {
		case 0:
			_soundResource1.play();
			break;
		case 1:
			_soundResource2.play();
			break;
		case 2:
			_soundResource3.play();
			break;
		}
		setVisible(true);
		startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], kClass490FrameIndices1[_fillLevel + 1]);
		_newStickFrameIndex = kClass490FrameIndices1[_fillLevel + 1];
	} else {
		switch (_fillLevel) {
		case 0:
			_soundResource4.play();
			break;
		case 1:
			_soundResource5.play();
			break;
		case 2:
			_soundResource6.play();
			break;
		case 3:
			_soundResource7.play();
			break;
		case 4:
			_soundResource8.play();
			break;
		case 5:
			_soundResource9.play();
			break;
		}
		setVisible(true);
		startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], kClass490FrameIndices2[_fillLevel + 1]);
		_newStickFrameIndex = kClass490FrameIndices2[_fillLevel + 1];
	}
	_ssDispenser->startCountdown(_fillLevel);
	_fillLevel++;
}

void AsScene2808TestTube::flush() {
	if (_fillLevel != 0) {
		if (_testTubeSetNum == 0) {
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], -1);
		} else {
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], -1);
		}
		_newStickFrameIndex = 0;
		_playBackwards = true;
		setVisible(true);
	}
}

AsScene2808Handle::AsScene2808Handle(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum)
	: AnimatedSprite(vm, 1300), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum), _isActivated(false),
	_soundResource(vm) {
	
	_soundResource.load(0xE18D1F30);
	_x = 320;
	_y = 240;
	if (_testTubeSetNum == 1)
		setDoDeltaX(1);
	createSurface1(0x040900D0, 1300);
	startAnimation(0x040900D0, 0, -1);
	_needRefresh = true;
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2808Handle::handleMessage);
	AnimatedSprite::updatePosition();
}

uint32 AsScene2808Handle::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isActivated) {
			sendMessage(_parentScene, 0x2001, 0);
			_soundResource.play();
			activate();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

uint32 AsScene2808Handle::hmActivating(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2808Handle::activate() {
	startAnimation(0x040900D0, 0, -1);
	SetMessageHandler(&AsScene2808Handle::hmActivating);
	NextState(&AsScene2808Handle::stActivated);
	_isActivated = true;
	_newStickFrameIndex = -1;
}

void AsScene2808Handle::stActivated() {
	stopAnimation();
	sendMessage(_parentScene, 0x2002, 0);
}

AsScene2808Flow::AsScene2808Flow(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum),
	_soundResource(vm) {

	if (testTubeSetNum == 0) {
		_x = 312;
		_y = 444;
	} else {
		_x = 328;
		_y = 444;
	}
	createSurface1(0xB8414818, 1200);
	startAnimation(0xB8414818, 0, -1);
	setVisible(false);
	_newStickFrameIndex = 0;
	_needRefresh = true;
	_soundResource.load(0x6389B652);
	SetUpdateHandler(&AnimatedSprite::update);
	AnimatedSprite::updatePosition();
}
	
uint32 AsScene2808Flow::hmFlowing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2808Flow::start() {
	startAnimation(0xB8414818, 0, -1);
	setVisible(true);
	SetMessageHandler(&AsScene2808Flow::hmFlowing);
	NextState(&AsScene2808Flow::stKeepFlowing);
	_soundResource.play();
}

void AsScene2808Flow::stKeepFlowing() {
	startAnimation(0xB8414818, 1, -1);
	NextState(&AsScene2808Flow::stKeepFlowing);
}

AsScene2808LightEffect::AsScene2808LightEffect(NeverhoodEngine *vm, int testTubeSetNum)
	: AnimatedSprite(vm, 800), _countdown(1) {
	
	_x = 320;
	_y = 240;
	if (testTubeSetNum == 1)
		setDoDeltaX(1);
	createSurface1(0x804C2404, 800);
	SetUpdateHandler(&AsScene2808LightEffect::update);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
}

void AsScene2808LightEffect::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		int16 frameIndex = _vm->_rnd->getRandomNumber(3 - 1);
		startAnimation(0x804C2404, frameIndex, frameIndex);
		updateAnim();
		updatePosition();
		_countdown = _vm->_rnd->getRandomNumber(3 - 1) + 1;
	}
}

Scene2808::Scene2808(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _countdown(0), _testTubeSetNum(which), _leaveResult(0), _isFlowing(false) {

	Sprite *tempSprite;

	if (which == 0) {
		_vm->gameModule()->initScene2808Vars1();
	} else {
		_vm->gameModule()->initScene2808Vars2();
	}
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2808::handleMessage);
	SetUpdateHandler(&Scene2808::update);

	setBackground(kScene2808FileHashes1[which]);
	setPalette(kScene2808FileHashes1[which]);

	tempSprite = insertSprite<AsScene2808Handle>(this, which);
	_vm->_collisionMan->addSprite(tempSprite);

	_asFlow = insertSprite<AsScene2808Flow>(this, which);
	insertSprite<AsScene2808LightEffect>(which);

	for (int testTubeIndex = 0; testTubeIndex < 3; testTubeIndex++) {
		SsScene2808Dispenser *ssDispenser = insertSprite<SsScene2808Dispenser>(this, which, testTubeIndex);
		_vm->_collisionMan->addSprite(ssDispenser);
		_asTestTubes[testTubeIndex] = insertSprite<AsScene2808TestTube>(which, testTubeIndex, ssDispenser);
		_vm->_collisionMan->addSprite(_asTestTubes[testTubeIndex]);
	}
	
	insertMouse433(kScene2808FileHashes2[which]);

}

uint32 Scene2808::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO DEBUG Stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !isAnyTestTubeFilled()) {
			leaveScene(1);
		}
		break;
	case 0x000D:
		// TODO DEBUG Stuff
		break;
	case 0x2000:
		if (!_isFlowing)
			_asTestTubes[param.asInteger()]->fill();
		break;
	case 0x2001:
		_isFlowing = true;
		break;
	case 0x2002:
		if (isAnyTestTubeFilled()) {
			_leaveResult = 3;
			if (!isMixtureGood())
				_leaveResult = 2;
			_asFlow->start();
			for (int i = 0; i < 3; i++)
				_asTestTubes[i]->flush();
			_mouseCursor->setVisible(false);
			_countdown = 16;
		} else {
			leaveScene(1);
		}
		break;
	}
	return 0;
}

void Scene2808::update() {

	// DEBUG: Show correct values
	debug("---------------");
	debug("%03d %03d %03d", getSubVar(0x0C601058, 0), getSubVar(0x0C601058, 1), getSubVar(0x0C601058, 2));
	debug("%03d %03d %03d", _asTestTubes[0]->getFillLevel(), _asTestTubes[1]->getFillLevel(), _asTestTubes[2]->getFillLevel());
	
	Scene::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		leaveScene(_leaveResult);
	}
}

bool Scene2808::isMixtureGood() {
	if (_testTubeSetNum == 0) {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(0x0C601058, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(0x0C601058, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(0x0C601058, 2);
	} else {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(0x40005834, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(0x40005834, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(0x40005834, 2);
	}
}

bool Scene2808::isAnyTestTubeFilled() {
	return
		_asTestTubes[0]->getFillLevel() > 0 ||
		_asTestTubes[1]->getFillLevel() > 0 ||
		_asTestTubes[2]->getFillLevel() > 0;
}

AsScene2809Spew::AsScene2809Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2809Spew::handleMessage);
	createSurface1(0x04211490, 1200);
	_x = 262;
	_y = 423;
	setDoDeltaX(0);
	setVisible(false);
}

uint32 AsScene2809Spew::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		_soundResource.play(0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

Scene2809::Scene2809(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	Sprite *tempSprite;
		
	_surfaceFlag = true;
	SetMessageHandler(&Scene2809::handleMessage);
	SetUpdateHandler(&Scene2809::update);
	
	loadDataResource(0x1830009A);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x064A310E);

	setBackground(0xB22116C5);
	setPalette(0xB22116C5);
	insertMouse433(0x116C1B2A);

	_sprite1 = insertStaticSprite(0x1FA2EB82, 1100);

	_clipRects[0].x1 = _sprite1->getDrawRect().x;
	_clipRects[0].y1 = _sprite1->getDrawRect().y;
	_clipRects[0].x2 = _sprite1->getDrawRect().x2();
	_clipRects[0].y2 = _sprite1->getDrawRect().y2();

	_sprite2 = insertStaticSprite(0x037321B2, 1100);
	_clipRects[1].y2 = _sprite2->getDrawRect().y2();

	_sprite3 = insertStaticSprite(0x82022E11, 1100);

	_sprite4 = insertStaticSprite(0x09236252, 1100);
	_clipRects[1].x2 = _sprite4->getDrawRect().x2();
	_clipRects[1].y1 = _sprite4->getDrawRect().y;

	tempSprite = insertStaticSprite(0x010C22F2, 1100);
	_clipRects[2].x2 = tempSprite->getDrawRect().x2();
	_clipRects[2].y2 = tempSprite->getDrawRect().y2();
	_clipRects[3].y1 = tempSprite->getDrawRect().y2();
	_clipRects[1].x1 = tempSprite->getDrawRect().x2();

	tempSprite = insertStaticSprite(0x877F6252, 1100);
	_clipRects[3].x2 = tempSprite->getDrawRect().x2();
	
	insertStaticSprite(0x01612A22, 1100);
	insertStaticSprite(0x877F6252, 1100);

	_asSpew = insertSprite<AsScene2809Spew>();
	_clipRects[2].y1 = 0;
	_clipRects[3].y2 = 480;
	_clipRects[2].x1 = 0;
	_clipRects[3].x1 = 0;

	if (which < 0) {
		insertKlayman<KmScene2809>(226, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 1) {
		insertKlayman<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 2) {
		insertKlayman<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5BD0);
	} else if (which == 3) {
		insertKlayman<KmScene2809>(262, 423, true, _clipRects, 4);
		setMessageList(0x004B5BA8, false);
		setGlobalVar(0x1860C990, 0);
	} else {
		insertKlayman<KmScene2809>(-30, 423, false, _clipRects, 4);
		setMessageList(0x004B5B88);
	}

	_pointIndex = -1;
	findClosestPoint();

}

void Scene2809::update() {
	Scene::update();
	findClosestPoint();
}

uint32 Scene2809::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x160DA937) {
			setMessageList(0x004B5B98);
		}
		break;
	case 0x2000:
		sendMessage(_asSpew, 0x2000, 0);
		break;
	}
	return 0;
}

void Scene2809::findClosestPoint() {

	static const uint32 kScene2809PaletteFileHashes[] = {
		0x04260848,
		0x12970401,
		0x128F0401,
		0x12830401,
		0x12850401,
		0x6A8B9008
	};

	int16 x = MAX<int16>(_klayman->getX(), 2);
	int index = 1;

	while (index < (int)_pointList->size() && (*_pointList)[index].x >= x)
		++index;
	--index;

	if (_pointIndex != index) {
		_pointIndex = index;
		_palette->addPalette(kScene2809PaletteFileHashes[index], 0, 64, 0);
	}
	
}

} // End of namespace Neverhood
