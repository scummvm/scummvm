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

#include "neverhood/modules/module2400_sprites.h"

namespace Neverhood {

static const uint32 kAsScene2401WaterSpitFileHashes2[] = {
	0x5C044690, 0x5C644690, 0x5CA44690,
	0x5D244690, 0x5E244690
};

static const uint32 kAsScene2401WaterSpitFileHashes1[] = {
	0xF4418408, 0xF4418808, 0xF4419008,
	0xF441A008, 0xCD4F8411
};

AsScene2401WaterSpit::AsScene2401WaterSpit(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {

	_x = 240;
	_y = 447;
	createSurface(100, 146, 74);
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2401WaterSpit::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 AsScene2401WaterSpit::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x120A0013)
			playSound(0, kAsScene2401WaterSpitFileHashes1[_soundIndex]);
		break;
	case NM_ANIMATION_UPDATE:
		_x = 240;
		_y = 447;
		_soundIndex = getSubVar(VA_CURR_WATER_PIPES_LEVEL, param.asInteger());
		startAnimation(kAsScene2401WaterSpitFileHashes2[param.asInteger()], 0, -1);
		setVisible(true);
		playSound(0, 0x48640244);
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

AsScene2401FlowingWater::AsScene2401FlowingWater(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _isWaterFlowing(false) {

	_x = 88;
	_y = 421;
	createSurface1(0x10203116, 100);
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2401FlowingWater::handleMessage);
}

AsScene2401FlowingWater::~AsScene2401FlowingWater() {
	_vm->_soundMan->deleteSoundGroup(0x40F11C09);
}

uint32 AsScene2401FlowingWater::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (_isWaterFlowing && param.asInteger() == 0x02421405)
			startAnimationByHash(0x10203116, 0x01084280, 0);
		break;
	case NM_POSITION_CHANGE:
		if (!_isWaterFlowing) {
			_vm->_soundMan->addSound(0x40F11C09, 0x980C1420);
			_vm->_soundMan->playSoundLooping(0x980C1420);
			startAnimation(0x10203116, 0, -1);
			setVisible(true);
			_isWaterFlowing = true;
		}
		break;
	case 0x2003:
		_vm->_soundMan->deleteSound(0x980C1420);
		_isWaterFlowing = false;
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

AsScene2401WaterFlushing::AsScene2401WaterFlushing(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 1200), _countdown(0), _flushLoopCount(0) {

	_x = x;
	_y = y;
	createSurface1(0xB8596884, 100);
	setVisible(false);
	SetUpdateHandler(&AsScene2401WaterFlushing::update);
	SetMessageHandler(&AsScene2401WaterFlushing::handleMessage);
}

void AsScene2401WaterFlushing::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		startAnimation(0xB8596884, 0, -1);
		setVisible(true);
	}
	AnimatedSprite::update();
}

uint32 AsScene2401WaterFlushing::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (_flushLoopCount > 0 && param.asInteger() == 0x02421405) {
			startAnimationByHash(0xB8596884, 0x01084280, 0);
			_flushLoopCount--;
		}
		break;
	case NM_POSITION_CHANGE:
		if (param.asInteger() > 0) {
			_flushLoopCount = param.asInteger() - 1;
			_countdown = _vm->_rnd->getRandomNumber(3) + 1;
		}
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

AsScene2401Door::AsScene2401Door(NeverhoodEngine *vm, bool isOpen)
	: AnimatedSprite(vm, 1100), _countdown(0), _isOpen(isOpen) {

	_x = 320;
	_y = 240;
	createSurface1(0x44687810, 100);
	_newStickFrameIndex = STICK_LAST_FRAME;
	if (_isOpen) {
		stopAnimation();
		setVisible(false);
		_countdown = 48;
	} else {
		startAnimation(0x44687810, 0, -1);
		_newStickFrameIndex = 0;
	}
	SetUpdateHandler(&AsScene2401Door::update);
	SetMessageHandler(&AsScene2401Door::handleMessage);
}

