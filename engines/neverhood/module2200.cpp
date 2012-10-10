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

#include "neverhood/module2200.h"
#include "neverhood/module1000.h"
#include "neverhood/module1200.h"
#include "neverhood/gamemodule.h"
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

Module2200::Module2200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module2200(%d)", which);

	_vm->_soundMan->addMusic(0x11391412, 0x601C908C); 

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else {
		createScene(0, 0);
	}

}

Module2200::~Module2200() {
	_vm->_soundMan->deleteGroup(0x11391412);
}

void Module2200::createScene(int sceneNum, int which) {
	// CHECKME if this can be used regardless of the new sceneNum
	if (sceneNum == 7 && which >= 0)
		_vm->gameState().which = _vm->gameState().sceneNum; 
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene2201(_vm, this, which);
		break;
	case 1:
		_vm->_soundMan->startMusic(0x601C908C, 0, 2);
		_childObject = new Scene2202(_vm, this, which);
		break;
	case 2:
		_vm->_soundMan->startMusic(0x601C908C, 0, 2);
		_childObject = new Scene2203(_vm, this, which);
		break;
	case 3:
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 3);
		break;
	case 4:
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new Scene2205(_vm, this, which);
		break;
	case 5:
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new Scene2206(_vm, this, which);
		break;
	case 6:
		_childObject = new Scene2207(_vm, this, which);
		break;
	case 7:
		_childObject = new Scene2208(_vm, this, which);
		break;
	case 8:
		_childObject = new Scene2208(_vm, this, which);
		break;
	case 9:
		createHallOfRecordsScene(which, 0x004B7180);
		break;
	case 10:
		createHallOfRecordsScene(which, 0x004B7198);
		break;
	case 11:
		createHallOfRecordsScene(which, 0x004B71B0);
		break;
	case 12:
		createHallOfRecordsScene(which, 0x004B71C8);
		break;
	case 13:
		createHallOfRecordsScene(which, 0x004B71E0);
		break;
	case 14:
		createHallOfRecordsScene(which, 0x004B71F8);
		break;
	case 15:
		createHallOfRecordsScene(which, 0x004B7210);
		break;
	case 16:
		createHallOfRecordsScene(which, 0x004B7228);
		break;
	case 17:
		createHallOfRecordsScene(which, 0x004B7240);
		break;
	case 18:
		createHallOfRecordsScene(which, 0x004B7258);
		break;
	case 19:
		createHallOfRecordsScene(which, 0x004B7270);
		break;
	case 20:
		createHallOfRecordsScene(which, 0x004B7288);
		break;
	case 21:
		createHallOfRecordsScene(which, 0x004B72A0);
		break;
	case 22:
		createHallOfRecordsScene(which, 0x004B72B8);
		break;
	case 23:
		createHallOfRecordsScene(which, 0x004B72D0);
		break;
	case 24:
		createHallOfRecordsScene(which, 0x004B72E8);
		break;
	case 25:
		createHallOfRecordsScene(which, 0x004B7300);
		break;
	case 26:
		createHallOfRecordsScene(which, 0x004B7318);
		break;
	case 27:
		createHallOfRecordsScene(which, 0x004B7330);
		break;
	case 28:
		createHallOfRecordsScene(which, 0x004B7348);
		break;
	case 29:
		createHallOfRecordsScene(which, 0x004B7360);
		break;
	case 30:
		createHallOfRecordsScene(which, 0x004B7378);
		break;
	case 31:
		createHallOfRecordsScene(which, 0x004B7390);
		break;
	case 32:
		createHallOfRecordsScene(which, 0x004B73A8);
		break;
	case 33:
		createHallOfRecordsScene(which, 0x004B73C0);
		break;
	case 34:
		createHallOfRecordsScene(which, 0x004B73D8);
		break;
	case 35:
		createHallOfRecordsScene(which, 0x004B73F0);
		break;
	case 36:
		createHallOfRecordsScene(which, 0x004B7408);
		break;
	case 37:
		createHallOfRecordsScene(which, 0x004B7420);
		break;
	case 38:
		createHallOfRecordsScene(which, 0x004B7438);
		break;
	case 39:
		createHallOfRecordsScene(which, 0x004B7450);
		break;
	case 40:
		createHallOfRecordsScene(which, 0x004B7468);
		break;
	case 41:
		_childObject = new Scene2242(_vm, this, which);
		break;
	case 42:
		createHallOfRecordsScene(which, 0x004B7480);
		break;
	case 43:
		createHallOfRecordsScene(which, 0x004B7498);
		break;
	case 44:
		createHallOfRecordsScene(which, 0x004B74B0);
		break;
	case 45:
		createHallOfRecordsScene(which, 0x004B74C8);
		break;
	case 46:
		_childObject = new Scene2247(_vm, this, which);
		break;
	case 47:
		if (!getGlobalVar(0x98109F12)) {
			if (getGlobalVar(0x4D080E54))
				createStaticScene(0x83110287, 0x10283839);
			else
				createStaticScene(0x83412B9D, 0x12B9983C);
		} else {
			if (getGlobalVar(0x4D080E54))
				createStaticScene(0x48632087, 0x3208348E);
			else
				createStaticScene(0x08C74886, 0x74882084);
		}
		break;
	}
	SetUpdateHandler(&Module2200::updateScene);
	_childObject->handleUpdate();
}

#define HallOfRecordsSceneLink(nextSceneNum, prevSceneNum) \
	if (_moduleResult == 1) createScene(nextSceneNum, 0); else if (_moduleResult == 2) createScene(7, 0); else createScene(prevSceneNum, 1)

void Module2200::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(2, 0);
			} else if (_moduleResult == 2) {
				createScene(1, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			createScene(0, 2);
			break;
		case 2:
			if (_moduleResult == 1) {
				createScene(4, 0);
			} else if (_moduleResult == 2) {
				createScene(3, 0);
			} else {
				createScene(0, 1);
			}
			break;
		case 3:
			createScene(2, 2);
			break;
		case 4:
			if (_moduleResult == 1) {
				createScene(5, 0);
			} else if (_moduleResult == 2) {
				createScene(4, 2);
			} else {
				createScene(2, 1);
			}
			break;
		case 5:
			if (_moduleResult == 1) {
				createScene(46, 0);
			} else if (_moduleResult == 2) {
				createScene(6, 0);
			} else if (_moduleResult == 3) {
				createScene(8, 0);
			} else {
				createScene(4, 1);
			}
			break;
		case 6:
			createScene(5, 2);
			break;
		case 7:
			createScene(_vm->gameState().which, 2);
			break;
		case 8:
			createScene(5, 3);
			break;
		case 9:
			HallOfRecordsSceneLink(10, 46);
			break;
		case 10:
			HallOfRecordsSceneLink(11, 9);
			break;
		case 11:
			HallOfRecordsSceneLink(12, 10);
			break;
		case 12:
			HallOfRecordsSceneLink(13, 11);
			break;
		case 13:
			HallOfRecordsSceneLink(14, 12);
			break;
		case 14:
			HallOfRecordsSceneLink(15, 13);
			break;
		case 15:
			HallOfRecordsSceneLink(16, 14);
			break;
		case 16:
			HallOfRecordsSceneLink(17, 15);
			break;
		case 17:
			HallOfRecordsSceneLink(18, 16);
			break;
		case 18:
			HallOfRecordsSceneLink(19, 17);
			break;
		case 19:
			HallOfRecordsSceneLink(20, 18);
			break;
		case 20:
			HallOfRecordsSceneLink(21, 19);
			break;
		case 21:
			HallOfRecordsSceneLink(22, 20);
			break;
		case 22:
			HallOfRecordsSceneLink(23, 21);
			break;
		case 23:
			HallOfRecordsSceneLink(24, 22);
			break;
		case 24:
			HallOfRecordsSceneLink(25, 23);
			break;
		case 25:
			HallOfRecordsSceneLink(26, 24);
			break;
		case 26:
			HallOfRecordsSceneLink(27, 25);
			break;
		case 27:
			HallOfRecordsSceneLink(28, 26);
			break;
		case 28:
			HallOfRecordsSceneLink(29, 27);
			break;
		case 29:
			HallOfRecordsSceneLink(30, 28);
			break;
		case 30:
			HallOfRecordsSceneLink(31, 29);
			break;
		case 31:
			HallOfRecordsSceneLink(32, 30);
			break;
		case 32:
			HallOfRecordsSceneLink(33, 31);
			break;
		case 33:
			HallOfRecordsSceneLink(34, 32);
			break;
		case 34:
			HallOfRecordsSceneLink(42, 33);
			break;
		case 35:
			HallOfRecordsSceneLink(36, 45);
			break;
		case 36:
			HallOfRecordsSceneLink(37, 35);
			break;
		case 37:
			HallOfRecordsSceneLink(38, 36);
			break;
		case 38:
			HallOfRecordsSceneLink(39, 37);
			break;
		case 39:
			HallOfRecordsSceneLink(40, 38);
			break;
		case 40:
			HallOfRecordsSceneLink(41, 39);
			break;
		case 41:
			HallOfRecordsSceneLink(47, 40);
			break;
		case 42:
			HallOfRecordsSceneLink(43, 34);
			break;
		case 43:
			HallOfRecordsSceneLink(44, 42);
			break;
		case 44:
			HallOfRecordsSceneLink(45, 43);
			break;
		case 45:
			HallOfRecordsSceneLink(35, 44);
			break;
		case 46:
			HallOfRecordsSceneLink(9, 5);
			break;
		case 47:
			createScene(41, 1);
			break;
		}
	}
}

