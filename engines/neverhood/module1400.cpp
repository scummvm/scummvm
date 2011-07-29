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
#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

Module1400::Module1400(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1400(%d)", which);

	// TODO Music18hList_add(0x00AD0012, 0x06333232);
	// TODO Music18hList_add(0x00AD0012, 0x624A220E);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			createScene1402(-1);
			break;
		case 2:
			createScene1403(-1);
			break;
		case 3:
			createScene1404(-1);
			break;
		case 4:
			createScene1405(-1);
			break;
		case 5:
			createScene1406(-1);
			break;
		case 6:
			createScene1407(-1);
			break;
		default:
			createScene1401(-1);
		}
	} else {
		createScene1401(0);
	}

}

Module1400::~Module1400() {
	// TODO Music18hList_deleteGroup(0x00AD0012);
}

void Module1400::createScene1401(int which) {
	_vm->gameState().sceneNum = 0;
	// TODO Music18hList_play(0x06333232, 0, 2, 1);
	_childObject = new Scene1401(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1401);
}

void Module1400::createScene1402(int which) {
	_vm->gameState().sceneNum = 1;
	// TODO Music18hList_stop(0x06333232, 0, 2);
	// TODO Music18hList_stop(0x624A220E, 0, 2);
	_childObject = new Scene1402(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1402);
}

void Module1400::createScene1403(int which) {
	_vm->gameState().sceneNum = 2;
	// TODO Music18hList_stop(0x06333232, 0, 2);
	// TODO Music18hList_play(0x624A220E, 0, 2, 1);
	_childObject = new Scene1403(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1403);
}

void Module1400::createScene1404(int which) {
	_vm->gameState().sceneNum = 3;
	// TODO Music18hList_play(0x06333232, 0, 2, 1);
	_childObject = new Scene1404(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1404);
}

void Module1400::createScene1405(int which) {
	_vm->gameState().sceneNum = 4;
	// TODO Music18hList_play(0x06333232, 0, 2, 1);
	_childObject = new Scene1405(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1405);
}

void Module1400::createScene1406(int which) {
	_vm->gameState().sceneNum = 5;
	// TODO Music18hList_stop(0x06333232, 0, 2);
	_childObject = new DiskplayerScene(_vm, this, 2);
	SetUpdateHandler(&Module1400::updateScene1406);
}

void Module1400::createScene1407(int which) {
	_vm->gameState().sceneNum = 6;
	// TODO Music18hList_stop(0x06333232, 0, 2);
	_childObject = new Scene1407(_vm, this, which);
	SetUpdateHandler(&Module1400::updateScene1407);
}

void Module1400::updateScene1401() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene1402(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene1404(0);
			_childObject->handleUpdate();
		} else {
			_parentModule->sendMessage(0x1009, 0, this);
		}
	}
}

void Module1400::updateScene1402() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		
		debug("Scene1402: _field20 = %d", _field20);
		
		if (_field20 == 1) {
			createScene1403(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene1407(-1);
			_childObject->handleUpdate();
		} else {
			createScene1401(1);
			_childObject->handleUpdate();
		}
	}
}

void Module1400::updateScene1403() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1402(1);
		_childObject->handleUpdate();
	}
}

void Module1400::updateScene1404() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		debug("Scene1404; _field20 = %d", _field20);
		if (_field20 == 1) {
			createScene1405(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene1406(-1);
			_childObject->handleUpdate();
		} else {
			createScene1401(2);
			_childObject->handleUpdate();
		}
	}
}

void Module1400::updateScene1405() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1404(1);
		_childObject->handleUpdate();
	}
}

void Module1400::updateScene1406() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1404(2);
		_childObject->handleUpdate();
	}
}

void Module1400::updateScene1407() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene1402(2);
		_childObject->handleUpdate();
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
	setFileHash(0x4C210500, 0, -1);
}

Class525::~Class525() {
	// TODO Sound1ChList_sub_407AF0(0x01104C08);
}

void Class525::update4662A0() {
	AnimatedSprite::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		sub466460();
	}
	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		// TODO Sound1ChList_addSoundResource(0x01104C08, 0x4A116437, true);
		// TODO Sound1ChList_playLooping(0x4A116437);
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
	setFileHash(0x4C240100, 0, -1);
	_soundResource1.play(0x4A30063F);
}

