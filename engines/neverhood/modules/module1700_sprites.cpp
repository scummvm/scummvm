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

#include "neverhood/modules/module1700_sprites.h"

namespace Neverhood {

SsScene1705WallSymbol::SsScene1705WallSymbol(NeverhoodEngine *vm, uint32 fileHash, int symbolIndex)
	: StaticSprite(vm, fileHash, 100) {

	_x = _spriteResource.getPosition().x + symbolIndex * 30;
	_y = _spriteResource.getPosition().y + 160;
	updatePosition();
}

SsScene1705Tape::SsScene1705Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 tapeIndex, int surfacePriority, int16 x, int16 y, uint32 fileHash)
	: StaticSprite(vm, fileHash, surfacePriority, x - 24, y - 4), _parentScene(parentScene), _tapeIndex(tapeIndex) {

	if (!getSubVar(VA_HAS_TAPE, _tapeIndex) && !getSubVar(VA_IS_TAPE_INSERTED, _tapeIndex)) {
		SetMessageHandler(&SsScene1705Tape::handleMessage);
	} else {
		setVisible(false);
		SetMessageHandler(NULL);
	}
	_collisionBoundsOffset = _drawOffset;
	_collisionBoundsOffset.x -= 4;
	_collisionBoundsOffset.y -= 8;
	_collisionBoundsOffset.width += 8;
	_collisionBoundsOffset.height += 16;
	Sprite::updateBounds();
}

uint32 SsScene1705Tape::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_USE_OBJECT:
		setSubVar(VA_HAS_TAPE, _tapeIndex, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene1705::KmScene1705(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1705::xHandleMessage(int messageNum, const MessageParam &param) {
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
	case 0x4803:
		GotoState(&Klaymen::stFallSkipJump);
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		if (_isSittingInTeleporter) {
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		}
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0x5E0A4905);
		break;
	case 0x483E:
		teleporterDisappear(0xD86E4477);
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
