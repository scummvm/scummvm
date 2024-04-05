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
#include "twp/object.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/squtil.h"

#define MIN_TALK_DIST 60
#define MIN_GIVE_DIST 30
#define MIN_USE_DIST 15

namespace Twp {

static float getVerbDist(VerbId verb) {
	if (verb.id == VERB_TALKTO)
		return MIN_TALK_DIST;
	if (verb.id == VERB_GIVE)
		return MIN_GIVE_DIST;
	return MIN_USE_DIST;
}

enum class BlinkState {
	Closed,
	Open
};

class Blink : public Motor {
public:
	Blink(Common::SharedPtr<Object> obj, float min, float max) : _obj(obj), _min(min), _max(max) {
		_obj->showLayer("blink", false);
		_state = BlinkState::Closed;
		_duration = g_twp->getRandom(min, max);
	}

	virtual void onUpdate(float elapsed) override {
		if (_state == BlinkState::Closed) {
			// wait to blink
			_elapsed += elapsed;
			if (_elapsed > _duration) {
				_state = BlinkState::Open;
				_obj->showLayer("blink", true);
				_elapsed = 0;
			}
		} else if (_state == BlinkState::Open) {
			// wait time the eyes are closed
			_elapsed += elapsed;
			if (_elapsed > 0.25) {
				_obj->showLayer("blink", false);
				_duration = g_twp->getRandom(_min, _max);
				_elapsed = 0;
				_state = BlinkState::Closed;
			}
		}
	}

private:
	Common::SharedPtr<Object> _obj;
	BlinkState _state = BlinkState::Closed;
	float _min = 0.f;
	float _max = 0.f;
	float _elapsed = 0.f;
	float _duration = 0.f;
};

Object::Object()
	: _talkOffset(0, 90) {
	_node = Common::SharedPtr<Node>(new Node("newObj"));
	_nodeAnim = Common::SharedPtr<Anim>(new Anim(this));
	_node->addChild(_nodeAnim.get());
	sq_resetobject(&_table);
	sq_resetobject(&_enter);
	sq_resetobject(&_leave);
}

Object::Object(HSQOBJECT o, const Common::String &key)
	: _talkOffset(0, 90), _table(o), _key(key) {
	sq_resetobject(&_enter);
	sq_resetobject(&_leave);
}

Object::~Object() {
	if (_nodeAnim)
		_nodeAnim->remove();
	_node->remove();

	if (_layer) {
		size_t i = find(_layer->_objects, this);
		if (i != (size_t)-1) {
			_layer->_objects.remove_at(i);
		}
		_layer = nullptr;
	}
}

Common::SharedPtr<Object> Object::createActor() {
	Common::SharedPtr<Object> result(new Object());
	result->_hotspot = Common::Rect(-18, 0, 37, 71);
	result->_facing = Facing::FACE_FRONT;
	result->_useWalkboxes = true;
	result->showLayer("blink", false);
	result->showLayer("eyes_left", false);
	result->showLayer("eyes_right", false);
	result->setHeadIndex(1);
	return result;
}

int Object::getId() const {
	SQInteger result = 0;
	(void)sqgetf(_table, "_id", result);
	return (int)result;
}

Common::String Object::getName() const {
	if ((_table._type == OT_TABLE) && (sqrawexists(_table, "name"))) {
		Common::String result;
		(void)sqgetf(_table, "name", result);
		return result;
	}
	return _name;
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
	for (size_t i = 0; i < _anims.size(); i++) {
		ObjectAnimation &anim = _anims[i];
		if (anim.name == state) {
			_animFlags = anim.flags;
			_nodeAnim->setAnim(&anim, _fps, loop, instant);
			return true;
		}
	}

	// if not found, clear the previous animation
	if (!g_twp->_resManager->isActor(getId())) {
		_nodeAnim->clearFrames();
		_nodeAnim->clear();
	}
	return false;
}

static Node* getChildByName(Node* node, const Common::String& name) {
	if(!node)
		return nullptr;
	for (auto child : node->getChildren()) {
		if (child->getName() == name) {
			return child;
		}
	}
	return nullptr;
}

static Node* getLayerByName(Node* node, const Common::String& name) {
	Node* child = getChildByName(node, name);
	if(child)
		return child;
	if(node->getChildren().size()==1) {
		return getChildByName(node->getChildren()[0], name);
	}
	return nullptr;
}

void Object::showLayer(const Common::String &layer, bool visible) {
	int index = -1;
	for (size_t i = 0; i < _hiddenLayers.size(); i++) {
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
	Node* node = getLayerByName(_node.get(), layer);
	if(node)
		node->setVisible(visible);
}

Facing Object::getFacing() const {
	if (_facingLockValue != 0)
		return (Facing)_facingLockValue;
	for (size_t i = 0; i < _facingMap.size(); i++) {
		if (_facingMap[i].key == _facing)
			return _facingMap[i].value;
	}
	return _facing;
}

void Object::trig(const Common::String &name) {
	// debug fmt"Trigger object #{self.id} ({self.name}) sound '{name}'"
	int trigNum;
	sscanf(name.c_str(), "@%d", &trigNum);
	if ((name.size() > 1) && Common::isDigit(name[1])) {
		if (_triggers.contains(trigNum)) {
			_triggers[trigNum]->trig();
		} else {
			warning("Trigger #%d not found in object #%i (%s)", trigNum, getId(), _key.c_str());
		}
	} else {
		SQInteger id = 0;
		(void)sqgetf(sqrootTbl(g_twp->getVm()), name.substr(1), id);
		Common::SharedPtr<SoundDefinition> sound = sqsounddef(id);
		if (!sound)
			warning("Cannot trig sound '%s', sound not found (id=%lld, %s)", name.c_str(), id, _key.c_str());
		else
			g_twp->_audio->play(sound, Audio::Mixer::SoundType::kPlainSoundType);
	}
}

Common::String Object::suffix() const {
	switch (getFacing()) {
	case Facing::FACE_BACK:
		return "_back";
	default:
	case Facing::FACE_FRONT:
		return "_front";
	case Facing::FACE_LEFT:
		// there is no animation with `left` suffix but use left and flip the sprite
		return "_right";
	case Facing::FACE_RIGHT:
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
	SQInteger result = VERB_LOOKAT;
	if (sqrawexists(_table, "defaultVerb") && SQ_FAILED(sqgetf(_table, "defaultVerb", result))) {
		error("Failed to get defaultVerb");
	} else if (g_twp->_resManager->isActor(getId())) {
		result = sqrawexists(_table, "verbTalkTo") ? VERB_TALKTO : VERB_WALKTO;
	}
	return result;
}

Math::Vector2d Object::getUsePos() {
	return g_twp->_resManager->isActor(getId()) ? _node->getPos() + _node->getOffset() : _node->getPos() + _node->getOffset() + _usePos;
}

bool Object::isTouchable() {
	if (_objType == otNone) {
		if (_state == GONE) {
			return false;
		} else if (_node && !_node->isVisible()) {
			return false;
		} else if (sqrawexists(_table, "_touchable")) {
			bool result;
			if(SQ_FAILED(sqgetf(_table, "_touchable", result)))
				error("Failed to get touchable");
			return result;
		} else if (sqrawexists(_table, "initTouchable")) {
			bool result;
			if(SQ_FAILED(sqgetf(_table, "initTouchable", result)))
				error("Failed to get touchable");
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
	HSQUIRRELVM v = g_twp->getVm();
	sq_newarray(v, 0);
	sqpush(v, fps);
	for (size_t i = 0; i < icons.size(); i++) {
		sqpush(v, icons[i]);
		sq_arrayappend(v, -2);
	}
	HSQOBJECT array;
	sq_resetobject(&array);
	sq_getstackobj(v, -1, &array);
	sqsetf(_table, "icon", array);

	_iconIndex = 0;
	_iconElapsed = 0.f;
}

void Object::setIcon(const Common::String &icon) {
	Common::StringArray icons;
	icons.push_back(icon);
	setIcon(0, icons);
	sqsetf(_table, "icon", icon);
}

struct GetIcons {
	GetIcons(int &fps, Common::StringArray &icons) : _fps(fps), _icons(icons) { _fps = 0; }
	void operator()(HSQOBJECT item) {
		if (_index == 0) {
			_fps = sq_objtointeger(&item);
		} else {
			Common::String icon = sq_objtostring(&item);
			_icons.push_back(icon);
		}
		_index++;
	}

public:
	int &_fps;
	Common::StringArray &_icons;

private:
	int _index = 0;
};

ObjectIcons Object::getIcons() const {
	ObjectIcons result;
	HSQOBJECT iconTable;
	sq_resetobject(&iconTable);
	(void)sqgetf(_table, "icon", iconTable);
	if (iconTable._type == OT_NULL) {
		return result;
	}
	if (iconTable._type == OT_STRING) {
		Common::String icons(sq_objtostring(&iconTable));
		result.icons.push_back(icons);
		return result;
	}
	if (iconTable._type == OT_ARRAY) {
		sqgetitems(iconTable, GetIcons(result.fps, result.icons));
		return result;
	}
	return result;
}

Common::String Object::getIcon() {
	ObjectIcons result = getIcons();
	if (result.icons.empty())
		return "";
	_iconIndex = _iconIndex % result.icons.size();
	return result.icons[_iconIndex];
}

int Object::getFlags() {
	SQInteger result = 0;
	if (sqrawexists(_table, "flags") && SQ_FAILED(sqgetf(_table, "flags", result)))
		error("Failed to get flags");
	return result;
}

void Object::setRoom(Common::SharedPtr<Object> object, Common::SharedPtr<Room> room) {
	bool roomChanged = object->_room != room;
	if (roomChanged || !object->_node->getParent()) {
		if (roomChanged) {
			object->stopObjectMotors();
		}
		Common::SharedPtr<Room> oldRoom = object->_room;
		if (oldRoom && object->_node->getParent()) {
			debugC(kDebugGame, "Remove %s from room %s", object->_key.c_str(), oldRoom->_name.c_str());
			Common::SharedPtr<Layer> layer = oldRoom->layer(0);
			if (layer) {
				int index = find(layer->_objects, object);
				if (index != -1)
					layer->_objects.remove_at(index);
				if (layer)
					layer->_node->removeChild(object->_node.get());
			}
		}
		if (room && room->layer(0) && room->layer(0)->_node) {
			debugC(kDebugGame, "Add %s in room %s", object->_key.c_str(), room->_name.c_str());
			Common::SharedPtr<Layer> layer = room->layer(0);
			if (layer) {
				int index = find(layer->_objects, object);
				if (index == -1)
					layer->_objects.push_back(object);
				layer->_node->addChild(object->_node.get());
			}
		}
		object->_room = room;

		if (roomChanged && g_twp->_resManager->isActor(object->getId())) {
			if (room == g_twp->_room) {
				g_twp->actorEnter(object);
			} else if (oldRoom == g_twp->_room) {
				g_twp->actorExit(object);
			}
		}
	}
}

static void disableMotor(Common::SharedPtr<Motor> motor) {
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
	disableMotor(_scaleTo);
	_node->setRotation(0);
	_node->setRotationOffset(0);
	_node->setOffset({0.f, 0.f});
	_node->setShakeOffset({0.f, 0.f});
	_node->setScale({1.f, 1.f});
	if (g_twp->_resManager->isActor(getId()))
		stand();
}

void Object::setFacing(Facing facing) {
	if (_facing != facing) {
		debugC(kDebugGame, "set facing: %d", facing);
		bool update = !(((_facing == Facing::FACE_LEFT) && (facing == Facing::FACE_RIGHT)) || ((_facing == Facing::FACE_RIGHT) && (facing == Facing::FACE_LEFT)));
		_facing = facing;
		if (update && _nodeAnim)
			play(_animName, _animLoop);
	}
}

Facing Object::getDoorFacing() {
	int flags = getFlags();
	if (flags & DOOR_LEFT)
		return Facing::FACE_LEFT;
	else if (flags & DOOR_RIGHT)
		return Facing::FACE_RIGHT;
	else if (flags & DOOR_FRONT)
		return Facing::FACE_FRONT;
	else
		return Facing::FACE_BACK;
}

bool Object::inInventory() {
	return g_twp->_resManager->isObject(getId()) && getIcon().size() > 0;
}

bool Object::contains(const Math::Vector2d &pos) {
	Math::Vector2d p = pos - _node->getPos() - _node->getOffset();
	return _hotspot.contains(p.getX(), p.getY());
}

void Object::dependentOn(Common::SharedPtr<Object> dependentObj, int state) {
	_dependentState = state;
	_dependentObj = dependentObj;
}

Common::String Object::getAnimName(const Common::String &key) {
	if (_animNames.contains(key))
		return _animNames[key];
	return key;
}

void Object::setHeadIndex(int head) {
	Node* node = getLayerByName(_node.get(), Common::String::format("%s%d", getAnimName(HEAD_ANIMNAME).c_str(), head));
	if(!node)
		return;
	for (int i = 0; i <= 6; i++) {
		showLayer(Common::String::format("%s%d", getAnimName(HEAD_ANIMNAME).c_str(), i), i == head);
	}
}

bool Object::isWalking() {
	return _walkTo && _walkTo->isEnabled();
}

void Object::stopWalking() {
	if (_walkTo)
		_walkTo->disable();
}

void Object::setAnimationNames(const Common::String &head, const Common::String &standAnim, const Common::String &walk, const Common::String &reach) {
	if (!head.empty()) {
		setHeadIndex(0);
		_animNames[HEAD_ANIMNAME] = head;
	} else {
		_animNames.erase(HEAD_ANIMNAME);
	}

	showLayer(getAnimName(HEAD_ANIMNAME), true);
	setHeadIndex(1);
	if (!standAnim.empty()) {
		_animNames[STAND_ANIMNAME] = standAnim;
	} else {
		_animNames.erase(STAND_ANIMNAME);
	}
	if (!walk.empty()) {
		_animNames[WALK_ANIMNAME] = walk;
	} else {
		_animNames.erase(WALK_ANIMNAME);
	}
	if (!reach.empty()) {
		_animNames[REACH_ANIMNAME] = reach;
	} else {
		_animNames.erase(REACH_ANIMNAME);
	}
	if (isWalking())
		play(getAnimName(WALK_ANIMNAME), true);
	else
		stand();
}

void Object::blinkRate(Common::SharedPtr<Object> obj, float min, float max) {
	if (min == 0.0 && max == 0.0) {
		obj->_blink.reset();
	} else {
		obj->_blink.reset(new Blink(obj, min, max));
	}
}

void Object::setCostume(const Common::String &name, const Common::String &sheet) {
	GGPackEntryReader entry;
	entry.open(*g_twp->_pack, name + ".json");

	GGHashMapDecoder dec;
	Common::ScopedPtr<Common::JSONValue> json(dec.open(&entry));
	if (!json) {
		warning("Costume %s(%s) for actor %s not found", name.c_str(), sheet.c_str(), _key.c_str());
		return;
	}
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
}

void Object::stand() {
	play(getAnimName(STAND_ANIMNAME), true);
}

#define SET_MOTOR(motorTo)     \
	if (_##motorTo) {          \
		_##motorTo->disable(); \
	}                          \
	_##motorTo = motorTo;

void Object::setAlphaTo(Common::SharedPtr<Motor> alphaTo) { SET_MOTOR(alphaTo); }
void Object::setRotateTo(Common::SharedPtr<Motor> rotateTo) { SET_MOTOR(rotateTo); }
void Object::setMoveTo(Common::SharedPtr<Motor> moveTo) { SET_MOTOR(moveTo); }
void Object::setReach(Common::SharedPtr<Motor> reach) { SET_MOTOR(reach); }
void Object::setTalking(Common::SharedPtr<Motor> talking) { SET_MOTOR(talking); }
void Object::setShakeTo(Common::SharedPtr<Motor> shakeTo) { SET_MOTOR(shakeTo); }
void Object::setScaleTo(Common::SharedPtr<Motor> scaleTo) { SET_MOTOR(scaleTo); }

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
	if (_scaleTo)
		_scaleTo->update(elapsedSec);

	if (_nodeAnim)
		_nodeAnim->update(elapsedSec);

	ObjectIcons icons = getIcons();
	if ((icons.icons.size() > 1) && (icons.fps > 0)) {
		_iconElapsed += elapsedSec;
		if (_iconElapsed > (1.f / icons.fps)) {
			_iconElapsed = 0.f;
			_iconIndex = (_iconIndex + 1) % icons.icons.size();
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

void Object::pickupObject(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj) {
	obj->_owner.reset(actor);
	actor->_inventory.push_back(obj);

	sqcall("onPickup", obj->_table, actor->_table);

	if (sqrawexists(obj->_table, "onPickUp")) {
		sqcall(obj->_table, "onPickUp", actor->_table);
	}
}

void Object::stopTalking() {
	if (_talking) {
		_talking->disable();
		setHeadIndex(1);
	}
}

void Object::say(Common::SharedPtr<Object> obj, const Common::StringArray &texts, const Color &color) {
	if (texts.size() == 0)
		return;
	obj->_talkingState._obj.reset(obj);
	obj->_talkingState._color = color;
	obj->_talkingState.say(texts, obj);
}

void Object::resetLockFacing() {
	_facingMap.clear();
	_facingLockValue = 0;
}

void Object::lockFacing(int facing) {
	_facingLockValue = facing;
}

void Object::lockFacing(Facing left, Facing right, Facing front, Facing back) {
	_facingMap.push_back({Facing::FACE_LEFT, left});
	_facingMap.push_back({Facing::FACE_RIGHT, right});
	_facingMap.push_back({Facing::FACE_FRONT, front});
	_facingMap.push_back({Facing::FACE_BACK, back});
}

UseFlag Object::useFlag() {
	int flags = getFlags();
	if (flags & USE_WITH)
		return UseFlag::ufUseWith;
	if (flags & USE_ON)
		return UseFlag::ufUseOn;
	if (flags & USE_IN)
		return UseFlag::ufUseIn;
	return UseFlag::ufNone;
}

float Object::getScale() {
	if (getPop() > 0)
		return 4.25f + popScale() * 0.25f;
	return 4.f;
}

void Object::removeInventory(Common::SharedPtr<Object> obj) {
	int i = find(_inventory, obj);
	if (i >= 0) {
		_inventory.remove_at(i);
		obj->_owner = nullptr;
	}
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

static void cantReach(Common::SharedPtr<Object> self, Common::SharedPtr<Object> noun2) {
	if (sqrawexists(self->_table, "verbCantReach")) {
		int nParams = sqparamCount(g_twp->getVm(), self->_table, "verbCantReach");
		debugC(kDebugGame, "verbCantReach found in obj '%s' with %d params", self->_key.c_str(), nParams);
		if (nParams == 1) {
			sqcall(self->_table, "verbCantReach");
		} else {
			HSQOBJECT table;
			sq_resetobject(&table);
			if (noun2)
				table = noun2->_table;
			sqcall(self->_table, "verbCantReach", table);
		}
	} else if (noun2) {
		cantReach(noun2, nullptr);
	} else {
		HSQOBJECT nilTbl;
		sq_resetobject(&nilTbl);
		sqcall(g_twp->_defaultObj, "verbCantReach", self->_table, nilTbl);
	}
}

void Object::execVerb(Common::SharedPtr<Object> obj) {
	if (obj->_exec.enabled) {
		VerbId verb = obj->_exec.verb;
		Common::SharedPtr<Object> noun1 = obj->_exec.noun1;
		Common::SharedPtr<Object> noun2 = obj->_exec.noun2;

		debugC(kDebugGame, "actorArrived: exec sentence");
		if (!noun1->inInventory()) {
			// Object became untouchable as we were walking there
			if (!noun1->isTouchable()) {
				debugC(kDebugGame, "actorArrived: noun1 untouchable");
				obj->_exec.enabled = false;
				return;
			}
			// Did we get close enough?
			float dist = distance(obj->getUsePos(), noun1->getUsePos());
			float min_dist = getVerbDist(verb);
			debugC(kDebugGame, "actorArrived: noun1 min_dist: %f > %f (actor: {self.getUsePos}, obj: {noun1.getUsePos}) ?", dist, min_dist);
			if (!verbNotClose(verb) && (dist > min_dist)) {
				cantReach(noun1, noun2);
				return;
			}
			if (noun1->_useDir != dNone) {
				obj->setFacing((Facing)noun1->_useDir);
			}
		}
		if (noun2 && !noun2->inInventory()) {
			if (!noun2->isTouchable()) {
				// Object became untouchable as we were walking there.
				debugC(kDebugGame, "actorArrived: noun2 untouchable");
				obj->_exec.enabled = false;
				return;
			}
			float dist = distance(obj->getUsePos(), noun2->getUsePos());
			float min_dist = getVerbDist(verb);
			debugC(kDebugGame, "actorArrived: noun2 min_dist: %f > %f ?", dist, min_dist);
			if (dist > min_dist) {
				cantReach(noun1, noun2);
				return;
			}
		}

		debugC(kDebugGame, "actorArrived: callVerb");
		obj->_exec.enabled = false;
		g_twp->callVerb(obj, verb, noun1, noun2);
	}
}

// Walks an actor to the `pos` or actor `obj` and then faces `dir`.
void Object::walk(Common::SharedPtr<Object> obj, const Math::Vector2d &pos, int facing) {
	debugC(kDebugGame, "walk to obj %s: %f,%f, %d", obj->_key.c_str(), pos.getX(), pos.getY(), facing);
	if (!obj->_walkTo || (!obj->_walkTo->isEnabled())) {
		obj->play(obj->getAnimName(WALK_ANIMNAME), true);
	}
	obj->_walkTo = Common::SharedPtr<WalkTo>(new WalkTo(obj, pos, facing));
}

// Walks an actor to the `obj` and then faces it.
void Object::walk(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj) {
	debugC(kDebugGame, "walk to obj %s: (%f,%f)", obj->_key.c_str(), obj->getUsePos().getX(), obj->getUsePos().getY());
	int facing = static_cast<int>(obj->_useDir);
	walk(actor, obj->getUsePos(), facing);
}

void Object::turn(Facing facing) {
	stand();
	setFacing(facing);
}

void Object::turn(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj) {
	Facing facing = getFacingToFaceTo(actor, obj);
	actor->stand();
	actor->setFacing(facing);
}

void Object::jiggle(float amount) {
	_jiggleTo = Common::SharedPtr<Jiggle>(new Jiggle(_node.get(), amount));
}

void Object::inventoryScrollUp() {
	if (_inventoryOffset == 0)
		return;
	_inventoryOffset--;
}

void Object::inventoryScrollDown() {
	_inventoryOffset++;
	_inventoryOffset = CLIP(_inventoryOffset, 0, MAX(0, ((int)_inventory.size() - 5) / 4));
}

void TalkingState::say(const Common::StringArray &texts, Common::SharedPtr<Object> obj) {
	Talking *talking = dynamic_cast<Talking *>(obj->getTalking().get());
	if (!talking) {
		obj->setTalking(Common::SharedPtr<Talking>(new Talking(obj, texts, _color)));
	} else {
		talking->append(texts);
	}
}

} // namespace Twp
