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

#include "neverhood/module1400.h"
#include "neverhood/module1000.h"
#include "neverhood/module2100.h"
#include "neverhood/module2200.h"
#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

Module1400::Module1400(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	_vm->_soundMan->addMusic(0x00AD0012, 0x06333232);
	_vm->_soundMan->addMusic(0x00AD0012, 0x624A220E);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else {
		createScene(0, 0);
	}

}

Module1400::~Module1400() {
	_vm->_soundMan->deleteMusicGroup(0x00AD0012);
}

void Module1400::createScene(int sceneNum, int which) {
	debug("Module1400::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1401(_vm, this, which);
		break;
	case 1:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->stopMusic(0x624A220E, 0, 2);
		_childObject = new Scene1402(_vm, this, which);
		break;
	case 2:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->startMusic(0x624A220E, 0, 2);
		_childObject = new Scene1403(_vm, this, which);
		break;
	case 3:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1404(_vm, this, which);
		break;
	case 4:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1405(_vm, this, which);
		break;
	case 5:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 2);
		break;
	case 6:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new Scene1407(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module1400::updateScene);
	_childObject->handleUpdate();
}

void Module1400::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 0);
			} else if (_moduleResult == 2) {
				createScene(3, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 1) {
				createScene(2, 0);
			} else if (_moduleResult == 2) {
				createScene(6, -1);
			} else {
				createScene(0, 1);
			}
			break;
		case 2:
			createScene(1, 1);
			break;
		case 3:
			if (_moduleResult == 1) {
				createScene(4, 0);
			} else if (_moduleResult == 2) {
				createScene(5, -1);
			} else {
				createScene(0, 2);
			}
			break;
		case 4:
			createScene(3, 1);
			break;
		case 5:
			createScene(3, 2);
			break;
		case 6:
			createScene(1, 2);
			break;
		}
	}
}

// Scene1401

Class525::Class525(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm),
	_countdown1(0), _countdown2(0) {
	
	// TODO createSurface3(900, dword_4B6768);
	createSurface(900, 640, 480); //TODO: Remeove once the line above is done
	_x = 454;
	_y = 217;
	SetUpdateHandler(&Class525::update4662A0);
	SetMessageHandler(&Class525::handleMessage466320);
	startAnimation(0x4C210500, 0, -1);
}

Class525::~Class525() {
	_vm->_soundMan->deleteSoundGroup(0x01104C08);
}

void Class525::update4662A0() {
	AnimatedSprite::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		sub466460();
	}
	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		_vm->_soundMan->addSound(0x01104C08, 0x4A116437);
		_vm->_soundMan->playSoundLooping(0x4A116437);
	}
}

void Class525::update466300() {
	AnimatedSprite::update();
	if (_countdown1 != 0) {
		_countdown1--;
	}
}

uint32 Class525::handleMessage466320(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0A8A1490) {
			_soundResource2.play(0x6AB6666F);
		}
		break;
	case 0x2000:
		_countdown1 = 70;
		_countdown2 = 8;
		sub466420();
		break;		
	case 0x483A:
		sub4664B0();
		break;		
	}
	return messageResult;
}

uint32 Class525::handleMessage4663C0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		if (_countdown1 != 0) {
			sub466420();
		} else {
			sub466460();
		}
		SetMessageHandler(&Class525::handleMessage466320);
		SetUpdateHandler(&Class525::update4662A0);
		break;
	}
	return messageResult;
}

void Class525::sub466420() {
	startAnimation(0x4C240100, 0, -1);
	_soundResource1.play(0x4A30063F);
}

void Class525::sub466460() {
	_vm->_soundMan->deleteSound(0x4A116437);
	_soundResource1.play(0x4A120435);
	startAnimation(0x4C210500, 0, -1);
}

void Class525::sub4664B0() {
	startAnimation(0x6C210810, 0, -1);
		SetMessageHandler(&Class525::handleMessage4663C0);
		SetUpdateHandler(&Class525::update466300);
}

Class526::Class526(NeverhoodEngine *vm, Sprite *class525)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _class525(class525) {
	
	// TODO createSurface3(100, dword_4B6778);
	createSurface(100, 640, 480); //TODO: Remeove once the line above is done
	_x = 478;
	_y = 433;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class526::handleMessage);
	startAnimation(0xA282C472, 0, -1);
}

uint32 Class526::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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
	case 0x4839:
		sub466770();
		break;		
	}
	return messageResult;
}

void Class526::spriteUpdate466720() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		_soundResource.play(0x0E32247F);
		stopAnimation();
		SetSpriteUpdate(NULL);
		SetMessageHandler(NULL);
		setVisible(false);
	}
}

void Class526::sub466770() {
	startAnimation(0x34880040, 0, -1);
	SetSpriteUpdate(&Class526::spriteUpdate466720);
}

Class527::Class527(NeverhoodEngine *vm, Sprite *class526)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _class526(class526) {

	// TODO createSurface3(200, dword_4B6768);
	createSurface(200, 640, 480); //TODO: Remeove once the line above is done
	_x = 427;
	_y = 433;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class527::handleMessage);
	startAnimation(0x461A1490, 0, -1);
}

uint32 Class527::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4839:
		sub466970();
		break;		
	}
	return messageResult;
}

void Class527::spriteUpdate466920() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		_soundResource.play(0x18020439);
		stopAnimation();
		SetSpriteUpdate(NULL);
		SetMessageHandler(NULL);
		setVisible(false);
	}
}

void Class527::sub466970() {
	startAnimation(0x103B8020, 0, -1);
	SetSpriteUpdate(&Class527::spriteUpdate466920);
}

