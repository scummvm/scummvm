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

	// DEBUG
	if (_hitRects) {
		debug("CollisionMan::setHitRects() count = %d", _hitRects->size());
		for (HitRectList::iterator it = _hitRects->begin(); it != _hitRects->end(); it++) {
			HitRect *hitRect = &(*it);
			debug("(%d, %d, %d, %d) -> %04X", hitRect->rect.x1, hitRect->rect.y1, hitRect->rect.x2, hitRect->rect.y2, hitRect->type);
		}
	}
	
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

void CollisionMan::addSprite(Sprite *sprite) {
	int index = 0, insertIndex = -1;
	for (Common::Array<Sprite*>::iterator iter = _sprites.begin(); iter != _sprites.end(); iter++) {
		if ((*iter)->getPriority() > sprite->getPriority()) {
			insertIndex = index;
			break;
		}
		index++;
	}
	if (insertIndex >= 0)
		_sprites.insert_at(insertIndex, sprite);
	else
		_sprites.push_back(sprite);		
}

void CollisionMan::removeSprite(Sprite *sprite) {
	for (uint index = 0; index < _sprites.size(); index++) {
		if (_sprites[index] == sprite) {
			_sprites.remove_at(index);
			break;
		}
	}
}

void CollisionMan::clearSprites() {
	_sprites.clear();
}

void CollisionMan::checkCollision(Sprite *sprite, uint16 flags, int messageNum, uint32 messageParam) {
	for (Common::Array<Sprite*>::iterator iter = _sprites.begin(); iter != _sprites.end(); iter++) {
		Sprite *collSprite = *iter;
		if ((sprite->getFlags() & flags) && collSprite->checkCollision(sprite->getRect())) {
			sprite->sendMessage(collSprite, messageNum, messageParam);
		}
	}	
}

void CollisionMan::save() {
	// TODO
}

void CollisionMan::restore() {
	// TODO
}

} // End of namespace Neverhood
