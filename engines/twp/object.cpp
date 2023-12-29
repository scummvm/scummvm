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
#include "twp/util.h"
#include "twp/ggpack.h"
#include "twp/motor.h"

#define MIN_TALK_DIST 60
#define MIN_USE_DIST 15

namespace Twp {

Object::Object()
	: _talkOffset(0, 90) {
	_node = new Node("newObj");
	_nodeAnim = new Anim(this);
	_node->addChild(_nodeAnim);
	sq_resetobject(&_table);
}

Object::Object(HSQOBJECT o, const Common::String &key)
	: _talkOffset(0, 90), _table(o), _key(key) {
}

Object::~Object() {
	_layer->_objects.erase(Common::find(_layer->_objects.begin(), _layer->_objects.end(), this));
	_node->getParent()->removeChild(_node);
}

Object *Object::createActor() {
	Object *result = new Object();
	result->_hotspot = Common::Rect(-18, 0, 37, 71);
	result->_facing = FACE_FRONT;
	result->_useWalkboxes = true;
	result->showLayer("blink", false);
	result->showLayer("eyes_left", false);
	result->showLayer("eyes_right", false);
	result->setHeadIndex(1);
	return result;
}

int Object::getId() const {
	SQInteger result = 0;
	sqgetf(_table, "_id", result);
	return (int)result;
}

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
	int index = -1;
	for (int i = 0; i < _hiddenLayers.size(); i++) {
		if (_hiddenLayers[i] == layer) {
			index = i;
			break;
		}
	}

