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

#include "neverhood/module3000.h"

namespace Neverhood {

Module3000::Module3000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _moduleDone(false), _soundVolume(0) {
	
	debug("Create Module3000(%d)", which);

	// TODO Sound1ChList_addSoundResources(0x81293110, dword_4B7FC8, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 1, 50, 600, 5, 150);
	// TODO Sound1ChList_setSoundValues(0x90F0D1C3, false, 20000, 30000, 20000, 30000);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);

    _flag = getGlobalVar(0x10938830) != 0;

	if (_flag) {
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
		// TODO Sound1ChList_playLooping(0x90F0D1C3);
	}

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		default:
		case 1:
			createScene3002(-1);
			break;
		case 2:
			createScene3003(-1);
			break;
		case 3:
			createScene3004(-1);
			break;
		case 4:
			createScene3005(-1);
			break;
		case 5:
			createScene3006(-1);
			break;
		case 6:
			createScene3007(-1);
			break;
		case 7:
			createScene3008(-1);
			break;
		case 8:
			createScene3009(-1);
			break;
		case 9:
			createScene3010(-1);
			break;
		case 10:
			createScene3011(-1);
			break;
		case 11:
			createScene3012(-1);
			break;
		case 12:
			createScene3013(-1);
			break;
		}
	} else if (which == 0) {
		createScene3002(0);
	} else if (which == 1) {
		createScene3005(2);
	} else if (which == 2) {
		createScene3005(1);
	} else if (which == 3) {
		createScene3006(1);
	}

	SetMessageHandler(&Module3000::handleMessage);

}

Module3000::~Module3000() {
	// TODO Sound1ChList_sub_407A50(0x81293110);
}

void Module3000::createScene3002(int which) {
	// TODO
}

void Module3000::createScene3003(int which) {
	// TODO
}

void Module3000::createScene3004(int which) {
	// TODO
}

void Module3000::createScene3005(int which) {
	// TODO
}

void Module3000::createScene3006(int which) {
	// TODO
}

void Module3000::createScene3007(int which) {
	// TODO
}

void Module3000::createScene3008(int which) {
	// TODO
}

void Module3000::createScene3009(int which) {
	// TODO
}

void Module3000::createScene3010(int which) {
	_vm->gameState().sceneNum = 9;
	_childObject = new Scene3010(_vm, this, 0);
	SetUpdateHandler(&Module3000::updateScene3010);
}

void Module3000::createScene3011(int which) {
	// TODO
}

void Module3000::createScene3012(int which) {
	// TODO
}

void Module3000::createScene3013(int which) {
	// TODO
}

void Module3000::updateScene3010() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
	}
}

// Scene3010

static const uint32 kScene3010VarNameHashes[] = {
	0x304008D2,
	0x40119852,
	0x01180951
};

static const uint32 kScene3010DeadBoltButtonFileHashes1[] = {
	0x301024C2,
	0x20280580,
	0x30200452
};

static const uint32 kScene3010DeadBoltButtonFileHashes2[] = {
	0x50C025A8,
	0x1020A0A0,
	0x5000A7E8
};

static const NPoint kAsScene3010DeadBoltPoints[] = {
	{550, 307},
	{564, 415},
	{560, 514}
};

static const uint32 kAsScene3010DeadBoltFileHashes2[] = {
	0x181A0042,
	0x580A08F2,
	0x18420076
};

static const uint32 kAsScene3010DeadBoltFileHashes1[] = {
	0x300E105A,
	0x804E0052,
	0x040E485A
};

SsScene3010DeadBoltButton::SsScene3010DeadBoltButton(NeverhoodEngine *vm, Scene *parentScene, int buttonIndex, int initCountdown, bool initDisabled)
	: StaticSprite(vm, 900), _parentScene(parentScene), _soundResource1(vm),
	_soundResource2(vm), _soundResource3(vm), _buttonLocked(false), _countdown1(0), 
	_countdown2(0), _buttonIndex(buttonIndex) {

	NDimensions dimensions1, dimensions2;
	 
	_buttonEnabled = getSubVar(0x14800353, kScene3010VarNameHashes[_buttonIndex]) != 0;
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	dimensions1 = _spriteResource.getDimensions();
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	dimensions2 = _spriteResource.getDimensions();
	createSurface(400, 
		MAX(dimensions1.width, dimensions2.width),
		MAX(dimensions1.height, dimensions2.height));
	setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	if (initDisabled) {
		disableButton();
	} else if (_buttonEnabled) {
		_countdown1 = initCountdown * 12 + 1;
	}
	_soundResource1.load(0xF4217243);
	_soundResource2.load(0x44049000);
	_soundResource3.load(0x6408107E);
	SetUpdateHandler(&SsScene3010DeadBoltButton::update);
	SetMessageHandler(&SsScene3010DeadBoltButton::handleMessage);
}

