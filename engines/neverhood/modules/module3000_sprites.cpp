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

#include "neverhood/modules/module3000.h"
#include "neverhood/modules/module3000_sprites.h"

namespace Neverhood {

// Scene3009

enum {
	kCTSNull				= 0,
	kCTSBreakWall			= 1,
	kCTSWall				= 2,
	kCTSEmptyness			= 3,
	kCTSFireRobotNoTarget	= 4,
	kCTSFireRobotIsTarget	= 5,
	kCTSFireNoRobot			= 6,
	kCTSRaiseCannon			= 7,
	kCTSRightRobotNoTarget	= 8,
	kCTSRightRobotIsTarget	= 9,
	kCTSRightNoRobot		= 10,
	kCTSLeftRobotNoTarget	= 11,
	kCTSLeftRobotIsTarget	= 12,
	kCTSLeftNoRobot			= 13,
	kCTSLowerCannon			= 14,
	kCTSCount				= 14
};

static const uint32 kSsScene3009SymbolEdgesFileHashes[] = {
	0x618827A0,
	0xB1A92322
};

static const uint32 kSsScene3009TargetLineFileHashes[] = {
	0x4011018C,
	0x15086623
};

static const NPoint kAsScene3009SymbolPoints[] = {
	{289, 338},
	{285, 375},
	{284, 419},
	{456, 372},
	{498, 372},
	{541, 372}
};

static const uint32 kAsScene3009SymbolFileHashes[] = {
	0x24542582,
	0x1CD61D96
};

static const uint32 kSsScene3009SymbolArrowFileHashes1[] = {
	0x24016060,
	0x21216221,
	0x486160A0,
	0x42216422,
	0x90A16120,
	0x84216824,
	0x08017029,
	0x08217029,
	0x10014032,
	0x10214032,
	0x20012004,
	0x20212004
};

static const uint32 kSsScene3009SymbolArrowFileHashes2[] = {
	0x40092024,
	0x01636002,
	0x8071E028,
	0x02A56064,
	0x00806031,
	0x052960A8,
	0x0A116130,
	0x0A316130,
	0x14216200,
	0x14016200,
	0x28416460,
	0x28616460
};

SsScene3009FireCannonButton::SsScene3009FireCannonButton(NeverhoodEngine *vm, Scene3009 *parentScene)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _isClicked(false) {

	loadSprite(0x120B24B0, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	SetUpdateHandler(&SsScene3009FireCannonButton::update);
	SetMessageHandler(&SsScene3009FireCannonButton::handleMessage);
	loadSound(0, 0x3901B44F);
}

void SsScene3009FireCannonButton::update() {
	updatePosition();
	if (_isClicked && !isSoundPlaying(0)) {
		sendMessage(_parentScene, 0x2000, 0);
		setVisible(false);
	}
}

uint32 SsScene3009FireCannonButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isClicked && !_parentScene->isTurning()) {
			_isClicked = true;
			setVisible(true);
			playSound(0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene3009SymbolEdges::SsScene3009SymbolEdges(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400), _blinkCountdown(0) {

	loadSprite(kSsScene3009SymbolEdgesFileHashes[index], kSLFDefDrawOffset | kSLFDefPosition, 600);
	if (getGlobalVar(V_ROBOT_HIT))
		hide();
	else
		startBlinking();
	SetUpdateHandler(&SsScene3009SymbolEdges::update);
}

void SsScene3009SymbolEdges::update() {
	if (_blinkCountdown != 0 && (--_blinkCountdown == 0)) {
		if (_blinkToggle) {
			setVisible(true);
		} else {
			setVisible(false);
		}
		updatePosition();
		_blinkCountdown = 3;
		_blinkToggle = !_blinkToggle;
	}
}

void SsScene3009SymbolEdges::show() {
	setVisible(true);
	updatePosition();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::hide() {
	setVisible(false);
	updatePosition();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::startBlinking() {
	setVisible(true);
	updatePosition();
	_blinkCountdown = 3;
	_blinkToggle = true;
}

SsScene3009TargetLine::SsScene3009TargetLine(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400) {

	loadSprite(kSsScene3009TargetLineFileHashes[index], kSLFDefDrawOffset | kSLFDefPosition, 600);
	setVisible(false);
}

void SsScene3009TargetLine::show() {
	setVisible(true);
	updatePosition();
}

SsScene3009SymbolArrow::SsScene3009SymbolArrow(NeverhoodEngine *vm, Sprite *asSymbol, int index)
	: StaticSprite(vm, 1400), _asSymbol(asSymbol), _index(index), _enabled(true), _countdown(0) {

	_incrDecr = _index % 2;

	createSurface(1200, 33, 31);
	loadSprite(kSsScene3009SymbolArrowFileHashes2[_index], kSLFDefPosition);
	_drawOffset.set(0, 0, 33, 31);
	_collisionBoundsOffset = _drawOffset;
	updateBounds();
	_needRefresh = true;

	SetUpdateHandler(&SsScene3009SymbolArrow::update);
	SetMessageHandler(&SsScene3009SymbolArrow::handleMessage);
	loadSound(0, 0x2C852206);
}

void SsScene3009SymbolArrow::hide() {
	_enabled = false;
	setVisible(false);
}

void SsScene3009SymbolArrow::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		loadSprite(kSsScene3009SymbolArrowFileHashes2[_index], kSLFDefDrawOffset);
	}
}

uint32 SsScene3009SymbolArrow::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled && _countdown == 0) {
			_countdown = 2;
			loadSprite(kSsScene3009SymbolArrowFileHashes1[_index], kSLFDefDrawOffset);
			playSound(0);
			sendMessage(_asSymbol, NM_KLAYMEN_CLIMB_LADDER, _incrDecr);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene3009VerticalIndicator::AsScene3009VerticalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, int index)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {

	_x = 300;
	_y = getGlobalVar(V_CANNON_RAISED) ? 52 : 266;
	createSurface1(0xC2463913, 1200);
	_needRefresh = true;
	updatePosition();
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009VerticalIndicator::handleMessage);
}

void AsScene3009VerticalIndicator::show() {
	startAnimation(0xC2463913, 0, -1);
	setVisible(true);
	updatePosition();
	_enabled = true;
}

uint32 AsScene3009VerticalIndicator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled) {
			sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene3009HorizontalIndicator::AsScene3009HorizontalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, uint32 cannonTargetStatus)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {

	_x = getGlobalVar(V_CANNON_TURNED) ? 533 : 92;
	_y = 150;
	createSurface1(0xC0C12954, 1200);
	_needRefresh = true;
	updatePosition();
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009HorizontalIndicator::handleMessage);
	if (cannonTargetStatus == kCTSRightRobotNoTarget || cannonTargetStatus == kCTSRightRobotIsTarget || cannonTargetStatus == kCTSRightNoRobot) {
		SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveRight);
		_x = 280;
	}
}