void AsScene2401Door::update() {
	if (_isOpen && _countdown != 0 && (--_countdown) == 0) {
		_isOpen = false;
		setVisible(true);
		startAnimation(0x44687810, -1, -1);
		_newStickFrameIndex = 0;
		_playBackwards = true;
		playSound(0, calcHash("fxDoorClose38"));
	}
	AnimatedSprite::update();
}

uint32 AsScene2401Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		if (_isOpen)
			_countdown = 168;
		messageResult = _isOpen ? 1 : 0;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		if (!_isOpen) {
			_countdown = 168;
			_isOpen = true;
			setVisible(true);
			startAnimation(0x44687810, 0, -1);
			playSound(0, calcHash("fxDoorOpen38"));
			NextState(&AsScene2401Door::stDoorOpenFinished);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2401Door::stDoorOpenFinished() {
	stopAnimation();
	setVisible(false);
}

AsScene2402Door::AsScene2402Door(NeverhoodEngine *vm, Scene *parentScene, bool isOpen)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _isOpen(isOpen), _countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x80495831, 100);
	if (_isOpen) {
		startAnimation(0x80495831, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
		_countdown = 48;
	} else {
		stopAnimation();
		setVisible(false);
	}
	SetUpdateHandler(&AsScene2402Door::update);
	SetMessageHandler(&AsScene2402Door::handleMessage);
}

void AsScene2402Door::update() {
	if (_isOpen && _countdown != 0 && (--_countdown) == 0) {
		_isOpen = false;
		setVisible(true);
		startAnimation(0x80495831, -1, -1);
		_playBackwards = true;
		playSound(0, calcHash("fxDoorClose38"));
		NextState(&AsScene2402Door::stDoorClosingFinished);
	}
	AnimatedSprite::update();
}

uint32 AsScene2402Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		if (_isOpen)
			_countdown = 144;
		messageResult = _isOpen ? 1 : 0;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		_countdown = 144;
		_isOpen = true;
		setVisible(true);
		startAnimation(0x80495831, 0, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
		playSound(0, calcHash("fxDoorOpen38"));
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2402Door::stDoorClosingFinished() {
	sendMessage(_parentScene, 0x2001, 0);
	setVisible(false);
}

AsScene2402TV::AsScene2402TV(NeverhoodEngine *vm, Klaymen *klaymen)
	: AnimatedSprite(vm, 1100), _klaymen(klaymen), _countdown1(0), _countdown2(0) {

	_x = 260;
	_y = 210;
	createSurface(100, 127, 90);
	setDoDeltaX(1);
	SetMessageHandler(&Sprite::handleMessage);
	if (!getGlobalVar(V_TV_JOKE_TOLD)) {
		loadSound(0, 0x58208810);
		_countdown1 = 48;
		startAnimation(0x4919397A, 0, -1);
		_newStickFrameIndex = 0;
		SetUpdateHandler(&AsScene2402TV::upWait);
	} else {
		int16 frameIndex;
		if (_klaymen->getX() > 320)
			_currFrameIndex = 29;
		frameIndex = CLIP<int16>((_klaymen->getX() - _x + 150) / 10, 0, 29);
		startAnimation(0x050A0103, frameIndex, -1);
		_newStickFrameIndex = frameIndex;
		_countdown1 = 0;
		SetUpdateHandler(&AsScene2402TV::upFocusKlaymen);
	}
}

AsScene2402TV::~AsScene2402TV() {
	_vm->_soundMan->deleteSoundGroup(0x01520123);
}

void AsScene2402TV::upWait() {
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		startAnimation(0x4919397A, 0, -1);
		SetMessageHandler(&AsScene2402TV::hmJoke);
		NextState(&AsScene2402TV::stJokeFinished);
	}
	AnimatedSprite::update();
}