void SsScene3010DeadBoltButton::update() {

	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		_soundResource1.play();
		_surface->setVisible(false);
		setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	}

	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		_surface->setVisible(true);
		setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	}

}

uint32 SsScene3010DeadBoltButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_buttonLocked && _countdown1 == 0) {
			if (_buttonEnabled) {
				_soundResource2.play();
				_soundResource3.play();
				_surface->setVisible(true);
				_buttonLocked = true;
				_parentScene->sendMessage(0x2000, _buttonIndex, this);
			} else {
				_parentScene->sendMessage(0x2002, _buttonIndex, this);
			}
			_needRefresh = true;
			StaticSprite::update();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void SsScene3010DeadBoltButton::disableButton() {
	_buttonLocked = true;
	setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	_surface->setVisible(true);
}

void SsScene3010DeadBoltButton::setSprite(uint32 fileHash) {
	_spriteResource.load(fileHash);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = _spriteResource.getDimensions().width;
	_deltaRect.height = _spriteResource.getDimensions().height;
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
}

void SsScene3010DeadBoltButton::setCountdown(int count) {
	_countdown2 = count * 18 + 1;
}

AsScene3010DeadBolt::AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_parentScene(parentScene), _boltIndex(boltIndex), _soundToggle(true), _unlocked(false), _locked(false),
	_countdown(0) {

    _x = kAsScene3010DeadBoltPoints[_boltIndex].x;
    _y = kAsScene3010DeadBoltPoints[_boltIndex].y;

	if (getSubVar(0x14800353, kScene3010VarNameHashes[_boltIndex])) {
		createSurface1(kAsScene3010DeadBoltFileHashes1[_boltIndex], 1200);
		setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
		_soundResource1.load(0x46005BC4);
	} else {
		createSurface1(kAsScene3010DeadBoltFileHashes2[_boltIndex], 1200);
		setFileHash(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		_soundResource1.load(0x420073DC);
		_soundResource2.load(0x420073DC);
	}
	
	_surface->setVisible(false);
	stIdle();
	if (initUnlocked)
		unlock(true);

	_needRefresh = true;
	AnimatedSprite::updatePosition();
	
}

void AsScene3010DeadBolt::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		stDisabled();
	}
}

uint32 AsScene3010DeadBolt::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene3010DeadBolt::stIdle() {
	setFileHash1();
	SetUpdateHandler(&AsScene3010DeadBolt::update);
	SetMessageHandler(&Sprite::handleMessage);
	_locked = false;
}

void AsScene3010DeadBolt::unlock(bool skipAnim) {
	if (!_unlocked) {
		_surface->setVisible(true);
		if (skipAnim) {
			setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], -1, 0);
			_newHashListIndex = -2;
		} else {
			setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
			SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
			setCallback1(AnimationCallback(&AsScene3010DeadBolt::stIdleMessage));
			SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
			_soundResource1.play();
		}
		_unlocked = true;
		_soundResource3.load(0x4010C345);
	}
}

void AsScene3010DeadBolt::stIdleMessage() {
	setFileHash1();
	SetMessageHandler(&Sprite::handleMessage);
	_parentScene->sendMessage(0x2001, _boltIndex, this);
}