void Class525::sub466460() {
	// TODO Sound1ChList_deleteSoundByHash(0x4A116437);
	_soundResource1.play(0x4A120435);
	setFileHash(0x4C210500, 0, -1);
}

void Class525::sub4664B0() {
	setFileHash(0x6C210810, 0, -1);
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
	setFileHash(0xA282C472, 0, -1);
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
		setFileHash1();
		SetSpriteCallback(NULL);
		SetMessageHandler(NULL);
		_surface->setVisible(false);
	}
}

void Class526::sub466770() {
	setFileHash(0x34880040, 0, -1);
	SetSpriteCallback(&Class526::spriteUpdate466720);
}

Class527::Class527(NeverhoodEngine *vm, Sprite *class526)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _class526(class526) {

	// TODO createSurface3(200, dword_4B6768);
	createSurface(200, 640, 480); //TODO: Remeove once the line above is done
	_x = 427;
	_y = 433;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class527::handleMessage);
	setFileHash(0x461A1490, 0, -1);
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
		setFileHash1();
		SetSpriteCallback(NULL);
		SetMessageHandler(NULL);
		_surface->setVisible(false);
	}
}

void Class527::sub466970() {
	setFileHash(0x103B8020, 0, -1);
	SetSpriteCallback(&Class527::spriteUpdate466920);
}

Class528::Class528(NeverhoodEngine *vm, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x04551900, 100);
	SetUpdateHandler(&Class528::update);
	SetMessageHandler(&Class528::handleMessage);
	_newHashListIndex = -2;
	if (flag) {
		_flag = true;
		setFileHash(0x04551900, -1,- 1);
		_countdown = 48;
	} else {
		_flag = false;
		setFileHash1();
		_surface->setVisible(false);
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
		removeCallbacks();
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
	_surface->setVisible(true);
	setFileHash(0x04551900, 0, -1);
	_newHashListIndex = -2;
	_soundResource.play(calcHash("fxDoorOpen24"));
}

void Class528::sub466C50() {
	_flag = false;
	_surface->setVisible(true);
	setFileHash(0x04551900, -1, -1);
	_soundResource.play(calcHash("fxDoorClose24"));
	_playBackwards = true;
	SetAnimationCallback3(&Class528::sub466CB0);
}

void Class528::sub466CB0() {
	setFileHash1();
	_surface->setVisible(false);
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
	setFileHash(0x10E3042B, 0, -1);
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
	// TODO Sound1ChList_sub_407AF0(0x05331081);
}

uint32 Class489::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
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
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
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
			_parentScene->sendMessage(0x4826, 1, this);
		} else {
			_parentScene->sendMessage(0x4826, 0, this);
		}
		messageResult = 1;
		break;
	case 0x4807:
		_parentScene->sendMessage(0x4807, 0, this);
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
		_parentScene->sendMessage(0x1022, 990, this);
		break;
	case 0x482B:
		_parentScene->sendMessage(0x1022, 1010, this);
		break;
	}
	return messageResult;
}

uint32 Class489::handleMessage434B20(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
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
			_parentScene->sendMessage(0x1019, 0, this);
			incGlobalVar(0x04A105B3, -1);
			setGlobalVar(0x04A10F33, kClass489Items[getGlobalVar(0x04A105B3)].varIndex1);
		} else if ((int8)getGlobalVar(0x04A10F33) == _class489Item->varIndex1 && _class489Item->flag != 0) {
			_parentScene->sendMessage(0x1019, 1, this);
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
		_class525->sendMessage(0x483A, 0, this);
		setFileHash1();
		SetMessageHandler(&Sprite::handleMessage);
		SetSpriteCallback(NULL);
		_surface->setVisible(false);
	}
}

void Class489::sub434DD0() {
	SetSpriteCallback(NULL);
	SetMessageHandler(&Class489::handleMessage);
	setFileHash(0x10E3042B, 0, -1);
}

void Class489::sub434DF0() {
	_remX = getGlobalVar(0x04A10F33) * 108 + _class489Item->point.x;
	setFileHash(0x14A10137, 0, -1);
	SetSpriteCallback(&Class489::spriteUpdate434B60);
	SetMessageHandler(&Class489::handleMessage);
	_soundResource2.play(0xEC008474);
}

