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

#include "twp/twp.h"
#include "twp/room.h"
#include "twp/ggpack.h"
#include "twp/squtil.h"
#include "twp/font.h"
#include "twp/scenegraph.h"
#include "twp/ids.h"
#include "twp/object.h"
#include "common/algorithm.h"

namespace Twp {

static Math::Vector2d parseVec2(const Common::String &s) {
	float x, y;
	sscanf(s.c_str(), "{%f,%f}", &x, &y);
	return {x, y};
}

static Common::Rect parseRect(const Common::String &s) {
	float x1, y1;
	float x2, y2;
	sscanf(s.c_str(), "{{%f,%f},{%f,%f}}", &x1, &y1, &x2, &y2);
	return Common::Rect(x1, y1, x2, y2);
}

static bool toBool(const Common::JSONObject &jNode, const Common::String &key) {
	return jNode.contains(key) && jNode[key]->asIntegerNumber() == 1;
}

static ObjectType toObjectType(const Common::JSONObject &jObject) {
	if (toBool(jObject, "prop"))
		return otProp;
	if (toBool(jObject, "spot"))
		return otSpot;
	if (toBool(jObject, "trigger"))
		return otTrigger;
	return otNone;
}

static Direction parseUseDir(const Common::String &s) {
	if (s == "DIR_FRONT")
		return dFront;
	if (s == "DIR_BACK")
		return dBack;
	if (s == "DIR_LEFT")
		return dLeft;
	if (s == "DIR_RIGHT")
		return dRight;
	error("invalid use direction: %s", s.c_str());
}

static Math::Vector2d parseParallax(const Common::JSONValue &v) {
	if (v.isIntegerNumber()) {
		return {(float)v.asIntegerNumber(), 1};
	}
	if (v.isNumber()) {
		return {(float)v.asNumber(), 1};
	}
	if (v.isString()) {
		return parseVec2(v.asString());
	}
	error("parseParallax expected a float, int or string, not this: %s", v.stringify().c_str());
}

static Walkbox parseWalkbox(const Common::String &text) {
	Common::Array<Math::Vector2d> points;
	int i = 1;
	int endPos;
	do {
		uint32 commaPos = text.find(',', i);
		long x = strtol(text.substr(i, commaPos - i).c_str(), nullptr, 10);
		endPos = text.find('}', commaPos + 1);
		long y = std::strtol(text.substr(commaPos + 1, endPos - commaPos - 1).c_str(), nullptr, 10);
		i = endPos + 3;
		points.push_back({(float)x, (float)y});
	} while ((text.size() - 1) != endPos);
	return Walkbox(points);
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
			result.triggers.push_back((*it)->asString());
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

static void parseObjectAnimations(const Common::JSONArray &jAnims, Common::Array<ObjectAnimation> &anims) {
	for (auto it = jAnims.begin(); it != jAnims.end(); it++) {
		anims.push_back(parseObjectAnimation((*it)->asObject()));
	}
}

static Scaling parseScaling(const Common::JSONArray &jScalings) {
	float scale;
	int y;
	Scaling result;
	for (auto it = jScalings.begin(); it != jScalings.end(); it++) {
		const Common::String &v = (*it)->asString();
		sscanf(v.c_str(), "%f@%d", &scale, &y);
		result.values.push_back(ScalingValue{scale, y});
	}
	return result;
}

void Room::load(Common::SeekableReadStream &s) {
	GGHashMapDecoder d;
	Common::JSONValue *value = d.open(&s);
	const Common::JSONObject &jRoom = value->asObject();

	_name = jRoom["name"]->asString();
	_sheet = jRoom["sheet"]->asString();

	_roomSize = parseVec2(jRoom["roomsize"]->asString());
	_height = jRoom.contains("height") ? jRoom["height"]->asIntegerNumber() : _roomSize.getY();
	_fullscreen = jRoom.contains("fullscreen") ? jRoom["fullscreen"]->asIntegerNumber() : 0;

	// backgrounds
	Common::StringArray backNames;
	if (jRoom["background"]->isString()) {
		backNames.push_back(jRoom["background"]->asString());
	} else {
		const Common::JSONArray &jBacks = jRoom["background"]->asArray();
		for (int i = 0; i < jBacks.size(); i++) {
			backNames.push_back(jBacks[i]->asString());
		}
	}

	{
		Layer *layer = new Layer();
		layer->_names.push_back(backNames);
		layer->_zsort = 0;
		layer->_parallax = Math::Vector2d(1, 1);
		_layers.push_back(layer);
	}

	// layers
	if (jRoom.contains("layers")) {
		const Common::JSONArray &jLayers = jRoom["layers"]->asArray();
		for (int i = 0; i < jLayers.size(); i++) {
			Layer *layer = new Layer();
			const Common::JSONObject &jLayer = jLayers[i]->asObject();
			if (jLayer["name"]->isArray()) {
				const Common::JSONArray &jNames = jLayer["name"]->asArray();
				for (int j = 0; j < jNames.size(); j++) {
					layer->_names.push_back(jNames[j]->asString());
				}
			} else if (jLayer["name"]->isString()) {
				layer->_names.push_back(jLayer["name"]->asString());
			}
			layer->_parallax = parseParallax(*jLayer["parallax"]);
			layer->_zsort = jLayer["zsort"]->asIntegerNumber();
			_layers.push_back(layer);
		}
	}

	// walkboxes
	if (jRoom.contains("walkboxes")) {
		const Common::JSONArray &jWalkboxes = jRoom["walkboxes"]->asArray();
		for (auto it = jWalkboxes.begin(); it != jWalkboxes.end(); it++) {
			const Common::JSONObject &jWalkbox = (*it)->asObject();
			Walkbox walkbox = parseWalkbox(jWalkbox["polygon"]->asString());
			if (jWalkbox.contains("name") && jWalkbox["name"]->isString()) {
				walkbox._name = jWalkbox["name"]->asString();
			}
			_walkboxes.push_back(walkbox);
		}
	}

	// objects
	if (jRoom.contains("objects")) {
		const Common::JSONArray &jobjects = jRoom["objects"]->asArray();
		for (auto it = jobjects.begin(); it != jobjects.end(); it++) {
			const Common::JSONObject &jObject = (*it)->asObject();
			Object obj;
			obj._state = -1;
			Node *objNode = new Node(obj._key);
			objNode->_pos = Math::Vector2d(parseVec2(jObject["pos"]->asString()));
			objNode->_zOrder = jObject["zsort"]->asIntegerNumber();
			obj._node = objNode;
			obj._nodeAnim = new Anim(&obj);
			obj._node->addChild(obj._nodeAnim);
			obj._key = jObject["name"]->asString();
			obj._usePos = parseVec2(jObject["usepos"]->asString());
			if (jObject.contains("usedir")) {
				obj._useDir = parseUseDir(jObject["usedir"]->asString());
			} else {
				obj._useDir = dNone;
			}
			obj._hotspot = parseRect(jObject["hotspot"]->asString());
			obj._objType = toObjectType(jObject);
			if (jObject.contains("parent"))
				jObject["parent"]->asString();
			obj._room = this;
			if (jObject.contains("animations")) {
				parseObjectAnimations(jObject["animations"]->asArray(), obj._anims);
			}
			obj._layer = layer(0);
			layer(0)->_objects.push_back(&obj);
		}
	}

	// scalings
	if (jRoom.contains("scaling")) {
		const Common::JSONArray &jScalings = jRoom["scaling"]->asArray();
		if (jScalings[0]->isString()) {
			_scalings.push_back(parseScaling(jScalings));
		} else {
			for (auto it = jScalings.begin(); it != jScalings.end(); it++) {
				const Common::JSONObject &jScaling = (*it)->asObject();
				Scaling scaling = parseScaling(jScaling["scaling"]->asArray());
				if (jScaling.contains("trigger") && jScaling["trigger"]->isString())
					scaling.trigger = jScaling["trigger"]->asString();
				_scalings.push_back(scaling);
			}
		}
		_scaling = _scalings[0];
	}

	delete value;
}

Layer *Room::layer(int zsort) {
	for (int i = 0; i < _layers.size(); i++) {
		Layer *l = _layers[i];
		if (l->_zsort == zsort)
			return l;
	}
	return NULL;
}

Math::Vector2d Room::getScreenSize() {
	switch (_height) {
	case 128:
		return {320, 180};
	case 172:
		return {428, 240};
	case 256:
		return {640, 360};
	default:
		return {_roomSize.getX(), (float)_height};
	}
}

Walkbox::Walkbox(const Common::Array<Math::Vector2d> &polygon, bool visible)
	: _polygon(polygon), _visible(visible) {
}

} // namespace Twp