void AsScene2402TV::upFocusKlaymen() {
	int16 frameIndex = CLIP<int16>((_klaymen->getX() - _x + 150) / 10, 0, 29);
	if (frameIndex != _currFrameIndex) {
		if (frameIndex > _currFrameIndex)
			_currFrameIndex++;
		else if (frameIndex < _currFrameIndex)
			_currFrameIndex--;
		startAnimation(0x050A0103, _currFrameIndex, -1);
		_newStickFrameIndex = _currFrameIndex;
		if (_countdown2 == 0) {
			_vm->_soundMan->addSound(0x01520123, 0xC42D4528);
			_vm->_soundMan->playSoundLooping(0xC42D4528);
		}
		_countdown2 = 5;
	} else if (_countdown2 != 0 && (--_countdown2 == 0))
		_vm->_soundMan->deleteSound(0xC42D4528);
	AnimatedSprite::update();
}

void AsScene2402TV::stJokeFinished() {
	setGlobalVar(V_TV_JOKE_TOLD, 1);
	startAnimation(0x050A0103, 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AsScene2402TV::upFocusKlaymen);
}

uint32 AsScene2402TV::hmJoke(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x431EA0B0)
			playSound(0);
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2401::KmScene2401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y),
	_canSpitPipe(false), _contSpitPipe(false), _readyToSpit(false),
	_spitPipeIndex(0), _spitDestPipeIndex(0), _spitContDestPipeIndex(0) {

	// Empty
}

uint32 KmScene2401::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
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
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 0)
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
	case 0x4832:
		GotoState(&Klaymen::stUseTube);
		break;
	case 0x4833:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAbout);
		else {
			_spitPipeIndex = sendMessage(_parentScene, 0x2000, 0);
			GotoState(&KmScene2401::stTrySpitIntoPipe);
		}
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
	return messageResult;
}

uint32 KmScene2401::hmSpit(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x16401CA6) {
			_canSpitPipe = true;
			if (_contSpitPipe)
				spitIntoPipe();
		} else if (param.asInteger() == 0xC11C0008) {
			_canSpitPipe = false;
			_acceptInput = false;
			_readyToSpit = false;
		} else if (param.asInteger() == 0x018A0001) {
			sendMessage(_parentScene, 0x2001, _spitDestPipeIndex);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void KmScene2401::stTrySpitIntoPipe() {
	if (_readyToSpit) {
		_contSpitPipe = true;
		_spitContDestPipeIndex = _spitPipeIndex;
		if (_canSpitPipe)
			spitIntoPipe();
	} else if (!stStartAction(AnimationCallback(&KmScene2401::stTrySpitIntoPipe))) {
		_busyStatus = 2;
		_acceptInput = true;
		_spitDestPipeIndex = _spitPipeIndex;
		_readyToSpit = true;
		_canSpitPipe = false;
		_contSpitPipe = false;
		startAnimation(0x1808B150, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&KmScene2401::hmSpit);
		SetSpriteUpdate(NULL);
	}
}

void KmScene2401::spitIntoPipe() {
	_contSpitPipe = false;
	_spitDestPipeIndex = _spitContDestPipeIndex;
	_canSpitPipe = false;
	_acceptInput = false;
	startAnimation(0x1B08B553, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&KmScene2401::hmSpit);
	SetSpriteUpdate(NULL);
	NextState(&KmScene2401::stContSpitIntoPipe);
}

void KmScene2401::stContSpitIntoPipe() {
	_canSpitPipe = true;
	_acceptInput = true;
	startAnimationByHash(0x1808B150, 0x16401CA6, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&KmScene2401::hmSpit);
	SetSpriteUpdate(NULL);
}

KmScene2402::KmScene2402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2402::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		if (!getGlobalVar(V_TV_JOKE_TOLD))
			GotoState(&Klaymen::stStandWonderAbout);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
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
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
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
	return messageResult;
}

KmScene2403::KmScene2403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2403::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
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
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
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
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
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
	return messageResult;
}

KmScene2406::KmScene2406(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2406::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
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
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
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
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