Class528::Class528(NeverhoodEngine *vm, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x04551900, 100);
	SetUpdateHandler(&Class528::update);
	SetMessageHandler(&Class528::handleMessage);
	_newStickFrameIndex = -2;
	if (flag) {
		_flag = true;
		startAnimation(0x04551900, -1,- 1);
		_countdown = 48;
	} else {
		_flag = false;
		stopAnimation();
		setVisible(false);
	}
}

void Class528::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sub466C50();
	}
	AnimatedSprite::update();
}


uint32 Class528::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		if (_flag)
			_countdown = 168;
		messageResult = _flag ? 1 : 0;			
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		_countdown = 168;
		if (_flag)
			sub466BF0();
		break;						
	}
	return messageResult;
}

void Class528::sub466BF0() {
	_flag = true;
	setVisible(true);
	startAnimation(0x04551900, 0, -1);
	_newStickFrameIndex = -2;
	_soundResource.play(calcHash("fxDoorOpen24"));
}

void Class528::sub466C50() {
	_flag = false;
	setVisible(true);
	startAnimation(0x04551900, -1, -1);
	_soundResource.play(calcHash("fxDoorClose24"));
	_playBackwards = true;
	NextState(&Class528::sub466CB0);
}

void Class528::sub466CB0() {
	stopAnimation();
	setVisible(false);
}

static const Class489Item kClass489Items[] = {
	{{154, 453}, 4,  2,  0, -1, 0, 1},
	{{104, 391}, 4, -1, -1, -1, 1, 1},
	{{ 22, 447}, 6, -1, -1, -1, 1, 1},
	{{112, 406}, 2, -1, -1, -1, 1, 0},
	{{262, 433}, 1,  1,  0, -1, 0, 0}
};

Class489::Class489(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, Sprite *class525)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _klayman(klayman), _class525(class525),
	_soundResource1(vm), _soundResource2(vm), _soundResource3(vm) {

	_class489Item = &kClass489Items[getGlobalVar(0x04A105B3)];
	// TODO createSurface3(990, dword_4B26D8);
	createSurface(990, 640, 480); //TODO: Remeove once the line above is done
	startAnimation(0x10E3042B, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class489::handleMessage);
	_x = getGlobalVar(0x04A10F33) * 108 + _class489Item->point.x;
	_flag = true;
	sub434C80();
	setDoDeltaX(1);
	if ((int8)getGlobalVar(0x04A10F33) == _class489Item->varIndex2) {
		sub434E90();
	}
	_soundResource3.load(0xC8C2507C);
}

Class489::~Class489() {
	_vm->_soundMan->deleteSoundGroup(0x05331081);
}

uint32 Class489::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4807:
		setGlobalVar(0x04A10F33, (_x - _class489Item->point.x) / 108);
		if ((int8)getGlobalVar(0x04A10F33) == _class489Item->varIndex2) {
			sub434E60();
		} else {
			sub434DD0();
		}
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(0x04A10F33) < _class489Item->varIndex1) {
				incGlobalVar(0x04A10F33, 1);
			}
		} else if (getGlobalVar(0x04A10F33) > 0) {
			incGlobalVar(0x04A10F33, -1);
		}
		sub434DF0();
		break;
	case 0x480C:
		if (param.asInteger() != 1) {
			messageResult = (int8)getGlobalVar(0x04A10F33) < _class489Item->varIndex1 ? 1 : 0;
		} else {
			messageResult = getGlobalVar(0x04A10F33) > 0 ? 1 : 0;
		}
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	case 0x4828:
		sub435040();
		break;
	}
	return messageResult;
}

uint32 Class489::handleMessage4348E0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (param.asPoint().x - _x >= 17 && param.asPoint().x - _x <= 56 &&
			param.asPoint().y - _y >= -120 && param.asPoint().y - _y <= -82) {
			sendMessage(_parentScene, 0x4826, 1);
		} else {
			sendMessage(_parentScene, 0x4826, 0);
		}
		messageResult = 1;
		break;
	case 0x4807:
		sendMessage(_parentScene, 0x4807, 0);
		sub434F80();
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(0x04A10F33) < _class489Item->varIndex1) {
				incGlobalVar(0x04A10F33, 1);
			}
		} else if (getGlobalVar(0x04A10F33) > 0) {
			incGlobalVar(0x04A10F33, -1);
		}
		sub434FF0();
		break;
	case 0x480C:
		if (param.asInteger() != 1) {
			messageResult = (int8)getGlobalVar(0x04A10F33) < _class489Item->varIndex1 ? 1 : 0;
		} else {
			messageResult = getGlobalVar(0x04A10F33) > 0 ? 1 : 0;
		}
		break;
	case 0x480F:
		sub434EC0();
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