#undef HallOfRecordsSceneLink
			
void Module2200::createHallOfRecordsScene(int which, uint32 sceneInfo140Id) {
	_childObject = new HallOfRecordsScene(_vm, this, which, sceneInfo140Id);
}

// Scene2201

AsScene2201CeilingFan::AsScene2201CeilingFan(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 403;
	_y = 259;
	createSurface(100, 233, 96);
	startAnimation(0x8600866, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
}

AsScene2201Door::AsScene2201Door(NeverhoodEngine *vm, Klayman *klayman, Sprite *ssDoorLight, bool flag1)
	: AnimatedSprite(vm, 1100), _klayman(klayman), _ssDoorLight(ssDoorLight), _countdown(0), _doorOpen(flag1) {

	_x = 408;
	_y = 290;	
	createSurface(900, 63, 266);
	SetUpdateHandler(&AsScene2201Door::update);
	SetMessageHandler(&AsScene2201Door::handleMessage);
	if (_doorOpen) {
		startAnimation(0xE2CB0412, -1, -1);
		_countdown = 48;
		_newStickFrameIndex = -2;
	} else {
		startAnimation(0xE2CB0412, 0, -1);
		_newStickFrameIndex = 0;
		_ssDoorLight->setVisible(false);
	}
}

void AsScene2201Door::update() {
	if (_countdown != 0 && _doorOpen && (--_countdown == 0)) {
		stCloseDoor();
	}
	AnimatedSprite::update();
}

uint32 AsScene2201Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x11001090) {
			if (_doorOpen)
				_ssDoorLight->setVisible(true);
		} else if (param.asInteger() == 0x11283090) {
			if (!_doorOpen)
				_ssDoorLight->setVisible(false);
		}
		break;
	case 0x2000:
		if (_doorOpen)
			_countdown = 144;
		messageResult = _doorOpen ? 1 : 0;
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		_countdown = 144;
		if (!_doorOpen)
			stOpenDoor();
		break;
	}
	return messageResult;
}

void AsScene2201Door::stOpenDoor() {
	_doorOpen = true;
	startAnimation(0xE2CB0412, 0, -1);
	_newStickFrameIndex = -2;
	playSound(0, calcHash("fxDoorOpen33"));
}

void AsScene2201Door::stCloseDoor() {
	_doorOpen = false;
	startAnimation(0xE2CB0412, -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
	playSound(0, calcHash("fxDoorClose33"));
}

SsScene2201PuzzleCube::SsScene2201PuzzleCube(NeverhoodEngine *vm, uint32 positionIndex, uint32 cubeIndex)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(kSsScene2201PuzzleCubeFileHashes[cubeIndex]);
	createSurface(100, 16, 16);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = kSsScene2201PuzzleCubePoints[positionIndex].x;
	_y = kSsScene2201PuzzleCubePoints[positionIndex].y;
	_needRefresh = true;
}

Scene2201::Scene2201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isSoundPlaying(false) {

	Sprite *tempSprite;

	if (!getSubVar(0x40050052, 0x60400854)) {
		// TODO _vm->gameModule()->initScene2201Vars();
	}

	_surfaceFlag = true;
	SetMessageHandler(&Scene2201::handleMessage);
	SetUpdateHandler(&Scene2201::update);
	
	loadDataResource(0x04104242);
	loadHitRectList();

	setBackground(0x40008208);
	setPalette(0x40008208);
	insertMouse433(0x0820C408);

	_asTape = insertSprite<AsScene1201Tape>(this, 7, 1100, 459, 432, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape); 

	_ssDoorButton = insertSprite<SsCommonPressButton>(this, 0xE4A43E29, 0xE4A43E29, 100, 0);
	
	for (uint32 cubeIndex = 0; cubeIndex < 9; cubeIndex++)
		if ((int16)getSubVar(0x484498D0, cubeIndex) >= 0)
			insertSprite<SsScene2201PuzzleCube>(cubeIndex, (int16)getSubVar(0x484498D0, cubeIndex));

	_clipRects[0].y1 = 0;
	_clipRects[0].x2 = 640;
	_clipRects[1].x2 = 640;
	_clipRects[1].y2 = 480;
	
	if (!getGlobalVar(0x404290D5)) {
		insertStaticSprite(0x00026027, 900);
	}
	
	tempSprite = insertStaticSprite(0x030326A0, 1100);
	_clipRects[0].x1 = tempSprite->getDrawRect().x;
	
	insertStaticSprite(0x811DA061, 1100);

	tempSprite = insertStaticSprite(0x11180022, 1100);
	_clipRects[1].x1 = tempSprite->getDrawRect().x;

	tempSprite = insertStaticSprite(0x0D411130, 1100);
	_clipRects[0].y2 = tempSprite->getDrawRect().y2();
	_clipRects[1].y1 = tempSprite->getDrawRect().y2();
	
	_ssDoorLight = insertStaticSprite(0xA4062212, 900);

	if (which < 0) {
		insertKlayman<KmScene2201>(300, 427, _clipRects, 2);
		setMessageList(0x004B8118);
		_asDoor = insertSprite<AsScene2201Door>(_klayman, _ssDoorLight, false);
	} else if (which == 1) {
		insertKlayman<KmScene2201>(412, 393, _clipRects, 2);
		setMessageList(0x004B8130);
		_asDoor = insertSprite<AsScene2201Door>(_klayman, _ssDoorLight, false);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene2201>(379, 427, _clipRects, 2);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene2201>(261, 427, _clipRects, 2);
		}
		setMessageList(0x004B8178);
		_asDoor = insertSprite<AsScene2201Door>(_klayman, _ssDoorLight, false);
	} else {
		NPoint pt = _dataResource.getPoint(0x0304D8DC);
		insertKlayman<KmScene2201>(pt.x, pt.y, _clipRects, 2);
		setMessageList(0x004B8120);
		_asDoor = insertSprite<AsScene2201Door>(_klayman, _ssDoorLight, true);
	}
	
	insertSprite<AsScene2201CeilingFan>();

	_vm->_soundMan->addSound(0x04106220, 0x81212040);

}

Scene2201::~Scene2201() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
	_vm->_soundMan->deleteSoundGroup(0x04106220);
}

void Scene2201::update() {
	Scene::update();
	if (!_isSoundPlaying) {
		_vm->_soundMan->playSoundLooping(0x81212040);
		_isSoundPlaying = true;
	}
}

uint32 Scene2201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssDoorButton);
		} else if (param.asInteger() == 0x35803198) {
			if (sendMessage(_asDoor, 0x2000, 0)) {
				setMessageList(0x004B81A0);
			} else {
				setMessageList(0x004B81B8);
			} 
		} else if (param.asInteger() == 0x51445010) {
			if (getGlobalVar(0x404290D5)) {
				setMessageList(0x004B8108);
			} else {
				setMessageList(0x004B8150);
			}
		} else if (param.asInteger() == 0x1D203082) {
			setMessageList(0x004B8180);
		} else if (param.asInteger() == 0x00049091) {
			if (getGlobalVar(0x404290D5)) {
				setMessageList(0x004B8138);
			} else {
				setMessageList(0x004B8108);
			}
		}
		break;
	case 0x480B:
		if (sender == _ssDoorButton) {
			sendMessage(_asDoor, 0x4808, 0);
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B81C8);
		}
		break;
	}
	return 0;
}

static const NPoint kSsScene2202PuzzleTilePoints[] = {
	{196, 105},
	{323, 102},
	{445, 106},
	{192, 216},
	{319, 220},
	{446, 216},
	{188, 320},
	{319, 319},
	{443, 322}
};

static const uint32 kSsScene2202PuzzleTileFileHashes1[] = {
	0xA500800C,
	0x2182910C,
	0x2323980C,
	0x23049084,
	0x21008080,
	0x2303900C,
	0x6120980C,
	0x2504D808
};

static const uint32 kSsScene2202PuzzleTileFileHashes2[] = {
	0x0AAD8080,
	0x0A290291,
	0x0A2BA398,
	0x822B8490,
	0x86298080,
	0x0A2B8390,
	0x0A69A098,
	0x0E2D84D8
};

SsScene2202PuzzleTile::SsScene2202PuzzleTile(NeverhoodEngine *vm, Scene *parentScene, int16 tileIndex, int16 value)
	: StaticSprite(vm, 900), _parentScene(parentScene), _value(value), _tileIndex(tileIndex), _isMoving(false) {
	
	SetUpdateHandler(&SsScene2202PuzzleTile::update);
	SetMessageHandler(&SsScene2202PuzzleTile::handleMessage);
	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes2[_value]);
	if (_tileIndex >= 0 && _tileIndex <= 2) {
		createSurface(100, 128, 128);
	} else	if (_tileIndex >= 3 && _tileIndex <= 5) {
		createSurface(300, 128, 128);
	} else {
		createSurface(500, 128, 128);
	}
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect = _drawRect;
	_x = kSsScene2202PuzzleTilePoints[_tileIndex].x;
	_y = kSsScene2202PuzzleTilePoints[_tileIndex].y;
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
	loadSound(0, 0x40958621);
	loadSound(1, 0x51108241);
}

