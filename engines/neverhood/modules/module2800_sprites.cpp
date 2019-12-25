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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/modules/module2800.h"
#include "neverhood/modules/module2800_sprites.h"

namespace Neverhood {

AsScene2803LightCord::AsScene2803LightCord(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _fileHash1(fileHash1), _fileHash2(fileHash2),
	_isPulled(false), _isBusy(false) {

	createSurface(1010, 28, 379);
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = y;
	stIdle();
}

uint32 AsScene2803LightCord::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (!_isBusy && param.asInteger() == calcHash("ClickSwitch")) {
			sendMessage(_parentScene, NM_KLAYMEN_LOWER_LEVER, 0);
			playSound(0, 0x4E1CA4A0);
		}
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		stPulled();
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene2803LightCord::hmPulled(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2803LightCord::stPulled() {
	_isBusy = false;
	_isPulled = true;
	startAnimation(_fileHash2, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::hmPulled);
	NextState(&AsScene2803LightCord::stIdle);
}

void AsScene2803LightCord::stIdle() {
	_isPulled = false;
	startAnimation(_fileHash1, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::handleMessage);
}

void AsScene2803LightCord::setFileHashes(uint32 fileHash1, uint32 fileHash2) {
	_fileHash1 = fileHash1;
	_fileHash2 = fileHash2;
	if (_isPulled) {
		startAnimation(_fileHash2, _currFrameIndex, -1);
		_isBusy = true;
	} else {
		startAnimation(_fileHash1, 0, -1);
	}
}

AsScene2803TestTubeOne::AsScene2803TestTubeOne(NeverhoodEngine *vm, uint32 fileHash1, uint32 fileHash2)
	: AnimatedSprite(vm, 1200), _fileHash1(fileHash1), _fileHash2(fileHash2) {

	createSurface1(fileHash1, 100);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2803TestTubeOne::handleMessage);
	_x = 529;
	_y = 326;
}

uint32 AsScene2803TestTubeOne::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		if (param.asInteger())
			startAnimation(_fileHash2, 0, -1);
		else
			startAnimation(_fileHash1, 0, -1);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2803Rope::AsScene2803Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&AsScene2803Rope::handleMessage);
	startAnimation(0x9D098C23, 35, 53);
	NextState(&AsScene2803Rope::stReleased);
	_x = x;
	_y = -276;
}