uint32 Class489::handleMessage434B20(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void Class489::spriteUpdate434B60() {
	if (_x <= _klayman->getX())
		_x = _klayman->getX() - 100;
	else
		_x = _klayman->getX() + 100;
	sub434C80();
	if (_remX == _x) {
		if (getGlobalVar(0x04A10F33) == 0 && _class489Item->flag4 != 0) {
			sendMessage(_parentScene, 0x1019, 0);
			incGlobalVar(0x04A105B3, -1);
			setGlobalVar(0x04A10F33, kClass489Items[getGlobalVar(0x04A105B3)].varIndex1);
		} else if ((int8)getGlobalVar(0x04A10F33) == _class489Item->varIndex1 && _class489Item->flag != 0) {
			sendMessage(_parentScene, 0x1019, 1);
			incGlobalVar(0x04A105B3, +1);
			setGlobalVar(0x04A10F33, 0);
		}
	}
	Sprite::processDelta();
}

void Class489::sub434C80() {

	bool soundFlag = false;

	_y = _class489Item->point.y;

	if (_class489Item->index1 != -1) {
		int16 elX = _class489Item->index1 * 108 + _class489Item->point.x;
		if (elX - 20 < _x && elX + 20 > _x) {
			soundFlag = true;
			_y = _class489Item->point.y + 10;
		}
	}

	if (_class489Item->flag2 != -1) {
		int16 elX = _class489Item->index1 * 108 + _class489Item->point.x;
		if (elX - 20 < _x && elX + 20 > _x) {
			soundFlag = true;
			_y = _class489Item->point.y + 10;
		}
	}

	if (_class489Item->varIndex2 != -1) {
		int16 elX = _class489Item->varIndex2 * 108 + _class489Item->point.x;
		if (elX - 20 < _x && elX + 20 > _x) {
			soundFlag = true;
			_y = _class489Item->point.y + 10;
		}
	}

	if (_flag) {
		if (!soundFlag) {
			_flag = false;
		}
	} else if (soundFlag) {
		_soundResource2.play(0x5440E474);
		_flag = true;
	}
	
}

void Class489::sub434D80() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		sendMessage(_class525, 0x483A, 0);
		stopAnimation();
		SetMessageHandler(&Sprite::handleMessage);
		SetSpriteUpdate(NULL);
		setVisible(false);
	}
}

void Class489::sub434DD0() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Class489::handleMessage);
	startAnimation(0x10E3042B, 0, -1);
}

void Class489::sub434DF0() {
	_remX = getGlobalVar(0x04A10F33) * 108 + _class489Item->point.x;
	startAnimation(0x14A10137, 0, -1);
	SetSpriteUpdate(&Class489::spriteUpdate434B60);
	SetMessageHandler(&Class489::handleMessage);
	_soundResource2.play(0xEC008474);
}

void Class489::sub434E60() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Class489::handleMessage434B20);
	startAnimation(0x80C32213, 0, -1);
	NextState(&Class489::sub434E90);
}

void Class489::sub434E90() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Class489::handleMessage4348E0);
	startAnimation(0xD23B207F, 0, -1);
}

void Class489::sub434EC0() {
	startAnimation(0x50A80517, 0, -1);
	SetMessageHandler(&Class489::handleMessage434B20);
	SetSpriteUpdate(NULL);
	NextState(&Class489::sub434F40);
	setGlobalVar(0x12A10DB3, 1);
	_soundResource1.play(0xCC4A8456);
	_vm->_soundMan->addSound(0x05331081, 0xCE428854);
	_vm->_soundMan->playSoundLooping(0xCE428854);
}

void Class489::sub434F40() {
	sendMessage(_parentScene, 0x480F, 0);
	startAnimation(0xD833207F, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Class489::handleMessage4348E0);
}

void Class489::sub434F80() {
	startAnimation(0x50A94417, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Class489::handleMessage434B20);
	NextState(&Class489::sub434E90);
	setGlobalVar(0x12A10DB3, 0);
	_soundResource1.play(0xCC4A8456);
	_vm->_soundMan->deleteSound(0xCE428854);
}

void Class489::sub434FF0() {
	_remX = getGlobalVar(0x04A10F33) * 108 + _class489Item->point.x;
	startAnimation(0x22CB4A33, 0, -1);
	SetSpriteUpdate(&Class489::spriteUpdate434B60);
	SetMessageHandler(&Class489::handleMessage434B20);
	NextState(&Class489::sub434DF0);
}

void Class489::sub435040() {
	setGlobalVar(0x04A105B3, 4);
	setGlobalVar(0x04A10F33, 0);
	SetSpriteUpdate(&Class489::sub434D80);
	SetMessageHandler(&Sprite::handleMessage);
	startAnimation(0x708D4712, 0, -1);
	_soundResource3.play();
}

Scene1401::Scene1401(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag(false), _class427(NULL), _class489(NULL), 
	_class525(NULL), _class526(NULL), _class527(NULL), _class528(NULL), 
	_sprite1(NULL), _sprite2(NULL), _sprite3(NULL), _ssButton(NULL) {

	SetMessageHandler(&Scene1401::handleMessage);
	SetUpdateHandler(&Scene1401::update);
	setRectList(0x004B6758);
	_surfaceFlag = true;

	setBackground(0x08221FA5);
	setPalette(0x08221FA5);
	insertMouse433(0x21FA108A);
	
	_class427 = insertSprite<Class427>(this, 0x980F3124, 0x12192892, 100, 0);
	_class525 = insertSprite<Class525>();

	if (!getGlobalVar(0x01023818)) {
		_class526 = insertSprite<Class526>(_class525);
		_class527 = insertSprite<Class527>(_class525);
	}

	_sprite3 = insertStaticSprite(0xA82BA811, 1100);
	insertStaticSprite(0x0A116C60, 1100);
	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0xB84B1100, 100, 0);
	_sprite1 = insertStaticSprite(0x38EA100C, 1005);
	_sprite2 = insertStaticSprite(0x98D0223C, 1200);
	_sprite2->setVisible(false);

	if (which < 0) {
		insertKlayman<KmScene1401>(380, 447);
		setMessageList(0x004B65C8);
		_sprite1->setVisible(false);
	} else if (which == 1) {
		insertKlayman<KmScene1401>(0, 447);
		setMessageList(0x004B65D0);
		_sprite1->setVisible(false);
	} else if (which == 2) {
		insertKlayman<KmScene1401>(660, 447);
		setMessageList(0x004B65D8);
		_sprite1->setVisible(false);
	} else {
		insertKlayman<KmScene1401>(290, 413);
		setMessageList(0x004B65E8);
		_sprite1->setVisible(false);
	}

	if (getGlobalVar(0x04A105B3) == 2) {
		_class489 = insertSprite<Class489>(this, _klayman, _class525);
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 6) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		} else if (getGlobalVar(0x04A10F33) == 0) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		}
		_class489->setClipRect(_sprite3->getDrawRect().x, _sprite2->getDrawRect().y, 640, 480);
	}
	
	_klayman->setClipRect(_sprite3->getDrawRect().x, 0, 640, 480);

	if (which == 0 && _class489) {
		sendMessage(_class489, 0x482B, 0);
	}

	_class528 = insertSprite<Class528>(_klayman, which == 1);

}