	if (visible) {
		if (index != -1)
			_hiddenLayers.remove_at(index);
	} else {
		if (index == -1)
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
	for (int i = 0; i < _facingMap.size(); i++) {
		if (_facingMap[i].key == _facing)
			return _facingMap[i].value;
	}
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
			// warning("Trigger #%d not found in object #%i (%s)", trigNum, getId(), _name.c_str());
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

void Object::setPop(int count) {
	_popCount = count;
	_popElapsed = 0.f;
}

float Object::popScale() const {
	return 0.5f + 0.5f * sin(-M_PI / 2.f + _popElapsed * 4.f * M_PI);
}

int Object::defaultVerbId() {
	int result = VERB_LOOKAT;
	if (sqrawexists(_table, "defaultVerb"))
		sqgetf(_table, "defaultVerb", result);
	else if (isActor(getId())) {
		result = sqrawexists(_table, "verbTalkTo") ? VERB_TALKTO : VERB_WALKTO;
	}
	return result;
}

Math::Vector2d Object::getUsePos() {
	return isActor(getId()) ? _node->getPos() + _node->getOffset() : _node->getPos() + _node->getOffset() + _usePos;
}

bool Object::touchable() {
	if (_objType == otNone) {
		if (_state == GONE) {
			return false;
		} else if (_node && !_node->isVisible()) {
			return false;
		} else if (sqrawexists(_table, "_touchable")) {
			bool result;
			sqgetf(_table, "_touchable", result);
			return result;
		} else if (sqrawexists(_table, "initTouchable")) {
			bool result;
			sqgetf(_table, "initTouchable", result);
			return result;
		} else {
			return true;
		}
	}
	return false;
}

void Object::setTouchable(bool value) {
	if (sqrawexists(_table, "_touchable"))
		sqsetf(_table, "_touchable", value);
	else
		sqnewf(_table, "_touchable", value);
}

void Object::setIcon(int fps, const Common::StringArray &icons) {
	_icons = icons;
	_iconFps = fps;
	_iconIndex = 0;
	_iconElapsed = 0.f;
}

void Object::setIcon(const Common::String &icon) {
	Common::StringArray icons;
	icons.push_back(icon);
	setIcon(0, icons);
	sqsetf(_table, "icon", icon);
}

Common::String Object::getIcon() {
	if (_icons.size() > 0)
		return _icons[_iconIndex];
	HSQOBJECT iconTable;
	sqgetf(_table, "icon", iconTable);
	if (iconTable._type == OT_NULL) {
		warning("object table is null");
		return "";
	}
	if (iconTable._type == OT_STRING) {
		Common::String result = sq_objtostring(&iconTable);
		setIcon(result);
		return result;
	}
	if (iconTable._type == OT_ARRAY) {
		int i = 0;
		int fps = 0;
		Common::StringArray icons;
		sqgetitems(iconTable, [&](HSQOBJECT &item) {
			if (i == 0) {
				fps = sq_objtointeger(&item);
			} else {
				Common::String icon = sq_objtostring(&item);
				icons.push_back(icon);
			}
			i++;
		});
		setIcon(fps, icons);
		return getIcon();
	}
	return "";
}

int Object::getFlags() {
	int result = 0;
	if (sqrawexists(_table, "flags"))
		sqgetf(_table, "flags", result);
	return result;
}

void Object::setRoom(Room *room) {
	if (_room != room) {
		stopObjectMotors();
		Room *oldRoom = _room;
		if (oldRoom) {
			debug("Remove %s from room %s", _key.c_str(), oldRoom->_name.c_str());
			Layer *layer = oldRoom->layer(0);
			if (layer) {
				int index = find(layer->_objects, this);
				if (index != -1)
					layer->_objects.remove_at(index);
				if (layer)
					layer->_node->removeChild(_node);
			}
		}
		if (room && room->layer(0) && room->layer(0)->_node) {
			debug("Add %s in room %s", _key.c_str(), room->_name.c_str());
			Layer *layer = room->layer(0);
			if (layer) {
				layer->_objects.push_back(this);
				layer->_node->addChild(_node);
			}
		}
		_room = room;
	}
}

static void disableMotor(Motor *motor) {
	if (motor)
		motor->disable();
}

void Object::stopObjectMotors() {
	disableMotor(_alphaTo);
	disableMotor(_rotateTo);
	disableMotor(_moveTo);
	disableMotor(_walkTo);
	disableMotor(_talking);
	disableMotor(_blink);
	disableMotor(_turnTo);
	disableMotor(_shakeTo);
	disableMotor(_jiggleTo);
}

void Object::setFacing(Facing facing) {
	if (_facing != facing) {
		debug("set facing: %d", facing);
		bool update = !(((_facing == FACE_LEFT) && (facing == FACE_RIGHT)) || ((_facing == FACE_RIGHT) && (facing == FACE_LEFT)));
		_facing = facing;
		if (update && _nodeAnim)
			play(_animName, _animLoop);
	}
}

Facing Object::getDoorFacing() {
	int flags = getFlags();
	if (flags & DOOR_LEFT)
		return FACE_LEFT;
	else if (flags & DOOR_RIGHT)
		return FACE_RIGHT;
	else if (flags & DOOR_FRONT)
		return FACE_FRONT;
	else
		return FACE_BACK;
}

bool Object::inInventory() {
	return isObject(getId()) && getIcon().size() > 0;
}

bool Object::contains(Math::Vector2d pos) {
	Math::Vector2d p = pos - _node->getPos() - _node->getOffset();
	return _hotspot.contains(p.getX(), p.getY());
}

void Object::dependentOn(Object *dependentObj, int state) {
	_dependentState = state;
	_dependentObj = dependentObj;
}

Common::String Object::getAnimName(const Common::String &key) {
	if (_animNames.contains(key))
		return _animNames[key];
	return key;
}

void Object::setHeadIndex(int head) {
	for (int i = 0; i <= 6; i++) {
		showLayer(Common::String::format("%s%d", getAnimName(STAND_ANIMNAME).c_str(), i), i == head);
	}
}

bool Object::isWalking() {
	return _walkTo && _walkTo->isEnabled();
}

void Object::stopWalking() {
	if (_walkTo)
		_walkTo->disable();
}

void Object::setAnimationNames(const Common::String &head, const Common::String &stand, const Common::String &walk, const Common::String &reach) {
	if (head.size() > 0)
		setHeadIndex(0);
	_animNames[HEAD_ANIMNAME] = head;
	showLayer(_animNames[HEAD_ANIMNAME], true);
	setHeadIndex(1);
	if (stand.size() > 0)
		_animNames[STAND_ANIMNAME] = stand;
	if (walk.size() > 0)
		_animNames[WALK_ANIMNAME] = walk;
	if (reach.size() > 0)
		_animNames[REACH_ANIMNAME] = reach;
	if (isWalking())
		play(getAnimName(WALK_ANIMNAME), true);
}

void Object::blinkRate(float min, float max) {
	// TODO:
	//   if (min == 0.0 && max == 0.0)
	//     _blink = nil;
	//   else:
	//     _blink = new Blink(this, min, max);
}

void Object::setCostume(const Common::String &name, const Common::String &sheet) {
	GGPackEntryReader entry;
	entry.open(g_engine->_pack, name + ".json");

	GGHashMapDecoder dec;
	Common::JSONValue *json = dec.open(&entry);
	const Common::JSONObject &jCostume = json->asObject();

	parseObjectAnimations(jCostume["animations"]->asArray(), _anims);
	_costumeName = name;
	_costumeSheet = sheet;
	if ((sheet.size() == 0) && jCostume.contains("sheet")) {
		_sheet = jCostume["sheet"]->asString();
	} else {
		_sheet = sheet;
	}
	stand();

	delete json;
}

void Object::stand() {
	play(getAnimName(STAND_ANIMNAME));
}

#define SET_MOTOR(motorTo)     \
	if (_##motorTo) {          \
		_##motorTo->disable(); \
		delete _##motorTo;     \
	}                          \
	_##motorTo = motorTo;

void Object::setAlphaTo(Motor *alphaTo) { SET_MOTOR(alphaTo); }
void Object::setRotateTo(Motor *rotateTo) { SET_MOTOR(rotateTo); }
void Object::setMoveTo(Motor *moveTo) { SET_MOTOR(moveTo); }
void Object::setWalkTo(Motor *walkTo) { SET_MOTOR(walkTo); }
void Object::setTalking(Motor *talking) { SET_MOTOR(talking); }
void Object::setBlink(Motor *blink) { SET_MOTOR(blink); }
void Object::setTurnTo(Motor *turnTo) { SET_MOTOR(turnTo); }
void Object::setShakeTo(Motor *shakeTo) { SET_MOTOR(shakeTo); }
void Object::setJiggleTo(Motor *jiggleTo) { SET_MOTOR(jiggleTo); }

void Object::update(float elapsedSec) {
	if (_dependentObj)
		_node->setVisible(_dependentObj->getState() == _dependentState);
	if (_alphaTo)
		_alphaTo->update(elapsedSec);
	if (_rotateTo)
		_rotateTo->update(elapsedSec);
	if (_moveTo)
		_moveTo->update(elapsedSec);
	if (_walkTo)
		_walkTo->update(elapsedSec);
	if (_talking)
		_talking->update(elapsedSec);
	if (_blink)
		_blink->update(elapsedSec);
	if (_turnTo)
		_turnTo->update(elapsedSec);
	if (_shakeTo)
		_shakeTo->update(elapsedSec);
	if (_jiggleTo)
		_jiggleTo->update(elapsedSec);

	if (_nodeAnim)
		_nodeAnim->update(elapsedSec);

	if ((_icons.size() > 1) && (_iconFps > 0)) {
		_iconElapsed += elapsedSec;
		if (_iconElapsed > (1.f / _iconFps)) {
			_iconElapsed = 0.f;
			_iconIndex = (_iconIndex + 1) % _icons.size();
		}
	}

	if (_popCount > 0) {
		_popElapsed += elapsedSec;
		if (_popElapsed > 0.5f) {
			_popCount--;
			_popElapsed -= 0.5f;
		}
	}
}

void Object::pickupObject(Object *obj) {
	obj->_owner = this;
	_inventory.push_back(obj);

	sqcall("onPickup", obj->_table, _table);

	if (sqrawexists(obj->_table, "onPickUp")) {
		sqcall(obj->_table, "onPickUp", _table);
	}
}

void Object::stopTalking() {
	if (_talking) {
		_talking->disable();
		setHeadIndex(1);
	}
}

void Object::say(const Common::StringArray &texts, Color color) {
	_talkingState._obj = this;
	_talkingState._color = color;
	_talkingState.say(texts, this);
}

void Object::resetLockFacing() {
	_facingMap.clear();
}

void Object::lockFacing(int facing) {
	_facingLockValue = facing;
}

void Object::lockFacing(Facing left, Facing right, Facing front, Facing back) {
	_facingMap.push_back({FACE_LEFT, left});
	_facingMap.push_back({FACE_RIGHT, right});
	_facingMap.push_back({FACE_FRONT, front});
	_facingMap.push_back({FACE_BACK, back});
}

int Object::flags() {
	int result = 0;
	if (sqrawexists(_table, "flags"))
		sqgetf(_table, "flags", result);
	return result;
}

UseFlag Object::useFlag() {
	int flags = getFlags();
	if (flags & USE_WITH)
		return ufUseWith;
	if (flags & USE_ON)
		return ufUseOn;
	if (flags & USE_IN)
		return ufUseIn;
	return ufNone;
}

float Object::getScale() {
	if (getPop() > 0)
		return 4.25f + popScale() * 0.25f;
	return 4.f;
}

void Object::removeInventory(Object *obj) {
	int i = find(_inventory, obj);
	if (i >= 0) {
		_inventory.remove_at(i);
		obj->_owner = nullptr;
	}
}

void Object::removeInventory() {
	if (_owner)
		_owner->removeInventory(this);
}

Common::String Object::getReachAnim() {
	int flags = getFlags();
	if (flags & REACH_LOW)
		return "_low";
	if (flags & REACH_HIGH)
		return "_high";
	return "_med";
}

// true of you don't have to be close to the object
static bool verbNotClose(VerbId id) {
	return id.id == VERB_LOOKAT;
}

static void cantReach(Object *self, Object *noun2) {
	// TODO: check if we need to use sqrawexists or sqexists
	if (sqrawexists(self->_table, "verbCantReach")) {
		int nParams = sqparamCount(g_engine->getVm(), self->_table, "verbCantReach");
		debug("verbCantReach found in obj '{self.key}' with {nParams} params");
		if (nParams == 1) {
			sqcall(self->_table, "verbCantReach");
		} else {
			HSQOBJECT table;
			sq_resetobject(&table);
			if (noun2)
				table = noun2->_table;
			sqcall(self->_table, "verbCantReach", self->_table, table);
		}
	} else if (!noun2) {
		cantReach(noun2, nullptr);
	} else {
		HSQOBJECT nilTbl;
		sqcall(g_engine->_defaultObj, "verbCantReach", self->_table, !noun2 ? nilTbl : noun2->_table);
	}
}

void Object::execVerb() {
	if (_exec.enabled) {
		VerbId verb = _exec.verb;
		Object *noun1 = _exec.noun1;
		Object *noun2 = _exec.noun2;

		debug("actorArrived: exec sentence");
		if (!noun1->inInventory()) {
			// Object became untouchable as we were walking there
			if (!noun1->_touchable) {
				debug("actorArrived: noun1 untouchable");
				_exec.enabled = false;
				return;
			}
			// Did we get close enough?
			float dist = distance(getUsePos(), noun1->getUsePos());
			float min_dist = verb.id == VERB_TALKTO ? MIN_TALK_DIST : MIN_USE_DIST;
			debug("actorArrived: noun1 min_dist: {dist} > {min_dist} (actor: {self.getUsePos}, obj: {noun1.getUsePos}) ?");
			if (!verbNotClose(verb) && (dist > min_dist)) {
				cantReach(this, noun1);
				return;
			}
			if (noun1->_useDir != dNone) {
				setFacing((Facing)noun1->_useDir);
			}
		}
		if (noun2 && !noun2->inInventory()) {
			if (!noun2->_touchable) {
				// Object became untouchable as we were walking there.
				debug("actorArrived: noun2 untouchable");
				_exec.enabled = false;
				return;
			}
			float dist = distance(getUsePos(), noun2->getUsePos());
			float min_dist = verb.id == VERB_TALKTO ? MIN_TALK_DIST : MIN_USE_DIST;
			debug("actorArrived: noun2 min_dist: {dist} > {min_dist} ?");
			if (dist > min_dist) {
				cantReach(this, noun2);
				return;
			}
		}

		debug("actorArrived: callVerb");
		_exec.enabled = false;
		g_engine->callVerb(this, verb, noun1, noun2);
	}
}

// Walks an actor to the `pos` or actor `obj` and then faces `dir`.
void Object::walk(Math::Vector2d pos, int facing) {
	debug("walk to obj %s: %f,%f, %d", _key.c_str(), pos.getX(), pos.getY(), facing);
	if (!_walkTo || (!_walkTo->isEnabled())) {
		play(getAnimName(WALK_ANIMNAME), true);
	}
	_walkTo = new WalkTo(this, pos, facing);
}

// Walks an actor to the `obj` and then faces it.
void Object::walk(Object *obj) {
	debug("walk to obj %s: (%f,%f)", obj->_key.c_str(), obj->getUsePos().getX(), obj->getUsePos().getY());
	Facing facing = (Facing)obj->_useDir;
	walk(obj->getUsePos(), facing);
}

void Object::turn(Facing facing) {
	setFacing(facing);
}

void Object::turn(Object *obj) {
	Facing facing = getFacingToFaceTo(this, obj);
	setFacing(facing);
}

void TalkingState::say(const Common::StringArray &texts, Object *obj) {
	obj->setTalking(new Talking(obj, texts, _color));
}

} // namespace Twp
