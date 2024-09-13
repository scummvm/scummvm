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

#include "twine/parser/entity.h"
#include "common/stream.h"
#include "twine/shared.h"

namespace TwinE {

bool EntityData::loadBody(Common::SeekableReadStream &stream, bool lba1) {
	EntityBody body;
	body.index = stream.readByte();
	const int64 pos = stream.pos();
	uint8 size = stream.readByte();
	body.hqrBodyIndex = (int16)stream.readUint16LE();
	const uint8 numActions = stream.readByte();
	for (uint8 i = 0U; i < numActions; ++i) {
		if ((ActionType)stream.readByte() == ActionType::ACTION_ZV) {
			body.actorBoundingBox.hasBoundingBox = true;
			body.actorBoundingBox.bbox.mins.x = stream.readSint16LE();
			body.actorBoundingBox.bbox.mins.y = stream.readSint16LE();
			body.actorBoundingBox.bbox.mins.z = stream.readSint16LE();
			body.actorBoundingBox.bbox.maxs.x = stream.readSint16LE();
			body.actorBoundingBox.bbox.maxs.y = stream.readSint16LE();
			body.actorBoundingBox.bbox.maxs.z = stream.readSint16LE();
		}
	}
	_bodies.push_back(body);
	stream.seek(pos + size);
	return !stream.err();
}

bool EntityData::loadAnim(Common::SeekableReadStream &stream, bool lba1) {
	EntityAnim anim;
	if (lba1) {
		anim.animation = (AnimationTypes)stream.readByte();
	} else {
		anim.animation = (AnimationTypes)stream.readUint16LE();
	}
	const int64 pos = stream.pos();
	uint8 size = stream.readByte();
	anim.animIndex = stream.readSint16LE();
	const uint8 numActions = stream.readByte();
	for (uint8 i = 0U; i < numActions; ++i) {
		EntityAnim::Action action;
		action.type = (ActionType)stream.readByte();
		switch (action.type) {
		case ActionType::ACTION_HITTING:
			action.animFrame = stream.readByte();
			action.strength = stream.readByte();
			break;
		case ActionType::ACTION_SAMPLE:
			action.animFrame = stream.readByte();
			action.sampleIndex = stream.readSint16LE();
			break;
		case ActionType::ACTION_SAMPLE_FREQ:
			action.animFrame = stream.readByte();
			action.sampleIndex = stream.readSint16LE();
			action.frequency = stream.readSint16LE();
			break;
		case ActionType::ACTION_THROW_MAGIC_BALL:
			action.animFrame = stream.readByte();
			action.yHeight = stream.readSint16LE();
			action.xAngle = ToAngle(stream.readSint16LE());
			action.xRotPoint = stream.readSint16LE();
			action.extraAngle = stream.readByte();
			break;
		case ActionType::ACTION_SAMPLE_REPEAT:
			action.animFrame = stream.readByte();
			action.sampleIndex = stream.readSint16LE();
			action.repeat = stream.readSint16LE();
			if (!lba1) {
				action.decal = stream.readSint16LE();
				action.sampleVolume = stream.readByte();
				action.frequency = stream.readSint16LE();
			}
			break;
		case ActionType::ACTION_THROW_SEARCH:
			action.animFrame = stream.readByte();
			action.yHeight = stream.readSint16LE();
			action.spriteIndex = stream.readByte();
			action.targetActor = stream.readByte();
			action.finalAngle = stream.readSint16LE();
			action.strength = stream.readByte();
			break;
		case ActionType::ACTION_THROW_EXTRA_BONUS:
		case ActionType::ACTION_THROW_ALPHA:
			action.animFrame = stream.readByte();
			action.yHeight = stream.readSint16LE();
			action.spriteIndex = stream.readByte();
			action.xAngle = ToAngle(stream.readSint16LE());
			action.yAngle = ToAngle(stream.readSint16LE());
			action.xRotPoint = stream.readSint16LE();
			// TODO: does lba2 need a scaling here for xRotPoint?
			action.extraAngle = ToAngle(stream.readByte());
			action.strength = stream.readByte();
			break;
		case ActionType::ACTION_LEFT_STEP:
		case ActionType::ACTION_RIGHT_STEP:
		case ActionType::ACTION_HERO_HITTING:
			action.animFrame = stream.readByte();
			break;
		case ActionType::ACTION_SAMPLE_STOP:
			action.animFrame = stream.readByte();
			if (lba1) {
				action.sampleIndex = stream.readByte();
				stream.skip(1);
			} else {
				action.sampleIndex = stream.readUint16LE();
			}
			break;
		case ActionType::ACTION_THROW_3D:
		case ActionType::ACTION_THROW_3D_ALPHA:
			action.animFrame = stream.readByte();
			action.distanceX = stream.readSint16LE();
			action.distanceY = stream.readSint16LE();
			action.distanceZ = stream.readSint16LE();
			action.spriteIndex = stream.readByte();
			action.xAngle = ToAngle(stream.readSint16LE());
			action.yAngle = ToAngle(stream.readSint16LE());
			action.xRotPoint = stream.readSint16LE();
			action.extraAngle = ToAngle(stream.readByte());
			action.strength = stream.readByte();
			break;
		case ActionType::ACTION_THROW_3D_SEARCH:
			action.animFrame = stream.readByte();
			action.distanceX = stream.readSint16LE();
			action.distanceY = stream.readSint16LE();
			action.distanceZ = stream.readSint16LE();
			action.spriteIndex = stream.readByte();
			action.targetActor = stream.readByte();
			action.finalAngle = ToAngle(stream.readSint16LE());
			action.strength = stream.readByte();
			break;
		case ActionType::ACTION_THROW_3D_MAGIC:
			action.animFrame = stream.readByte();
			action.distanceX = stream.readSint16LE();
			action.distanceY = stream.readSint16LE();
			action.distanceZ = stream.readSint16LE();
			action.xAngle = stream.readSint16LE();
			action.yAngle = stream.readSint16LE();
			action.finalAngle = stream.readByte();
			break;
		case ActionType::ACTION_ZV:
		default:
			break;
		}
		if (!lba1) {
			switch (action.type) {
			case ActionType::ACTION_ZV:
				action.animFrame = stream.readByte();
				action.bbox.mins.x = stream.readSint16LE();
				action.bbox.mins.y = stream.readSint16LE();
				action.bbox.mins.z = stream.readSint16LE();
				action.bbox.maxs.x = stream.readSint16LE();
				action.bbox.maxs.y = stream.readSint16LE();
				action.bbox.maxs.z = stream.readSint16LE();
				break;
			case ActionType::ACTION_SUPER_HIT:
				action.animFrame = stream.readByte();
				action.strength = stream.readByte();
				action.superHitX = stream.readSint16LE();
				action.superHitY = stream.readSint16LE();
				action.superHitZ = stream.readSint16LE();
				action.sizeSuperHit = stream.readSint16LE();
				break;
			case ActionType::ACTION_THROW_OBJ_3D:
				action.animFrame = stream.readByte();
				action.distanceX = stream.readSint16LE();
				action.distanceY = stream.readSint16LE();
				action.distanceZ = stream.readSint16LE();
				action.spriteIndex = stream.readByte();
				action.xAngle = ToAngle(stream.readSint16LE());
				action.yAngle = ToAngle(stream.readSint16LE());
				action.xRotPoint = stream.readSint16LE();
				action.extraAngle = ToAngle(stream.readByte());
				action.strength = stream.readByte();
				break;
			case ActionType::ACTION_NEW_SAMPLE:
				action.animFrame = stream.readByte();
				action.sampleIndex = stream.readSint16LE();
				action.decal = stream.readSint16LE();
				action.sampleVolume = stream.readByte();
				action.frequency = stream.readSint16LE();
				break;
			case ActionType::ACTION_THROW_DART:
				action.animFrame = stream.readByte();
				action.distanceY = stream.readSint16LE();
				action.xAngle = ToAngle(stream.readSint16LE());
				action.speed = stream.readSint16LE();
				action.weight = stream.readSByte();
				break;
			case ActionType::ACTION_SHIELD:
				action.animFrame = stream.readByte();
				action.lastAnimFrame = stream.readByte();
				break;
			case ActionType::ACTION_FLOW_3D:
			case ActionType::ACTION_THROW_3D_CONQUE:
				action.animFrame = stream.readByte();
				action.xAngle = ToAngle(stream.readSint16LE());
				action.yHeight = stream.readSint16LE();
				action.yAngle = ToAngle(stream.readSint16LE());
				action.spriteIndex = stream.readByte();
				break;
			case ActionType::ACTION_IMPACT:
			case ActionType::ACTION_RENVOYABLE:
				action.animFrame = stream.readByte();
				action.strength = stream.readSint16LE();
				break;
			case ActionType::ACTION_SCALE:
				action.animFrame = stream.readByte();
				action.scale = stream.readSint32LE();
				break;
			case ActionType::ACTION_IMPACT_3D:
				action.animFrame = stream.readByte();
				action.xAngle = ToAngle(stream.readSint16LE());
				action.yHeight = stream.readSint16LE();
				action.yAngle = ToAngle(stream.readSint16LE());
				action.spriteIndex = stream.readSint16LE();
				break;
			case ActionType::ACTION_THROW_MAGIC_EXTRA:
				action.animFrame = stream.readByte();
				action.pointIndex = stream.readSint16LE();
				action.spriteIndex = stream.readByte();
				action.xAngle = ToAngle(stream.readSint16LE());
				action.speed = stream.readSint16LE();
				action.weight = stream.readSByte();
				break;
			case ActionType::ACTION_ZV_ANIMIT:
			case ActionType::ACTION_RENVOIE:
			case ActionType::ACTION_TRANSPARENT:
			case ActionType::ACTION_SAMPLE_MAGIC:
			case ActionType::ACTION_LEFT_JUMP:
			case ActionType::ACTION_RIGHT_JUMP:
			case ActionType::ACTION_THROW_FOUDRE:
				action.animFrame = stream.readByte();
				/* empty */
				break;
			case ActionType::ACTION_PATH:
			case ActionType::ACTION_FLOW:
			default:
				break;
			}
		}
		if (action.type > ActionType::ACTION_THROW_FOUDRE) {
			error("Unknown action type %d", (int)action.type);
		}
		anim._actions.push_back(action);
	}
	_animations.push_back(anim);
	stream.seek(pos + size);
	return !stream.err();
}

void EntityData::reset() {
	_animations.clear();
	_bodies.clear();
}

bool EntityData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	reset();
	do {
		const uint8 opcode = stream.readByte();
		if (opcode == 1) {
			if (!loadBody(stream, lba1)) {
				return false;
			}
		} else if (opcode == 3) {
			if (!loadAnim(stream, lba1)) {
				return false;
			}
		} else if (opcode == 0xFF) {
			break;
		}
	} while (!stream.eos() && !stream.err());

	return true;
}

const Common::Array<EntityAnim::Action> *EntityData::getActions(AnimationTypes animation) const {
	for (const EntityAnim &anim : _animations) {
		if (anim.animation == animation) {
			if (anim._actions.empty()) {
				return nullptr;
			}
			return &anim._actions;
		}
	}
	return nullptr;
}

const EntityBody *EntityData::getBody(const int index) const {
	for (const EntityBody &body : _bodies) {
		if (body.index == index) {
			return &body;
		}
	}
	return nullptr;
}

int32 EntityData::getAnimIndex(AnimationTypes animation) const {
	for (const EntityAnim &anim : _animations) {
		if (anim.animation == animation) {
			return anim.animIndex;
		}
	}
	return -1;
}

} // End of namespace TwinE