void Class489::sub434E60() {
	SetSpriteCallback(NULL);
	SetMessageHandler(&Class489::handleMessage434B20);
	setFileHash(0x80C32213, 0, -1);
	SetAnimationCallback3(&Class489::sub434E90);
}

void Class489::sub434E90() {
	SetSpriteCallback(NULL);
	SetMessageHandler(&Class489::handleMessage4348E0);
	setFileHash(0xD23B207F, 0, -1);
}

void Class489::sub434EC0() {
	setFileHash(0x50A80517, 0, -1);
	SetMessageHandler(&Class489::handleMessage434B20);
	SetSpriteCallback(NULL);
	SetAnimationCallback3(&Class489::sub434F40);
	setGlobalVar(0x12A10DB3, 1);
	_soundResource1.play(0xCC4A8456);
	// TODO Sound1ChList_addSoundResource(0x05331081, 0xCE428854, true);
	// TODO Sound1ChList_playLooping(0xCE428854);
}

void Class489::sub434F40() {
	_parentScene->sendMessage(0x480F, 0, this);
	setFileHash(0xD833207F, 0, -1);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Class489::handleMessage4348E0);
}

void Class489::sub434F80() {
	setFileHash(0x50A94417, 0, -1);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Class489::handleMessage434B20);
	SetAnimationCallback3(&Class489::sub434E90);
	setGlobalVar(0x12A10DB3, 0);
	_soundResource1.play(0xCC4A8456);
	// TODO Sound1ChList_deleteSoundByHash(0xCE428854);
}

void Class489::sub434FF0() {
	_remX = getGlobalVar(0x04A10F33) * 108 + _class489Item->point.x;
	setFileHash(0x22CB4A33, 0, -1);
	SetSpriteCallback(&Class489::spriteUpdate434B60);
	SetMessageHandler(&Class489::handleMessage434B20);
	SetAnimationCallback3(&Class489::sub434DF0);
}

void Class489::sub435040() {
	setGlobalVar(0x04A105B3, 4);
	setGlobalVar(0x04A10F33, 0);
	SetSpriteCallback(&Class489::sub434D80);
	SetMessageHandler(&Sprite::handleMessage);
	setFileHash(0x708D4712, 0, -1);
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

	_background = addBackground(new DirtyBackground(_vm, 0x08221FA5, 0, 0));
	_palette = new Palette(_vm, 0x08221FA5);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x21FA108A, NULL));
	
	// TODO _class427 = addSprite(new Class427(_vm, this, 0x980F3124, 0x12192892, 100, 0));
	_class525 = addSprite(new Class525(_vm));

	if (!getGlobalVar(0x01023818)) {
		_class526 = addSprite(new Class526(_vm, _class525));
		_class527 = addSprite(new Class527(_vm, _class525));
	}

	_sprite3 = addSprite(new StaticSprite(_vm, 0xA82BA811, 1100));
	addSprite(new StaticSprite(_vm, 0x0A116C60, 1100));
	_ssButton = addSprite(new SsCommonButtonSprite(_vm, this, 0xB84B1100, 100, 0));
	_sprite1 = addSprite(new StaticSprite(_vm, 0x38EA100C, 1005));
	_sprite2 = addSprite(new StaticSprite(_vm, 0x98D0223C, 1200));
	_sprite2->getSurface()->setVisible(false);

	if (which < 0) {
		_klayman = new KmScene1401(_vm, this, 380, 447);
		setMessageList(0x004B65C8);
		_sprite1->getSurface()->setVisible(false);
	} else if (which == 1) {
		_klayman = new KmScene1401(_vm, this, 0, 447);
		setMessageList(0x004B65D0);
		_sprite1->getSurface()->setVisible(false);
	} else if (which == 2) {
		_klayman = new KmScene1401(_vm, this, 660, 447);
		setMessageList(0x004B65D8);
		_sprite1->getSurface()->setVisible(false);
	} else {
		_klayman = new KmScene1401(_vm, this, 290, 413);
		setMessageList(0x004B65E8);
		_sprite1->getSurface()->setVisible(false);
	}
	addSprite(_klayman);

	if (getGlobalVar(0x04A105B3) == 2) {
		_class489 = addSprite(new Class489(_vm, this, _klayman, _class525));
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 6) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		} else if (getGlobalVar(0x04A10F33) == 0) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		}
		_class489->getSurface()->getClipRect().x1 = _sprite3->getSurface()->getDrawRect().x;
		_class489->getSurface()->getClipRect().y1 = _sprite2->getSurface()->getDrawRect().y;
		_class489->getSurface()->getClipRect().x2 = 640;
		_class489->getSurface()->getClipRect().y2 = 480;
	}
	
	_klayman->getSurface()->getClipRect().x1 = _sprite3->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

	if (which == 0 && _class489 && _class489->hasMessageHandler()) {
		_class489->sendMessage(0x482B, 0, this);
	}

	_class528 = addSprite(new Class528(_vm, _klayman, which == 1));

}

