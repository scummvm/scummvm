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
#include "twp/detection.h"
#include "twp/resmanager.h"
#include "twp/object.h"
#include "twp/room.h"
#include "twp/squtil.h"
#include "twp/clipper/clipper.hpp"

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
		pts.push_back(Vector2i(static_cast<int>(pt.X), static_cast<int>(pt.Y)));
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
	setId(_table, g_twp->_resManager->newRoomId());
	_name = name;
	_scene = Common::SharedPtr<Scene>(new Scene());
	_scene->addChild(&_overlayNode);
}

Room::~Room() {
}

Common::SharedPtr<Object> Room::createObject(const Common::String &sheet, const Common::Array<Common::String> &frames) {
	Common::SharedPtr<Object> obj(new Object());
	obj->_temporary = true;

	HSQUIRRELVM v = g_twp->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->_table);
	sq_addref(v, &obj->_table);
	sq_pop(v, 1);

	// assign an id
	const int id = g_twp->_resManager->newObjId();
	setId(obj->_table, id);
	g_twp->_resManager->_allObjects[id] = obj;
	Common::String name = frames.size() > 0 ? frames[0] : "noname";
	sqsetf(obj->_table, "name", name);
	obj->_key = name;
	obj->_node->setName(name);
	debugC(kDebugGame, "Create object with new table: %s #%d", obj->_name.c_str(), obj->getId());

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
	layer(0)->_node->addChild(obj->_node.get());
	obj->_layer = layer(0);
	obj->setState(0);

	return obj;
}

Common::SharedPtr<Object> Room::createTextObject(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign, TextVAlignment vAlign, float maxWidth) {
	Common::SharedPtr<Object> obj(new Object());
	obj->_temporary = true;

	HSQUIRRELVM v = g_twp->getVm();

	// create a table for this object
	sq_newtable(v);
	sq_getstackobj(v, -1, &obj->_table);
	sq_addref(v, &obj->_table);
	sq_pop(v, 1);

	// assign an id
	const int id = g_twp->_resManager->newObjId();
	setId(obj->_table, id);
	g_twp->_resManager->_allObjects[id] = obj;
	debugC(kDebugGame, "Create object with new table: %s #%d", obj->_name.c_str(), obj->getId());
	obj->_name = Common::String::format("text#%d: %s", obj->getId(), text.c_str());

	Text txt(fontName, text, hAlign, vAlign, maxWidth);

	Common::SharedPtr<TextNode> node(new TextNode());
	node->setName(obj->_name);
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
	obj->_nodeAnim = nullptr;
	obj->_node = node;
	layer(0)->_objects.push_back(obj);
	layer(0)->_node->addChild(obj->_node.get());
	obj->_layer = layer(0);

	return obj;
}

void Room::load(Common::SharedPtr<Room> room, Common::SeekableReadStream &s) {
	GGHashMapDecoder d;
	Common::ScopedPtr<Common::JSONValue> value(d.open(&s));
	// debugC(kDebugGame, "Room: %s", value->stringify().c_str());
	const Common::JSONObject &jRoom = value->asObject();

	room->_name = jRoom["name"]->asString();
	room->_sheet = jRoom["sheet"]->asString();

	room->_roomSize = parseVec2(jRoom["roomsize"]->asString());
	room->_height = jRoom.contains("height") ? jRoom["height"]->asIntegerNumber() : room->_roomSize.getY();
	room->_fullscreen = jRoom.contains("fullscreen") ? jRoom["fullscreen"]->asIntegerNumber() : 0;

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
		Common::SharedPtr<Layer> layer(new Layer(backNames, Math::Vector2d(1, 1), 0));
		room->_layers.push_back(layer);
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

			Common::SharedPtr<Layer> layer(new Layer(names, parallax, zsort));
			room->_layers.push_back(layer);
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
			room->_walkboxes.push_back(walkbox);
		}
	}

	// objects
	if (jRoom.contains("objects")) {
		const Common::JSONArray &jobjects = jRoom["objects"]->asArray();
		for (auto it = jobjects.begin(); it != jobjects.end(); it++) {
			const Common::JSONObject &jObject = (*it)->asObject();
			Common::SharedPtr<Object> obj(new Object());
			const int id = g_twp->_resManager->newObjId();
			Twp::setId(obj->_table, id);
			g_twp->_resManager->_allObjects[id] = obj;
			obj->_key = jObject["name"]->asString();
			obj->_node->setName(obj->_key.c_str());
			obj->_node->setPos(Math::Vector2d(parseVec2(jObject["pos"]->asString())));
			obj->_node->setZSort(jObject["zsort"]->asIntegerNumber());
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
			obj->_room = room;
			if (jObject.contains("animations")) {
				parseObjectAnimations(jObject["animations"]->asArray(), obj->_anims);
			}
			obj->_layer = room->layer(0);
			room->layer(0)->_objects.push_back(obj);
		}
	}

	// scalings
	if (jRoom.contains("scaling")) {
		const Common::JSONArray &jScalings = jRoom["scaling"]->asArray();
		if (jScalings[0]->isString()) {
			room->_scalings.push_back(parseScaling(jScalings));
		} else {
			for (auto it = jScalings.begin(); it != jScalings.end(); it++) {
				const Common::JSONObject &jScaling = (*it)->asObject();
				Scaling scaling = parseScaling(jScaling["scaling"]->asArray());
				if (jScaling.contains("trigger") && jScaling["trigger"]->isString())
					scaling.trigger = jScaling["trigger"]->asString();
				room->_scalings.push_back(scaling);
			}
		}
		room->_scaling = room->_scalings[0];
	}

	room->_mergedPolygon = merge(room->_walkboxes);

	// Fix room size (why ?)
	int width = 0;
	for (size_t i = 0; i < backNames.size(); i++) {
		Common::String name = backNames[i];
		width += g_twp->_resManager->spriteSheet(room->_sheet)->getFrame(name).sourceSize.getX();
	}
	room->_roomSize.setX(width);
}

