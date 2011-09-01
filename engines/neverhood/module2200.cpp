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
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

Module2200::Module2200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module2200(%d)", which);

	// TODO: Music18hList_add(0x11391412, 0x601C908C); 

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		default:
			createScene2201(-1);
			break;
		case 1:
			createScene2202(-1);
			break;
		case 2:
			createScene2203(-1);
			break;
		case 3:
			createScene2204(-1);
			break;
		case 4:
			createScene2205(-1);
			break;
		case 5:
			createScene2206(-1);
			break;
		case 6:
			createScene2207(-1);
			break;
		case 7:
			createScene2208(-1);
			break;
		case 8:
			createScene2209(-1);
			break;
		case 9:
			createScene2210(-1);
			break;
		case 10:
			createScene2211(-1);
			break;
		case 11:
			createScene2212(-1);
			break;
		case 12:
			createScene2213(-1);
			break;
		case 13:
			createScene2214(-1);
			break;
		case 14:
			createScene2215(-1);
			break;
		case 15:
			createScene2216(-1);
			break;
		case 16:
			createScene2217(-1);
			break;
		case 17:
			createScene2218(-1);
			break;
		case 18:
			createScene2219(-1);
			break;
		case 19:
			createScene2220(-1);
			break;
		case 20:
			createScene2221(-1);
			break;
		case 21:
			createScene2222(-1);
			break;
		case 22:
			createScene2223(-1);
			break;
		case 23:
			createScene2224(-1);
			break;
		case 24:
			createScene2225(-1);
			break;
		case 25:
			createScene2226(-1);
			break;
		case 26:
			createScene2227(-1);
			break;
		case 27:
			createScene2228(-1);
			break;
		case 28:
			createScene2229(-1);
			break;
		case 29:
			createScene2230(-1);
			break;
		case 30:
			createScene2231(-1);
			break;
		case 31:
			createScene2232(-1);
			break;
		case 32:
			createScene2233(-1);
			break;
		case 33:
			createScene2234(-1);
			break;
		case 34:
			createScene2235(-1);
			break;
		case 35:
			createScene2236(-1);
			break;
		case 36:
			createScene2237(-1);
			break;
		case 37:
			createScene2238(-1);
			break;
		case 38:
			createScene2239(-1);
			break;
		case 39:
			createScene2240(-1);
			break;
		case 40:
			createScene2241(-1);
			break;
		case 41:
			createScene2242(-1);
			break;
		case 42:
			createScene2243(-1);
			break;
		case 43:
			createScene2244(-1);
			break;
		case 44:
			createScene2245(-1);
			break;
		case 45:
			createScene2246(-1);
			break;
		case 46:
			createScene2247(-1);
			break;
		case 47:
			createScene2248(-1);
			break;
		}
	} else {
		createScene2201(0);
	}

}

Module2200::~Module2200() {
	// TODO Sound1ChList_sub_407A50(0x11391412);
}

void Module2200::createScene2201(int which) {
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene2201(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2201);
}

void Module2200::createScene2202(int which) {
	// TODO Music18hList_play(0x601C908C, 0, 2, 1);
	_vm->gameState().sceneNum = 1;
	_childObject = new Scene2202(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2202);
}

void Module2200::createScene2203(int which) {
	// TODO Music18hList_play(0x601C908C, 0, 2, 1);
	_vm->gameState().sceneNum = 2;
	_childObject = new Scene2203(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2203);
}

void Module2200::createScene2204(int which) {
	_vm->gameState().sceneNum = 3;
	// TODO Music18hList_stop(0x601C908C, 0, 2);
	_childObject = new DiskplayerScene(_vm, this, 3);
	SetUpdateHandler(&Module2200::updateScene2204);
}
			
void Module2200::createScene2205(int which) {
	_vm->gameState().sceneNum = 4;
	// TODO Music18hList_stop(0x601C908C, 0, 2);
	_childObject = new Scene2205(_vm, this, 3);
	SetUpdateHandler(&Module2200::updateScene2205);
}
			
void Module2200::createScene2206(int which) {
}
			
void Module2200::createScene2207(int which) {
}
			
void Module2200::createScene2208(int which) {
}
			
