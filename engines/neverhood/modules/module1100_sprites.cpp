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

#include "neverhood/modules/module1100_sprites.h"

namespace Neverhood {

static const uint32 kSsScene1105SymbolDieFileHashes[] = {
	0,
	0x90898414,
	0x91098414,
	0x92098414,
	0x94098414,
	0x98098414,
	0x80098414,
	0xB0098414,
	0xD0098414,
	0x10098414
};

SsScene1105Button::SsScene1105Button(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, NRect &collisionBounds)
	: StaticSprite(vm, fileHash, 200), _parentScene(parentScene), _countdown(0) {

	_collisionBounds = collisionBounds;
	SetMessageHandler(&SsScene1105Button::handleMessage);
	SetUpdateHandler(&SsScene1105Button::update);
	setVisible(false);
}

void SsScene1105Button::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sendMessage(_parentScene, NM_KLAYMEN_RAISE_LEVER, 0);
		setVisible(false);
	}
}

uint32 SsScene1105Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			sendMessage(_parentScene, 0x4826, 0);
			messageResult = 1;
		}
		break;
	case 0x480B:
		_countdown = 8;
		setVisible(true);
		playSound(0, 0x44141000);
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene1105Symbol::SsScene1105Symbol(NeverhoodEngine *vm, uint32 fileHash, int16 x, int16 y)
	: StaticSprite(vm, 0) {

	loadSprite(fileHash, kSLFCenteredDrawOffset | kSLFSetPosition, 200, x, y);
}

void SsScene1105Symbol::hide() {
	setVisible(false);
	_needRefresh = true;
	updatePosition();
}

SsScene1105SymbolDie::SsScene1105SymbolDie(NeverhoodEngine *vm, uint dieIndex, int16 x, int16 y)
	: StaticSprite(vm, 1100), _dieIndex(dieIndex) {

	_x = x;
	_y = y;
	createSurface(200, 50, 50);
	loadSymbolSprite();
	SetMessageHandler(&SsScene1105SymbolDie::handleMessage);
}

uint32 SsScene1105SymbolDie::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		loadSymbolSprite();
		break;
	default:
		break;
	}
	return messageResult;
}

void SsScene1105SymbolDie::loadSymbolSprite() {
	loadSprite(kSsScene1105SymbolDieFileHashes[getSubVar(VA_CURR_DICE_NUMBERS, _dieIndex)], kSLFCenteredDrawOffset);
}

void SsScene1105SymbolDie::hide() {
	setVisible(false);
	_needRefresh = true;
	updatePosition();
}

AsScene1105TeddyBear::AsScene1105TeddyBear(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(100, 556, 328);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1105TeddyBear::handleMessage);
	startAnimation(0x65084002, 0, -1);
	_newStickFrameIndex = 0;
	setVisible(false);
	_needRefresh = true;
	updatePosition();
	loadSound(0, 0xCE840261);
	loadSound(1, 0xCCA41A62);
}

uint32 AsScene1105TeddyBear::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_POSITION_CHANGE:
		if (getGlobalVar(V_ROBOT_TARGET)) {
			startAnimation(0x6B0C0432, 0, -1);
			playSound(0);
		} else {
			startAnimation(0x65084002, 0, -1);
			playSound(1);
		}
		break;
	case NM_ANIMATION_STOP:
		sendMessage(_parentScene, 0x2003, 0);
		stopAnimation();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1105TeddyBear::show() {
	setVisible(true);
	_needRefresh = true;
	updatePosition();
}

void AsScene1105TeddyBear::hide() {
	setVisible(false);
	_needRefresh = true;
	updatePosition();
}

SsScene1105OpenButton::SsScene1105OpenButton(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 900), _parentScene(parentScene), _countdown(0), _isClicked(false) {

	loadSprite(0x8228A46C, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44045140);
	SetUpdateHandler(&SsScene1105OpenButton::update);
	SetMessageHandler(&SsScene1105OpenButton::handleMessage);
}

void SsScene1105OpenButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		setVisible(false);
		sendMessage(_parentScene, 0x2001, 0);
	}
}

uint32 SsScene1105OpenButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_isClicked) {
			playSound(0);
			setVisible(true);
			_isClicked = true;
			_countdown = 4;
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene1109::KmScene1109(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1109::xHandleMessage(int messageNum, const MessageParam &param) {
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
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
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
		teleporterAppear(0x2C2A4A1C);
		break;
	case 0x483E:
		teleporterDisappear(0x3C2E4245);
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
