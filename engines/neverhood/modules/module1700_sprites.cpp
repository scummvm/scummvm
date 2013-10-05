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
	case 0x4806:
		setSubVar(VA_HAS_TAPE, _tapeIndex, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