void Scene1401::update() {
	Scene::update();
	if (_class489 && !_flag && _class489->getY() < 360) {
		_sprite2->setVisible(true);
		_flag = true;
	} else {
		_sprite2->setVisible(false);
	}
}

uint32 Scene1401::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02144CB1) {
			sendEntityMessage(_klayman, 0x1014, _class427);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_class528, 0x2001, 0) != 0) {
				setMessageList(0x004B6690);
			} else {
				setMessageList(0x004B66B0);
			}
		}
		break;
	case 0x1019:
		if (param.asInteger() != 0) {
			leaveScene(2);
		} else {
			leaveScene(1);
		}			
		break;
	case 0x480B:
		if (sender == _class427) {
			sendMessage(_class525, 0x2000, 0);
			if (!getGlobalVar(0x01023818)) {
				sendMessage(_class526, 0x4839, 0);
				sendMessage(_class527, 0x4839, 0);
				setGlobalVar(0x01023818, 1);
			}
			if (_class489 && _class489->getX() > 404 && _class489->getX() < 504) {
				sendMessage(_class489 , 0x4839, 0);
			}
		} else if (sender == _ssButton) {
			sendMessage(_ssButton, 0x4808, 0);
		}
		break;
	case 0x4826:
		if (sender == _class489) {
			if (sendMessage(_class489, 0x480C, _klayman->getX() > _class489->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _class489);
				setMessageList2(0x004B6658);
			} else {
				setMessageList2(0x004B65F0);
			}
		}						
		break;
	case 0x482A:
		_sprite1->setVisible(true);
		if (_class489) {
			sendMessage(_class489, 0x482B, 0);
		}
		break;
	case 0x482B:
		_sprite1->setVisible(false);
		if (_class489) {
			sendMessage(_class489, 0x482A, 0);
		}
		break;
	}
	return 0;
}

// Scene1402

Class454::Class454(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority)
	: StaticSprite(vm, fileHash, surfacePriority) {
	
	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&StaticSprite::update);
	
}

Class482::Class482(NeverhoodEngine *vm, Scene *parentScene, int which)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _soundResource1(vm),
	_soundResource2(vm) {

	// TODO createSurface3(900, dword_4B6768);
	createSurface(900, 640, 480); //TODO: Remeove once the line above is done

	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class482::handleMessage);
	_x = 279;
	_y = 270;
	if (which == 2) {
		startAnimation(0x20060259, 0, -1);
		_soundResource1.play(0x419014AC);
		_soundResource2.load(0x61901C29);
	} else if (which == 1) {
		startAnimation(0x210A0213, 0, -1);
		_soundResource1.play(0x41809C6C);
	} else {
		startAnimation(0x20060259, 0, -1);
		_soundResource2.load(0x61901C29);
		_newStickFrameIndex = -2;
	}
}

uint32 Class482::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		_soundResource2.play();
		startAnimation(0x20060259, -1, -1);
		_playBackwards = true;
		NextState(&Class482::sub428530);
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void Class482::sub428500() {
	sendMessage(_parentScene, 0x2000, 0);
	stopAnimation();
	setVisible(false);
}

void Class482::sub428530() {
	sendMessage(_parentScene, 0x2001, 0);
	stopAnimation();
	setVisible(false);
}

void Class482::sub428560() {
	sendMessage(_parentScene, 0x2003, 0);
	stopAnimation();
}

Scene1402::Scene1402(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag(false), _class482(NULL), _class489(NULL) {

	SetMessageHandler(&Scene1402::handleMessage);

	setBackground(0x231482F0);
	setBackgroundY(-10);
	// TODO g_screen->field_26 = 0;
	setPalette(0x231482F0);
	_palette->addPalette(0x91D3A391, 0, 64, 0);
	insertMouse433(0x482F4239);

	_class454_1 = insertSprite<Class454>(0x15402D64, 1100);
	_class454_2 = insertSprite<Class454>(0x10A02120, 1100);
	_class454_3 = insertSprite<Class454>(0x60882BE0, 1100);

	if (getGlobalVar(0x70A1189C))
		setRectList(0x004B0C48);
	else
		setRectList(0x004B0C98);

	if (which < 0) {
		insertKlayman<KmScene1402>(377, 391);
		setMessageList(0x004B0B48);
		if (!getGlobalVar(0x70A1189C)) {
			_class482 = insertSprite<Class482>(this, 0);
		}
	} else if (which == 1) {
		insertKlayman<KmScene1402>(42, 391);
		setMessageList(0x004B0B50);
	} else if (which == 2) {
		insertKlayman<KmScene1402>(377, 391);
		setMessageList(0x004B0B60);
		_klayman->setDoDeltaX(1);
		if (getGlobalVar(0x70A1189C)) {
			_class482 = insertSprite<Class482>(this, 1);
			clearRectList();
			showMouse(false);
			sub428220();
		} else {
			_class482 = insertSprite<Class482>(this, 0);
		}
	} else {
		insertKlayman<KmScene1402>(513, 391);
		setMessageList(0x004B0B58);
		if (!getGlobalVar(0x70A1189C)) {
			_class482 = insertSprite<Class482>(this, 2);
			sub428220();
		}
	}

	if (_class482) {
		_class482->setClipRect(0, 0, 640, _class454_3->getDrawRect().y2());
	}

	if (getGlobalVar(0x4A105B3) == 1) {
		_class489 = insertSprite<Class489>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x4A10F33) == 4) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		} else if (getGlobalVar(0x4A10F33) == 0) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		}
		_class489->setClipRect(_class454_1->getDrawRect().x, 0, _class454_2->getDrawRect().x, _class454_3->getDrawRect().y2());
	}

	_klayman->setClipRect(_class454_1->getDrawRect().x, 0, _class454_2->getDrawRect().x2(), _class454_3->getDrawRect().y2());
	
}

