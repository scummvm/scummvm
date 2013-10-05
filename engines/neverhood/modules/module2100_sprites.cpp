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
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		stOpenDoor();
		break;
	case 0x4809:
		stCloseDoor();
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
	case 0x3002:
		stopAnimation();
		setVisible(false);
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
		sendMessage(_parentScene, 0x1022, 1010);
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
		sendMessage(_parentScene, 0x1022, 990);
		loadSprite(_fileHash2, kSLFDefDrawOffset | kSLFDefPosition);
		_countdown = 16;
		playSound(0, _soundFileHash);
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
