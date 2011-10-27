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

#include "neverhood/module2100.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1200.h"

namespace Neverhood {

Module2100::Module2100(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	// TODO Music18hList_add(0x10A10C14, 0x11482B95);
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(0, 0);
	} else if (which == 2) {
		createScene(0, 3);
	} else {
		createScene(0, 1);
	}

}

Module2100::~Module2100() {
	// TODO Music18hList_deleteGroup(0x10A10C14);
}

void Module2100::createScene(int sceneNum, int which) {
	debug("Module2100::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		// TODO Music18hList_play(0x11482B95, 0, 1, 1);
		_childObject = new Scene2101(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module2100::updateScene);
	_childObject->handleUpdate();
}

void Module2100::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				setGlobalVar(0x2090590C, 1);
				leaveModule(0);
			} else {
				leaveModule(1);
			}
			break;
		}
	}
}

// Scene2101

Class538::Class538(NeverhoodEngine *vm, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm) {

	// TODO createSurface3(100, dword_4B9018);
	createSurface(100, 640, 480); //TODO: Remove once the line above is done
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class538::handleMessage);
	if (flag) {
		startAnimation(0x0C202B9C, -1, -1);
		_newStickFrameIndex = -2;
	} else {
		setVisible(false);
	}
}

uint32 Class538::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		openDoor();
		break;
	case 0x4809:
		closeDoor();
		break;
	}
	return messageResult;
}

void Class538::openDoor() {
	startAnimation(0x0C202B9C, 0, -1);
	_newStickFrameIndex = -2;
	setVisible(true);
	_soundResource.play(calcHash("fxDoorOpen32"));
}

void Class538::closeDoor() {
	startAnimation(0xC222A8D4, 0, -1);
	_newStickFrameIndex = -2;
	setVisible(true);
	NextState(&Class538::hide);
	_soundResource.play(calcHash("fxDoorClose32"));
}

void Class538::hide() {
	stopAnimation();
	setVisible(false);
}

Class539::Class539(NeverhoodEngine *vm, Sprite *klayman)
	: AnimatedSprite(vm, 1400), _klayman(klayman) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class539::handleMessage);
	createSurface(1200, 88, 165);
	setVisible(false);
}

uint32 Class539::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		_x = _klayman->getX();
		_y = _klayman->getY() - 132;
		startAnimation(0x0422255A, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

Class427::Class427(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash)
	: StaticSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _countdown(0),
	_fileHash1(fileHash1), _fileHash2(fileHash2), _soundFileHash(soundFileHash) {

	SetUpdateHandler(&Class427::update);
	SetMessageHandler(&Class427::handleMessage);
	if (_soundFileHash == 0)
		_soundFileHash = 0x44141000;
	createSurface(1010, 61, 30);
	if (_fileHash1) {
		load(_fileHash1, true, true);
		StaticSprite::update();
	} else
		setVisible(false);
}

void Class427::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sendMessage(_parentScene, 0x1022, 1010);
		if (_fileHash1) {
			load(_fileHash1, true, true);
			StaticSprite::update();
		} else
			setVisible(false);
	}
}	
	
uint32 Class427::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		sendMessage(_parentScene, 0x480B, 0);
		setVisible(true);
		sendMessage(_parentScene, 0x1022, 990);
		load(_fileHash2, true, true);
		StaticSprite::update();
		_countdown = 16;
		_soundResource.play(_soundFileHash);
		break;
	}
	return messageResult;
}

