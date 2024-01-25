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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "twp/twp.h"
#include "twp/room.h"
#include "twp/ggpack.h"
#include "twp/squtil.h"
#include "twp/font.h"
#include "twp/scenegraph.h"
#include "twp/ids.h"
#include "twp/object.h"
#include "twp/util.h"
#include "twp/clipper/clipper.hpp"
#include "common/algorithm.h"

namespace Twp {

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
	Common::Array<Vector2i> points;
	size_t i = 1;
	size_t endPos;
	do {
		uint32 commaPos = text.find(',', i);
		int x = (int)strtol(text.substr(i, commaPos - i).c_str(), nullptr, 10);
		endPos = text.find('}', commaPos + 1);
		int y = (int)strtol(text.substr(commaPos + 1, endPos - commaPos - 1).c_str(), nullptr, 10);
		i = endPos + 3;
		points.push_back({x, y});
	} while ((text.size() - 1) != endPos);
	return Walkbox(points);
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

static ClipperLib::Path toPolygon(const Walkbox &walkbox) {
	ClipperLib::Path path;
	const Common::Array<Vector2i> &points = walkbox.getPoints();
	for (size_t i = 0; i < points.size(); i++) {
		path.push_back(ClipperLib::IntPoint(points[i].x, points[i].y));
	}
	return path;
}

static Walkbox toWalkbox(const ClipperLib::Path &path) {
	Common::Array<Vector2i> pts;
	for (size_t i = 0; i < path.size(); i++) {
		const ClipperLib::IntPoint &pt = path[i];
		pts.push_back(Vector2i{pt.X, pt.Y});
	}
	return Walkbox(pts, ClipperLib::Orientation(path));
}

static Common::Array<Walkbox> merge(const Common::Array<Walkbox> &walkboxes) {
	Common::Array<Walkbox> result;
	if (walkboxes.size() > 0) {
		ClipperLib::Paths subjects, clips;
		for (size_t i = 0; i < walkboxes.size(); i++) {
			const Walkbox &wb = walkboxes[i];
			if (wb.isVisible()) {
				subjects.push_back(toPolygon(wb));
			} else {
				clips.push_back(toPolygon(wb));
			}
		}

		ClipperLib::Paths solutions;
		ClipperLib::Clipper c;
		c.AddPaths(subjects, ClipperLib::ptSubject, true);
		c.Execute(ClipperLib::ClipType::ctUnion, solutions, ClipperLib::pftEvenOdd);

		ClipperLib::Paths solutions2;
		ClipperLib::Clipper c2;
		c2.AddPaths(solutions, ClipperLib::ptSubject, true);
		c2.AddPaths(clips, ClipperLib::ptClip, true);
        c2.Execute(ClipperLib::ClipType::ctDifference, solutions2, ClipperLib::pftEvenOdd);

		for (size_t i = 0; i < solutions2.size(); i++) {
			result.push_back(toWalkbox(solutions2[i]));
		}
	}
	return result;
}

Room::Room(const Common::String &name, HSQOBJECT &table) : _table(table) {
	setId(_table, newRoomId());
	_name = name;
	_scene = new Scene();
	_scene->addChild(&_overlayNode);
}

Room::~Room() {
	for (size_t i = 0; i < _layers.size(); i++) {
		delete _layers[i];
	}
	delete _scene;
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
	setId(obj->_table, newObjId());
	Common::String name = frames.size() > 0 ? frames[0] : "noname";
	sqsetf(obj->_table, "name", name);
	obj->_key = name;
	obj->_node->setName(name);
	debug("Create object with new table: %s #%d", obj->_name.c_str(), obj->getId());

	obj->_room = this;
	obj->_sheet = sheet;

	// create anim if any
	if (frames.size() > 0) {
		ObjectAnimation objAnim;
		objAnim.name = "state0";
		objAnim.frames.push_back(frames);
		obj->_anims.push_back(objAnim);
	}

	obj->_node->setZSort(1);
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
	setId(obj->_table, newObjId());
	debug("Create object with new table: %s #%d", obj->_name.c_str(), obj->getId());
	obj->_name = Common::String::format("text#%d: %s", obj->getId(), text.c_str());
	obj->_node->setName(obj->_key);

	Text txt(fontName, text, hAlign, vAlign, maxWidth);

	TextNode *node = new TextNode();
	node->setText(txt);
	float y = 0.5f;
	switch (vAlign) {
	case tvTop:
		y = 0.f;
		break;
	case tvCenter:
		y = 0.5f;
		break;
	case tvBottom:
		y = 1.f;
		break;
	}
	switch (hAlign) {
	case thLeft:
		node->setAnchorNorm(Math::Vector2d(0.f, y));
		break;
	case thCenter:
		node->setAnchorNorm(Math::Vector2d(0.5f, y));
		break;
	case thRight:
		node->setAnchorNorm(Math::Vector2d(1.f, y));
		break;
	}
	obj->_node = node;
	layer(0)->_objects.push_back(obj);
	layer(0)->_node->addChild(obj->_node);
	obj->_layer = layer(0);

	g_engine->_objects.push_back(obj);

	return obj;
}

void Room::load(Common::SeekableReadStream &s) {
	GGHashMapDecoder d;
	Common::JSONValue *value = d.open(&s);
	// debug("Room: %s", value->stringify().c_str());
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
		for (size_t i = 0; i < jBacks.size(); i++) {
			backNames.push_back(jBacks[i]->asString());
		}
	}