void Module2200::createScene2209(int which) {
}
			
void Module2200::createScene2210(int which) {
}
			
void Module2200::createScene2211(int which) {
}
			
void Module2200::createScene2212(int which) {
}
			
void Module2200::createScene2213(int which) {
}
			
void Module2200::createScene2214(int which) {
}
			
void Module2200::createScene2215(int which) {
}
			
void Module2200::createScene2216(int which) {
}
			
void Module2200::createScene2217(int which) {
}
			
void Module2200::createScene2218(int which) {
}
			
void Module2200::createScene2219(int which) {
}
			
void Module2200::createScene2220(int which) {
}
			
void Module2200::createScene2221(int which) {
}
			
void Module2200::createScene2222(int which) {
}
			
void Module2200::createScene2223(int which) {
}
			
void Module2200::createScene2224(int which) {
}
			
void Module2200::createScene2225(int which) {
}
			
void Module2200::createScene2226(int which) {
}
			
void Module2200::createScene2227(int which) {
}
			
void Module2200::createScene2228(int which) {
}
			
void Module2200::createScene2229(int which) {
}
			
void Module2200::createScene2230(int which) {
}
			
void Module2200::createScene2231(int which) {
}
			
void Module2200::createScene2232(int which) {
}
			
void Module2200::createScene2233(int which) {
}
			
void Module2200::createScene2234(int which) {
}
			
void Module2200::createScene2235(int which) {
}
			
void Module2200::createScene2236(int which) {
}
			
void Module2200::createScene2237(int which) {
}
			
void Module2200::createScene2238(int which) {
}
			
void Module2200::createScene2239(int which) {
}
			
void Module2200::createScene2240(int which) {
}
			
void Module2200::createScene2241(int which) {
}
			
void Module2200::createScene2242(int which) {
}
			
void Module2200::createScene2243(int which) {
}
			
void Module2200::createScene2244(int which) {
}
			
void Module2200::createScene2245(int which) {
}
			
void Module2200::createScene2246(int which) {
}
			
void Module2200::createScene2247(int which) {
}
			
void Module2200::createScene2248(int which) {
}
			
void Module2200::updateScene2201() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2203(0);
		} else if (_field20 == 2) {
			createScene2202(0);
		} else {
			_parentModule->sendMessage(0x1009, 0, this);
		}
	}
}
			
void Module2200::updateScene2202() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2201(2);
	}
}

void Module2200::updateScene2203() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2205(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene2204(0);
			_childObject->handleUpdate();
		} else {
			createScene2201(1);
			_childObject->handleUpdate();
		}
	}
}
			
void Module2200::updateScene2204() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2203(2);
	}
}
			
void Module2200::updateScene2205() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2206(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene2205(2);
			_childObject->handleUpdate();
		} else {
			createScene2203(1);
			_childObject->handleUpdate();
		}
	}
}
			
void Module2200::updateScene2206() {
}
			
void Module2200::updateScene2207() {
}
			
void Module2200::updateScene2208() {
}
			
void Module2200::updateScene2209() {
}
			
void Module2200::updateScene2210() {
}
			
void Module2200::updateScene2211() {
}
			
void Module2200::updateScene2212() {
}
			
void Module2200::updateScene2213() {
}
			
void Module2200::updateScene2214() {
}
			
void Module2200::updateScene2215() {
}
			
void Module2200::updateScene2216() {
}
			
void Module2200::updateScene2217() {
}
			
void Module2200::updateScene2218() {
}
			
void Module2200::updateScene2219() {
}
			
void Module2200::updateScene2220() {
}
			
void Module2200::updateScene2221() {
}
			
void Module2200::updateScene2222() {
}
			
void Module2200::updateScene2223() {
}
			
void Module2200::updateScene2224() {
}
			
void Module2200::updateScene2225() {
}
			
void Module2200::updateScene2226() {
}
			
void Module2200::updateScene2227() {
}
			
void Module2200::updateScene2228() {
}
			
void Module2200::updateScene2229() {
}
			
void Module2200::updateScene2230() {
}
			
void Module2200::updateScene2231() {
}
			
void Module2200::updateScene2232() {
}
			