Scene2101::Scene2101(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2101::handleMessage);
	SetUpdateHandler(&Scene2101::update);
	
	setBackground(0x44242305);
	setPalette(0x44242305);
	insertMouse433(0x4230144A);

	insertStaticSprite(0x00502330, 1100);
	_sprite1 = insertStaticSprite(0x78492010, 1100);
	_class427 = insertSprite<Class427>(this, 0x72427010, 0x32423010, 200, 0);
	_asTape1 = insertSprite<AsScene1201Tape>(this, 18, 1100, 412, 443, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape1);
	_asTape2 = insertSprite<AsScene1201Tape>(this, 11, 1100, 441, 443, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape2);
	
	if (which < 0) {
		insertKlayman<KmScene2101>(380, 438);
		setMessageList(0x004B8E48);
		sendMessage(this, 0x2000, 0);
		_class538 = insertSprite<Class538>(false);
		_value1 = 1;
		_countdown1 = 0;
	} else if (which == 1) {
		insertKlayman<KmScene2101>(640, 438);
		setMessageList(0x004B8E50);
		sendMessage(this, 0x2000, 0);
		_class538 = insertSprite<Class538>(true);
		_value1 = 2;
		_countdown1 = 48;
	} else if (which == 2) {
		insertKlayman<KmScene2101>(115, 438);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B8F58);
		sendMessage(this, 0x2000, 1);
		_class538 = insertSprite<Class538>(false);
		_value1 = 1;
		_countdown1 = 0;
	} else if (which == 3) {
		insertKlayman<KmScene2101>(115, 438);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B8EB0);
		sendMessage(this, 0x2000, 1);
		_class538 = insertSprite<Class538>(false);
		_value1 = 1;
		_countdown1 = 0;
	} else {
		insertKlayman<KmScene2101>(115, 438);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B8EA0);
		sendMessage(this, 0x2000, 1);
		_class538 = insertSprite<Class538>(false);
		_value1 = 1;
		_countdown1 = 0;
	}
	
	_class539 = insertSprite<Class539>(_klayman);
	_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), 480);
	
}

void Scene2101::update() {
	if (_countdown1 != 0) {
		if (_value1 == 2) {
			if (--_countdown1 == 0) {
				sendMessage(_class538, 0x4809, 0);
				_value1 = 1;
			}
		} else {
			if (_klayman->getX() > 575)
				_messageListFlag  = false;
			if (--_countdown1 == 0) {
				if (_klayman->getX() < 480) {
					sendMessage(_class538, 0x4809, 0);
					_value1 = 1;
				} else if (_klayman->getX() >= 480 && _klayman->getX() <= 575) {
					_klayman->setDoDeltaX(0);
					setMessageList2(0x004B8F48);
					sendMessage(_class538, 0x4809, 0);
					sendMessage(_class539, 0x2001, 0);
					_value1 = 1;
				}
			}
		}
	} else if (_value1 == 1 && _messageValue >= 0 && _klayman->getX() > 470 /* TODO ! && _messageList2 != 0x004B8F48*/) {
		setMessageList2(0x004B8F50);
	}
	Scene::update();
}

uint32 Scene2101::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02144CB1) {
			sendEntityMessage(_klayman, 0x1014, _class427);
		} else if (param.asInteger() == 0x21E64A00) {
			if (_value1 == 0) {
				setMessageList(0x004B8E80);
			} else {
				setMessageList(0x004B8EC8);
			}
		} else if (param.asInteger() == 0x41442820) {
			messageList402220();
		}
		break;
	case 0x2000:
		if (param.asInteger() != 0) {
			setRectList(0x004B9008);
			_klayman->setKlaymanIdleTable3();
		} else {
			setRectList(0x004B8FF8);
			_klayman->setKlaymanIdleTable1();
		}
		break;
	case 0x480B:
		if (sender == _class427 && _value1 == 1) {
			sendMessage(_class538, 0x4808, 0);
			_value1 = 0;
			_countdown1 = 90;
		}
		break;
	case 0x4826:
		if (sender == _asTape1 || sender == _asTape2) {
			if (_klayman->getX() >= 228 && _klayman->getX() <= 500) {
				sendEntityMessage(_klayman, 0x1014, sender);
				setMessageList(0x004B8F78);
			} else if (_klayman->getX() < 228) {
				setMessageList2(0x004B8F00);
			}
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