void SsScene2202PuzzleTile::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 SsScene2202PuzzleTile::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isMoving && !getGlobalVar(0x404290D5)) {
			sendMessage(_parentScene, 0x2000, _tileIndex);
		}
		messageResult = 1;
		break;
	case 0x2001:
		_isMoving = true;
		moveTile(param.asInteger());
		break;
	}
	return messageResult;
}
				
void SsScene2202PuzzleTile::suMoveTileX() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_x += _xIncr;
		_errValue += _yDelta;
		if (_errValue >= _xDelta) {
			_errValue -= _xDelta;
			_y += _yIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}
	
	if (done) {
		stopMoving();			
	}

	processDelta();

}

void SsScene2202PuzzleTile::suMoveTileY() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_y += _yIncr;
		_errValue += _xDelta;
		if (_errValue >= _yDelta) {
			_errValue -= _yDelta;
			_x += _xIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}
	
	if (done) {
		stopMoving();			
	}

	processDelta();

}

void SsScene2202PuzzleTile::moveTile(int16 newTileIndex) {

	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes1[_value]);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;

	setSubVar(0x484498D0, _tileIndex, (uint32)-1);
	setSubVar(0x484498D0, newTileIndex, (uint32)_value);
	
	_tileIndex = newTileIndex;
	
	_errValue = 0;
	_counterDirection = false;
	_counter = 0;

	_newX = kSsScene2202PuzzleTilePoints[newTileIndex].x;
	_newY = kSsScene2202PuzzleTilePoints[newTileIndex].y;

	if (_x == _newX && _y == _newY)
		return;

	if (_x <= _newX) {
		if (_y <= _newY) {
			_xDelta = _newX - _x;
			_yDelta = _newY - _y;
			_xIncr = 1;
			_yIncr = 1;
		} else {
			_xDelta = _newX - _x;
			_yDelta = _y - _newY;
			_xIncr = 1;
			_yIncr = -1;
		}
	} else {
		if (_y <= _newY) {
			_xDelta = _x - _newX;
			_yDelta = _newY - _y;
			_xIncr = -1;
			_yIncr = 1;
		} else {
			_xDelta = _x - _newX;
			_yDelta = _y - _newY;
			_xIncr = -1;
			_yIncr = -1;
		}
	}

	if (_xDelta > _yDelta) {
		SetSpriteUpdate(&SsScene2202PuzzleTile::suMoveTileX);
		if (_xIncr > 0) {
			if (_newX - _x >= 180)
				_xFlagPos = _newX - 90;
			else
				_xFlagPos = _x + _newX / 2;				
		} else {
			if (_x - _newX >= 180)
				_xFlagPos = _x + 90;
			else
				_xFlagPos = _x / 2 + _newX;
		}
		playSound(0);
	} else {
		SetSpriteUpdate(&SsScene2202PuzzleTile::suMoveTileY);
		if (_yIncr > 0) {
			if (_newY - _y >= 180)
				_xFlagPos = _newY - 90;
			else
				_xFlagPos = _y + _newY / 2;				
		} else {
			if (_y - _newY >= 180)
				_xFlagPos = _y + 90;
			else
				_xFlagPos = _y / 2 + _newY;
		}
		playSound(1);
	}
	
}

void SsScene2202PuzzleTile::stopMoving() {
	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes2[_value]);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;
	SetSpriteUpdate(NULL);
	_isMoving = false;
	sendMessage(_parentScene, 0x2002, _tileIndex);
}

Scene2202::Scene2202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isSolved(false), _leaveScene(false), _isTileMoving(false),
	_ssMovingTile(NULL), _ssDoneMovingTile(NULL) {

	// TODO initScene2201Vars();
	SetMessageHandler(&Scene2202::handleMessage);
	SetUpdateHandler(&Scene2202::update);

	_surfaceFlag = true;

	setBackground(0x08100A0C);
	setPalette(0x08100A0C);
	addEntity(_palette);
	insertMouse435(0x00A08089, 20, 620);

	//DEBUG>>>
	for (uint32 index = 0; index < 9; index++)
		setSubVar(0x484498D0, index, 7 - index);
	//DEBUG<<<

	for (uint32 index = 0; index < 9; index++) {
		int16 value = (int16)getSubVar(0x484498D0, index);
		if (value >= 0) {
			Sprite *puzzleTileSprite = insertSprite<SsScene2202PuzzleTile>(this, index, value);
			_vm->_collisionMan->addSprite(puzzleTileSprite);
		}
	}

	insertStaticSprite(0x55C043B8, 200);
	insertStaticSprite(0x85500158, 400);
	insertStaticSprite(0x25547028, 600);

	loadSound(0, 0x68E25540);
	loadSound(1, 0x40400457);

	_vm->_soundMan->addSound(0x60400854, 0x8101A241);
	_vm->_soundMan->playSoundLooping(0x8101A241);

}

Scene2202::~Scene2202() {
	_vm->_soundMan->deleteSoundGroup(0x60400854);
}

void Scene2202::update() {
	Scene::update();

	if (_leaveScene && !isSoundPlaying(1)) {
		leaveScene(0);
	}

	if (_isSolved && !isSoundPlaying(0)) {
		playSound(1);
		_isSolved = false;
		_leaveScene = true;
	}

	if (_ssMovingTile && !_isTileMoving) {
		int16 freeTileIndex = getFreeTileIndex(_movingTileIndex);
		if (freeTileIndex != -1) {
			setSurfacePriority(_ssMovingTile->getSurface(), 700);
			sendMessage(_ssMovingTile, 0x2001, freeTileIndex);
			_ssMovingTile = NULL;
			_isTileMoving = true;
		}
	}

	if (_ssDoneMovingTile) {
		setSurfacePriority(_ssDoneMovingTile->getSurface(), _surfacePriority);
		_ssDoneMovingTile = NULL;
		if (testIsSolved()) {
			playSound(0);
			setGlobalVar(0x404290D5, 1);
			_isSolved = true;
		}
	}
	
}

uint32 Scene2202::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO Debug stuff
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		// TODO Debug stuff
		break;
	case 0x2000:
		_movingTileIndex = (int16)param.asInteger();
		_ssMovingTile = (Sprite*)sender;
		break;
	case 0x2002:
		_isTileMoving = false;
		_ssDoneMovingTile = (Sprite*)sender;
		if (param.asInteger() <= 2) {
			_surfacePriority = 100;
		} else if (param.asInteger() >= 3 && param.asInteger() <= 5) {
			_surfacePriority = 300;
		} else {
			_surfacePriority = 500;
		}
		break;
	}
	return 0;
}

int16 Scene2202::getFreeTileIndex(int16 index) {
	if (index >= 3 && (int16)getSubVar(0x484498D0, index - 3) == -1) {
		return index - 3;
	} else if (index <= 5 && (int16)getSubVar(0x484498D0, index + 3) == -1) {
		return index + 3;
	} else if (index != 0 && index != 3 && index != 6 && (int16)getSubVar(0x484498D0, index - 1) == -1) {
		return index - 1;
	} else if (index != 2 && index != 5 && index != 8 && (int16)getSubVar(0x484498D0, index + 1) == -1) {
		return index + 1;
	} else
		return -1;
}

bool Scene2202::testIsSolved() {
	return 
		getSubVar(0x484498D0, 0) == 0 &&
		getSubVar(0x484498D0, 2) == 2 &&
		getSubVar(0x484498D0, 3) == 3 &&
		getSubVar(0x484498D0, 4) == 4 &&
		getSubVar(0x484498D0, 5) == 5 &&
		getSubVar(0x484498D0, 6) == 6 &&
		getSubVar(0x484498D0, 8) == 7;
}

static const uint32 kAsCommonKeyFileHashes[] = {
	0x2450D850,	0x0C9CE8D0,	0x2C58A152
};

AsCommonKey::AsCommonKey(NeverhoodEngine *vm, Scene *parentScene, int keyIndex, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, kAsCommonKeyFileHashes[keyIndex], surfacePriority, x, y), _parentScene(parentScene), _keyIndex(keyIndex) {

	if (!getSubVar(0x0090EA95, _keyIndex) && !getSubVar(0x08D0AB11, _keyIndex)) {
		SetMessageHandler(&AsCommonKey::handleMessage);
	} else {
		// If Klayman already has the key or it's already inserted then don't show it
		setVisible(false);
		SetMessageHandler(NULL);
	}
}

uint32 AsCommonKey::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setSubVar(0x0090EA95, _keyIndex, 1);
		setVisible(false);
		SetMessageHandler(NULL);
	}
	return messageResult;
}

static const uint32 kAsScene2203DoorFileHashes[] = {
	0x7868AE10,	0x1A488110
};

AsScene2203Door::AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, uint doorIndex)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _doorIndex(doorIndex) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2203Door::handleMessage);
	_x = 320;
	_y = 240;
	createSurface1(kAsScene2203DoorFileHashes[_doorIndex], 900);
	if (getGlobalVar(0x9A500914) == _doorIndex) {
		startAnimation(kAsScene2203DoorFileHashes[_doorIndex], -1, -1);
		_newStickFrameIndex = -2;
	} else {
		startAnimation(kAsScene2203DoorFileHashes[_doorIndex], 0, -1);
		_newStickFrameIndex = 0;
	}
}

