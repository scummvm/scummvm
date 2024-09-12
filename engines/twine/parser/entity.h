/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_PARSER_ENTITY_H
#define TWINE_PARSER_ENTITY_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct EntityBody {
	int index; /**< index in file3d.hqr */
	ActorBoundingBox actorBoundingBox;
	int hqrBodyIndex; /**< index in body.hqr */
};

struct EntityAnim {
	AnimationTypes animation;
	int animIndex;

	struct Action {
		ActionType type = ActionType::ACTION_NOP;
		uint8 animFrame = 0;
		uint8 lastAnimFrame = 0;
		int8 weight = 0;
		byte sampleVolume = 0;
		int16 pointIndex = 0;
		int16 spriteIndex = 0;
		uint8 targetActor = 0;
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
		int16 repeat = 0;
		int16 speed = 0;
		int16 superHitX = 0;
		int16 superHitY = 0;
		int16 superHitZ = 0;
		int16 sizeSuperHit = 0;
		int16 decal = 0;
		int32 scale = 0;
		BoundingBox bbox;
	};

	Common::Array<Action> _actions;
};

/**
 * @brief Associate 3d models from body hqr with animations from anim.hqr for the game characters
 */
class EntityData : public Parser {
private:
	Common::Array<EntityBody> _bodies;
	Common::Array<EntityAnim> _animations;

	bool loadBody(Common::SeekableReadStream &stream, bool lba1);
	bool loadAnim(Common::SeekableReadStream &stream, bool lba1);

protected:
	void reset() override;

public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const Common::Array<EntityAnim::Action> *getActions(AnimationTypes animation) const;
	const EntityBody *getBody(const int index) const;
	int32 getAnimIndex(AnimationTypes animation) const;
};

} // End of namespace TwinE

#endif