void Scene1401::update() {
	Scene::update();
	if (_class489 && !_flag && _class489->getY() < 360) {
		_sprite2->getSurface()->setVisible(true);
		_flag = true;
	} else {
		_sprite2->getSurface()->setVisible(false);
	}
}

uint32 Scene1401::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02144CB1) {
			_klayman->sendEntityMessage(0x1014, _class427, this);
		} else if (param.asInteger() == 0x402064D8) {
			_klayman->sendEntityMessage(0x1014, _ssButton, this);
		} else if (param.asInteger() == 0x01C66840) {
			if (_class528->hasMessageHandler() && _class528->sendMessage(0x2001, 0, this) != 0) {
				setMessageList(0x004B6690);
			} else {
				setMessageList(0x004B66B0);
			}
		}
		break;
	case 0x1019:
		if (param.asInteger() != 0) {
			_parentModule->sendMessage(0x1009, 2, this);
		} else {
			_parentModule->sendMessage(0x1009, 1, this);
		}			
		break;
	case 0x480B:
		if (sender == _class427) {
			_class525->sendMessage(0x2000, 0, this);
			if (!getGlobalVar(0x01023818)) {
				_class526->sendMessage(0x4839, 0, this);
				_class527->sendMessage(0x4839, 0, this);
				setGlobalVar(0x01023818, 1);
			}
			if (_class489 && _class489->getX() > 404 && _class489->getX() < 504) {
				_class489 ->sendMessage(0x4839, 0, this);
			}
		} else if (sender == _ssButton) {
			_ssButton->sendMessage(0x4808, 0, this);
		}
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_class489->sendMessage(0x480C, _klayman->getX() > _class489->getX() ? 1 : 0, this) != 0) {
				_klayman->sendEntityMessage(0x1014, _class489, this);
				setMessageList2(0x004B6658);
			} else {
				setMessageList2(0x004B65F0);
			}
		}						
		break;
	case 0x482A:
		_sprite1->getSurface()->setVisible(true);
		if (_class489) {
			_class489->sendMessage(0x482B, 0, this);
		}
		break;
	case 0x482B:
		_sprite1->getSurface()->setVisible(false);
		if (_class489) {
			_class489->sendMessage(0x482A, 0, this);
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
		setFileHash(0x20060259, 0, -1);
		_soundResource1.play(0x419014AC);
		_soundResource2.load(0x61901C29);
	} else if (which == 1) {
		setFileHash(0x210A0213, 0, -1);
		_soundResource1.play(0x41809C6C);
	} else {
		setFileHash(0x20060259, 0, -1);
		_soundResource2.load(0x61901C29);
		_newHashListIndex = -2;
	}
}

uint32 Class482::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		_soundResource2.play();
		setFileHash(0x20060259, -1, -1);
		_playBackwards = true;
		SetAnimationCallback3(&Class482::sub428530);
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class482::sub428500() {
	_parentScene->sendMessage(0x2000, 0, this);
	setFileHash1();
	_surface->setVisible(false);
}

void Class482::sub428530() {
	_parentScene->sendMessage(0x2001, 0, this);
	setFileHash1();
	_surface->setVisible(false);
}

void Class482::sub428560() {
	_parentScene->sendMessage(0x2003, 0, this);
	setFileHash1();
}