uint32 AsScene2803Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		startAnimation(0x9D098C23, 50, -1);
		SetMessageHandler(&AsScene2803Rope::hmReleased);
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene2803Rope::hmReleased(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2803Rope::stReleased() {
	startAnimation(0x8258A030, 0, 1);
	NextState(&AsScene2803Rope::stHide);
}

void AsScene2803Rope::stHide() {
	stopAnimation();
	setVisible(false);
}

SsScene2804RedButton::SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene) {

	loadSprite(getGlobalVar(V_SHRINK_LIGHTS_ON) ? 0x51A10202 : 0x11814A21, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	SetUpdateHandler(&SsScene2804RedButton::update);
	SetMessageHandler(&SsScene2804RedButton::handleMessage);
	loadSound(0, 0x44241240);
}

void SsScene2804RedButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804RedButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			playSound(0);
			setVisible(true);
			_countdown = 4;
			sendMessage(_parentScene, 0x2000, 0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene2804LightCoil::SsScene2804LightCoil(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	loadSprite(0x8889B008, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	SetMessageHandler(&SsScene2804LightCoil::handleMessage);
}

uint32 SsScene2804LightCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_POSITION_CHANGE:
		setVisible(true);
		updatePosition();
		messageResult = 1;
		break;
	case 0x2003:
		setVisible(false);
		updatePosition();
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene2804LightTarget::SsScene2804LightTarget(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	loadSprite(0x06092132, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	SetMessageHandler(&SsScene2804LightTarget::handleMessage);
}

uint32 SsScene2804LightTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		updatePosition();
		messageResult = 1;
		break;
	case NM_KLAYMEN_CLIMB_LADDER:
		setVisible(false);
		updatePosition();
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene2804Flash::SsScene2804Flash(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	loadSprite(0x211003A0, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	loadSound(0, 0xCB36BA54);
}

void SsScene2804Flash::show() {
	setVisible(true);
	updatePosition();
	playSound(0);
}

SsScene2804BeamCoilBody::SsScene2804BeamCoilBody(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	loadSprite(0x9A816000, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
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
		setDoDeltaY(1);
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
	: AnimatedSprite(vm, 1100), _asCrystalWaves(asCrystalWaves), _crystalIndex(crystalIndex), _isShowing(false) {

	static const NPoint kAsScene2804CrystalPoints[] = {
		{204, 196},
		{272, 316},
		{334, 206},
		{410, 334},
		{470, 180}
	};

	_colorNum = (int16)getSubVar(VA_CURR_CRYSTAL_COLORS, crystalIndex);
	_isLightOn = getGlobalVar(V_SHRINK_LIGHTS_ON) != 0;
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
	loadSound(0, 0x725294D4);
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene2804Crystal::show() {
	if (!_isLightOn) {
		setVisible(true);
		_isShowing = true;
		if (_asCrystalWaves)
			_asCrystalWaves->show();
		playSound(0);
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
		setSubVar(VA_CURR_CRYSTAL_COLORS, _crystalIndex, _colorNum);
	}
}

SsScene2804CrystalButton::SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene), _asCrystal(asCrystal), _crystalIndex(crystalIndex) {

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

	loadSprite(getGlobalVar(V_SHRINK_LIGHTS_ON) ? kSsScene2804CrystalButtonFileHashes1[crystalIndex] : kSsScene2804CrystalButtonFileHashes2[crystalIndex],
		kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44045140);
	SetUpdateHandler(&SsScene2804CrystalButton::update);
	SetMessageHandler(&SsScene2804CrystalButton::handleMessage);
}

void SsScene2804CrystalButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804CrystalButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			playSound(0);
			setVisible(true);
			_countdown = 4;
			_asCrystal->activate();
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2804BeamCoil::AsScene2804BeamCoil(NeverhoodEngine *vm, Scene *parentScene, SsScene2804BeamCoilBody *ssBeamCoilBody)
	: AnimatedSprite(vm, 1400), _parentScene(parentScene), _ssBeamCoilBody(ssBeamCoilBody), _countdown(0) {

	createSurface1(0x00494891, 1000);
	_x = 125;
	_y = 184;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	loadSound(0, 0x6352F051);
	_vm->_soundMan->addSound(0xC5EA0B28, 0xEF56B094);
	SetUpdateHandler(&AsScene2804BeamCoil::update);
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
}

AsScene2804BeamCoil::~AsScene2804BeamCoil() {
	_vm->_soundMan->deleteSoundGroup(0xC5EA0B28);
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
	case NM_POSITION_CHANGE:
		show();
		_countdown = 92;
		messageResult = 1;
		break;
	case 0x2003:
		hide();
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2804BeamCoil::show() {
	_ssBeamCoilBody->setVisible(true);
	setVisible(true);
	startAnimation(0x00494891, 0, -1);
	playSound(0);
	SetMessageHandler(&AsScene2804BeamCoil::hmBeaming);
	NextState(&AsScene2804BeamCoil::stBeaming);
}

void AsScene2804BeamCoil::hide() {
	stopAnimation();
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
	setVisible(false);
	_ssBeamCoilBody->setVisible(false);
	_vm->_soundMan->stopSound(0xEF56B094);
}

void AsScene2804BeamCoil::stBeaming() {
	startAnimation(0x00494891, 93, -1);
	NextState(&AsScene2804BeamCoil::stBeaming);
	_vm->_soundMan->playSoundLooping(0xEF56B094);
}

uint32 AsScene2804BeamCoil::hmBeaming(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
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
	case NM_KLAYMEN_CLIMB_LADDER:
		setVisible(false);
		stopAnimation();
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2806Spew::AsScene2806Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {

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
	case NM_ANIMATION_UPDATE:
		playSound(0, 0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case NM_ANIMATION_STOP:
		stopAnimation();
		setVisible(false);
		break;
	default:
		break;
	}
	return messageResult;
}

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

	loadSprite(kClass428FileHashes[testTubeSetNum * 3 + testTubeIndex], kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 1500);
	setVisible(false);
	SetUpdateHandler(&SsScene2808Dispenser::update);
	SetMessageHandler(&SsScene2808Dispenser::handleMessage);
}

void SsScene2808Dispenser::update() {
	updatePosition();
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
	default:
		break;
	}
	return messageResult;
}

void SsScene2808Dispenser::startCountdown(int index) {
	setVisible(true);
	updatePosition();
	if (_testTubeSetNum == 0) {
		_countdown = kClass428Countdowns1[index];
	} else {
		_countdown = kClass428Countdowns2[index];
	}
}

AsScene2808TestTube::AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser)
	: AnimatedSprite(vm, 1100), _testTubeSetNum(testTubeSetNum), _testTubeIndex(testTubeIndex), _ssDispenser(ssDispenser), _fillLevel(0) {

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
		loadSound(0, 0x30809E2D);
		loadSound(1, 0x72811E2D);
		loadSound(2, 0x78B01625);
	} else {
		loadSound(3, 0x70A41E0C);
		loadSound(4, 0x50205E2D);
		loadSound(5, 0xF8621E2D);
		loadSound(6, 0xF1A03C2D);
		loadSound(7, 0x70A43D2D);
		loadSound(8, 0xF0601E2D);
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
	default:
		break;
	}
	return messageResult;
}

void AsScene2808TestTube::fill() {
	if ((int)_fillLevel < _testTubeSetNum * 3 + 3) {
		if (_testTubeSetNum == 0) {
			playSound(_fillLevel);
			setVisible(true);
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], kClass490FrameIndices1[_fillLevel + 1]);
			_newStickFrameIndex = kClass490FrameIndices1[_fillLevel + 1];
		} else {
			playSound(3 + _fillLevel);
			setVisible(true);
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], kClass490FrameIndices2[_fillLevel + 1]);
			_newStickFrameIndex = kClass490FrameIndices2[_fillLevel + 1];
		}
		_ssDispenser->startCountdown(_fillLevel);
		_fillLevel++;
	}
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
	: AnimatedSprite(vm, 1300), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum), _isActivated(false) {

	loadSound(0, 0xE18D1F30);
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
			playSound(0);
			activate();
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene2808Handle::hmActivating(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
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
	sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
}

AsScene2808Flow::AsScene2808Flow(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum) {

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
	loadSound(0, 0x6389B652);
	SetUpdateHandler(&AnimatedSprite::update);
	AnimatedSprite::updatePosition();
}

uint32 AsScene2808Flow::hmFlowing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2808Flow::start() {
	startAnimation(0xB8414818, 0, -1);
	setVisible(true);
	SetMessageHandler(&AsScene2808Flow::hmFlowing);
	NextState(&AsScene2808Flow::stKeepFlowing);
	playSound(0);
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

AsScene2809Spew::AsScene2809Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {

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
	case NM_ANIMATION_UPDATE:
		playSound(0, 0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case NM_ANIMATION_STOP:
		stopAnimation();
		setVisible(false);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2810Rope::AsScene2810Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2810Rope::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = -276;
	startAnimation(0x9D098C23, 35, 53);
}

uint32 AsScene2810Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		startAnimation(0x9D098C23, 35, 53);
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2812Winch::AsScene2812Winch(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	createSurface1(0x20DA08A0, 1200);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2812Winch::handleMessage);
	setVisible(false);
	_x = 280;
	_y = 184;
}

AsScene2812Winch::~AsScene2812Winch() {
	_vm->_soundMan->deleteSoundGroup(0x00B000E2);
}

uint32 AsScene2812Winch::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		startAnimation(0x20DA08A0, 0, -1);
		setVisible(true);
		_vm->_soundMan->addSound(0x00B000E2, 0xC874EE6C);
		_vm->_soundMan->playSoundLooping(0xC874EE6C);
		break;
	case NM_ANIMATION_STOP:
		startAnimation(0x20DA08A0, 7, -1);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2812Rope::AsScene2812Rope(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2812Rope::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	startAnimation(0xAE080551, 0, -1);
	_x = 334;
	_y = 201;
}

uint32 AsScene2812Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_USE_OBJECT:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		stRopingDown();
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene2812Rope::hmRopingDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2812Rope::stRopingDown() {
	sendMessage(_parentScene, NM_KLAYMEN_USE_OBJECT, 0);
	startAnimation(0x9D098C23, 0, -1);
	SetMessageHandler(&AsScene2812Rope::hmRopingDown);
}

AsScene2812TrapDoor::AsScene2812TrapDoor(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 0x805D0029, 100, 320, 240) {

	SetMessageHandler(&AsScene2812TrapDoor::handleMessage);
	_newStickFrameIndex = 0;
}

uint32 AsScene2812TrapDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		startAnimation(0x805D0029, 0, -1);
		playSound(0, 0xEA005F40);
		_newStickFrameIndex = STICK_LAST_FRAME;
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2801::KmScene2801(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2801::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStep);
		else
			GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToFront);
		else
			GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

KmScene2803::KmScene2803(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
	_dataResource.load(0x00900849);
}

uint32 KmScene2803::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klaymen::stFinishGrow);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else
			GotoState(&Klaymen::stWonderAboutHalf);
		break;
	case 0x482E:
		GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4838:
		GotoState(&Klaymen::stJumpToGrabRelease);
		break;
	default:
		break;
	}
	return 0;
}

