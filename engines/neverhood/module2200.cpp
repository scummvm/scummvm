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
}

void Module2200::createScene2203(int which) {
}

void Module2200::createScene2204(int which) {
}
			
void Module2200::createScene2205(int which) {
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
	// TODO
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
	}
}
			
void Module2200::updateScene2202() {
}

void Module2200::updateScene2203() {
}
			
void Module2200::updateScene2204() {
}
			
void Module2200::updateScene2205() {
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
				
} // End of namespace Neverhood
