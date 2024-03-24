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

#include "twp/object.h"
#include "twp/scenegraph.h"

namespace Twp {

Math::Vector2d operator*(const Math::Vector2d &v, float f) {
	return Math::Vector2d(v.getX() * f, v.getY() * f);
}

Facing getFacing(int dir, Facing facing) {
	if (dir == 0)
		return facing;
	if (dir == 0x10)
		return flip(facing);
	return (Facing)dir;
}

Facing flip(Facing facing) {
	switch (facing) {
	case Facing::FACE_BACK:
		return Facing::FACE_FRONT;
	default:
	case Facing::FACE_FRONT:
		return Facing::FACE_BACK;
	case Facing::FACE_LEFT:
		return Facing::FACE_RIGHT;
	case Facing::FACE_RIGHT:
		return Facing::FACE_LEFT;
	}
}

Facing getFacingToFaceTo(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj) {
	Math::Vector2d d = obj->_node->getPos() + obj->_node->getOffset() - (actor->_node->getPos() + actor->_node->getOffset());
	if (abs(d.getY()) > abs(d.getX())) {
		return d.getY() > 0 ? Facing::FACE_BACK : Facing::FACE_FRONT;
	}
	return d.getX() > 0 ? Facing::FACE_RIGHT : Facing::FACE_LEFT;
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
	anims.clear();
	for (auto it = jAnims.begin(); it != jAnims.end(); it++) {
		anims.push_back(parseObjectAnimation((*it)->asObject()));
	}
}

float distanceSquared(const Math::Vector2d &p1, const Math::Vector2d &p2) {
	const float dx = p1.getX() - p2.getX();
	const float dy = p1.getY() - p2.getY();
	return dx * dx + dy * dy;
}

float distanceToSegmentSquared(const Math::Vector2d &p, const Math::Vector2d &v, const Math::Vector2d &w) {
	const float l2 = distanceSquared(v, w);
	if (l2 == 0)
		return distanceSquared(p, v);
	const float t = ((p.getX() - v.getX()) * (w.getX() - v.getX()) + (p.getY() - v.getY()) * (w.getY() - v.getY())) / l2;
	if (t < 0)
		return distanceSquared(p, v);
	if (t > 1)
		return distanceSquared(p, w);
	return distanceSquared(p, Math::Vector2d(v.getX() + t * (w.getX() - v.getX()), v.getY() + t * (w.getY() - v.getY())));
}

float distanceToSegment(const Math::Vector2d &p, const Math::Vector2d &v, const Math::Vector2d &w) {
	return sqrt(distanceToSegmentSquared(p, v, w));
}

float distance(const Math::Vector2d &p1, const Math::Vector2d &p2) {
	return sqrt(distanceSquared(p1, p2));
}

Common::String join(const Common::Array<Common::String> &array, const Common::String &sep) {
	Common::String result;
	if (array.size() > 0) {
		result += array[0];
		for (uint i = 1; i < array.size(); i++) {
			result += (sep + array[i]);
		}
	}
	return result;
}

Common::String remove(const Common::String &txt, char startC, char endC) {
	if ((txt.size() > 0) && txt[0] == startC) {
		uint32 i = txt.find(endC);
		if (i != Common::String::npos) {
			return txt.substr(i + 1);
		}
	}
	return txt;
}

Common::String replaceAll(const Common::String &s, const Common::String &what, const Common::String &by) {
	Common::String result;
	uint i = 0;
	size_t whatSize = what.size();
	while (true) {
		uint j = s.find(what, i);
		if (j == Common::String::npos)
			break;
		result += s.substr(i, j - i);
		result += by;
		i = j + whatSize;
	}
	result += s.substr(i);
	return result;
}

void scale(Math::Matrix4 &m, const Math::Vector2d &v) {
	m(0, 0) *= v.getX();
	m(1, 1) *= v.getY();
}

float dot(const Math::Vector2d &u, const Math::Vector2d &v) {
	return (u.getX() * v.getX()) + (u.getY() * v.getY());
}

float length(const Math::Vector2d &v) { return sqrt(dot(v, v)); }

bool lineSegmentsCross(const Math::Vector2d &a, const Math::Vector2d &b, const Math::Vector2d &c, const Math::Vector2d &d) {
	const float EPSILON = 1e-3f;
	const float denominator = ((b.getX() - a.getX()) * (d.getY() - c.getY())) - ((b.getY() - a.getY()) * (d.getX() - c.getX()));
	if (abs(denominator) < EPSILON) {
		return false;
	}

	const float numerator1 = ((a.getY() - c.getY()) * (d.getX() - c.getX())) - ((a.getX() - c.getX()) * (d.getY() - c.getY()));
	const float numerator2 = ((a.getY() - c.getY()) * (b.getX() - a.getX())) - ((a.getX() - c.getX()) * (b.getY() - a.getY()));
	if ((abs(numerator1) < EPSILON) || (abs(numerator2) < EPSILON)) {
		return false;
	}

	const float r = numerator1 / denominator;
	const float s = numerator2 / denominator;
	return ((r > 0.f) && (r < 1.f)) && ((s > 0.f) && (s < 1.f));
}

} // namespace Twp