KmScene2803Small::KmScene2803Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	_dataResource.load(0x81120132);
}

uint32 KmScene2803Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stStandIdleSmall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToXSmall(_dataResource.getPoint(param.asInteger()).x);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalfSmall);
		else
			GotoState(&Klaymen::stWonderAboutSmall);
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStepSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stWalkToFront2Small);
		else
			GotoState(&Klaymen::stWalkToFrontSmall);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToBackHalfSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stTurnToBackWalkSmall);
		else
			GotoState(&Klaymen::stTurnToBackSmall);
		break;
	case 0x4830:
		GotoState(&KmScene2803Small::stShrink);
		break;
	default:
		break;
	}
	return 0;
}

uint32 KmScene2803Small::hmShrink(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x80C110B5)
			sendMessage(_parentScene, NM_MOVE_TO_BACK, 0);
		else if (param.asInteger() == 0x33288344)
			playSound(2, 0x10688664);
		break;
	default:
		break;
	}
	return messageResult;
}

void KmScene2803Small::stShrink() {
	_busyStatus = 0;
	_acceptInput = false;
	playSound(0, 0x4C69EA53);
	startAnimation(0x1AE88904, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&KmScene2803Small::hmShrink);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

KmScene2805::KmScene2805(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2805::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klaymen::stSitInTeleporter);
		break;
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klaymen::stGetUpFromTeleporter);
		break;
	case 0x483D:
		teleporterAppear(0xDE284B74);
		break;
	case 0x483E:
		teleporterDisappear(0xD82A4094);
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2806::KmScene2806(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
	bool needsLargeSurface, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	if (needsLargeSurface) {
		NDimensions dimensions = _animResource.loadSpriteDimensions(0x2838C010);
		delete _surface;
		createSurface(1000, dimensions.width, dimensions.height);
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}

	_dataResource.load(0x98182003);
	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2806::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(440, true);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klaymen::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stDrinkPotion);
		else
			GotoState(&Klaymen::stUseTube);
		break;
	default:
		break;
	}
	return 0;
}