void Scene1402::update() {
	if (_flag) {
		setBackgroundY(_vm->_rnd->getRandomNumber(10 - 1) - 10);
		// TODO g_screen->field_26 = -10 - _background->getDrawRect().y;
	} else {
		setBackgroundY(-10);
		// TODO g_screen->field_26 = 0;
		SetUpdateHandler(&Scene::update);
	}
	Scene::update();
	if (_class482) {
		_class482->setClipRect(0, 0, 640, _class454_3->getDrawRect().y2());
	}
	_klayman->setClipRect(_class454_1->getDrawRect().x, 0, _class454_2->getDrawRect().x2(), _class454_3->getDrawRect().y2());
}

uint32 Scene1402::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00F43389) {
			if (getGlobalVar(0x70A1189C)) {
				leaveScene(0);
			} else {
				clearRectList();
				_klayman->setVisible(false);
				showMouse(false);
				sendMessage(_class482, 0x2002, 0);
				sub428220();
			}
		}
		break;
	case 0x1019:
		if (param.asInteger()) {
			leaveScene(0);
		} else {
			leaveScene(1);
		}
		break;
	case 0x2000:
		sub428230();
		showMouse(true);
		setRectList(0x004B0C48);
		break;
	case 0x2001:
		sub428230();
		leaveScene(0);
		break;
	case 0x2003:
		sub428230();
		break;
	case 0x4826:
		if (sender == _class489) {
			if (sendMessage(_class489, 0x408C, _klayman->getX() > _class489->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _class489);
				setMessageList2(0x004B0BB8);
			} else {
				setMessageList2(0x004B0B68);
			}
		}
	}
	return 0;
}

void Scene1402::sub428220() {
	_flag = true;
	SetUpdateHandler(&Scene1402::update);
}

void Scene1402::sub428230() {
	_flag = false;
}

// Scene1407

static const int16 kScene1407MouseFloorY[] = {
	106, 150, 191, 230, 267, 308, 350, 395
};

static const struct {
	int16 x;
	int16 floorIndex;
	int16 sectionIndex;
	int16 nextHoleIndex;
} kScene1407MouseHoles[] = {
	{125, 0,  0,  7},
	{452, 7, 21,  0},
	{337, 4, 11,  4},
	{286, 6, 17,  6},
	{348, 6, 17, 39},
	{536, 6, 18, 42},
	{111, 1,  3, 18},
	{203, 1,  3, 38},
	{270, 1,  3,  9},
	{197, 5, 14,  3},
	{252, 5, 14, 35},
	{297, 5, 14,  7},
	{359, 5, 14,  8},
	{422, 4, 12, 26},
	{467, 4, 12,  2},
	{539, 4, 12, 40},
	{111, 5, 13, 17},
	{211, 0,  1, 20},
	{258, 0,  1, 11},
	{322, 0,  1, 16},
	{ 99, 6, 16, 31},
	{142, 6, 16, 27},
	{194, 6, 16, 12},
	{205, 2,  6, 45},
	{264, 2,  6, 10},
	{ 98, 4, 10,  2},
	{152, 4, 10, 37},
	{199, 4, 10, 13},
	{258, 4, 10, 16},
	{100, 7, 19, 43},
	{168, 7, 19, 23},
	{123, 3,  8, 14},
	{181, 3,  8, 39},
	{230, 3,  8, 28},
	{292, 3,  8, 22},
	{358, 3,  8, 36},
	{505, 3,  9, 44},
	{400, 2,  7, 34},
	{454, 2,  7, 32},
	{532, 2,  7, 46},
	{484, 5, 15, 25},
	{529, 5, 15, 30},
	{251, 7, 20, 48},
	{303, 7, 20, 21},
	{360, 7, 20, 33},
	{503, 0,  2,  5},
	{459, 1,  4, 19},
	{530, 1,  4, 42},
	{111, 2,  5, 47},
	{442, 6, 18,  1}
};

static const struct {
	int16 x1, x2;
	int16 goodHoleIndex;
} kScene1407MouseSections[] = {
	{100, 149,  0},
	{182, 351, 17},
	{430, 524, 45},
	{ 89, 293,  7},
	{407, 555, 47},
	{ 89, 132, 48},
	{178, 303, 23},
	{367, 551, 38},
	{105, 398, 31},
	{480, 537, 36},
	{ 84, 275, 27},
	{318, 359,  2},
	{402, 560, 15},
	{ 91, 132, 16},
	{179, 400, 10},
	{461, 552, 41},
	{ 86, 218, 21},
	{267, 376,  4},
	{420, 560, 49},
	{ 77, 188, 30},
	{237, 394, 44},
	{438, 515,  5}
};