uint32 AsScene3009HorizontalIndicator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled) {
			sendMessage(_parentScene, 0x2004, 0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene3009HorizontalIndicator::suMoveLeft() {
	_x -= 6;
	if (_x < 92) {
		SetSpriteUpdate(NULL);
		_x = 92;
	}
}

void AsScene3009HorizontalIndicator::suMoveRight() {
	_x += 6;
	if (_x > 533) {
		SetSpriteUpdate(NULL);
		_x = 533;
	}
}

void AsScene3009HorizontalIndicator::show() {
	startAnimation(0xC0C12954, 0, -1);
	setVisible(true);
	updatePosition();
	_enabled = true;
}

void AsScene3009HorizontalIndicator::stMoveLeft() {
	_x = 533;
	SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveLeft);
}

void AsScene3009HorizontalIndicator::stMoveRight() {
	_x = 330;
	SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveRight);
}

AsScene3009Symbol::AsScene3009Symbol(NeverhoodEngine *vm, Scene3009 *parentScene, int symbolPosition)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _symbolPosition(symbolPosition) {

	_symbolIndex = getSubVar(VA_CURR_CANNON_SYMBOLS, _symbolPosition);

	_x = kAsScene3009SymbolPoints[_symbolPosition].x;
	_y = kAsScene3009SymbolPoints[_symbolPosition].y;
	createSurface1(kAsScene3009SymbolFileHashes[_symbolPosition / 3], 1200);
	startAnimation(kAsScene3009SymbolFileHashes[_symbolPosition / 3], _symbolIndex, -1);
	_newStickFrameIndex = _symbolIndex;
	_needRefresh = true;
	updatePosition();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009Symbol::handleMessage);
	_ssArrowPrev = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _symbolPosition * 2 + 0);
	_parentScene->addCollisionSprite(_ssArrowPrev);
	_ssArrowNext = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _symbolPosition * 2 + 1);
	_parentScene->addCollisionSprite(_ssArrowNext);
}

