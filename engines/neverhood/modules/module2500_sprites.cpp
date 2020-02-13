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

#include "neverhood/modules/module2500_sprites.h"

namespace Neverhood {

SsScene2504Button::SsScene2504Button(NeverhoodEngine *vm)
	: StaticSprite(vm, 1400), _countdown(0), _isSoundPlaying(false) {

	loadSprite(0x070220D9, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x4600204C);
	loadSound(1, 0x408C0034);
	loadSound(2, 0x44043000);
	loadSound(3, 0x44045000);
	SetMessageHandler(&SsScene2504Button::handleMessage);
	SetUpdateHandler(&SsScene2504Button::update);
}

void SsScene2504Button::update() {
	updatePosition();
	if (_isSoundPlaying && !isSoundPlaying(0) && !isSoundPlaying(1)) {
		playSound(3);
		setVisible(false);
		_isSoundPlaying = false;
	}
	if (_countdown != 0 && (--_countdown) == 0) {
		if (getSubVar(VA_LOCKS_DISABLED, 0x01180951))
			playSound(0);
		else
			playSound(1);
		_isSoundPlaying = true;
	}
}

uint32 SsScene2504Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_isSoundPlaying) {
			setVisible(true);
			_countdown = 2;
			if (getSubVar(VA_LOCKS_DISABLED, 0x01180951))
				setSubVar(VA_LOCKS_DISABLED, 0x01180951, 0);
			else
				setSubVar(VA_LOCKS_DISABLED, 0x01180951, 1);
			playSound(2);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2501::KmScene2501(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2501::xHandleMessage(int messageNum, const MessageParam &param) {
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
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