uint32 AsScene2203Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_doorIndex == getGlobalVar(0x9A500914))
			sendMessage(_parentScene, 0x2002, 0);
		else
			sendMessage(_parentScene, 0x2001, 0);
		messageResult = 1;
		break;
	case 0x2000:
		_otherDoor = (Sprite*)param.asEntity();
		break;
	case 0x3002:
		if (_doorIndex == getGlobalVar(0x9A500914))
			sendMessage(_parentScene, 0x4808, 0);
		stopAnimation();
		break;
	case 0x4808:
		setGlobalVar(0x9A500914, _doorIndex);
		sendMessage(_otherDoor, 0x4809, 0);
		openDoor();
		break;
	case 0x4809:
		closeDoor();
		sendMessage(_parentScene, 0x2003, 0);
		break;
	}
	return messageResult;
}

void AsScene2203Door::openDoor() {
	playSound(0, 0x341014C4);
	startAnimation(kAsScene2203DoorFileHashes[_doorIndex], 1, -1);
}

void AsScene2203Door::closeDoor() {
	startAnimation(kAsScene2203DoorFileHashes[_doorIndex], -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
}

Scene2203::Scene2203(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	if (getGlobalVar(0xC0780812) && getGlobalVar(0x13382860) == 0)
		setGlobalVar(0x13382860, 1);

	SetMessageHandler(&Scene2203::handleMessage);
	_surfaceFlag = true;

	setBackground(0x82C80334);
	setPalette(0x82C80334);
	insertMouse433(0x80330824);

	setHitRects(0x004B8320);

	//DEBUG setGlobalVar(0x13382860, 1);//###//TODO CHECK IF Below...

	if (getGlobalVar(0x13382860) == 1) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 282, 432);
		_vm->_collisionMan->addSprite(_asKey);
	}

	_asTape = insertSprite<AsScene1201Tape>(this, 1, 1100, 435, 432, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape);

	_asLeftDoor = insertSprite<AsScene2203Door>(this, 0);
	_asRightDoor = insertSprite<AsScene2203Door>(this, 1);
	
	_ssSmallLeftDoor = insertStaticSprite(0x542CC072, 1100);
	_ssSmallRightDoor = insertStaticSprite(0x0A2C0432, 1100);
	
	_leftDoorClipRect.set(_ssSmallLeftDoor->getDrawRect().x, 0, 640, 480);
	_rightDoorClipRect.set(0, 0, _ssSmallRightDoor->getDrawRect().x2(), 480);

	sendEntityMessage(_asLeftDoor, 0x2000, _asRightDoor);
	sendEntityMessage(_asRightDoor, 0x2000, _asLeftDoor);
	
	_vm->_collisionMan->addSprite(_asLeftDoor);
	_vm->_collisionMan->addSprite(_asRightDoor);

	if (which < 0) {
		insertKlayman<KmScene2203>(200, 427);
		setMessageList(0x004B8340);
	} else if (which == 1) {
		insertKlayman<KmScene2203>(640, 427);
		setMessageList(0x004B8350);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene2203>(362, 427);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene2203>(202, 427);
		}
		setMessageList(0x004B8358);
	} else {
		insertKlayman<KmScene2203>(0, 427);
		setMessageList(0x004B8348);
	}

	if (getGlobalVar(0x9A500914)) {
		_ssSmallLeftDoor->setVisible(false);
		_klayman->setClipRect(_rightDoorClipRect);
	} else {
		_ssSmallRightDoor->setVisible(false);
		_klayman->setClipRect(_leftDoorClipRect);
	}
	
	setRectList(0x004B8420);

}

Scene2203::~Scene2203() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2203::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		sendEntityMessage(_klayman, 0x1014, sender);
		if (sender == _asLeftDoor) {
			setMessageList2(0x004B83B0);
		} else {
			setMessageList2(0x004B83C8);
		}
		break;
	case 0x2002:
		if (sender == _asLeftDoor) {
			setMessageList2(0x004B8370);
		} else {
			setMessageList2(0x004B8360);
		}
		break;
	case 0x2003:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->setVisible(false);
		} else {
			_ssSmallRightDoor->setVisible(false);
		}
		break;
	case 0x4808:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->setVisible(true);
			_klayman->setClipRect(_leftDoorClipRect);
		} else {
			_ssSmallRightDoor->setVisible(true);
			_klayman->setClipRect(_rightDoorClipRect);
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B83E0);
		} else if (sender == _asKey) {
			sendEntityMessage(_klayman, 0x1014, _asKey);
			setMessageList(0x004B83F0);
		}
		break;
	}
	return messageResult;
}

SsScene2205DoorFrame::SsScene2205DoorFrame(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	SetMessageHandler(&SsScene2205DoorFrame::handleMessage);
	_spriteResource.load2(getGlobalVar(0x4D080E54) ? 0x24306227 : 0xD90032A0);
	createSurface(1100, 45, 206);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_needRefresh = true;
	StaticSprite::update();
}

uint32 SsScene2205DoorFrame::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		_spriteResource.load2(getGlobalVar(0x4D080E54) ? 0x24306227 : 0xD90032A0);
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_needRefresh = true;
		StaticSprite::update();
	}
	return messageResult;
}

Scene2205::Scene2205(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	SetMessageHandler(&Scene2205::handleMessage);
	SetUpdateHandler(&Scene2205::update);
	
	setHitRects(0x004B0620);
	_surfaceFlag = true;

	if (getGlobalVar(0x4D080E54)) {
		_isLightOn = true;
		setBackground(0x0008028D);
		setPalette(0x0008028D);
		addEntity(_palette);
		insertMouse433(0x80289008);
		_ssLightSwitch = insertSprite<SsCommonPressButton>(this, 0x2D339030, 0x2D309030, 100, 0);
	} else {
		_isLightOn = false;
		setBackground(0xD00A028D);
		setPalette(0xD00A028D);
		addEntity(_palette);
		insertMouse433(0xA0289D08);
		_ssLightSwitch = insertSprite<SsCommonPressButton>(this, 0x2D339030, 0xDAC86E84, 100, 0);
	}

	_palette->addBasePalette(0xD00A028D, 0, 256, 0);
	
	_ssDoorFrame = insertSprite<SsScene2205DoorFrame>();

	if (which < 0) {
		insertKlayman<KmScene2205>(320, 417);
		setMessageList(0x004B0658);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else if (which == 1) {
		insertKlayman<KmScene2205>(640, 417);
		setMessageList(0x004B0648);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else {
		insertKlayman<KmScene2205>(0, 417);
		setMessageList(0x004B0640);
		_isKlaymanInLight = true;
	}

	_klayman->setClipRect(_ssDoorFrame->getDrawRect().x, 0, 640, 480);

	loadDataResource(0x00144822);
	_klayman->setSoundFlag(true);

}
	
void Scene2205::update() {
	Scene::update();

	if (!_isLightOn && getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0x0008028D, 0, 256, 0);
		changeBackground(0x0008028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0x2D309030);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		changeMouseCursor(0x80289008);
		_isLightOn = true;
	} else if (_isLightOn && !getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0xD00A028D, 0, 256, 0);
		changeBackground(0xD00A028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0xDAC86E84);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		changeMouseCursor(0xA0289D08);
		_isKlaymanInLight = true;
		if (_klayman->getX() > 85) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
			_isKlaymanInLight = false;
		}
		_isLightOn = false;
	}

	if (!getGlobalVar(0x4D080E54)) {
		if (_isKlaymanInLight && _klayman->getX() > 85) {
			_palette->addBasePalette(0x68033B1C, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymanInLight = false;
		} else if (!_isKlaymanInLight && _klayman->getX() <= 85) {
			_palette->addBasePalette(0xD00A028D, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymanInLight = true;
		}
	}
	
}

uint32 Scene2205::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x6449569A) {
			setMessageList(0x004B0690);
		} else if (param.asInteger() == 0x2841369C) {
			setMessageList(0x004B0630);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssLightSwitch);
		}
		break;
	case 0x480B:
		setGlobalVar(0x4D080E54, getGlobalVar(0x4D080E54) ? 0 : 1);
		break;
	}
	return 0;
}

static const int16 kScene2206XPositions[] = {
	384, 
	480, 
	572
};

static const uint32 kScene2206MessageIds1[] = {
	0x004B8998,
	0x004B89B8,
	0x004B89D8
};

static const uint32 kScene2206MessageIds2[] = {
	0x004B89F8,
	0x004B8A20,
	0x004B8A48
};

static const int16 kAsScene2206DoorSpikesXDeltasOpen[] = {
	-24, -28, -18, 6, 9, -8
};

static const int16 kAsScene2206DoorSpikesXDeltasClose[] = {
	-8, 7, 11, 26, 13, 14
};