void Module2200::updateScene2233() {
}
			
void Module2200::updateScene2234() {
}
			
void Module2200::updateScene2235() {
}
			
void Module2200::updateScene2236() {
}
			
void Module2200::updateScene2237() {
}
			
void Module2200::updateScene2238() {
}
			
void Module2200::updateScene2239() {
}
			
void Module2200::updateScene2240() {
}
			
void Module2200::updateScene2241() {
}
			
void Module2200::updateScene2242() {
}
			
void Module2200::updateScene2243() {
}
			
void Module2200::updateScene2244() {
}
			
void Module2200::updateScene2245() {
}
			
void Module2200::updateScene2246() {
}
			
void Module2200::updateScene2247() {
}
			
void Module2200::updateScene2248() {
}

// Scene2201

AsScene2201CeilingFan::AsScene2201CeilingFan(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 403;
	_y = 259;
	createSurface(100, 233, 96);
	setFileHash(0x8600866, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
}

AsScene2201Door::AsScene2201Door(NeverhoodEngine *vm, Klayman *klayman, Sprite *doorLightSprite, bool flag1)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _doorLightSprite(doorLightSprite), 
	_countdown(0), _doorOpen(flag1) {

	_x = 408;
	_y = 290;	
	createSurface(900, 63, 266);
	SetUpdateHandler(&AsScene2201Door::update);
	SetMessageHandler(&AsScene2201Door::handleMessage);
	if (_doorOpen) {
		setFileHash(0xE2CB0412, -1, -1);
		_countdown = 48;
		_newHashListIndex = -2;
	} else {
		setFileHash(0xE2CB0412, 0, -1);
		_newHashListIndex = 0;
		_doorLightSprite->getSurface()->setVisible(false);
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
				_doorLightSprite->getSurface()->setVisible(true);
		} else if (param.asInteger() == 0x11283090) {
			if (!_doorOpen)
				_doorLightSprite->getSurface()->setVisible(false);
		}
		break;
	case 0x2000:
		if (_doorOpen)
			_countdown = 144;
		messageResult = _doorOpen ? 1 : 0;
		break;
	case 0x3002:
		removeCallbacks();
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
	setFileHash(0xE2CB0412, 0, -1);
	_newHashListIndex = -2;
	_soundResource.play(calcHash("fxDoorOpen33"));
}

void AsScene2201Door::stCloseDoor() {
	_doorOpen = false;
	setFileHash(0xE2CB0412, -1, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
	_soundResource.play(calcHash("fxDoorClose33"));
}

Class444::Class444(NeverhoodEngine *vm, int pointIndex, int spriteIndex)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(kClass444FileHashes[spriteIndex]);
	createSurface(100, 16, 16);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = kClass444Points[pointIndex].x;
	_y = kClass444Points[pointIndex].y;
	_needRefresh = true;
}

Scene2201::Scene2201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundFlag(false) {

	Sprite *tempSprite;

	if (!getSubVar(0x40050052, 0x60400854)) {
		// TODO _vm->gameModule()->initScene2201Vars();
	}

	_surfaceFlag = true;
	SetMessageHandler(&Scene2201::handleMessage);
	SetUpdateHandler(&Scene2201::update);
	
	loadDataResource(0x04104242);
	loadHitRectList();

	_background = addBackground(new DirtyBackground(_vm, 0x40008208, 0, 0));
	_palette = new Palette(_vm, 0x40008208);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x0820C408, NULL));

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 7, 1100, 459, 432, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape); 

	_ssDoorButton = addSprite(new Class426(_vm, this, 0xE4A43E29, 0xE4A43E29, 100, 0));
	
	for (uint32 i = 0; i < 9; i++) {
		if ((int16)getSubVar(0x484498D0, i) >= 0) {
			addSprite(new Class444(_vm, i, (int16)getSubVar(0x484498D0, i)));
		}
	}

	_rect1.y1 = 0;
	_rect1.x2 = 640;
	_rect2.x2 = 640;
	_rect2.y2 = 480;
	
	if (!getGlobalVar(0x404290D5)) {
		addSprite(new StaticSprite(_vm, 0x00026027, 900));
	}
	
	tempSprite = addSprite(new StaticSprite(_vm, 0x030326A0, 1100));
	_rect1.x1 = tempSprite->getSurface()->getDrawRect().x;
	
	addSprite(new StaticSprite(_vm, 0x811DA061, 1100));

	tempSprite = addSprite(new StaticSprite(_vm, 0x11180022, 1100));
	_rect2.x1 = tempSprite->getSurface()->getDrawRect().x;

	tempSprite = addSprite(new StaticSprite(_vm, 0x0D411130, 1100));
	_rect1.y2 = tempSprite->getSurface()->getDrawRect().y + tempSprite->getSurface()->getDrawRect().height;
	_rect2.y1 = tempSprite->getSurface()->getDrawRect().y + tempSprite->getSurface()->getDrawRect().height;
	
	_doorLightSprite = addSprite(new StaticSprite(_vm, 0xA4062212, 900));

	if (which < 0) {
		_klayman = new KmScene2201(_vm, this, 300, 427, &_rect1, 2);
		setMessageList(0x004B8118);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else if (which == 1) {
		_klayman = new KmScene2201(_vm, this, 412, 393, &_rect1, 2);
		setMessageList(0x004B8130);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			_klayman = new KmScene2201(_vm, this, 379, 427, &_rect1, 2);
			_klayman->setDoDeltaX(1);
		} else {
			_klayman = new KmScene2201(_vm, this, 261, 427, &_rect1, 2);
		}
		setMessageList(0x004B8178);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else {
		NPoint pt = _dataResource.getPoint(0x0304D8DC);
		_klayman = new KmScene2201(_vm, this, pt.x, pt.y, &_rect1, 2);
		setMessageList(0x004B8120);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, true));
	}
	addSprite(_klayman);
	
	addSprite(new AsScene2201CeilingFan(_vm));

	// TODO Sound1ChList_addSoundResource(0x04106220, 0x81212040, true);

}

