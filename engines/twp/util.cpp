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

#include "twp/util.h"
#include "twp/scenegraph.h"

namespace Twp {

Math::Vector2d operator*(Math::Vector2d v, float f) {
	return Math::Vector2d(v.getX() * f, v.getY() * f);
}

Facing getFacing(int dir, Facing facing) {
	if (dir == 0)
		return facing;
	if (dir == 0x10)
		return getOppositeFacing(facing);
	return (Facing)dir;
}

Facing flip(Facing facing) {
	switch (facing) {
	case FACE_BACK:
		return FACE_FRONT;
	case FACE_FRONT:
		return FACE_BACK;
	case FACE_LEFT:
		return FACE_RIGHT;
	case FACE_RIGHT:
		return FACE_LEFT;
	}
}

Facing getFacingToFaceTo(Object *actor, Object *obj) {
	Math::Vector2d d = obj->_node->getPos() + obj->_node->getOffset() - (actor->_node->getPos() + actor->_node->getOffset());
	if (abs(d.getY()) > abs(d.getX())) {
		return d.getY() > 0 ? FACE_BACK : FACE_FRONT;
	}
	return d.getX() > 0 ? FACE_RIGHT : FACE_LEFT;
}

static float parseFps(const Common::JSONValue &jFps) {
	if (jFps.isNumber())
		return jFps.asNumber();
	if (jFps.isIntegerNumber())
		return jFps.asIntegerNumber();
	error("fps should be a number: %s", jFps.stringify().c_str());
}

static ObjectAnimation parseObjectAnimation(const Common::JSONObject &jAnim) {
	ObjectAnimation result;
	if (jAnim.contains("sheet"))
		result.sheet = jAnim["sheet"]->asString();
	result.name = jAnim["name"]->asString();
	result.loop = toBool(jAnim, "loop");
	result.fps = jAnim.contains("fps") ? parseFps(*jAnim["fps"]) : 0.f;
	result.flags = jAnim.contains("flags") && jAnim["flags"]->isIntegerNumber() ? jAnim["flags"]->asIntegerNumber() : 0;
	if (jAnim.contains("frames") && jAnim["frames"]->isArray()) {
		const Common::JSONArray &jFrames = jAnim["frames"]->asArray();
		for (auto it = jFrames.begin(); it != jFrames.end(); it++) {
			Common::String name = (*it)->asString();
			result.frames.push_back(name);
		}
	}

	if (jAnim.contains("layers") && jAnim["layers"]->isArray()) {
		const Common::JSONArray &jLayers = jAnim["layers"]->asArray();
		for (auto it = jLayers.begin(); it != jLayers.end(); it++) {
			ObjectAnimation layer = parseObjectAnimation((*it)->asObject());
			result.layers.push_back(layer);
		}
	}

	if (jAnim.contains("triggers") && jAnim["triggers"]->isArray()) {
		const Common::JSONArray &jTriggers = jAnim["triggers"]->asArray();
		for (auto it = jTriggers.begin(); it != jTriggers.end(); it++) {
			result.triggers.push_back((*it)->isString() ? (*it)->asString() : "null");
		}
	}

	if (jAnim.contains("offsets") && jAnim["offsets"]->isArray()) {
		const Common::JSONArray &jOffsets = jAnim["offsets"]->asArray();
		for (auto it = jOffsets.begin(); it != jOffsets.end(); it++) {
			result.offsets.push_back(parseVec2((*it)->asString()));
		}
	}
	return result;
}

bool toBool(const Common::JSONObject &jNode, const Common::String &key) {
	return jNode.contains(key) && jNode[key]->asIntegerNumber() == 1;
}

Math::Vector2d parseVec2(const Common::String &s) {
	float x, y;
	sscanf(s.c_str(), "{%f,%f}", &x, &y);
	return {x, y};
}

Common::Rect parseRect(const Common::String &s) {
	float x1, y1;
	float x2, y2;
	sscanf(s.c_str(), "{{%f,%f},{%f,%f}}", &x1, &y1, &x2, &y2);
	return Common::Rect(x1, y1, x2, y2);
}

void parseObjectAnimations(const Common::JSONArray &jAnims, Common::Array<ObjectAnimation> &anims) {
	for (auto it = jAnims.begin(); it != jAnims.end(); it++) {
		anims.push_back(parseObjectAnimation((*it)->asObject()));
	}
}

} // namespace Twp