AsScene2206DoorSpikes::AsScene2206DoorSpikes(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 200) {
	
	if (getGlobalVar(0x18890C91))
		_x -= 63;
	SetUpdateHandler(&AsScene2206DoorSpikes::update);
	SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene2206DoorSpikes::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 AsScene2206DoorSpikes::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		_deltaIndex = 0;
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206DoorSpikes::suOpen);
		playSound(0, 0x032746E0);
		break;
	case 0x4809:
		_deltaIndex = 0;
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206DoorSpikes::suClose);
		playSound(0, 0x002642C0);
		break;
	}
	return messageResult;
}

void AsScene2206DoorSpikes::suOpen() {
	if (_deltaIndex < 6) {
		_x += kAsScene2206DoorSpikesXDeltasOpen[_deltaIndex];
		_deltaIndex++;
	} else {
		SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

void AsScene2206DoorSpikes::suClose() {
	if (_deltaIndex < 6) {
		_x += kAsScene2206DoorSpikesXDeltasClose[_deltaIndex];
		_deltaIndex++;
	} else {
		SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

AsScene2206Platform::AsScene2206Platform(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 50) {

	SetUpdateHandler(&AsScene2206Platform::update);
	SetMessageHandler(&AsScene2206Platform::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene2206Platform::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 AsScene2206Platform::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4803:
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206Platform::suMoveDown);
		_yDelta = 0;
		break;
	}
	return messageResult;
}

void AsScene2206Platform::suMoveDown() {
	_yDelta++;
	_y += _yDelta;
}

SsScene2206TestTube::SsScene2206TestTube(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, uint32 fileHash)
	: StaticSprite(vm, fileHash, surfacePriority), _parentScene(parentScene) {

	if (getGlobalVar(0x45080C38)) {
		setVisible(false);
		SetMessageHandler(NULL);
	} else {
		SetMessageHandler(&SsScene2206TestTube::handleMessage);
	}
	_deltaRect = _drawRect;
	processDelta();
}

uint32 SsScene2206TestTube::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setGlobalVar(0x45080C38, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

Scene2206::Scene2206(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	uint32 fileHash;
	
	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene2206::handleMessage);
	_surfaceFlag = true;
	
	if (getGlobalVar(0x4D080E54)) {
		fileHash = 0x41983216;
		_sprite1 = insertStaticSprite(0x2201266A, 100);
		_sprite2 = insertStaticSprite(0x3406A333, 300);
		_sprite3 = insertStaticSprite(0x24A223A2, 100);
		_asDoorSpikes = insertSprite<AsScene2206DoorSpikes>(0x26133023);
		_asDoorSpikes->setClipRect(_sprite2->getDrawRect().x, 0, 640, 480);
		setRectList(0x004B8AF8);
		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x0E038022, 100, 0);
		insertMouse433(0x83212411);
		_ssTestTube = insertSprite<SsScene2206TestTube>(this, 1100, /*464, 433, */0x5E00E262);
		_asPlatform = insertSprite<AsScene2206Platform>(0x085E25E0);
	} else {
		fileHash = 0xE0102A45;
		_sprite1 = insertStaticSprite(0x1C1106B8, 100);
		_sprite2 = insertStaticSprite(0x020462E0, 300);
		_sprite3 = insertStaticSprite(0x900626A2, 100);
		_asDoorSpikes = insertSprite<AsScene2206DoorSpikes>(0x544822A8);
		_asDoorSpikes->setClipRect(_sprite2->getDrawRect().x, 0, 640, 480);
		setRectList(0x004B8B58);
		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x16882608, 100, 0);
		insertMouse433(0x02A41E09);
		_ssTestTube = insertSprite<SsScene2206TestTube>(this, 1100, /*464, 433, */0x52032563);
		_asPlatform = insertSprite<AsScene2206Platform>(0x317831A0);
	}

	_asPlatform->setClipRect(_sprite2->getDrawRect().x, 0, _sprite3->getDrawRect().x2(), _sprite1->getDrawRect().y2());

	setBackground(fileHash);

	setPalette(fileHash);
	addEntity(_palette);

	_palette->addBasePalette(fileHash, 0, 256, 0);

	if (!getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0x0263D144, 0, 65, 0);
	}
	
	_vm->_collisionMan->addSprite(_ssTestTube);
	
	if (which < 0) {
		insertKlayman<KmScene2206>(200, 430);
		setMessageList(0x004B88A8);
	} else if (which == 1) {
		insertKlayman<KmScene2206>(640, 430);
		setMessageList(0x004B88B8);
	} else if (which == 2) {
		insertKlayman<KmScene2206>(205, 396);
		setMessageList(0x004B88C8);
		_palette->addPalette(getGlobalVar(0x4D080E54) ? 0xB103B604 : 0x0263D144, 0, 65, 0);
		klaymanBehindSpikes();
		playSound(0, 0x53B8284A);
	} else if (which == 3) {
		insertKlayman<KmScene2206>(kScene2206XPositions[getGlobalVar(0x48A68852)], 430);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
		setMessageList(0x004B8A70);
	} else {
		insertKlayman<KmScene2206>(0, 430);
		setMessageList(0x004B88B0);
	}

	_klayman->setSoundFlag(true);
	_klayman->setKlaymanIdleTable2();

}

Scene2206::~Scene2206() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2206::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x11C40840) {
			if (getGlobalVar(0x18890C91))
				setMessageList(0x004B8948);
			else
				setMessageList(0x004B8970);
		}
		break;
	case 0x4803:
		sendMessage(_asPlatform, 0x4803, 0);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			setGlobalVar(0x18890C91, getGlobalVar(0x18890C91) ? 0 : 1);
			if (getGlobalVar(0x18890C91))
				sendMessage(_asDoorSpikes, 0x4808, 0);
			else
				sendMessage(_asDoorSpikes, 0x4809, 0);
		}
		break;
	case 0x4826:
		sendEntityMessage(_klayman, 0x1014, _ssTestTube);
		setMessageList(0x004B8988);
		break;
	case 0x482A:
		klaymanBehindSpikes();
		break;
	case 0x482B:
		klaymanInFrontSpikes();
		break;
	}
	return messageResult;
}

void Scene2206::klaymanInFrontSpikes() {
	if (getGlobalVar(0x4D080E54)) {
		_palette->addBasePalette(0x41983216, 0, 65, 0);
		_palette->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 100);
	setSurfacePriority(_sprite2->getSurface(), 300);
	setSurfacePriority(_sprite3->getSurface(), 100);
	setSurfacePriority(_asDoorSpikes->getSurface(), 200);
	_klayman->setClipRect(0, 0, 640, 480);
}

void Scene2206::klaymanBehindSpikes() {
	if (!getGlobalVar(0x4D080E54)) {
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 1100);
	setSurfacePriority(_sprite2->getSurface(), 1300);
	setSurfacePriority(_sprite3->getSurface(), 1100);
	setSurfacePriority(_asDoorSpikes->getSurface(), 1200);
	_klayman->setClipRect(_sprite2->getDrawRect().x, 0, _sprite3->getDrawRect().x2(), _sprite1->getDrawRect().y2());
}

void Scene2206::readClickedColumn() {
	setGlobalVar(0x48A68852, (_mouseClickPos.x - 354) / 96);
	if (getGlobalVar(0x48A68852) > 2)
		setGlobalVar(0x48A68852, 2);
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 183) / 7);
	setGlobalVar(0xC8C28808, calcHash("stLineagex"));
	setGlobalVar(0x4CE79018, 0);
	if (ABS(kScene2206XPositions[getGlobalVar(0x48A68852)] - _klayman->getX()) >= 144) {
		setMessageList2(kScene2206MessageIds1[getGlobalVar(0x48A68852)]);
	} else {
		setMessageList2(kScene2206MessageIds2[getGlobalVar(0x48A68852)]);
	}
}

static const uint32 kScene2207FileHashes[] = {
	0x33B1E12E,
	0x33D1E12E,
	0x3311E12E,
	0x3291E12E,
	0x3191E12E,
	0x3791E12E,
	0x3B91E12E,
	0x2391E12E,
	0x1391E12E,
	0x3BB1E12E,
	0x23B1E12E,
	0x13B1E12E
};

AsScene2207Elevator::AsScene2207Elevator(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _pointIndex(0), _destPointIndex(0), _destPointIndexDelta(0) {

	NPoint pt;

	_dataResource.load(0x00524846);
	_pointArray = _dataResource.getPointArray(0x005B02B7);
	pt = _dataResource.getPoint(0x403A82B1);
	_x = pt.x;
	_y = pt.y;
	createSurface(1100, 129, 103);
	startAnimation(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, 0, 0);
	SetUpdateHandler(&AsScene2207Elevator::update);
	SetSpriteUpdate(&AsScene2207Elevator::suSetPosition);
	SetMessageHandler(&AsScene2207Elevator::handleMessage);
	_newStickFrameIndex = 0;
}

AsScene2207Elevator::~AsScene2207Elevator() {
	_vm->_soundMan->deleteSoundGroup(0x02700413);
}

void AsScene2207Elevator::update() {

	if (_destPointIndex + _destPointIndexDelta > _pointIndex) {
		_pointIndex++;
		startAnimation(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newStickFrameIndex = _pointIndex;		
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0) {
				_destPointIndexDelta = 0;
			} else {
				_vm->_soundMan->deleteSound(0xD3B02847);
				playSound(0, 0x53B8284A);
			}
		}
	}

	if (_destPointIndex + _destPointIndexDelta < _pointIndex) {
		_pointIndex--;
		if (_pointIndex == 0)
			sendMessage(_parentScene, 0x2003, 0);
		startAnimation(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newStickFrameIndex = _pointIndex;		
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0) {
				_destPointIndexDelta = 0;
			} else {
				_vm->_soundMan->deleteSound(0xD3B02847);
				playSound(0, 0x53B8284A);
			}
		}
	}

	if (_pointIndex > 20 && _surface->getPriority() != 900) {
		sendMessage(_parentScene, 0x2002, 900);
	} else if (_pointIndex < 20 && _surface->getPriority() != 1100) {
		sendMessage(_parentScene, 0x2002, 1100);
	}
	
	AnimatedSprite::update();
	
	if (_destPointIndex + _destPointIndexDelta == _pointIndex && _isMoving) {
		sendMessage(_parentScene, 0x2004, 0);
		_isMoving = false;
	}
	
}