AsScene1407Mouse::AsScene1407Mouse(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _currSectionIndex(0) {
	
	// TODO createSurface3(100, dword_4B05B0);
	createSurface(100, 640, 480); //TODO: Remeove once the line above is done

	SetUpdateHandler(&AnimatedSprite::update);
	_x = 108;
	_y = 106;
	stIdleLookAtGoodHole();
}

void AsScene1407Mouse::suWalkTo() {
	int16 xdelta = _walkDestX - _x;
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
	_deltaX = 0;		
	if (_walkDestX == _x) {
		sendMessage(this, 0x1019, 0);
	} else {
		_x += xdelta;
		processDelta();
	}
}

void AsScene1407Mouse::upGoThroughHole() {
	if (_countdown != 0 && (--_countdown == 0)) {
		SetUpdateHandler(&AnimatedSprite::update);
		gotoNextState();
	}
	AnimatedSprite::update();
}

uint32 AsScene1407Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		{
			int16 mouseX = param.asPoint().x;
			int16 mouseY = param.asPoint().y;
			int holeIndex;
			for (holeIndex = 0; holeIndex < 50; holeIndex++) {
				int16 holeX = kScene1407MouseHoles[holeIndex].x;
				int16 holeY = kScene1407MouseFloorY[kScene1407MouseHoles[holeIndex].floorIndex];
				if (mouseX >= holeX - 14 && mouseX <= holeX + 14 && mouseY >= holeY - 36 && mouseY <= holeY)
					break;
			}
			if (holeIndex < 50 && kScene1407MouseHoles[holeIndex].sectionIndex == _currSectionIndex) {
				_nextHoleIndex = kScene1407MouseHoles[holeIndex].nextHoleIndex;
				_walkDestX = kScene1407MouseHoles[holeIndex].x;
				stWalkToHole();
			} else {
				if (mouseX < kScene1407MouseSections[_currSectionIndex].x1) {
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x1;
				} else if (mouseX > kScene1407MouseSections[_currSectionIndex].x2) {
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x2;
				} else {
					_walkDestX = mouseX;
				}
				stWalkToDest();
			}
		}
		break;
	case 0x1019:
		gotoNextState();
		break;
	case 0x2001:
		{
			// Reset the position
			// Find the nearest hole and go through it, and exit at the first hole
			int16 distance = 640;
			int matchIndex = 50;
			for (int index = 0; index < 50; index++) {
				if (kScene1407MouseHoles[index].sectionIndex == _currSectionIndex) {
					if (ABS(kScene1407MouseHoles[index].x - _x) < distance) {
						matchIndex = index;
						distance = ABS(kScene1407MouseHoles[index].x - _x);
					}
				}
			}
			if (matchIndex < 50) {
				_nextHoleIndex = 0;
				_walkDestX = kScene1407MouseHoles[matchIndex].x;
				stWalkToHole();
			}
		}
		break;
	}
	return messageResult;
}

void AsScene1407Mouse::stIdleLookAtGoodHole() {
	setDoDeltaX(kScene1407MouseHoles[kScene1407MouseSections[_currSectionIndex].goodHoleIndex].x < _x ? 1 : 0);
	startAnimation(0x72215194, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
}

void AsScene1407Mouse::stWalkToDest() {
	if (_walkDestX != _x) {
		setDoDeltaX(_walkDestX < _x ? 1 : 0);
		startAnimation(0x22291510, 0, -1);
		SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
		SetMessageHandler(&AsScene1407Mouse::handleMessage);
		NextState(&AsScene1407Mouse::stIdleLookAtGoodHole);
	}
}

void AsScene1407Mouse::stWalkToHole() {
	setDoDeltaX(_walkDestX < _x ? 1 : 0);
	startAnimation(0x22291510, 0, -1);
	SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
	NextState(&AsScene1407Mouse::stGoThroughHole);
}

void AsScene1407Mouse::stGoThroughHole() {
	startAnimation(0x72215194, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(NULL);
	SetUpdateHandler(&AsScene1407Mouse::upGoThroughHole);
	NextState(&AsScene1407Mouse::stArriveAtHole);
	setVisible(false);
	_countdown = 12;
}

void AsScene1407Mouse::stArriveAtHole() {
	_currSectionIndex = kScene1407MouseHoles[_nextHoleIndex].sectionIndex;
	_x = kScene1407MouseHoles[_nextHoleIndex].x;
	_y = kScene1407MouseFloorY[kScene1407MouseHoles[_nextHoleIndex].floorIndex];
	if (_nextHoleIndex == 1) {
		sendMessage(_parentScene, 0x2000, 0);
		_walkDestX = 512;
		stWalkToDest();
		setVisible(true);
	} else {
		_walkDestX = _x + 14;
		stWalkToDest();
		setVisible(true);
	}
}

Scene1407::Scene1407(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _puzzleSolvedCountdown(0),
	_resetButtonCountdown(0) {

	_surfaceFlag = true;

	SetMessageHandler(&Scene1407::handleMessage);
	SetUpdateHandler(&Scene1407::update);

	setBackground(0x00442225);
	setPalette(0x00442225);
	insertMouse435(0x4222100C, 20, 620);

	_asMouse = insertSprite<AsScene1407Mouse>(this);
	_ssResetButton = insertStaticSprite(0x12006600, 100);
	_ssResetButton->setVisible(false); 

}

void Scene1407::update() {
	Scene::update();
	if (_puzzleSolvedCountdown != 0 && (--_puzzleSolvedCountdown == 0)) {
		leaveScene(1);
	} else if (_resetButtonCountdown != 0 && (--_resetButtonCountdown == 0)) {
		_ssResetButton->setVisible(false);
	}
}

uint32 Scene1407::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (_puzzleSolvedCountdown == 0) {
			// TODO: Debug/Cheat stuff
			if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
				// Exit scene
				leaveScene(0);
			} else if (param.asPoint().x >= 75 && param.asPoint().x <= 104 &&
				param.asPoint().y >= 62 && param.asPoint().y <= 90) {
				// The reset button was clicked
				sendMessage(_asMouse, 0x2001, 0);
				_ssResetButton->setVisible(true);
				_soundResource.play(0x44045000);
				_resetButtonCountdown = 12;
			} else {
				// Handle the mouse
				sendMessage(_asMouse, messageNum, param);
			}
		}
		break;
	case 0x000D:
		// TODO: Debug/Cheat stuff
		break;
	case 0x2000:
		// The mouse got the cheese (nomnom)
		setGlobalVar(0x70A1189C, 1);
		_soundResource.play(0x68E25540);
		showMouse(false);
		_puzzleSolvedCountdown = 72;
		break;
	}
	return 0;
}

