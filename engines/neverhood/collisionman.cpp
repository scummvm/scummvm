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

#include "neverhood/collisionman.h"

namespace Neverhood {

static HitRect defaultHitRect = {NRect(), 0x5000};

CollisionMan::CollisionMan(NeverhoodEngine *vm)
	: _vm(vm), _hitRects(NULL) {
}

CollisionMan::~CollisionMan() {
}

void CollisionMan::setHitRects(uint32 id) {
	setHitRects(_vm->_staticData->getHitRectList(id));
}

void CollisionMan::setHitRects(HitRectList *hitRects) {
	_hitRects = hitRects;
}

void CollisionMan::clearHitRects() {
	_hitRects = NULL;
}

HitRect *CollisionMan::findHitRectAtPos(int16 x, int16 y) {
	if (_hitRects)
		for (HitRectList::iterator it = _hitRects->begin(); it != _hitRects->end(); it++)
			if ((*it).rect.contains(x, y))
				return &(*it);
	return &defaultHitRect; 
}

void CollisionMan::addCollisionSprite(Sprite *sprite) {
	int index = 0, insertIndex = -1;
	for (Common::Array<Sprite*>::iterator iter = _collisionSprites.begin(); iter != _collisionSprites.end(); iter++) {
		if ((*iter)->getPriority() > sprite->getPriority()) {
			insertIndex = index;
			break;
		}
		index++;
	}
	if (insertIndex >= 0)
		_collisionSprites.insert_at(insertIndex, sprite);
	else
		_collisionSprites.push_back(sprite);		
}

void CollisionMan::removeCollisionSprite(Sprite *sprite) {
	for (uint index = 0; index < _collisionSprites.size(); index++) {
		if (_collisionSprites[index] == sprite) {
			_collisionSprites.remove_at(index);
			break;
		}
	}
}

void CollisionMan::clearCollisionSprites() {
	_collisionSprites.clear();
}

void CollisionMan::checkCollision(Sprite *sprite, uint16 flags, int messageNum, uint32 messageParam) {
	for (Common::Array<Sprite*>::iterator iter = _collisionSprites.begin(); iter != _collisionSprites.end(); iter++) {
		Sprite *collSprite = *iter;
		if ((sprite->getFlags() & flags) && collSprite->checkCollision(sprite->getCollisionBounds())) {
			sprite->sendMessage(collSprite, messageNum, messageParam);
		}
	}	
}

} // End of namespace Neverhood