Scene2201::~Scene2201() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
	// TODO Sound1ChList_sub_407AF0(0x04106220);
}

void Scene2201::update() {
	Scene::update();
	if (!_soundFlag) {
		// TODO Sound1ChList_playLooping(0x81212040);
		_soundFlag = true;
	}
}

uint32 Scene2201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8) {
			_klayman->sendEntityMessage(0x1014, _ssDoorButton, this);
		} else if (param.asInteger() == 0x35803198) {
			if (_asDoor->hasMessageHandler() && _asDoor->sendMessage(0x2000, 0, this)) {
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
			_asDoor->sendMessage(0x4808, 0, this);
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			_klayman->sendEntityMessage(0x1014, _asTape, this);
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
	: StaticSprite(vm, 900), _soundResource1(vm), _soundResource2(vm), _parentScene(parentScene),
	_value(value), _tileIndex(tileIndex), _isMoving(false) {
	
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
	_soundResource1.load(0x40958621);
	_soundResource2.load(0x51108241);
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
			_parentScene->sendMessage(0x2000, _tileIndex, this);
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
		SetSpriteCallback(&SsScene2202PuzzleTile::suMoveTileX);
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
		_soundResource1.play();
	} else {
		SetSpriteCallback(&SsScene2202PuzzleTile::suMoveTileY);
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
		_soundResource2.play();
	}
	
}

void SsScene2202PuzzleTile::stopMoving() {
	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes2[_value]);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;
	SetSpriteCallback(NULL);
	_isMoving = false;
	_parentScene->sendMessage(0x2002, _tileIndex, this);
}

