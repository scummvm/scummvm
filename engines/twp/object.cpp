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
#include "twp/ids.h"
#include "twp/object.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"

namespace Twp {

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

bool Object::playCore(const Common::String &state, bool loop, bool instant) {
	for (int i = 0; i < _anims.size(); i++) {
		ObjectAnimation &anim = _anims[i];
		if (anim.name == state) {
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
	Common::String *s = Common::find(_hiddenLayers.begin(), _hiddenLayers.end(), layer);
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

void Object::trig(const Common::String &name) {
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
	switch (getFacing()) {
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

} // namespace Twp