// Scene1403

Scene1403::Scene1403(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _class489(NULL), _flag(false) {
	
	SetMessageHandler(&Scene1403::handleMessage);
	
	setRectList(0x004B1FF8);
	_surfaceFlag = true;

	setBackground(0x2110A234);
	setPalette(0x2110A234);
	insertMouse433(0x0A230219);

	_class401_1 = insertStaticSprite(0x01102A33, 100);
	_class401_1->setVisible(false);

	_class401_2 = insertStaticSprite(0x04442520, 995);
	
	_class401_3 = insertStaticSprite(0x08742271, 995);

	_asTape1 = insertSprite<AsScene1201Tape>(this, 12, 1100, 201, 468, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape1);
	_asTape1->setRepl(64, 0);

	_asTape2 = insertSprite<AsScene1201Tape>(this, 16, 1100, 498, 468, 0x9048A093);
	_vm->_collisionMan->addSprite(_asTape2);
	_asTape2->setRepl(64, 0);

	if (which < 0) {
		insertKlayman<KmScene1402>(380, 463);
		setMessageList(0x004B1F18);
	} else {
		insertKlayman<KmScene1402>(640, 463);
		setMessageList(0x004B1F20);
	}
	_klayman->setRepl(64, 0);

	if (getGlobalVar(0x04A105B3) == 4) {
		_class489 = insertSprite<Class489>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 4) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B1F70);
		}
		_class489->setClipRect(0, 0, 640, _class401_2->getDrawRect().y2());
		_class489->setRepl(64, 0);
	}

}

uint32 Scene1403::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x88C11390) {
			setRectList(0x004B2008);
			_flag = true;
		} else if (param.asInteger() == 0x08821382) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			setRectList(0x004B1FF8);
			_flag = false;
		}
		break;
	case 0x1019:
		leaveScene(0);
		break;
	case 0x1022:
		if (sender == _class489) {
			if (param.asInteger() >= 1000) {
				setSurfacePriority(_class401_3->getSurface(), 1100);
			} else {
				setSurfacePriority(_class401_3->getSurface(), 995);
			}
		}
		break;
	case 0x4807:
		_class401_1->setVisible(false);
		break;
	case 0x480F:
		_class401_1->setVisible(true);
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (param.asInteger() == 1) {
				sendEntityMessage(_klayman, 0x1014, _class489);
				setMessageList2(0x004B1F88);
			} else if (sendMessage(_class489, 0x480C, _klayman->getX() > _class489->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _class489);
				setMessageList2(0x004B1F58);
			} else {
				setMessageList2(0x004B1F28);
			}
		} else if (sender == _asTape1 || sender == _asTape2) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (_messageListStatus != 2) {
				sendEntityMessage(_klayman, 0x1014, sender);
				setMessageList2(0x004B1FB8);
			}
		}
		break;
	}
	return 0;
}

// Scene1404

Scene1404::Scene1404(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _class489(NULL), _class545(NULL) {
	
	if (getGlobalVar(0xC0780812) && !getGlobalVar(0x13382860)) {
		setGlobalVar(0x13382860, 5);
	}
	
	SetMessageHandler(&Scene1404::handleMessage);
	_surfaceFlag = true;

	setRectList(0x004B8D80);

	setBackground(0xAC0B006F);
	setPalette(0xAC0B006F);
	_palette->addPalette(0x00801510, 0, 65, 0);
	insertMouse433(0xB006BAC8);

	if (getGlobalVar(0x13382860) == 5) {
		_class545 = insertSprite<Class545>(this, 2, 1100, 267, 411);
		_vm->_collisionMan->addSprite(_class545);
	}

	_sprite1 = insertStaticSprite(0x1900A1F8, 1100);

	_asTape = insertSprite<AsScene1201Tape>(this, 14, 1100, 281, 411, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape);

	if (which < 0) {
		insertKlayman<KmScene1404>(376, 406);
		setMessageList(0x004B8C28);
	} else if (which == 1) {
		insertKlayman<KmScene1404>(376, 406);
		setMessageList(0x004B8C30);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene1404>(347, 406);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene1404>(187, 406);
		}
		setMessageList(0x004B8D28);
	} else {
		insertKlayman<KmScene1404>(30, 406);
		setMessageList(0x004B8C38);
	}

	if (getGlobalVar(0x04A105B3) == 3) {
		_class489 = insertSprite<Class489>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 0) {
			sendEntityMessage(_klayman, 0x1014, _class489);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B8CB8);
		}
		_class489->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
	}

	_klayman->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

}