Scene2202::Scene2202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm),
	_isSolved(false), _leaveScene(false), _isTileMoving(false), _movingTileSprite(NULL), _doneMovingTileSprite(NULL) {

	Palette2 *palette2;

	// TODO initScene2201Vars();
	SetMessageHandler(&Scene2202::handleMessage);
	SetUpdateHandler(&Scene2202::update);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x08100A0C, 0, 0));
	palette2 = new Palette2(_vm, 0x08100A0C);
	_palette = palette2;
	_palette->usePalette();
	addEntity(palette2);
	_mouseCursor = addSprite(new Mouse435(_vm, 0x00A08089, 20, 620));

	//DEBUG!
	for (uint32 index = 0; index < 9; index++)
		setSubVar(0x484498D0, index, 7 - index);

	for (uint32 index = 0; index < 9; index++) {
		int16 value = (int16)getSubVar(0x484498D0, index);
		if (value >= 0) {
			Sprite *puzzleTileSprite = addSprite(new SsScene2202PuzzleTile(_vm, this, index, value));
			_vm->_collisionMan->addSprite(puzzleTileSprite);
		}
	}

	addSprite(new StaticSprite(_vm, 0x55C043B8, 200));
	addSprite(new StaticSprite(_vm, 0x85500158, 400));
	addSprite(new StaticSprite(_vm, 0x25547028, 600));

	_soundResource1.load(0x68E25540);
	_soundResource2.load(0x40400457);

	// TODO Sound1ChList_addSoundResource(0x60400854, 0x8101A241, true);
	// TODO Sound1ChList_playLooping(0x8101A241);

}

Scene2202::~Scene2202() {
	// TODO Sound1ChList_sub_407AF0(0x60400854);
}

void Scene2202::update() {
	Scene::update();

	if (_leaveScene && !_soundResource2.isPlaying()) {
		_parentModule->sendMessage(0x1009, 0, this);
	}

	if (_isSolved && !_soundResource1.isPlaying()) {
		_soundResource2.play();
		_isSolved = false;
		_leaveScene = true;
	}

	if (_movingTileSprite && !_isTileMoving) {
		int16 value = getFreeTileIndex(_movingTileIndex);
		if (value != -1) {
			setSurfacePriority(_movingTileSprite->getSurface(), 700);
			_movingTileSprite->sendMessage(0x2001, value, this);
			_movingTileSprite = NULL;
			_isTileMoving = true;
		}
	}

	if (_doneMovingTileSprite) {
		setSurfacePriority(_doneMovingTileSprite->getSurface(), _surfacePriority);
		_doneMovingTileSprite = NULL;
		if (testIsSolved()) {
			_soundResource1.play();
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
			_parentModule->sendMessage(0x1009, 0, this);
		}
		break;
	case 0x000D:
		// TODO Debug stuff
		break;
	case 0x2000:
		_movingTileIndex = (int16)param.asInteger();
		_movingTileSprite = (Sprite*)sender;
		break;
	case 0x2002:
		_isTileMoving = false;
		_doneMovingTileSprite = (Sprite*)sender;
		if (param.asInteger() >= 0 && param.asInteger() <= 2) {
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

static const uint32 kClass545FileHashes[] = {
	0x2450D850,
	0x0C9CE8D0,
	0x2C58A152
};

Class545::Class545(NeverhoodEngine *vm, Scene *parentScene, int index, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, kClass545FileHashes[index], surfacePriority, x, y), _parentScene(parentScene), _index(index) {

	if (!getSubVar(0x0090EA95, _index) && !getSubVar(0x08D0AB11, _index)) {
		SetMessageHandler(&Class545::handleMessage);
	} else {
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	}
}

uint32 Class545::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
		messageResult = 1;
		break;
	case 0x4806:
		setSubVar(0x0090EA95, _index, 1);
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	}
	return messageResult;
}

static const uint32 kAsScene2203DoorFileHashes[] = {
	0x7868AE10,
	0x1A488110
};

AsScene2203Door::AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, int index)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene),
	_index(index) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2203Door::handleMessage);
	_x = 320;
	_y = 240;
	createSurface1(kAsScene2203DoorFileHashes[_index], 900);
	if (getGlobalVar(0x9A500914) == _index) {
		setFileHash(kAsScene2203DoorFileHashes[_index], -1, -1);
		_newHashListIndex = -2;
	} else {
		setFileHash(kAsScene2203DoorFileHashes[_index], 0, -1);
		_newHashListIndex = 0;
	}
}