uint32 AsScene3009Symbol::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_CLIMB_LADDER:
		if (param.asInteger()) {
			if (_symbolIndex == 11)
				_symbolIndex = 0;
			else
				_symbolIndex++;
		} else {
			if (_symbolIndex == 0)
				_symbolIndex = 11;
			else
				_symbolIndex--;
		}
		startAnimation(kAsScene3009SymbolFileHashes[_symbolPosition / 3], _symbolIndex, -1);
		_newStickFrameIndex = _symbolIndex;
		setSubVar(VA_CURR_CANNON_SYMBOLS, _symbolPosition, _symbolIndex);
		if (_symbolPosition / 3 == 0) {
			sendMessage(_parentScene, 0x2001, 0);
		} else {
			sendMessage(_parentScene, 0x2003, 0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene3009Symbol::hide() {
	_ssArrowPrev->hide();
	_ssArrowNext->hide();
}

// Scene3010

static const uint32 kScene3010ButtonNameHashes[] = {
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
	: StaticSprite(vm, 900), _parentScene(parentScene), _buttonLocked(false), _countdown1(0), _countdown2(0), _buttonIndex(buttonIndex) {

	_buttonEnabled = getSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[_buttonIndex]) != 0;
	createSurface(400, 88, 95);
	setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	if (initDisabled)
		disableButton();
	else if (_buttonEnabled)
		_countdown1 = initCountdown * 12 + 1;
	loadSound(0, 0xF4217243);
	loadSound(1, 0x44049000);
	loadSound(2, 0x6408107E);
	SetUpdateHandler(&SsScene3010DeadBoltButton::update);
	SetMessageHandler(&SsScene3010DeadBoltButton::handleMessage);
}

void SsScene3010DeadBoltButton::update() {

	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		playSound(0);
		setVisible(false);
		setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	}

	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		setVisible(true);
		setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	}

}