	{
		Layer *layer = new Layer(backNames, Math::Vector2d(1, 1), 0);
		_layers.push_back(layer);
	}

	// layers
	if (jRoom.contains("layers")) {
		const Common::JSONArray &jLayers = jRoom["layers"]->asArray();
		for (size_t i = 0; i < jLayers.size(); i++) {
			Common::StringArray names;
			const Common::JSONObject &jLayer = jLayers[i]->asObject();
			if (jLayer["name"]->isArray()) {
				const Common::JSONArray &jNames = jLayer["name"]->asArray();
				for (size_t j = 0; j < jNames.size(); j++) {
					names.push_back(jNames[j]->asString());
				}
			} else if (jLayer["name"]->isString()) {
				names.push_back(jLayer["name"]->asString());
			}
			Math::Vector2d parallax = parseParallax(*jLayer["parallax"]);
			int zsort = jLayer["zsort"]->asIntegerNumber();

			Layer *layer = new Layer(names, parallax, zsort);
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
			Object *obj = new Object();
			Twp::setId(obj->_table, newObjId());
			obj->_key = jObject["name"]->asString();
			Node *objNode = new Node(obj->_key);
			objNode->setPos(Math::Vector2d(parseVec2(jObject["pos"]->asString())));
			objNode->setZSort(jObject["zsort"]->asIntegerNumber());
			obj->_node = objNode;
			obj->_nodeAnim = new Anim(obj);
			obj->_node->addChild(obj->_nodeAnim);
			obj->_usePos = parseVec2(jObject["usepos"]->asString());
			if (jObject.contains("usedir")) {
				obj->_useDir = parseUseDir(jObject["usedir"]->asString());
			} else {
				obj->_useDir = dNone;
			}
			obj->_hotspot = parseRect(jObject["hotspot"]->asString());
			obj->_objType = toObjectType(jObject);
			if (jObject.contains("parent"))
				obj->_parent = jObject["parent"]->asString();
			obj->_room = this;
			if (jObject.contains("animations")) {
				parseObjectAnimations(jObject["animations"]->asArray(), obj->_anims);
			}
			obj->_layer = layer(0);
			layer(0)->_objects.push_back(obj);
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

	_mergedPolygon = merge(_walkboxes);

	// Fix room size (why ?)
	int width = 0;
	for (size_t i = 0; i < backNames.size(); i++) {
		Common::String name = backNames[i];
		width += g_engine->_resManager.spriteSheet(_sheet)->frameTable[name].sourceSize.getX();
	}
	_roomSize.setX(width);

	delete value;
}

Layer *Room::layer(int zsort) {
	for (size_t i = 0; i < _layers.size(); i++) {
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

Object *Room::getObj(const Common::String &key) {
	for (size_t i = 0; i < _layers.size(); i++) {
		Layer *layer = _layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Object *obj = layer->_objects[j];
			if (obj->_key == key)
				return obj;
		}
	}
	return nullptr;
}

Light *Room::createLight(Color color, Math::Vector2d pos) {
	Light *result = &_lights._lights[_lights._numLights];
	result->id = 100000 + _lights._numLights;
	result->on = true;
	result->color = color;
	result->pos = pos;
	_lights._numLights++;
	return result;
}

float Room::getScaling(float yPos) {
	return _scaling.getScaling(yPos);
}

void Room::objectParallaxLayer(Object *obj, int zsort) {
	Layer *l = layer(zsort);
	if (obj->_layer != l) {
		// removes object from old layer
		if (obj->_layer) {
			int i = find(obj->_layer->_objects, obj);
			obj->_layer->_node->removeChild(obj->_node);
			obj->_layer->_objects.remove_at(i);
		}
		// adds object to the new one
		l->_objects.push_back(obj);
		// update scenegraph
		l->_node->addChild(obj->_node);
		obj->_layer = l;
	}
}

void Room::setOverlay(Color color) {
	_overlayNode.setOverlayColor(color);
}

Color Room::getOverlay() const {
	return _overlayNode.getOverlayColor();
}

void Room::update(float elapsed) {
	if (_overlayTo)
		_overlayTo->update(elapsed);
	if (_rotateTo)
		_rotateTo->update(elapsed);
	for (size_t j = 0; j < _layers.size(); j++) {
		Layer *layer = _layers[j];
		for (size_t k = 0; k < layer->_objects.size(); k++) {
			Object *obj = layer->_objects[k];
			obj->update(elapsed);
		}
	}
}

void Room::walkboxHidden(const Common::String &name, bool hidden) {
	for (size_t i = 0; i < _walkboxes.size(); i++) {
		Walkbox &wb = _walkboxes[i];
		if (wb._name == name) {
			wb.setVisible(!hidden);
			// 1 walkbox has change so update merged polygon
			_pathFinder.setDirty(true);
			return;
		}
	}
}

Common::Array<Vector2i> Room::calculatePath(Vector2i frm, Vector2i to) {
	if (_mergedPolygon.size() > 0) {
		if (_pathFinder.isDirty()) {
			_mergedPolygon = merge(_walkboxes);
			_pathFinder.setWalkboxes(_mergedPolygon);
			_pathFinder.setDirty(false);
		}
		return _pathFinder.calculatePath(frm, to);
	}
	return {};
}

Layer::Layer(const Common::String &name, Math::Vector2d parallax, int zsort) {
	_names.push_back(name);
	_parallax = parallax;
	_zsort = zsort;
}

Layer::Layer(const Common::StringArray &name, Math::Vector2d parallax, int zsort) {
	_names.push_back(name);
	_parallax = parallax;
	_zsort = zsort;
}

Walkbox::Walkbox(const Common::Array<Vector2i> &polygon, bool visible)
	: _polygon(polygon), _visible(visible) {
}

bool Walkbox::concave(int vertex) const {
	Vector2i current = _polygon[vertex];
	Vector2i next = _polygon[(vertex + 1) % _polygon.size()];
	Vector2i previous = _polygon[vertex == 0 ? _polygon.size() - 1 : vertex - 1];

	Vector2i left{current.x - previous.x, current.y - previous.y};
	Vector2i right{next.x - current.x, next.y - current.y};

	float cross = (left.x * right.y) - (left.y * right.x);
	return cross < 0;
}

bool Walkbox::contains(Vector2i position, bool toleranceOnOutside) const {
	Vector2i point = position;
	const float epsilon = 2.0f;
	bool result = false;

	// Must have 3 or more edges
	if (_polygon.size() < 3)
		return false;

	Vector2i oldPoint(_polygon[_polygon.size() - 1]);
	float oldSqDist = distanceSquared(oldPoint, point);

	for (size_t i = 0; i < _polygon.size(); i++) {
		Vector2i newPoint = _polygon[i];
		float newSqDist = distanceSquared(newPoint, point);

		if (oldSqDist + newSqDist + 2.0f * sqrt(oldSqDist * newSqDist) - distanceSquared(newPoint, oldPoint) < epsilon)
			return toleranceOnOutside;

		Vector2i left;
		Vector2i right;
		if (newPoint.x > oldPoint.x) {
			left = oldPoint;
			right = newPoint;
		} else {
			left = newPoint;
			right = oldPoint;
		}

		if ((left.x < point.x) && (point.x <= right.x) && ((point.y - left.y) * (right.x - left.x)) < ((right.y - left.y) * (point.x - left.x)))
			result = !result;

		oldPoint = newPoint;
		oldSqDist = newSqDist;
	}
	return result;
}

float Scaling::getScaling(float yPos) {
	if (values.size() == 0)
		return 1.0f;
	for (size_t i = 0; i < values.size(); i++) {
		ScalingValue scaling = values[i];
		if (yPos < scaling.y) {
			if (i == 0)
				return values[i].scale;
			ScalingValue prevScaling = values[i - 1];
			float dY = scaling.y - prevScaling.y;
			float dScale = scaling.scale - prevScaling.scale;
			float p = (yPos - prevScaling.y) / dY;
			float scale = prevScaling.scale + (p * dScale);
			return scale;
		}
	}
	return values[values.size() - 1].scale;
}

} // namespace Twp
