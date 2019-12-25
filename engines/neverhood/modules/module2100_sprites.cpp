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

#include "neverhood/modules/module2100_sprites.h"

namespace Neverhood {

AsScene2101Door::AsScene2101Door(NeverhoodEngine *vm, bool isOpen)
	: AnimatedSprite(vm, 1100) {

	createSurface(100, 328, 347);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2101Door::handleMessage);
	if (isOpen) {
		startAnimation(0x0C202B9C, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else
		setVisible(false);
}

uint32 AsScene2101Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		stOpenDoor();
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		stCloseDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2101Door::stOpenDoor() {
	startAnimation(0x0C202B9C, 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
	setVisible(true);
	playSound(0, calcHash("fxDoorOpen32"));
}

void AsScene2101Door::stCloseDoor() {
	startAnimation(0xC222A8D4, 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
	setVisible(true);
	playSound(0, calcHash("fxDoorClose32"));
	NextState(&AsScene2101Door::stCloseDoorDone);
}

void AsScene2101Door::stCloseDoorDone() {
	stopAnimation();
	setVisible(false);
}

AsScene2101HitByDoorEffect::AsScene2101HitByDoorEffect(NeverhoodEngine *vm, Sprite *klaymen)
	: AnimatedSprite(vm, 1400), _klaymen(klaymen) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2101HitByDoorEffect::handleMessage);
	createSurface(1200, 88, 165);
	setVisible(false);
}

uint32 AsScene2101HitByDoorEffect::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		_x = _klaymen->getX();
		_y = _klaymen->getY() - 132;
		startAnimation(0x0422255A, 0, -1);
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

SsCommonFloorButton::SsCommonFloorButton(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash)
	: StaticSprite(vm, 1100), _parentScene(parentScene), _countdown(0),
	_fileHash1(fileHash1), _fileHash2(fileHash2), _soundFileHash(soundFileHash) {

	SetUpdateHandler(&SsCommonFloorButton::update);
	SetMessageHandler(&SsCommonFloorButton::handleMessage);
	if (_soundFileHash == 0)
		_soundFileHash = 0x44141000;
	createSurface(1010, 61, 30);
	if (_fileHash1)
		loadSprite(_fileHash1, kSLFDefDrawOffset | kSLFDefPosition);
	else
		setVisible(false);
}

void SsCommonFloorButton::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		if (_fileHash1)
			loadSprite(_fileHash1, kSLFDefDrawOffset | kSLFDefPosition);
		else
			setVisible(false);
	}
}

uint32 SsCommonFloorButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		sendMessage(_parentScene, 0x480B, 0);
		setVisible(true);
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		loadSprite(_fileHash2, kSLFDefDrawOffset | kSLFDefPosition);
		_countdown = 16;
		playSound(0, _soundFileHash);
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2101::KmScene2101(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2101::xHandleMessage(int messageNum, const MessageParam &param) {
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
	case 0x4811:
		GotoState(&KmScene2101::stHitByDoor);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
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
		teleporterAppear(0xFF290E30);
		break;
	case 0x483E:
		teleporterDisappear(0x9A28CA1C);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 KmScene2101::hmHitByDoor(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymenSpeedUpHash);
		if (_currFrameIndex < speedUpFrameIndex) {
			startAnimation(0x35AA8059, speedUpFrameIndex, -1);
			_y = 438;
		}
		messageResult = 0;
		break;
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x1A1A0785) {
			playSound(0, 0x40F0A342);
		} else if (param.asInteger() == 0x60428026) {
			playSound(0, 0x40608A59);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void KmScene2101::stHitByDoor() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&KmScene2101::hmHitByDoor);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	playSound(0, 0x402E82D4);
}

} // End of namespace Neverhood