KmScene2809::KmScene2809(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
	bool needsLargeSurface, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	if (needsLargeSurface) {
		NDimensions dimensions = _animResource.loadSpriteDimensions(0x2838C010);
		delete _surface;
		createSurface(1000, dimensions.width, dimensions.height);
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}

	_dataResource.load(0x1830009A);
	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2809::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(226, true);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klaymen::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stDrinkPotion);
		else
			GotoState(&Klaymen::stUseTube);
		break;
	default:
		break;
	}
	return 0;
}

KmScene2810Small::KmScene2810Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2810Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stStandIdleSmall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToXSmall(_dataResource.getPoint(param.asInteger()).x);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalfSmall);
		else
			GotoState(&Klaymen::stWonderAboutSmall);
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStepSmall);
		else
			GotoState(&Klaymen::stWalkToFrontSmall);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToBackHalfSmall);
		else
			GotoState(&Klaymen::stTurnToBackSmall);
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

KmScene2810::KmScene2810(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2810::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klaymen::stFinishGrow);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 5)
			GotoState(&Klaymen::stTurnToUseExt);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x4820:
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);
		break;
	case 0x4821:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4822:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stClimbLadderHalf);
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

KmScene2812::KmScene2812(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2812::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4805:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrabFall);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMEN_INSERT_DISK:
		GotoState(&Klaymen::stInsertDisk);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x4820:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);
		break;
	case 0x4821:
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4822:
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x4823:
		sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
		GotoState(&Klaymen::stClimbLadderHalf);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStep);
		else
			GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToFront);
		else
			GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