void AsScene2207Elevator::suSetPosition() {
	_x = (*_pointArray)[_pointIndex].x;
	_y = (*_pointArray)[_pointIndex].y - 60;
	processDelta();
}

uint32 AsScene2207Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		moveToY(param.asInteger());
		break;
	}
	return messageResult;
}

void AsScene2207Elevator::moveToY(int16 y) {
	int16 minDistance = 480;

	if (!_pointArray || _pointArray->size() == 0)
		return;
	
	for (uint i = 0; i < _pointArray->size(); i++) {
		int16 distance = ABS(y - (*_pointArray)[i].y);
		if (distance < minDistance) {
			minDistance = distance;
			_destPointIndex = i;
		}
	}	

	if (_destPointIndex != _pointIndex) {
		if (_destPointIndex == 0 || _destPointIndex == (int)_pointArray->size() - 1) {
			_destPointIndexDelta = 0;
		} else if (_destPointIndex < _pointIndex) {
			_destPointIndexDelta = -2;
		} else {
			_destPointIndexDelta = 2;
		}
		_vm->_soundMan->addSound(0x02700413, 0xD3B02847);
		_vm->_soundMan->playSoundLooping(0xD3B02847);
	}

	_isMoving = true;

}

AsScene2207Lever::AsScene2207Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int doDeltaX)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207Lever::handleMessage);
	createSurface(1010, 71, 73);
	setDoDeltaX(doDeltaX);
	startAnimation(0x80880090, 0, -1);
	_newStickFrameIndex = 0;
	_x = x;
	_y = y;
}

uint32 AsScene2207Lever::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x3002:
		gotoNextState();
		stopAnimation();
		break;
	case 0x4807:
		stLeverUp();
		break;
	case 0x480F:
		stLeverDown();
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

void AsScene2207Lever::stLeverDown() {
	startAnimation(0x80880090, 1, -1);
	FinalizeState(&AsScene2207Lever::stLeverDownEvent);
	playSound(0, 0x40581882);
}

void AsScene2207Lever::stLeverDownEvent() {
	sendMessage(_parentScene, 0x480F, 0);
}

void AsScene2207Lever::stLeverUp() {
	startAnimation(0x80880090, 6, -1);
	FinalizeState(&AsScene2207Lever::stLeverUpEvent);
	_playBackwards = true;
	playSound(0, 0x40581882);
}

void AsScene2207Lever::stLeverUpEvent() {
	sendMessage(_parentScene, 0x4807, 0);
}

AsScene2207WallRobotAnimation::AsScene2207WallRobotAnimation(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1200), _idle(true) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallRobotAnimation::handleMessage);
	createSurface1(0xCCFD6090, 100);
	_x = 309;
	_y = 320;
	startAnimation(0xCCFD6090, 0, -1);
	_newStickFrameIndex = 0;
	loadSound(1, 0x40330872);
	loadSound(2, 0x72A2914A);
	loadSound(3, 0xD4226080);
}

AsScene2207WallRobotAnimation::~AsScene2207WallRobotAnimation() {
	_vm->_soundMan->deleteSoundGroup(0x80D00820);
}

uint32 AsScene2207WallRobotAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_idle) {
			if (param.asInteger() == 0x3423093) {
				_vm->_soundMan->addSound(0x80D00820, 0x12121943);
				_vm->_soundMan->playSoundLooping(0x12121943);
			} else if (param.asInteger() == 0x834AB011) {
				stopSound(0);
				stopSound(1);
				stopSound(2);
				stopSound(3);
				_vm->_soundMan->deleteSound(0x12121943);
			} else if (param.asInteger() == 0x3A980501) {
				playSound(1);
			} else if (param.asInteger() == 0x2A2AD498) {
				playSound(2);
			} else if (param.asInteger() == 0xC4980008) {
				playSound(3);
			} else if (param.asInteger() == 0x06B84228) {
				playSound(0, 0xE0702146);
			}
		}
		break;
	case 0x2006:
		stStartAnimation();
		break;
	case 0x2007:
		stStopAnimation();
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2207WallRobotAnimation::stStartAnimation() {
	if (!_idle) {
		NextState(NULL);
	} else {
		startAnimation(0xCCFD6090, 0, -1);
		_idle = false;
		setVisible(true);
	}
}

void AsScene2207WallRobotAnimation::stStopAnimation() {
	NextState(&AsScene2207WallRobotAnimation::cbStopAnimation);
}

void AsScene2207WallRobotAnimation::cbStopAnimation() {
	stopAnimation();
	stopSound(0);
	stopSound(1);
	stopSound(2);
	stopSound(3);
	_vm->_soundMan->deleteSound(0x12121943);
	_idle = true;
	setVisible(false);
}

AsScene2207WallCannonAnimation::AsScene2207WallCannonAnimation(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _idle(true) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallCannonAnimation::handleMessage);
	createSurface1(0x8CAA0099, 100);
	_x = 309;
	_y = 320;
	startAnimation(0x8CAA0099, 0, -1);
	_newStickFrameIndex = 0;
}

uint32 AsScene2207WallCannonAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2006:
		stStartAnimation();
		break;
	case 0x2007:
		stStopAnimation();
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2207WallCannonAnimation::stStartAnimation() {
	if (!_idle) {
		NextState(NULL);
	} else {
		setVisible(true);
		startAnimation(0x8CAA0099, 0, -1);
		_idle = false;
	}
}

void AsScene2207WallCannonAnimation::stStopAnimation() {
	NextState(&AsScene2207WallCannonAnimation::cbStopAnimation);
}

void AsScene2207WallCannonAnimation::cbStopAnimation() {
	stopAnimation();
	setVisible(false);
	_idle = true;
}

SsScene2207Symbol::SsScene2207Symbol(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {

	_x = 330;
	_y = 246 + index * 50;
	StaticSprite::update();	
}

Scene2207::Scene2207(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _klaymanAtElevator(true), _elevatorSurfacePriority(0) {

	//DEBUG>>>
	setGlobalVar(0x4D080E54, 1);
	//DEBUG<<<

	_vm->gameModule()->initScene3009Vars();

	if (!getSubVar(0x40050052, 0x88460852))
		setSubVar(0x40050052, 0x88460852, 1);

	SetMessageHandler(&Scene2207::handleMessage);
	SetUpdateHandler(&Scene2207::update);
	_surfaceFlag = true;

	insertKlayman<KmScene2207>(0, 0);
	_klayman->setRepl(64, 0);
	
	setMessageList(0x004B38E8);
	
	_asElevator = insertSprite<AsScene2207Elevator>(this);
	
	if (getGlobalVar(0x4D080E54)) {

		setBackground(0x88C00241);
		setPalette(0x88C00241);
		insertMouse433(0x00245884);
	
		_ssMaskPart1 = insertStaticSprite(0xE20A28A0, 1200);
		_ssMaskPart2 = insertStaticSprite(0x688F62A5, 1100);
		_ssMaskPart3 = insertStaticSprite(0x0043B038, 1100);
	
		_asTape = insertSprite<AsScene1201Tape>(this, 4, 1100, 277, 428, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape); 
	
		_asLever = insertSprite<AsScene2207Lever>(this, 527, 333, 0);
		_vm->_collisionMan->addSprite(_asLever);
		
		_asWallRobotAnimation = insertSprite<AsScene2207WallRobotAnimation>(this);
		_asWallCannonAnimation = insertSprite<AsScene2207WallCannonAnimation>();
		
		_asWallRobotAnimation->setVisible(false);
		_asWallCannonAnimation->setVisible(false);

		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x2C4061C4, 100, 0);
	
		_asLever->setClipRect(0, 0, _ssMaskPart3->getDrawRect().x2(), 480);
		_klayman->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, _ssMaskPart2->getDrawRect().y2());
		_asElevator->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, _ssMaskPart2->getDrawRect().y2());
	
	} else {

		setGlobalVar(0x81890D14, 1);

		setBackground(0x05C02A55);
		setPalette(0x05C02A55);
		insertMouse433(0x02A51054);

		_ssMaskPart1 = insertStaticSprite(0x980E46A4, 1200);

		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(0x00504B86, 0)], 0);
		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(0x00504B86, 1)], 1);
		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(0x00504B86, 2)], 2);

		_asTape = NULL;
		_asLever = NULL;
		_asWallRobotAnimation = NULL;
		_asWallCannonAnimation = NULL;
		_ssButton = NULL;

		_klayman->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, 480);
		_asElevator->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, 480);

	}

	_dataResource.load(0x00524846);

	setRectList(0x004B38B8);

	sendEntityMessage(_klayman, 0x1014, _asElevator);
	sendMessage(_klayman, 0x2001, 0);
	sendMessage(_asElevator, 0x2000, 480);

	loadSound(1, calcHash("fxFogHornSoft"));

}