Scene1402::Scene1402(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag(false), _class482(NULL), _class489(NULL) {

	SetMessageHandler(&Scene1402::handleMessage);

	_background = addBackground(new Background(_vm, 0x231482F0, 0, 0));
	_background->getSurface()->getDrawRect().y = -10;
	// TODO g_screen->field_26 = 0;
	_palette = new Palette(_vm, 0x231482F0);
	_palette->addPalette(0x91D3A391, 0, 64, 0);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x482F4239, NULL));

	_class454_1 = addSprite(new Class454(_vm, 0x15402D64, 1100));
	_class454_2 = addSprite(new Class454(_vm, 0x10A02120, 1100));
	_class454_3 = addSprite(new Class454(_vm, 0x60882BE0, 1100));

	if (getGlobalVar(0x70A1189C))
		setRectList(0x004B0C48);
	else
		setRectList(0x004B0C98);

	if (which < 0) {
		_klayman = new KmScene1402(_vm, this, 377, 391);
		setMessageList(0x004B0B48);
		if (!getGlobalVar(0x70A1189C)) {
			_class482 = addSprite(new Class482(_vm, this, 0));
		}
	} else if (which == 1) {
		_klayman = new KmScene1402(_vm, this, 42, 391);
		setMessageList(0x004B0B50);
	} else if (which == 2) {
		_klayman = new KmScene1402(_vm, this, 377, 391);
		setMessageList(0x004B0B60);
		_klayman->setDoDeltaX(1);
		if (getGlobalVar(0x70A1189C)) {
			_class482 = addSprite(new Class482(_vm, this, 1));
			clearRectList();
			_mouseCursor->getSurface()->setVisible(false);
			sub428220();
		} else {
			_class482 = addSprite(new Class482(_vm, this, 0));
		}
	} else {
		_klayman = new KmScene1402(_vm, this, 513, 391);
		setMessageList(0x004B0B58);
		if (!getGlobalVar(0x70A1189C)) {
			_class482 = addSprite(new Class482(_vm, this, 2));
			sub428220();
		}
	}
	addSprite(_klayman);

	if (_class482) {
		_class482->getSurface()->getClipRect().x1 = 0;
		_class482->getSurface()->getClipRect().y1 = 0;
		_class482->getSurface()->getClipRect().x2 = 640;
		_class482->getSurface()->getClipRect().y2 = _class454_3->getSurface()->getDrawRect().y + _class454_3->getSurface()->getDrawRect().height;
	}

	if (getGlobalVar(0x4A105B3) == 1) {
		_class489 = addSprite(new Class489(_vm, this, _klayman, 0));
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x4A10F33) == 4) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		} else if (getGlobalVar(0x4A10F33) == 0) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		}
		_class489->getSurface()->getClipRect().x1 = _class454_1->getSurface()->getDrawRect().x;
		_class489->getSurface()->getClipRect().y1 = 0;
		_class489->getSurface()->getClipRect().x2 = _class454_2->getSurface()->getDrawRect().x;
		_class489->getSurface()->getClipRect().y2 = _class454_3->getSurface()->getDrawRect().y + _class454_3->getSurface()->getDrawRect().height;
	}

	_klayman->getSurface()->getClipRect().x1 = _class454_1->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = _class454_2->getSurface()->getDrawRect().x + _class454_2->getSurface()->getDrawRect().width; 
	_klayman->getSurface()->getClipRect().y2 = _class454_3->getSurface()->getDrawRect().y + _class454_3->getSurface()->getDrawRect().height;
	
}

void Scene1402::update() {
	if (_flag) {
		_background->getSurface()->getDrawRect().y = _vm->_rnd->getRandomNumber(10 - 1) - 10;
		// TODO g_screen->field_26 = -10 - _background->getSurface()->getDrawRect().y;
	} else {
		_background->getSurface()->getDrawRect().y = -10;
		// TODO g_screen->field_26 = 0;
		SetUpdateHandler(&Scene::update);
	}
	Scene::update();
	if (_class482) {
		_class482->getSurface()->getClipRect().x1 = 0;
		_class482->getSurface()->getClipRect().y1 = 0;
		_class482->getSurface()->getClipRect().x2 = 640;
		_class482->getSurface()->getClipRect().y2 = _class454_3->getSurface()->getDrawRect().y + _class454_3->getSurface()->getDrawRect().height;
	}
	_klayman->getSurface()->getClipRect().x1 = _class454_1->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = _class454_2->getSurface()->getDrawRect().x + _class454_2->getSurface()->getDrawRect().width; 
	_klayman->getSurface()->getClipRect().y2 = _class454_3->getSurface()->getDrawRect().y + _class454_3->getSurface()->getDrawRect().height;
}