Common::SharedPtr<Layer> Room::layer(int zsort) {
	for (size_t i = 0; i < _layers.size(); i++) {
		Common::SharedPtr<Layer> l = _layers[i];
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

Common::SharedPtr<Object> Room::getObj(const Common::String &key) {
	for (size_t i = 0; i < _layers.size(); i++) {
		Common::SharedPtr<Layer> layer = _layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj = layer->_objects[j];
			if (obj->_key == key)
				return obj;
		}
	}
	return nullptr;
}

Light *Room::createLight(const Color &color, const Math::Vector2d &pos) {
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

void Room::objectParallaxLayer(Common::SharedPtr<Object> obj, int zsort) {
	Common::SharedPtr<Layer> l = layer(zsort);
	if (obj->_layer != l) {
		// removes object from old layer
		if (obj->_layer) {
			int i = find(obj->_layer->_objects, obj);
			obj->_layer->_node->removeChild(obj->_node.get());
			obj->_layer->_objects.remove_at(i);
		}
		// adds object to the new one
		l->_objects.push_back(obj);
		// update scenegraph
		l->_node->addChild(obj->_node.get());
		obj->_layer = l;
	}
}

void Room::setOverlay(const Color &color) {
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
		Common::SharedPtr<Layer> layer = _layers[j];
		for (size_t k = 0; k < layer->_objects.size(); k++) {
			Common::SharedPtr<Object> obj = layer->_objects[k];
			obj->update(elapsed);
		}
	}
}

void Room::walkboxHidden(const Common::String &name, bool hidden) {
	for (size_t i = 0; i < _walkboxes.size(); i++) {
		Walkbox &wb = _walkboxes[i];
		if (wb._name == name) {
			wb.setVisible(!hidden);
			// 1 walkbox has changed so update merged polygon
			_pathFinder.setDirty(true);
			return;
		}
	}
}

Common::Array<Math::Vector2d> Room::calculatePath(const Math::Vector2d &frm, const Math::Vector2d &to) {
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

Layer::Layer(const Common::String &name, const Math::Vector2d &parallax, int zsort) {
	_names.push_back(name);
	_parallax = parallax;
	_zsort = zsort;
}

Layer::Layer(const Common::StringArray &name, const Math::Vector2d &parallax, int zsort) {
	_names.push_back(name);
	_parallax = parallax;
	_zsort = zsort;
}

Walkbox::Walkbox(const Common::Array<Vector2i> &polygon, bool visible)
	: _polygon(polygon), _visible(visible) {
}

bool Walkbox::concave(int vertex) const {
	Math::Vector2d current = (Math::Vector2d)_polygon[vertex];
	Math::Vector2d next = (Math::Vector2d)_polygon[(vertex + 1) % _polygon.size()];
	Math::Vector2d previous = (Math::Vector2d)_polygon[vertex == 0 ? _polygon.size() - 1 : vertex - 1];

	Math::Vector2d left{current.getX() - previous.getX(), current.getY() - previous.getY()};
	Math::Vector2d right{next.getX() - current.getX(), next.getY() - current.getY()};

	float cross = (left.getX() * right.getY()) - (left.getY() * right.getX());
	return cross < 0;
}

bool Walkbox::contains(const Math::Vector2d &position, bool toleranceOnOutside) const {
	Math::Vector2d point = position;
	const float epsilon = 2.0f;
	bool result = false;

	// Must have 3 or more edges
	if (_polygon.size() < 3)
		return false;

	Math::Vector2d oldPoint(_polygon[_polygon.size() - 1]);
	float oldSqDist = distanceSquared(oldPoint, point);

	for (size_t i = 0; i < _polygon.size(); i++) {
		Math::Vector2d newPoint = (Math::Vector2d)_polygon[i];
		float newSqDist = distanceSquared(newPoint, point);

		if (oldSqDist + newSqDist + 2.0f * sqrt(oldSqDist * newSqDist) - distanceSquared(newPoint, oldPoint) < epsilon)
			return toleranceOnOutside;

		Math::Vector2d left;
		Math::Vector2d right;
		if (newPoint.getX() > oldPoint.getX()) {
			left = oldPoint;
			right = newPoint;
		} else {
			left = newPoint;
			right = oldPoint;
		}

		if ((left.getX() < point.getX()) && (point.getX() <= right.getX()) && ((point.getY() - left.getY()) * (right.getX() - left.getX())) < ((right.getY() - left.getY()) * (point.getX() - left.getX())))
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