void AsScene3010DeadBolt::lock() {
	if (!_locked) {
		_locked = true;
		_surface->setVisible(true);
		setFileHash(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
		setCallback1(AnimationCallback(&AsScene3010DeadBolt::stDisabledMessage));
		SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
		if (_soundToggle) {
			_soundResource1.play();
		} else {
			_soundResource2.play();
		}
		_soundToggle = !_soundToggle;
	}
}

void AsScene3010DeadBolt::setCountdown(int count) {
	_countdown = count * 18 + 1;
}

void AsScene3010DeadBolt::stDisabled() {
	_surface->setVisible(true);
	setFileHash(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
	SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
	setCallback1(AnimationCallback(&AsScene3010DeadBolt::stDisabledMessage));
	SetAnimationCallback3(&AsScene3010DeadBolt::stIdle);
	_playBackwards = true;
	_soundResource3.play();
}

void AsScene3010DeadBolt::stDisabledMessage() {
	_surface->setVisible(false);
	_parentScene->sendMessage(0x2003, _boltIndex, this);
}

Scene3010::Scene3010(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _countdown(0),
	_doorUnlocked(false), _checkUnlocked(false) {
	
	int initCountdown = 0;

	// DEBUG: Enable all buttons
    setSubVar(0x14800353, kScene3010VarNameHashes[0], 1);
    setSubVar(0x14800353, kScene3010VarNameHashes[1], 1);
    setSubVar(0x14800353, kScene3010VarNameHashes[2], 1);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x80802626, 0, 0));
	_palette = new Palette(_vm, 0x80802626);
	_palette->usePalette();

	for (int i = 0; i < 3; i++) {
		_asDeadBolts[i] = new AsScene3010DeadBolt(_vm, this, i, which == 1);//CHECKME
		addSprite(_asDeadBolts[i]);
		_ssDeadBoltButtons[i] = new SsScene3010DeadBoltButton(_vm, this, i, initCountdown, which == 1);//CHECKME
		addSprite(_ssDeadBoltButtons[i]);
		_vm->_collisionMan->addSprite(_ssDeadBoltButtons[i]);
		if (getSubVar(0x14800353, kScene3010VarNameHashes[i]))
			initCountdown++;
		_boltUnlocking[i] = false;
		_boltUnlocked[i] = false;
	}

	if (which == 0) {
		_mouseCursor = addSprite(new Mouse435(_vm, 0x02622800, 20, 620));
	}

	_soundResource.load(0x68E25540);

	SetMessageHandler(&Scene3010::handleMessage);
	SetUpdateHandler(&Scene3010::update);

	if (which == 1) {
		_checkUnlocked = true;
		for (int i = 0; i < 3; i++) {
			_boltUnlocked[i] = true;
			_ssDeadBoltButtons[i]->setCountdown(i + 1);
			_asDeadBolts[i]->setCountdown(i + 1);
		}
	}

}

void Scene3010::update() {
	Scene::update();
	if (_checkUnlocked && !_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
		_countdown = 24;
		_checkUnlocked = false;
	}
	if (_countdown != 0 && (--_countdown == 0)) {
		_parentModule->sendMessage(0x1009, _doorUnlocked ? 1 : 0, this);
	}
}

uint32 Scene3010::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && _countdown == 0 && !_checkUnlocked) {
			if (!_boltUnlocking[0] && !_boltUnlocking[1] && !_boltUnlocking[2]) {
				_mouseCursor->getSurface()->setVisible(false);
				if (!_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
					_countdown = 1;
				} else {
					_checkUnlocked = true;
					for (int i = 0; i < 3; i++) {
						_ssDeadBoltButtons[i]->setCountdown(i);
						if (_boltUnlocked[i]) {
							_asDeadBolts[i]->setCountdown(i);
						}
					}
				}
			}
		}
		break;
	case 0x000D:
		// TODO: Debug stuff
		break;
	case 0x2000:
		if (!_boltUnlocked[param.asInteger()] && !_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->unlock(false);
			_boltUnlocking[param.asInteger()] = true;
		}
		break;
	case 0x2001:
		_boltUnlocked[param.asInteger()] = true;
		_boltUnlocking[param.asInteger()] = false;
		if (_boltUnlocked[0] && _boltUnlocked[1] && _boltUnlocked[2]) {
			if (!getGlobalVar(0x00040153)) {
				setGlobalVar(0x00040153, 1);
				_soundResource.play();
				_countdown = 60;
			} else {
				_countdown = 48;
			}
			_doorUnlocked = true;
		}
		break;
	case 0x2002:
		if (!_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->lock();
		}
		break;
	case 0x2003:
		_boltUnlocked[param.asInteger()] = false;
		break;
	}
	return 0;
}

} // End of namespace Neverhood
