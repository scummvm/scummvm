
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
#include "twp/motor.h"
#include "twp/object.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"

namespace Twp {

OffsetTo::~OffsetTo() {}

OffsetTo::OffsetTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getOffset(), pos, duration, im) {
}

void OffsetTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setOffset(_tween.current());
	if (!_tween.running())
		disable();
}

MoveTo::~MoveTo() {}

MoveTo::MoveTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getPos(), pos, duration, im) {
}

void MoveTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setPos(_tween.current());
	if (!_tween.running())
		disable();
}

AlphaTo::~AlphaTo() {}

AlphaTo::AlphaTo(float duration, Object *obj, float to, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getAlpha(), to, duration, im) {
}

void AlphaTo::update(float elapsed) {
	_tween.update(elapsed);
	float alpha = _tween.current();
	_obj->_node->setAlpha(alpha);
	if (!_tween.running())
		disable();
}

RotateTo::~RotateTo() {}

RotateTo::RotateTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getRotation(), to, duration, im) {
}

void RotateTo::update(float elapsed) {
	_tween.update(elapsed);
	_node->setRotation(_tween.current());
	if (!_tween.running())
		disable();
}

ScaleTo::~ScaleTo() {}

ScaleTo::ScaleTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getScale().getX(), to, duration, im) {
}

void ScaleTo::update(float elapsed) {
	_tween.update(elapsed);
	float x = _tween.current();
	_node->setScale(Math::Vector2d(x, x));
	if (!_tween.running())
		disable();
}

Shake::~Shake() {}

Shake::Shake(Node *node, float amount)
	: _node(node),
	  _amount(amount) {
}

void Shake::update(float elapsed) {
	_shakeTime += 40.f * elapsed;
	_elapsed += elapsed;
	// TODO: check if it's necessary to create a _shakeOffset in a node
	_node->setOffset(Math::Vector2d(_amount * cos(_shakeTime + 0.3f), _amount * sin(_shakeTime)));
}

OverlayTo::OverlayTo(float duration, Room *room, Color to)
	: _room(room),
	  _to(to),
	  _tween(g_engine->_room->getOverlay(), to, duration, InterpolationMethod()) {
}

OverlayTo::~OverlayTo() {}

void OverlayTo::update(float elapsed) {
	_tween.update(elapsed);
	_room->setOverlay(_tween.current());
	if (!_tween.running())
		disable();
}

ReachAnim::ReachAnim(Object *actor, Object *obj)
	: _actor(actor), _obj(obj) {
}

ReachAnim::~ReachAnim() {
}

void ReachAnim::playReachAnim() {
	Common::String anim = _actor->getAnimName(REACH_ANIMNAME + _obj->getReachAnim());
	_actor->play(anim);
}

void ReachAnim::update(float elapsed) {
	switch (_state) {
	case 0:
		playReachAnim();
		_state = 1;
		break;
	case 1:
		_elapsed += elapsed;
		if (_elapsed > 0.5)
			_state = 2;
		break;
	case 2:
		_actor->stand();
		_actor->execVerb();
		disable();
		_state = 3;
		break;
	default:
		break;
		;
	}
}

WalkTo::WalkTo(Object *obj, Math::Vector2d dest, int facing)
	: _obj(obj), _facing(facing) {
	if (obj->_useWalkboxes) {
		_path = obj->_room->calculatePath(obj->_node->getAbsPos(), dest);
	} else {
		_path = {obj->_node->getAbsPos(), dest};
	}
	_wsd = sqrt(obj->_walkSpeed.getX() * obj->_walkSpeed.getX() + obj->_walkSpeed.getY() * obj->_walkSpeed.getY());
	if (sqrawexists(obj->_table, "preWalking"))
		sqcall(obj->_table, "preWalking");
}

void WalkTo::disable() {
	Motor::disable();
	if (_path.size() != 0) {
		debug("actor walk cancelled");
	}
	_obj->play("stand");
}

static bool needsReachAnim(int verbId) {
	return (verbId == VERB_PICKUP) || (verbId == VERB_OPEN) || (verbId == VERB_CLOSE) || (verbId == VERB_PUSH) || (verbId == VERB_PULL) || (verbId == VERB_USE);
}

void WalkTo::actorArrived() {
	bool needsReach = _obj->_exec.enabled && needsReachAnim(_obj->_exec.verb.id);
	if (!needsReach)
		disable();

	debug("actorArrived");
	_obj->play("stand");
	// the faces to the specified direction (if any)
	if (_facing) {
		debug("actor arrived with facing %d", _facing);
		_obj->setFacing((Facing)_facing);
	}

	// call `actorArrived` callback
	if (sqrawexists(_obj->_table, "actorArrived")) {
		debug("call actorArrived callback");
		sqcall(_obj->_table, "actorArrived");
	}

	// we need to execute a sentence when arrived ?
	if (_obj->_exec.enabled) {
		VerbId verb = _obj->_exec.verb;
		Object *noun1 = _obj->_exec.noun1;
		Object *noun2 = _obj->_exec.noun2;
		// call `postWalk`callback
		Common::String funcName = isActor(noun1->getId()) ? "actorPostWalk" : "objectPostWalk";
		if (sqrawexists(_obj->_table, funcName)) {
			debug("call %s callback", funcName.c_str());
			HSQOBJECT n2Table;
			if (noun2)
				n2Table = noun2->_table;
			else
				sq_resetobject(&n2Table);
			sqcall(_obj->_table, funcName.c_str(), verb.id, noun1->_table, n2Table);
		}

		if (needsReach)
			_reach = new ReachAnim(_obj, noun1);
		else
			_obj->execVerb();
	}
}

void WalkTo::update(float elapsed) {
	if (_path.size() != 0) {
		Math::Vector2d dest = _path[0];
		float d = distance(dest, _obj->_node->getAbsPos());

		// arrived at destination ?
		if (d < 1.0) {
			_obj->_node->setPos(_path[0]);
			_path.remove_at(0);
			if (_path.size() == 0) {
				actorArrived();
			}
		} else {
			Math::Vector2d delta = dest - _obj->_node->getAbsPos();
			float duration = d / _wsd;
			float factor = Twp::clamp(elapsed / duration, 0.f, 1.f);

			Math::Vector2d dd = delta * factor;
			_obj->_node->setPos(_obj->_node->getPos() + dd);
			if (abs(delta.getX()) >= abs(delta.getY())) {
				_obj->setFacing(delta.getX() >= 0 ? FACE_RIGHT : FACE_LEFT);
			} else {
				_obj->setFacing(delta.getY() > 0 ? FACE_BACK : FACE_FRONT);
			}
		}
	}

	if (_reach && _reach->isEnabled()) {
		_reach->update(elapsed);
		if (!_reach->isEnabled())
			disable();
	}
}

} // namespace Twp
