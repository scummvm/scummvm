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

#ifndef TWINE_ENTITY_H
#define TWINE_ENTITY_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct EntityBody {
	int index;		/**< index in file3d.hqr */
	ActorBoundingBox actorBoundingBox;
	int bodyIndex;	/**< index in body.hqr */
};

struct EntityAnim {
	AnimationTypes animation;
	int animIndex;

	struct Action {
		uint8 type = 0;
		uint8 animFrame = 0;
		int16 sampleIndex = 0;
		int16 frequency = 0;
		int16 xAngle = 0;
		int16 yAngle = 0;
		int16 xRotPoint = 0;
		int16 extraAngle = 0;
		int16 finalAngle = 0;
		int16 strength = 0;
		int16 distanceX = 0;
		int16 distanceY = 0;
		int16 distanceZ = 0;
		int16 yHeight = 0;
		uint8 spriteIndex = 0;
		uint8 targetActor = 0;
		int16 repeat = 0;
	};

	Common::Array<Action> _actions;
};

class EntityData : public Parser {
private:
	Common::Array<EntityBody> _bodies;
	Common::Array<EntityAnim> _animations;

	bool loadBody(Common::SeekableReadStream &stream);
	bool loadAnim(Common::SeekableReadStream &stream);

public:
	bool loadFromStream(Common::SeekableReadStream &stream) override;

	const Common::Array<EntityAnim::Action> *getActions(AnimationTypes animation) const;
	const EntityBody *getBody(const int index) const;
	int32 getAnimIndex(AnimationTypes animation) const;
};

} // End of namespace TwinE

#endif
