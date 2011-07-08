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

#ifndef NEVERHOOD_COLLISIONMAN_H
#define NEVERHOOD_COLLISIONMAN_H

#include "neverhood/neverhood.h"
#include "neverhood/sprite.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

class CollisionMan {
public:
	CollisionMan(NeverhoodEngine *vm);
	~CollisionMan();
	void setHitRects(uint32 id);
	void setHitRects(HitRectList *hitRects);
	void clearHitRects();
	HitRect *findHitRectAtPos(int16 x, int16 y);
	void addSprite(Sprite *sprite);
	void removeSprite(Sprite *sprite);
	void clearSprites();
	void save();
	void restore();
	uint getSpriteCount() const { return _sprites.size(); }
	Sprite *getSprite(uint index) const { return _sprites[index]; }
protected:
	NeverhoodEngine *_vm;
	HitRectList *_hitRects;
	Common::Array<Sprite*> _sprites;
};


} // End of namespace Neverhood

#endif /* NEVERHOOD_COLLISIONMAN_H */
