#include "twp/twp.h"
#include "twp/room.h"
#include "twp/ggpack.h"
#include "twp/squtil.h"
#include "twp/font.h"

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
		Layer layer;
		layer.names.push_back(backNames);
		layer.zsort = 0;
		layer.parallax = Math::Vector2d(1, 1);
		_layers.push_back(layer);
	}

	// layers
	if (jRoom.contains("layers")) {
		const Common::JSONArray &jLayers = jRoom["layers"]->asArray();
		for (int i = 0; i < jLayers.size(); i++) {
			Layer layer;
			const Common::JSONObject &jLayer = jLayers[i]->asObject();
			if (jLayer["name"]->isArray()) {
				const Common::JSONArray &jNames = jLayer["name"]->asArray();
				for (int j = 0; j < jNames.size(); j++) {
					layer.names.push_back(jNames[j]->asString());
				}
			} else if (jLayer["name"]->isString()) {
				layer.names.push_back(jLayer["name"]->asString());
			}
			layer.parallax = parseParallax(*jLayer["parallax"]);
			layer.zsort = jLayer["zsort"]->asIntegerNumber();
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
			obj.state = -1;
			//   Node objNode(obj.key)
			// objNode.pos = Math::Vector2d(parseVec2(jObject["pos"]->asString()));
			//   objNode.zOrder = jObject["zsort"].getInt().int32
			//   obj.node = objNode
			//   obj.nodeAnim = newAnim(obj)
			//   obj.node.addChild obj.nodeAnim
			obj.key = jObject["name"]->asString();
			obj.usePos = parseVec2(jObject["usepos"]->asString());
			if (jObject.contains("usedir")) {
				obj.useDir = parseUseDir(jObject["usedir"]->asString());
			} else {
				obj.useDir = dNone;
			}
			obj.hotspot = parseRect(jObject["hotspot"]->asString());
			obj.objType = toObjectType(jObject);
			if (jObject.contains("parent"))
				jObject["parent"]->asString();
			obj.room = this;
			if (jObject.contains("animations")) {
				parseObjectAnimations(jObject["animations"]->asArray(), obj.anims);
			}
			//   obj.layer = result.layer(0);
			//   result.layer(0).objects.add(obj);
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

static int gId = 3000;

Object *Room::createObject(const Common::String &sheet, const Common::Array<Common::String> &frames) {
	Object *obj = new Object();
	obj->temporary = true;

	HSQUIRRELVM v = g_engine->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->table);
	sq_addref(v, &obj->table);
	sq_pop(v, 1);

	// assign an id
	setId(obj->table, gId++);
	Common::String name = frames.size() > 0 ? frames[0] : "noname";
	setf(obj->table, "name", name);
	obj->key = name;
	debug("Create object with new table: %s #%d", obj->name.c_str(), obj->getId());

	obj->room = this;
	obj->sheet = sheet;
	obj->touchable = false;

	// create anim if any
	if (frames.size() > 0) {
		ObjectAnimation objAnim;
		objAnim.name = "state0";
		objAnim.frames.push_back(frames);
		obj->anims.push_back(objAnim);
	}

	// TODO: adds object to the scenegraph
	// obj->node.zOrder = 1
	//   layer(0).objects.add(obj)
	//   layer(0).node.addChild obj.node
	//   obj->layer = self.layer(0)
	// TODO: obj->setState(0);

	g_engine->_objects.push_back(obj);

	return obj;
}

Object *Room::createTextObject(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign, TextVAlignment vAlign, float maxWidth) {
	Object *obj = new Object();
	obj->temporary = true;

	HSQUIRRELVM v = g_engine->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->table);
	sq_addref(v, &obj->table);
	sq_pop(v, 1);

	// assign an id
	setId(obj->table, gId++);
	debug("Create object with new table: %s #%d}", obj->name.c_str(), obj->getId());
	obj->name = Common::String::format("text#%d: %s", obj->getId(), text.c_str());
	obj->touchable = false;

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
	getf(table, "_id", result);
	return (int)result;
}

Walkbox::Walkbox(const Common::Array<Math::Vector2d> &polygon, bool visible)
	: _polygon(polygon), _visible(visible) {
}

} // namespace Twp