uint32 AsScene2203Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_index == getGlobalVar(0x9A500914))
			_parentScene->sendMessage(0x2002, 0, this);
		else
			_parentScene->sendMessage(0x2001, 0, this);
		messageResult = 1;
		break;
	case 0x2000:
		_otherDoor = (Sprite*)param.asEntity();
		break;
	case 0x3002:
		if (_index == getGlobalVar(0x9A500914))
			_parentScene->sendMessage(0x4808, 0, this);
		setFileHash1();
		break;
	case 0x4808:
		setGlobalVar(0x9A500914, _index);
		_otherDoor->sendMessage(0x4809, 0, this);
		openDoor();
		break;
	case 0x4809:
		closeDoor();
		_parentScene->sendMessage(0x2003, 0, this);
		break;
	}
	return messageResult;
}

void AsScene2203Door::openDoor() {
	_soundResource.play(0x341014C4);
	setFileHash(kAsScene2203DoorFileHashes[_index], 1, -1);
}

void AsScene2203Door::closeDoor() {
	setFileHash(kAsScene2203DoorFileHashes[_index], -1, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
}

Scene2203::Scene2203(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	if (getGlobalVar(0xC0780812) && !getGlobalVar(0x13382860))
		setGlobalVar(0x13382860, 1);

	SetMessageHandler(&Scene2203::handleMessage);
	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x82C80334, 0, 0));
	_palette = new Palette(_vm, 0x82C80334);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x80330824, NULL));

	_vm->_collisionMan->setHitRects(0x004B8320);

	if (getGlobalVar(0x13382860) == 1) {
		_class545 = addSprite(new Class545(_vm, this, 2, 1100, 282, 432));
		_vm->_collisionMan->addSprite(_class545);
	}

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 1, 1100, 435, 432, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape);

	_asLeftDoor = addSprite(new AsScene2203Door(_vm, this, 0));
	_asRightDoor = addSprite(new AsScene2203Door(_vm, this, 1));
	
	_ssSmallLeftDoor = addSprite(new StaticSprite(_vm, 0x542CC072, 1100));
	_ssSmallRightDoor = addSprite(new StaticSprite(_vm, 0x0A2C0432, 1100));
	
	_leftDoorClipRect.x1 = _ssSmallLeftDoor->getSurface()->getDrawRect().x;
	_leftDoorClipRect.y1 = 0;
	_leftDoorClipRect.x2 = 640;
	_leftDoorClipRect.y2 = 480;
	
	_rightDoorClipRect.x1 = 0;
	_rightDoorClipRect.y1 = 0;
	_rightDoorClipRect.x2 = _ssSmallRightDoor->getSurface()->getDrawRect().x + _ssSmallRightDoor->getSurface()->getDrawRect().width;
	_rightDoorClipRect.y2 = 480;

	_asLeftDoor->sendEntityMessage(0x2000, _asRightDoor, this);
	_asRightDoor->sendEntityMessage(0x2000, _asLeftDoor, this);
	
	_vm->_collisionMan->addSprite(_asLeftDoor);
	_vm->_collisionMan->addSprite(_asRightDoor);

	if (which < 0) {
		_klayman = new KmScene2203(_vm, this, 200, 427);
		setMessageList(0x004B8340);
	} else if (which == 1) {
		_klayman = new KmScene2203(_vm, this, 640, 427);
		setMessageList(0x004B8350);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			_klayman = new KmScene2203(_vm, this, 362, 427);
			_klayman->setDoDeltaX(1);
		} else {
			_klayman = new KmScene2203(_vm, this, 202, 427);
		}
		setMessageList(0x004B8358);
	} else {
		_klayman = new KmScene2203(_vm, this, 0, 427);
		setMessageList(0x004B8348);
	}
	addSprite(_klayman); 

	if (getGlobalVar(0x9A500914)) {
		_ssSmallLeftDoor->getSurface()->setVisible(false);
		_klayman->getSurface()->getClipRect() = _rightDoorClipRect;
	} else {
		_ssSmallRightDoor->getSurface()->setVisible(false);
		_klayman->getSurface()->getClipRect() = _leftDoorClipRect;
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
		_klayman->sendEntityMessage(0x1014, sender, this);
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
			_ssSmallLeftDoor->getSurface()->setVisible(false);
		} else {
			_ssSmallRightDoor->getSurface()->setVisible(false);
		}
		break;
	case 0x4808:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->getSurface()->setVisible(true);
			_klayman->getSurface()->getClipRect() = _leftDoorClipRect;
		} else {
			_ssSmallRightDoor->getSurface()->setVisible(true);
			_klayman->getSurface()->getClipRect() = _rightDoorClipRect;
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			_klayman->sendEntityMessage(0x1014, _asTape, this);
			setMessageList(0x004B83E0);
		} else if (sender == _class545) {
			_klayman->sendEntityMessage(0x1014, _class545, this);
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

	Palette2 *palette2;
	
	SetMessageHandler(&Scene2205::handleMessage);
	SetUpdateHandler(&Scene2205::update);
	
	_vm->_collisionMan->setHitRects(0x004B0620);
	_surfaceFlag = true;

	if (getGlobalVar(0x4D080E54)) {
		_isLightOn = true;
		_background = addBackground(new DirtyBackground(_vm, 0x0008028D, 0, 0));
		palette2 = new Palette2(_vm, 0x0008028D);
		_palette = palette2;
		_palette->usePalette();
		addEntity(palette2);
		_mouseCursor = addSprite(new Mouse433(_vm, 0x80289008, NULL));
		_ssLightSwitch = new Class426(_vm, this, 0x2D339030, 0x2D309030, 100, 0);
		addSprite(_ssLightSwitch);
	} else {
		_isLightOn = false;
		_background = addBackground(new DirtyBackground(_vm, 0xD00A028D, 0, 0));
		palette2 = new Palette2(_vm, 0xD00A028D);
		_palette = palette2;
		_palette->usePalette();
		addEntity(palette2);
		_mouseCursor = addSprite(new Mouse433(_vm, 0xA0289D08, NULL));
		_ssLightSwitch = new Class426(_vm, this, 0x2D339030, 0xDAC86E84, 100, 0);
		addSprite(_ssLightSwitch);
	}

	palette2->addPalette(0xD00A028D, 0, 256, 0);
	
	_ssDoorFrame = addSprite(new SsScene2205DoorFrame(_vm));

	if (which < 0) {
		_klayman = new KmScene2205(_vm, this, 320, 417);
		setMessageList(0x004B0658);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else if (which == 1) {
		_klayman = new KmScene2205(_vm, this, 640, 417);
		setMessageList(0x004B0648);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else {
		_klayman = new KmScene2205(_vm, this, 0, 417);
		setMessageList(0x004B0640);
		_isKlaymanInLight = true;
	}
	addSprite(_klayman);

	_klayman->getSurface()->getClipRect().x1 = _ssDoorFrame->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

	loadDataResource(0x00144822);
	_klayman->setSoundFlag(true);

}
	
void Scene2205::update() {
	Scene::update();

	if (!_isLightOn && getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0x0008028D, 0, 256, 0);
		_background->load(0x0008028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0x2D309030);
		_ssDoorFrame->sendMessage(0x2000, 0, this);
		((Mouse433*)_mouseCursor)->load(0x80289008);
		((Mouse433*)_mouseCursor)->updateCursor();
		_isLightOn = true;
	} else if (_isLightOn && !getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0xD00A028D, 0, 256, 0);
		_background->load(0xD00A028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0xDAC86E84);
		_ssDoorFrame->sendMessage(0x2000, 0, this);
		((Mouse433*)_mouseCursor)->load(0xA0289D08);
		((Mouse433*)_mouseCursor)->updateCursor();
		_isKlaymanInLight = true;
		if (_klayman->getX() > 85) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
			_isKlaymanInLight = false;
		}
		_isLightOn = false;
	}

	if (!getGlobalVar(0x4D080E54)) {
		if (_isKlaymanInLight && _klayman->getX() > 85) {
			((Palette2*)_palette)->addPalette(0x68033B1C, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
			_isKlaymanInLight = false;
		} else if (!_isKlaymanInLight && _klayman->getX() <= 85) {
			((Palette2*)_palette)->addPalette(0xD00A028D, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
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
			_klayman->sendEntityMessage(0x1014, _ssLightSwitch, this);
		}
		break;
	case 0x480B:
		setGlobalVar(0x4D080E54, getGlobalVar(0x4D080E54) ? 0 : 1);
		break;
	}
	return 0;
}

} // End of namespace Neverhood