void Scene2207::update() {
	Scene::update();
	if (_elevatorSurfacePriority != 0) {
		setSurfacePriority(_asElevator->getSurface(), _elevatorSurfacePriority);
		_elevatorSurfacePriority = 0;
	}
	if (_klayman->getY() == 423) {
		_klaymanAtElevator = _klayman->getX() > 459 && _klayman->getX() < 525;
	}
}

uint32 Scene2207::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0014F275) {
			if (_klaymanAtElevator) {
				sendMessage(_asElevator, 0x2000, _mouseClickPos.y);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				cancelMessageList();
			}
		} else if (param.asInteger() == 0x34569073) {
			if (_klaymanAtElevator) {
				_isKlaymanBusy = true;
				sendMessage(_asElevator, 0x2000, 0);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				cancelMessageList();
			}
		} else if (param.asInteger() == 0x4054C877) {
			if (_klaymanAtElevator) {
				sendMessage(_asElevator, 0x2000, 480);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				cancelMessageList();
			}
		} else if (param.asInteger() == 0x0CBC6211) {
			sendEntityMessage(_klayman, 0x1014, _asElevator);
			sendMessage(_klayman, 0x2001, 0);
			setRectList(0x004B38B8);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x231DA241) {
			if (_ssButton) {
				setMessageList(0x004B38F0);
			} else {
				setMessageList(0x004B37D8);
			}
		}
		break;
	case 0x2002:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2003:
		_isKlaymanBusy = false;
		break;
	case 0x4807:
		sendMessage(_asWallRobotAnimation, 0x2007, 0);
		sendMessage(_asWallCannonAnimation, 0x2007, 0);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			if (getSubVar(0x14800353, 0x40119852)) {
				setSubVar(0x14800353, 0x40119852, 0);
				playSound(0, calcHash("fx3LocksDisable"));
			} else {
				setSubVar(0x14800353, 0x40119852, 1);
				playSound(1);
			}
		}
		break;
	case 0x480F:
		sendMessage(_asWallRobotAnimation, 0x2006, 0);
		sendMessage(_asWallCannonAnimation, 0x2006, 0);
		_asWallRobotAnimation->setVisible(true);
		_asWallCannonAnimation->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asTape) {
			if (_klayman->getY() == 423) {
				sendEntityMessage(_klayman, 0x1014, _asTape);
				setMessageList(0x004B3958);
			}
		} else if (_klaymanAtElevator) {
			SetMessageHandler(&Scene2207::handleMessage2);
			sendMessage(_asElevator, 0x2000, 347);
			sendEntityMessage(_klayman, 0x1014, _asElevator);
			sendMessage(_klayman, 0x2001, 0);
		}
		break;
	}
	return messageResult;
}

uint32 Scene2207::handleMessage2(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2004:
		SetMessageHandler(&Scene2207::handleMessage);
		sendMessage(_klayman, 0x2005, 0);
		sendEntityMessage(_klayman, 0x1014, _asLever);
		setMessageList(0x004B3920);
		setRectList(0x004B3948);
		break;
	}
	return messageResult;
}

static const uint32 kScene2208FileHashes1[] = {
	0x041023CB, 0x041020CB, 0x041026CB, 0x04102ACB,
	0x041032CB, 0x041002CB
};
	
static const uint32 kScene2208FileHashes2[] = {
	0x091206C9, 0x091406C9, 0x091806C9, 0x090006C9,
	0x093006C9, 0x095006C9
};

Scene2208::Scene2208(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _textResource(vm) {

	SpriteResource spriteResource(_vm);
	const char *textStart, *textEnd;

	if (!getGlobalVar(0xC8C28808))
		setGlobalVar(0xC8C28808, calcHash("stLineagex"));

	_textResource.load(getGlobalVar(0xC8C28808));
	
	textStart = _textResource.getString(getGlobalVar(0x48A68852), textEnd);
	while (textStart < textEnd) {
		_strings.push_back(textStart);
		textStart += strlen(textStart) + 1;
	}
	
	_maxRowIndex = 8 + 10 * (3 - (getGlobalVar(0xC8C28808) == calcHash("stLineagex") ? 1 : 0));

	_background = new Background(_vm, 0);
	_background->createSurface(0, 640, 528);
	_background->getSpriteResource().getPosition().y = 480;
	addBackground(_background);
	
	setPalette(0x08100289);
	addEntity(_palette); // Why?

	insertMouse435(0x0028D089, 40, 600);
	
	createFontSurface();
	
	_backgroundSurface = new BaseSurface(_vm, 0, 640, 480);
	spriteResource.load2(0x08100289);
	_backgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_topBackgroundSurface = new BaseSurface(_vm, 0, 640, 192);
	spriteResource.load2(!getGlobalVar(0x4CE79018) ? kScene2208FileHashes1[getGlobalVar(0x48A68852) % 6] : getGlobalVar(0x4CE79018));
	_topBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_bottomBackgroundSurface = new BaseSurface(_vm, 0, 640, 192);
	spriteResource.load2(kScene2208FileHashes2[getGlobalVar(0x48A68852) % 6]);
	_bottomBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);
	
	SetUpdateHandler(&Scene2208::update);
	SetMessageHandler(&Scene2208::handleMessage);

	_visibleRowsCount = 10;

	_newRowIndex = (int16)getGlobalVar(0x49C40058);
	if (_newRowIndex + _visibleRowsCount > _maxRowIndex)
		_newRowIndex = _maxRowIndex - _visibleRowsCount;
	if (_newRowIndex < 6)
		_newRowIndex = 0;

	_rowScrollY = 0;

	_backgroundScrollY = 48 * _newRowIndex;		

	_currRowIndex = _newRowIndex;

	for (int16 rowIndex = 0; rowIndex < _visibleRowsCount; rowIndex++)
		drawRow(_newRowIndex + rowIndex);

	_background->getSurface()->getSysRect().y = _backgroundScrollY;

	// TODO Screen.yOffset = _backgroundScrollY;
	// TODO Scene2208_sub409080 (creates background Sprites via the text, doesn't seem to be used?)

}

Scene2208::~Scene2208() {
	delete _fontSurface;
	delete _backgroundSurface;
	delete _topBackgroundSurface;
	delete _bottomBackgroundSurface;
}

void Scene2208::update() {

	int16 mouseY = _vm->getMouseY();
	
	if (mouseY < 48) {
		if (_currRowIndex > 0)
			_newRowIndex = _currRowIndex - 1;
	} else if (mouseY > 432) {
		if (_currRowIndex < _maxRowIndex - _visibleRowsCount)
			_newRowIndex = _currRowIndex + 1;
	} else {
		if (_currRowIndex > _newRowIndex)
			_newRowIndex = _currRowIndex;
	}

	if (_currRowIndex < _newRowIndex) {
		if (_rowScrollY == 0) {
			drawRow(_currRowIndex + _visibleRowsCount);
		}
		_backgroundScrollY += 4;
		_rowScrollY += 4;
		if (_rowScrollY == 48) {
			_rowScrollY = 0;
			_currRowIndex++;
		}
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	} else if (_currRowIndex > _newRowIndex || _rowScrollY > 0) {
		if (_rowScrollY == 0) {
			drawRow(_currRowIndex - 1);
			_currRowIndex--;
		}
		_backgroundScrollY -= 4;
		if (_rowScrollY == 0)
			_rowScrollY = 48;
		_rowScrollY -= 4;
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	}

	// TODO Screen.yOffset = _backgroundScrollY;
	Scene::update();

}

uint32 Scene2208::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 40 || param.asPoint().x >= 600) {
			leaveScene(0);
		}
		break;
	}
	return messageResult;
}

void Scene2208::createFontSurface() {
	DataResource fontData(_vm);
	SpriteResource spriteResource(_vm);
	fontData.load(calcHash("asRecFont"));
	uint16 numRows = fontData.getPoint(calcHash("meNumRows")).x;
	uint16 firstChar = fontData.getPoint(calcHash("meFirstChar")).x;
	uint16 charWidth = fontData.getPoint(calcHash("meCharWidth")).x;
	uint16 charHeight = fontData.getPoint(calcHash("meCharHeight")).x;
	NPointArray *tracking = fontData.getPointArray(calcHash("meTracking"));
	spriteResource.load2(0x0800090C);
	_fontSurface = new FontSurface(_vm, tracking, numRows, firstChar, charWidth, charHeight);
	_fontSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);
}