Scene1404::~Scene1404() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1404::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x410650C2) {
			if (_class489 && _class489->getX() == 220) {
				setMessageList(0x004B8C40);
			} else {
				setMessageList(0x004B8CE8);
			}
		}
		break;
	case 0x1019:
		leaveScene(0);
		break;
	case 0x4826:
		if (sender == _class489) {
			if (sendMessage(_class489, 0x480C, _klayman->getX() > _class489->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _class489);
				setMessageList2(0x004B8CA0);
			} else {
				setMessageList2(0x004B8C40);
			}
		} else if (sender == _asTape && _messageListStatus != 2) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B8CD0);
		} else if (sender == _class545 && _messageListStatus != 2) {
			sendEntityMessage(_klayman, 0x1014, _class545);
			setMessageList(0x004B8D18);
		}
		break;
	}
	return 0;
}

// Scene1405

static const NPoint kAsScene1405TileItemPositions[] = {
	{100,  80},
	{162,  78},
	{222,  76},
	{292,  76},
	{356,  82},
	{422,  84},
	{488,  86},
	{550,  90},
	{102, 134},
	{164, 132},
	{224, 136},
	{294, 136},
	{360, 136},
	{422, 138},
	{484, 144},
	{548, 146},
	{ 98, 196},
	{160, 200},
	{228, 200},
	{294, 202},
	{360, 198},
	{424, 200},
	{482, 202},
	{548, 206},
	{ 98, 260},
	{160, 264},
	{226, 260},
	{296, 262},
	{358, 260},
	{424, 262},
	{486, 264},
	{550, 266},
	{ 94, 322},
	{160, 316},
	{226, 316},
	{296, 320},
	{358, 322},
	{422, 324},
	{488, 322},
	{550, 322},
	{ 98, 380},
	{160, 376},
	{226, 376},
	{294, 378},
	{356, 380},
	{420, 380},
	{490, 378},
	{552, 376}
};

AsScene1405Tile::AsScene1405Tile(NeverhoodEngine *vm, Scene1405 *parentScene, uint32 index)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _soundResource(vm),
	_index(index), _countdown(0), _flag(false) {

	_soundResource.load(0x05308101);
	// TODO _soundResource.setPan
	_x = kAsScene1405TileItemPositions[_index].x;
	_y = kAsScene1405TileItemPositions[_index].y;
	createSurface1(0x844B805C, 1100);
	setVisible(false);
	if (getSubVar(0xCCE0280F, _index))
		_countdown = _vm->_rnd->getRandomNumber(36 - 1) + 1;
	SetUpdateHandler(&AsScene1405Tile::update);
	SetMessageHandler(&AsScene1405Tile::handleMessage);
	
	debug("getSubVar(0x0C65F80B, _index) = %d", getSubVar(0x0C65F80B, _index));
	
	startAnimation(0x844B805C, getSubVar(0x0C65F80B, _index), -1);
	_newStickFrameIndex = (int16)getSubVar(0x0C65F80B, _index);
}

void AsScene1405Tile::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		show();
	}
}

uint32 AsScene1405Tile::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (getSubVar(0xCCE0280F, _index) == 0 && _parentScene->getCountdown() == 0) {
			show();
			sendMessage(_parentScene, 0x2000, _index);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void AsScene1405Tile::show() {
	if (!_flag) {
		_flag = true;
		_soundResource.play();
		setVisible(true);
	}
}

void AsScene1405Tile::hide() {
	if (_flag) {
		_flag = false;
		_soundResource.play();
		setVisible(false);
	}
}

Scene1405::Scene1405(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _selectFirstTile(true),
	_tilesLeft(48), _countdown(0) {

	_vm->gameModule()->initScene1405Vars();
	_surfaceFlag = true;
	
	setBackground(0x0C0C007D);
	setPalette(0x0C0C007D);
	insertMouse435(0xC00790C8, 20, 620);
	
	// TODO: Some debug code: Leave two matching tiles open
	for (int i = 0; i < 48; i++)
		setSubVar(0xCCE0280F, i, 1);
	int debugIndex = 0;
	setSubVar(0xCCE0280F, debugIndex, 0);
	for (int i = 0; i < 48; i++) {
		if (i != debugIndex && getSubVar(0x0C65F80B, i) == getSubVar(0x0C65F80B, debugIndex)) {
			setSubVar(0xCCE0280F, i, 0);
			break;
		}
	}
	
	for (uint32 index = 0; index < 48; index++) {
		_tiles[index] = insertSprite<AsScene1405Tile>(this, index);
		_vm->_collisionMan->addSprite(_tiles[index]);
		if (getSubVar(0xCCE0280F, index))
			_tilesLeft--;
	}
	
	_soundResource.load(0x68E25540);
	
	SetMessageHandler(&Scene1405::handleMessage);
	SetUpdateHandler(&Scene1405::update);

}

void Scene1405::update() {
	Scene::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		_tilesLeft = 48;
		_tiles[_firstTileIndex]->hide();
		_tiles[_secondTileIndex]->hide();
		for (uint32 i = 0; i < 48; i++) {
			if (getSubVar(0xCCE0280F, i)) {
				_tiles[i]->hide();
				setSubVar(0xCCE0280F, i, 0);
			}
		}
	}
}

uint32 Scene1405::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug/Cheat stuff
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		// TODO: Debug/Cheat stuff
		break;
	case 0x2000:
		if (_selectFirstTile) {
			_firstTileIndex = param.asInteger();
			_selectFirstTile = false;
		} else {
			_secondTileIndex = param.asInteger();
			if (_firstTileIndex != _secondTileIndex) {
				_selectFirstTile = true;
				if (getSubVar(0x0C65F80B, _secondTileIndex) == getSubVar(0x0C65F80B, _firstTileIndex)) {
					setSubVar(0xCCE0280F, _firstTileIndex, 1);
					setSubVar(0xCCE0280F, _secondTileIndex, 1);
					_tilesLeft -= 2;
					if (_tilesLeft == 0) {
						_soundResource.play();
					}
				} else {
					_countdown = 10;
				}
			}
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
