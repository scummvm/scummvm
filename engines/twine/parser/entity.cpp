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

#include "twine/parser/entity.h"
#include "common/stream.h"

namespace TwinE {

bool EntityData::loadBody(Common::SeekableReadStream &stream) {
	EntityBody body;
	body.index = stream.readByte();
	const int32 pos = stream.pos();
	uint8 size = stream.readByte();
	body.bodyIndex = stream.readUint16LE();
	body.actorBoundingBox.hasBoundingBox = stream.readByte();
	if (body.actorBoundingBox.hasBoundingBox) {
		if (stream.readByte() == ActionType::ACTION_ZV) {
			body.actorBoundingBox.bottomLeftX = stream.readUint16LE();
			body.actorBoundingBox.bottomLeftY = stream.readUint16LE();
			body.actorBoundingBox.bottomLeftZ = stream.readUint16LE();
			body.actorBoundingBox.topRightX = stream.readUint16LE();
			body.actorBoundingBox.topRightY = stream.readUint16LE();
			body.actorBoundingBox.topRightZ = stream.readUint16LE();
		}
	}
	_bodies.push_back(body);
	stream.seek(pos + size);
	return !stream.err();
}

bool EntityData::loadAnim(Common::SeekableReadStream &stream) {
	EntityAnim anim;
	anim.animation = (AnimationTypes)stream.readByte();
	const int32 pos = stream.pos();
	uint8 size = stream.readByte();
	anim.animIndex = stream.readSint16LE();
	const uint8 numActions = stream.readByte();
	for (uint8 i = 0U; i < numActions; ++i) {
		EntityAnim::Action action;
		action.type = stream.readByte();
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
			action.sampleIndex = stream.readByte();
			stream.skip(1);
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
		case ActionType::ACTION_ZV:
		default:
			break;
		}
		anim._actions.push_back(action);
	}
	_animations.push_back(anim);
	stream.seek(pos + size);
	return !stream.err();
}

bool EntityData::loadFromStream(Common::SeekableReadStream &stream) {
	do {
		const uint8 opcode = stream.readByte();
		if (opcode == 1) {
			if (!loadBody(stream)) {
				return false;
			}
		} else if (opcode == 3) {
			if (!loadAnim(stream)) {
				return false;
			}
		} else if (opcode == 0xFF) {
			break;
		}
	} while (!stream.eos() && !stream.err());

	return true;
}

const Common::Array<EntityAnim::Action>* EntityData::getActions(AnimationTypes animation) const {
	for (const EntityAnim& anim : _animations) {
		if (anim.animation == animation) {
			if (anim._actions.empty()) {
				return nullptr;
			}
			return &anim._actions;
		}
	}
	return nullptr;
}

const EntityBody* EntityData::getBody(const int index) const {
	for (const EntityBody& body : _bodies) {
		if (body.index == index) {
			return &body;
		}
	}
	return nullptr;
}

int32 EntityData::getAnimIndex(AnimationTypes animation) const {
	for (const EntityAnim& anim : _animations) {
		if (anim.animation == animation) {
			return anim.animIndex;
		}
	}
	return -1;
}

} // End of namespace TwinE