uint32 Scene1402::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00F43389) {
			if (getGlobalVar(0x70A1189C)) {
				_parentModule->sendMessage(0x1009, 0, this);
			} else {
				clearRectList();
				_klayman->getSurface()->setVisible(false);
				_mouseCursor->getSurface()->setVisible(false);
				_class482->sendMessage(0x2002, 0, this);
				sub428220();
			}
		}
		break;
	case 0x1019:
		if (param.asInteger()) {
			_parentModule->sendMessage(0x1009, 0, this);
		} else {
			_parentModule->sendMessage(0x1009, 1, this);
		}
		break;
	case 0x2000:
		sub428230();
		_mouseCursor->getSurface()->setVisible(true);
		setRectList(0x004B0C48);
		break;
	case 0x2001:
		sub428230();
		_parentModule->sendMessage(0x1009, 0, this);
		break;
	case 0x2003:
		sub428230();
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_class489->sendMessage(0x408C, _klayman->getX() > _class489->getX() ? 1 : 0, this) != 0) {
				_klayman->sendEntityMessage(0x1014, _class489, this);
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
		sendMessage(0x1019, 0, this);
	} else {
		_x += xdelta;
		processDelta();
	}
}

void AsScene1407Mouse::upGoThroughHole() {
	if (_countdown != 0 && (--_countdown == 0)) {
		SetUpdateHandler(&AnimatedSprite::update);
		removeCallbacks();
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
		removeCallbacks();
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
	setFileHash(0x72215194, 0, -1);
	SetSpriteCallback(NULL);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
}

void AsScene1407Mouse::stWalkToDest() {
	if (_walkDestX != _x) {
		setDoDeltaX(_walkDestX < _x ? 1 : 0);
		setFileHash(0x22291510, 0, -1);
		SetSpriteCallback(&AsScene1407Mouse::suWalkTo);
		SetMessageHandler(&AsScene1407Mouse::handleMessage);
		SetAnimationCallback3(&AsScene1407Mouse::stIdleLookAtGoodHole);
	}
}

void AsScene1407Mouse::stWalkToHole() {
	setDoDeltaX(_walkDestX < _x ? 1 : 0);
	setFileHash(0x22291510, 0, -1);
	SetSpriteCallback(&AsScene1407Mouse::suWalkTo);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
	SetAnimationCallback3(&AsScene1407Mouse::stGoThroughHole);
}

void AsScene1407Mouse::stGoThroughHole() {
	setFileHash(0x72215194, 0, -1);
	SetSpriteCallback(NULL);
	SetMessageHandler(NULL);
	SetUpdateHandler(&AsScene1407Mouse::upGoThroughHole);
	SetAnimationCallback3(&AsScene1407Mouse::stArriveAtHole);
	_surface->setVisible(false);
	_countdown = 12;
}

void AsScene1407Mouse::stArriveAtHole() {
	_currSectionIndex = kScene1407MouseHoles[_nextHoleIndex].sectionIndex;
	_x = kScene1407MouseHoles[_nextHoleIndex].x;
	_y = kScene1407MouseFloorY[kScene1407MouseHoles[_nextHoleIndex].floorIndex];
	if (_nextHoleIndex == 1) {
		_parentScene->sendMessage(0x2000, 0, this);
		_walkDestX = 512;
		stWalkToDest();
		_surface->setVisible(true);
	} else {
		_walkDestX = _x + 14;
		stWalkToDest();
		_surface->setVisible(true);
	}
}

Scene1407::Scene1407(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _puzzleSolvedCountdown(0),
	_resetButtonCountdown(0) {

	_surfaceFlag = true;

	SetMessageHandler(&Scene1407::handleMessage);
	SetUpdateHandler(&Scene1407::update);

	_background = addBackground(new DirtyBackground(_vm, 0x00442225, 0, 0));
	_palette = new Palette(_vm, 0x00442225);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse435(_vm, 0x4222100C, 20, 620));

	_asMouse = addSprite(new AsScene1407Mouse(_vm, this));
	_ssResetButton = addSprite(new StaticSprite(_vm, 0x12006600, 100));
	_ssResetButton->getSurface()->setVisible(false); 

}

void Scene1407::update() {
	Scene::update();
	if (_puzzleSolvedCountdown != 0 && (--_puzzleSolvedCountdown == 0)) {
		_parentModule->sendMessage(0x1009, 1, this);
	} else if (_resetButtonCountdown != 0 && (--_resetButtonCountdown == 0)) {
		_ssResetButton->getSurface()->setVisible(false);
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
				_parentModule->sendMessage(0x1009, 0, this);
			} else if (param.asPoint().x >= 75 && param.asPoint().x <= 104 &&
				param.asPoint().y >= 62 && param.asPoint().y <= 90) {
				// The reset button was clicked
				_asMouse->sendMessage(0x2001, 0, this);
				_ssResetButton->getSurface()->setVisible(true);
				_soundResource.play(0x44045000);
				_resetButtonCountdown = 12;
			} else {
				// Handle the mouse
				_asMouse->sendMessage(messageNum, param, this);
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
		_mouseCursor->getSurface()->setVisible(false);
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

	_background = addBackground(new DirtyBackground(_vm, 0x2110A234, 0, 0));
	_palette = new Palette(_vm, 0x2110A234);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x0A230219, NULL));

	_class401_1 = addSprite(new StaticSprite(_vm, 0x01102A33, 100));
	_class401_1->getSurface()->setVisible(false);

	_class401_2 = addSprite(new StaticSprite(_vm, 0x04442520, 995));
	    
	_class401_3 = addSprite(new StaticSprite(_vm, 0x08742271, 995));

	_asTape1 = new AsScene1201Tape(_vm, this, 12, 1100, 201, 468, 0x9148A011);
	addSprite(_asTape1);
	_vm->_collisionMan->addSprite(_asTape1);
	_asTape1->setRepl(64, 0);

	_asTape2 = new AsScene1201Tape(_vm, this, 16, 1100, 498, 468, 0x9048A093);
	addSprite(_asTape2);
	_vm->_collisionMan->addSprite(_asTape2);
	_asTape2->setRepl(64, 0);

	if (which < 0) {
		_klayman = new KmScene1403(_vm, this, 380, 463);
		setMessageList(0x004B1F18);
	} else {
		_klayman = new KmScene1403(_vm, this, 640, 463);
		setMessageList(0x004B1F20);
	}
	addSprite(_klayman);
	_klayman->setRepl(64, 0);

	if (getGlobalVar(0x04A105B3) == 4) {
		Class489 *class489;
		class489 = new Class489(_vm, this, _klayman, 0);
		_class489 = class489;
		addSprite(_class489);
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 4) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B1F70);
		}
		_class489->getSurface()->getClipRect().x1 = 0;
		_class489->getSurface()->getClipRect().y1 = 0;
		_class489->getSurface()->getClipRect().x2 = 640;
		_class489->getSurface()->getClipRect().y2 = _class401_2->getSurface()->getDrawRect().y + _class401_2->getSurface()->getDrawRect().height;
		class489->setRepl(64, 0);
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
			_klayman->sendEntityMessage(0x1014, _class489, this);
			setRectList(0x004B1FF8);
			_flag = false;
		}
		break;
	case 0x1019:
		_parentModule->sendMessage(0x1009, 0, this);
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
		_class401_1->getSurface()->setVisible(false);
		break;
	case 0x480F:
		_class401_1->getSurface()->setVisible(true);
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (param.asInteger() == 1) {
				_klayman->sendEntityMessage(0x1014, _class489, this);
				setMessageList2(0x004B1F88);
			} else if (_class489->sendMessage(0x480C, _klayman->getX() > _class489->getX() ? 1 : 0, this) != 0) {
				_klayman->sendEntityMessage(0x1014, _class489, this);
				setMessageList2(0x004B1F58);
			} else {
				setMessageList2(0x004B1F28);
			}
		} else if (sender == _asTape1 || sender == _asTape2) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (_messageListStatus != 2) {
				_klayman->sendEntityMessage(0x1014, sender, this);
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

	_background = addBackground(new DirtyBackground(_vm, 0xAC0B006F, 0, 0));
	_palette = new Palette(_vm, 0xAC0B006F);
	_palette->addPalette(0x00801510, 0, 65, 0);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0xB006BAC8, NULL));

	if (getGlobalVar(0x13382860) == 5) {
		// TODO _class545 = addSprite(new Class545(_vm, this, 2, 1100, 267, 411));
		// TODO _vm->_collisionMan->addSprite(_class545);
	}

	_sprite1 = addSprite(new StaticSprite(_vm, 0x1900A1F8, 1100));

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 14, 1100, 281, 411, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape);

	if (which < 0) {
		_klayman = new KmScene1404(_vm, this, 376, 406);
		setMessageList(0x004B8C28);
	} else if (which == 1) {
		_klayman = new KmScene1404(_vm, this, 376, 406);
		setMessageList(0x004B8C30);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			_klayman = new KmScene1404(_vm, this, 347, 406);
			_klayman->setDoDeltaX(1);
		} else {
			_klayman = new KmScene1404(_vm, this, 187, 406);
		}
		setMessageList(0x004B8D28);
	} else {
		_klayman = new KmScene1404(_vm, this, 30, 406);
		setMessageList(0x004B8C38);
	}
	addSprite(_klayman);

	if (getGlobalVar(0x04A105B3) == 3) {
		_class489 = addSprite(new Class489(_vm, this, _klayman, 0));
		_vm->_collisionMan->addSprite(_class489);
		if (getGlobalVar(0x04A10F33) == 0) {
			_klayman->sendEntityMessage(0x1014, _class489, this);
			_klayman->setX(_class489->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B8CB8);
		}
		_class489->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
		_class489->getSurface()->getClipRect().y1 = 0;
		_class489->getSurface()->getClipRect().x2 = 640;
		_class489->getSurface()->getClipRect().y2 = 480;
	}

	_klayman->getSurface()->getClipRect().x1 = _sprite1->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

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
		_parentModule->sendMessage(0x1009, 0, this);
		break;
	case 0x4826:
		if (sender == _class489) {
			if (_class489->sendMessage(0x480C, _klayman->getX() > _class489->getX() ? 1 : 0, this) != 0) {
				_klayman->sendEntityMessage(0x1014, _class489, this);
				setMessageList2(0x004B8CA0);
			} else {
				setMessageList2(0x004B8C40);
			}
		} else if (sender == _asTape && _messageListStatus != 2) {
			_klayman->sendEntityMessage(0x1014, _asTape, this);
			setMessageList(0x004B8CD0);
		} else if (sender == _class545 && _messageListStatus != 2) {
			_klayman->sendEntityMessage(0x1014, _class545, this);
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
	_surface->setVisible(false);
	if (getSubVar(0xCCE0280F, _index))
		_countdown = _vm->_rnd->getRandomNumber(36 - 1) + 1;
	SetUpdateHandler(&AsScene1405Tile::update);
	SetMessageHandler(&AsScene1405Tile::handleMessage);
	
	debug("getSubVar(0x0C65F80B, _index) = %d", getSubVar(0x0C65F80B, _index));
	
	setFileHash(0x844B805C, getSubVar(0x0C65F80B, _index), -1);
	_newHashListIndex = (int16)getSubVar(0x0C65F80B, _index);
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
			_parentScene->sendMessage(0x2000, _index, this);
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
		_surface->setVisible(true);
	}
}

void AsScene1405Tile::hide() {
	if (_flag) {
		_flag = false;
		_soundResource.play();
		_surface->setVisible(false);
	}
}

Scene1405::Scene1405(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _selectFirstTile(true),
	_tilesLeft(48), _countdown(0) {

	_vm->gameModule()->initScene1405Vars();
	_surfaceFlag = true;
	
	_background = addBackground(new DirtyBackground(_vm, 0x0C0C007D, 0, 0));
	_palette = new Palette(_vm, 0x0C0C007D);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse435(_vm, 0xC00790C8, 20, 620));
	
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
		_tiles[index] = new AsScene1405Tile(_vm, this, index);
		addSprite(_tiles[index]);
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
			_parentModule->sendMessage(0x1009, 0, this);
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