void Scene2208::drawRow(int16 rowIndex) {
	NDrawRect sourceRect;	
	int16 y = (rowIndex * 48) % 528;
	if (rowIndex < 4) {
		sourceRect.x = 0;
		sourceRect.y = y;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_topBackgroundSurface->getSurface(), 0, y, sourceRect, true);
	} else if (rowIndex > _maxRowIndex - 5) {
		sourceRect.x = 0;
		sourceRect.y = (rowIndex - _maxRowIndex + 4) * 48;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_bottomBackgroundSurface->getSurface(), 0, y, sourceRect, true);
	} else {
		rowIndex -= 4;
		sourceRect.x = 0;
		sourceRect.y = (rowIndex * 48) % 480;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_backgroundSurface->getSurface(), 0, y, sourceRect, true);
		if (rowIndex < (int)_strings.size()) {
			const char *text = _strings[rowIndex];
			// TODO/CHECKME: Use temporary string up to '{' character (see above)
			_fontSurface->drawString(_background->getSurface(), 95, y, (const byte*)text);
		}
	}
}

static const int16 kScene2242XPositions[] = {
	68, 158
};

static const uint32 kScene2242MessageListIds2[] = {
	0x004B3CB8, 0x004B3CD8
};

static const uint32 kScene2242MessageListIds1[] = {
	0x004B3CF8, 0x004B3D20
};

Scene2242::Scene2242(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isKlaymanInLight(false) {

	_surfaceFlag = true;
	SetMessageHandler(&Scene2242::handleMessage);
	SetUpdateHandler(&Scene2242::update);
	
	if (getGlobalVar(0x4D080E54)) {
		setBackground(0x11840E24);
		setPalette(0x11840E24);
		insertMouse433(0x40E20110);
		setRectList(0x004B3DC8);
	} else {
		setBackground(0x25848E24);
		setPalette(0x25848E24);
		addEntity(_palette);
		_palette->copyBasePalette(0, 256, 0);
		_palette->addPalette(0x68033B1C, 0, 65, 0);
		insertMouse433(0x48E20250);
		setRectList(0x004B3E18);
	}

	_asTape = insertSprite<AsScene1201Tape>(this, 10, 1100, 464, 435, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape); 

	if (which < 0) {
		insertKlayman<KmScene2242>(200, 430);
		setMessageList(0x004B3C18);
	} else if (which == 1) {
		insertKlayman<KmScene2242>(530, 430);
		setMessageList(0x004B3D60);
	} else if (which == 2) {
		insertKlayman<KmScene2242>(kScene2242XPositions[!getGlobalVar(0x48A68852) ? 0 : 1], 430);
		setMessageList(0x004B3D48);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
	} else {
		insertKlayman<KmScene2242>(0, 430);
		setMessageList(0x004B3C20);
	}

	_klayman->setSoundFlag(true);

}

Scene2242::~Scene2242() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

void Scene2242::update() {
	if (!getGlobalVar(0x4D080E54)) {
		if (_isKlaymanInLight && _klayman->getX() < 440) {
			_palette->addBasePalette(0x68033B1C, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymanInLight = false;
		} else if (!_isKlaymanInLight && _klayman->getX() >= 440) {
			_palette->addBasePalette(0x25848E24, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymanInLight = true;
		}
	}
	Scene::update();
}

uint32 Scene2242::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B3D50);
		}
		break;
	}
	return messageResult;
}

void Scene2242::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 108) {
		setGlobalVar(0xC8C28808, 0x04290188);
		setGlobalVar(0x48A68852, 42);
		setGlobalVar(0x4CE79018, calcHash("bgRecPanelStart1"));
		index = 0;
	} else {
		setGlobalVar(0xC8C28808, 0x04290188);
		setGlobalVar(0x48A68852, 43);
		setGlobalVar(0x4CE79018, calcHash("bgRecPanelStart2"));
		index = 1;
	}
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klayman->getX() - kScene2242XPositions[index]) < 133) {
		setMessageList2(kScene2242MessageListIds1[index]);
	} else {
		setMessageList2(kScene2242MessageListIds2[index]);
	}
}

static const int16 kHallOfRecordsKlaymanXPos[] = {
	 68, 157, 246, 335, 
	424, 513, 602
};

static const uint32 kHallOfRecordsSceneMessageListIds2[] = {
	0x004B2978,	0x004B2998,	0x004B29B8,	0x004B29D8,
	0x004B29F8,	0x004B2A18,	0x004B2A38
};

static const uint32 kHallOfRecordsSceneMessageListIds1[] = {
	0x004B2A58,	0x004B2A80,	0x004B2AA8,	0x004B2AD0,
	0x004B2AF8,	0x004B2B20,	0x004B2B48
};

HallOfRecordsScene::HallOfRecordsScene(NeverhoodEngine *vm, Module *parentModule, int which, uint32 sceneInfo140Id)
	: Scene(vm, parentModule, true) {
	
	_sceneInfo140 = _vm->_staticData->getSceneInfo140Item(sceneInfo140Id);

	_surfaceFlag = true;
	SetMessageHandler(&HallOfRecordsScene::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (!getGlobalVar(0x4D080E54) && _sceneInfo140->bgFilename2) {
		setRectList(0x004B2BF8);
		setBackground(_sceneInfo140->bgFilename2);
		setPalette(_sceneInfo140->bgFilename2);
		insertMouse433(0x14320138);
	} else {
		setRectList(0x004B2BB8);
		setBackground(_sceneInfo140->bgFilename1);
		setPalette(_sceneInfo140->bgFilename1);
		insertMouse433(0x63A40028);
	}

	if (which < 0) {
		insertKlayman<KmHallOfRecords>(200, 430);
		setMessageList(0x004B2900);
	} else if (which == 1) {
		insertKlayman<KmHallOfRecords>(640, 430);
		setMessageList(0x004B2910);
	} else if (which == 2) {
		insertKlayman<KmHallOfRecords>(kHallOfRecordsKlaymanXPos[getGlobalVar(0x48A68852) - _sceneInfo140->xPosIndex], 430);
		setMessageList(0x004B2B70);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
	} else {
		insertKlayman<KmHallOfRecords>(0, 430);
		setMessageList(0x004B2908);
	}

	_klayman->setSoundFlag(true);
	_klayman->setKlaymanIdleTable2();

}

HallOfRecordsScene::~HallOfRecordsScene() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 HallOfRecordsScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	}
	return messageResult;
}

void HallOfRecordsScene::readClickedColumn() {
	int16 index = (_mouseClickPos.x - 23) / 89;
	if (index >= _sceneInfo140->count) {
		setMessageList2(0x004B2920);
	} else {
		setGlobalVar(0x48A68852, _sceneInfo140->xPosIndex + index);
		setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
		setGlobalVar(0xC8C28808, _sceneInfo140->txFilename);
		if (index == 0 && _sceneInfo140->bgFilename3) {
			setGlobalVar(0x4CE79018, _sceneInfo140->bgFilename3);
		} else {
			setGlobalVar(0x4CE79018, 0);
		}
		if (ABS(_klayman->getX() - kHallOfRecordsKlaymanXPos[index]) < 133) {
			setMessageList2(kHallOfRecordsSceneMessageListIds1[index]);
		} else {
			setMessageList2(kHallOfRecordsSceneMessageListIds2[index]);
		}
	}
}

static const int16 kScene2247XPositions[] = {
	513, 602
};

static const uint32 kScene2247MessageListIds2[] = {
	0x004B54A0,	0x004B54C0
};

static const uint32 kScene2247MessageListIds1[] = {
	0x004B54E0,	0x004B5508
};

Scene2247::Scene2247(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	//DEBUG
	setGlobalVar(0x4D080E54, 1);

	_surfaceFlag = true;
	SetMessageHandler(&Scene2247::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (getGlobalVar(0x4D080E54)) {
		setRectList(0x004B5588);
		setBackground(0x40339414);
		setPalette(0x40339414);
		insertMouse433(0x3941040B);
	} else {
		setRectList(0x004B55C8);
		setBackground(0x071963E5);
		setPalette(0x071963E5);
		insertMouse433(0x14320138);
	}

	if (which < 0) {
		insertKlayman<KmScene2247>(200, 430);
		setMessageList(0x004B5428);
	} else if (which == 1) {
		insertKlayman<KmScene2247>(640, 430);
		setMessageList(0x004B5438);
	} else if (which == 2) {
		insertKlayman<KmScene2247>(kScene2247XPositions[getGlobalVar(0xC8C28808) == 0x0008E486 ? 0 : 1], 430);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
		setMessageList(0x004B5530);
	} else {
		insertKlayman<KmScene2247>(0, 430);
		setMessageList(0x004B5430);
	}

	_klayman->setSoundFlag(true);
	
}

Scene2247::~Scene2247() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2247::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	// TODO: Debug stuff
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	}
	return messageResult;
}

void Scene2247::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 553) {
		setGlobalVar(0xC8C28808, 0x0008E486);
		setGlobalVar(0x4CE79018, calcHash("bgFatherHeader"));
		index = 0;
	} else {
		setGlobalVar(0xC8C28808, 0x03086004);
		setGlobalVar(0x4CE79018, calcHash("bgQuaterHeader"));
		index = 1;
	}
	setGlobalVar(0x48A68852, 0);
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klayman->getX() - kScene2247XPositions[index]) < 133) {
		setMessageList2(kScene2247MessageListIds1[index]);
	} else {
		setMessageList2(kScene2247MessageListIds2[index]);
	}
}

} // End of namespace Neverhood