uint32 SsScene3010DeadBoltButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_buttonLocked && _countdown1 == 0) {
			if (_buttonEnabled) {
				playSound(1);
				playSound(2);
				setVisible(true);
				_buttonLocked = true;
				sendMessage(_parentScene, 0x2000, _buttonIndex);
			} else {
				sendMessage(_parentScene, NM_POSITION_CHANGE, _buttonIndex);
			}
			_needRefresh = true;
			updatePosition();
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void SsScene3010DeadBoltButton::disableButton() {
	_buttonLocked = true;
	setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	setVisible(true);
}

void SsScene3010DeadBoltButton::setSprite(uint32 fileHash) {
	loadSprite(fileHash, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset);
}

void SsScene3010DeadBoltButton::setCountdown(int count) {
	_countdown2 = count * 18 + 1;
}

AsScene3010DeadBolt::AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _boltIndex(boltIndex), _soundToggle(true),
	_unlocked(false), _locked(false), _countdown(0) {

	_x = kAsScene3010DeadBoltPoints[_boltIndex].x;
	_y = kAsScene3010DeadBoltPoints[_boltIndex].y;

	if (getSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[_boltIndex])) {
		createSurface1(kAsScene3010DeadBoltFileHashes1[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
		loadSound(0, 0x46005BC4);
	} else {
		createSurface1(kAsScene3010DeadBoltFileHashes2[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		loadSound(0, 0x420073DC);
		loadSound(1, 0x420073DC);
	}

	setVisible(false);
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
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene3010DeadBolt::stIdle() {
	stopAnimation();
	SetUpdateHandler(&AsScene3010DeadBolt::update);
	SetMessageHandler(&Sprite::handleMessage);
	_locked = false;
}

void AsScene3010DeadBolt::unlock(bool skipAnim) {
	if (!_unlocked) {
		setVisible(true);
		if (skipAnim) {
			startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], -1, 0);
			_newStickFrameIndex = STICK_LAST_FRAME;
		} else {
			startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
			SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
			FinalizeState(&AsScene3010DeadBolt::stIdleMessage);
			NextState(&AsScene3010DeadBolt::stIdle);
			playSound(0);
		}
		_unlocked = true;
		loadSound(2, 0x4010C345);
	}
}

void AsScene3010DeadBolt::stIdleMessage() {
	stopAnimation();
	SetMessageHandler(&Sprite::handleMessage);
	sendMessage(_parentScene, 0x2001, _boltIndex);
}

void AsScene3010DeadBolt::lock() {
	if (!_locked) {
		_locked = true;
		setVisible(true);
		startAnimation(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
		FinalizeState(&AsScene3010DeadBolt::stDisabledMessage);
		NextState(&AsScene3010DeadBolt::stIdle);
		if (_soundToggle) {
			playSound(0);
		} else {
			playSound(1);
		}
		_soundToggle = !_soundToggle;
	}
}

void AsScene3010DeadBolt::setCountdown(int count) {
	_countdown = count * 18 + 1;
}

void AsScene3010DeadBolt::stDisabled() {
	setVisible(true);
	startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
	SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
	FinalizeState(&AsScene3010DeadBolt::stDisabledMessage);
	NextState(&AsScene3010DeadBolt::stIdle);
	_playBackwards = true;
	playSound(2);
}

void AsScene3010DeadBolt::stDisabledMessage() {
	setVisible(false);
	sendMessage(_parentScene, 0x2003, _boltIndex);
}

// Scene3011

static const uint32 kAsScene3011SymbolFileHashes[] = {
	0x00C88050,
	0x01488050,
	0x02488050,
	0x04488050,
	0x08488050,
	0x10488050,
	0x20488050,
	0x40488050,
	0x80488050,
	0x00488051,
	0x00488052,
	0x00488054,
	0x008B0000,
	0x008D0000,
	0x00810000,
	0x00990000,
	0x00A90000,
	0x00C90000,
	0x00090000,
	0x01890000,
	0x02890000,
	0x04890000,
	0x08890000,
	0x10890000
};

SsScene3011Button::SsScene3011Button(NeverhoodEngine *vm, Scene *parentScene, bool flag)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _countdown(0) {

	loadSprite(flag ? 0x11282020 : 0x994D0433, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44061000);
	SetUpdateHandler(&SsScene3011Button::update);
	SetMessageHandler(&SsScene3011Button::handleMessage);
}

void SsScene3011Button::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		setVisible(false);
	}
}

uint32 SsScene3011Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	StaticSprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			setVisible(true);
			_countdown = 4;
			sendMessage(_parentScene, 0x2000, 0);
			playSound(0);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene3011Symbol::AsScene3011Symbol(NeverhoodEngine *vm, int symbolIndex, bool largeSymbol)
	: AnimatedSprite(vm, 1000), _symbolIndex(symbolIndex), _largeSymbol(largeSymbol), _isNoisy(false) {

	if (_largeSymbol) {
		_x = 310;
		_y = 200;
		createSurface1(kAsScene3011SymbolFileHashes[_symbolIndex], 1200);
		loadSound(0, 0x6052C60F);
		loadSound(1, 0x6890433B);
	} else {
		_symbolIndex = 12;
		_x = symbolIndex * 39 + 96;
		_y = 225;
		createSurface(1200, 41, 48);
		loadSound(0, 0x64428609);
		loadSound(1, 0x7080023B);
	}
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene3011Symbol::show(bool isNoisy) {
	_isNoisy = isNoisy;
	startAnimation(kAsScene3011SymbolFileHashes[_symbolIndex], 0, -1);
	setVisible(true);
	if (_isNoisy) {
		playSound(1);
	} else {
		playSound(0);
	}
}

void AsScene3011Symbol::hide() {
	stopAnimation();
	setVisible(false);
}

void AsScene3011Symbol::stopSymbolSound() {
	if (_isNoisy) {
		stopSound(1);
	} else {
		stopSound(0);
	}
}

void AsScene3011Symbol::change(int symbolIndex, bool isNoisy) {
	_symbolIndex = symbolIndex;
	_isNoisy = isNoisy;
	startAnimation(kAsScene3011SymbolFileHashes[_symbolIndex], 0, -1);
	setVisible(true);
	if (_isNoisy) {
		playSound(1);
	} else {
		playSound(0);
	}
}

} // End of namespace Neverhood
