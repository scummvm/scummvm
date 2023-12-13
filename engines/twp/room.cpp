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
				walkbox.name = jWalkbox["name"]->asString();
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

static int gObjectId = START_OBJECTID;

Object::Object()
: _state(-1),
_talkOffset(0, 90) {
  _node = new Node("newObj");
  _nodeAnim = new Anim(this);
  _node->addChild(_nodeAnim);
  sq_resetobject(&_table);
}

Object *Room::createObject(const Common::String &sheet, const Common::Array<Common::String> &frames) {
	Object *obj = new Object();
	obj->_temporary = true;

	HSQUIRRELVM v = g_engine->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->_table);
	sq_addref(v, &obj->_table);
	sq_pop(v, 1);

	// assign an id
	setId(obj->_table, gObjectId++);
	Common::String name = frames.size() > 0 ? frames[0] : "noname";
	setf(obj->_table, "name", name);
	obj->_key = name;
	debug("Create object with new table: %s #%d", obj->_name.c_str(), obj->getId());

	obj->_room = this;
	obj->_sheet = sheet;
	obj->_touchable = false;

	// create anim if any
	if (frames.size() > 0) {
		ObjectAnimation objAnim;
		objAnim.name = "state0";
		objAnim.frames.push_back(frames);
		obj->_anims.push_back(objAnim);
	}

	obj->_node->_zOrder = 1;
	layer(0)->_objects.push_back(obj);
	layer(0)->_node->addChild(obj->_node);
	obj->_layer = layer(0);
	obj->setState(0);

	g_engine->_objects.push_back(obj);

	return obj;
}

Object *Room::createTextObject(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign, TextVAlignment vAlign, float maxWidth) {
	Object *obj = new Object();
	obj->_temporary = true;

	HSQUIRRELVM v = g_engine->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->_table);
	sq_addref(v, &obj->_table);
	sq_pop(v, 1);

	// assign an id
	setId(obj->_table, gObjectId++);
	debug("Create object with new table: %s #%d}", obj->_name.c_str(), obj->getId());
	obj->_name = Common::String::format("text#%d: %s", obj->getId(), text.c_str());
	obj->_touchable = false;

	Text txt(fontName, text, hAlign, vAlign, maxWidth);

	// TODO:
	//   let node = newTextNode(text)
	//   var v = 0.5f
	//   case vAlign:
	//   of tvTop:
	//     v = 0f
	//   of tvCenter:
	//     v = 0.5f
	//   of tvBottom:
	//     v = 1f
	//   case hAlign:
	//   of thLeft:
	//     node.setAnchorNorm(vec2(0f, v))
	//   of thCenter:
	//     node.setAnchorNorm(vec2(0.5f, v))
	//   of thRight:
	//     node.setAnchorNorm(vec2(1f, v))
	//   obj.node = node
	//   self.layer(0).objects.add(obj);
	//   self.layer(0).node.addChild obj.node;
	//   obj.layer = self.layer(0);

	g_engine->_objects.push_back(obj);

	return obj;
}

int Object::getId() {
	SQInteger result = 0;
	getf(_table, "_id", result);
	return (int)result;
}

// Changes the `state` of an object, although this can just be a internal state,
//
// it is typically used to change the object's image as it moves from it's current state to another.
// Behind the scenes, states as just simple ints. State0, State1, etc.
// Symbols like `CLOSED` and `OPEN` and just pre-defined to be 0 or 1.
// State 0 is assumed to be the natural state of the object, which is why `OPEN` is 1 and `CLOSED` is 0 and not the other way around.
// This can be a little confusing at first.
// If the state of an object has multiple frames, then the animation is played when changing state, such has opening the clock.
// `GONE` is a unique in that setting an object to `GONE` both sets its graphical state to 1, and makes it untouchable.
// Once an object is set to `GONE`, if you want to make it visible and touchable again, you have to set both:
//
// .. code-block:: Squirrel
// objectState(coin, HERE)
// objectTouchable(coin, YES)
void Object::setState(int state, bool instant) {
	play(state, false, instant);
	_state = state;
}

void Object::play(int state, bool loop, bool instant) {
	play(Common::String::format("state%d", state), loop, instant);
	_state = state;
}

void Object::play(const Common::String &state, bool loop, bool instant) {
	if (state == "eyes_right") {
		showLayer("eyes_front", false);
		showLayer("eyes_left", false);
		showLayer("eyes_right", true);
	} else if (state == "eyes_left") {
		showLayer("eyes_front", false);
		showLayer("eyes_left", true);
		showLayer("eyes_right", false);
	} else if (state == "eyes_front") {
		showLayer("eyes_front", true);
		showLayer("eyes_left", false);
		showLayer("eyes_right", false);
	} else {
		_animName = state;
		_animLoop = loop;
		if (!playCore(state, loop, instant))
			playCore(state + suffix(), loop, instant);
	}
}

bool Object::playCore(const Common::String& state, bool loop, bool instant) {
  for(int i=0;i<_anims.size();i++) {
    ObjectAnimation& anim = _anims[i];
    if(anim.name == state) {
      _animFlags = anim.flags;
      _nodeAnim->setAnim(&anim, _fps, loop, instant);
      return true;
	}
  }

  // if not found, clear the previous animation
  if (!isActor(getId())) {
    _nodeAnim->clearFrames();
    _nodeAnim->clear();
  }
  return false;
}

void Object::showLayer(const Common::String &layer, bool visible) {
	Common::String* s = Common::find(_hiddenLayers.begin(), _hiddenLayers.end(), layer);
	if (visible) {
		if (s)
			_hiddenLayers.remove_at(s - &_hiddenLayers[0]);
	} else {
		if (!s)
			_hiddenLayers.push_back(layer);
	}
	if (_node != NULL) {
		for (int i = 0; i < _node->getChildren().size(); i++) {
			Node *node = _node->getChildren()[i];
			if (node->getName() == layer) {
				node->setVisible(visible);
			}
		}
	}
}

Facing Object::getFacing() const {
  if (_facingLockValue != 0)
    return (Facing)_facingLockValue;
  else if (_facingMap.contains(_facing))
    return _facingMap[_facing];
  return _facing;
}

void Object::trig(const Common::String& name) {
  // debug fmt"Trigger object #{self.id} ({self.name}) sound '{name}'"
  int trigNum;
  sscanf(name.c_str(), "@%d", &trigNum);
  if (trigNum != 0) {
    if (_triggers.contains(trigNum)) {
      _triggers[trigNum]->trig();
	} else {
      warning("Trigger #%d not found in object #%i (%s)", trigNum, getId(), _name.c_str());
	}
  } else {
	error("todo: trig %s", name.c_str());
    // TODO: gEventMgr.trig(name.substr(1));
  }
}

Common::String Object::suffix() const {
  switch(getFacing()) {
  case FACE_BACK:
    return "_back";
  case FACE_FRONT:
    return "_front";
  case FACE_LEFT:
    // there is no animation with `left` suffix but use left and flip the sprite
    return "_right";
  case FACE_RIGHT:
    return "_right";
  }
}

Walkbox::Walkbox(const Common::Array<Math::Vector2d> &polygon, bool visible)
	: _polygon(polygon), _visible(visible) {
}

} // namespace Twp
