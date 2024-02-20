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
#include "twp/enginedialogtarget.h"
#include "twp/squtil.h"

namespace Twp {

class Pause : public Motor {
public:
	explicit Pause(float time) : _time(time) {}

	void update(float elapsed) override {
		_time -= elapsed;
		if (_time <= 0)
			disable();
	}

private:
	float _time = 0.f;
};

class WaitWhile : public Motor {
public:
	WaitWhile(EngineDialogTarget *target, const Common::String &cond) : _target(target), _cond(cond) {}

	void update(float elapsed) override {
		if (!_target->execCond(_cond))
			disable();
	}

private:
	EngineDialogTarget *_target = nullptr;
	Common::String _cond;
};

static Common::SharedPtr<Object> actor(const Common::String &name) {
	HSQOBJECT obj;
	sq_resetobject(&obj);
	sqgetf(name, obj);
	return sqactor(obj);
}

static Common::SharedPtr<Object> actorOrCurrent(const Common::String &name) {
	Common::SharedPtr<Object> result = actor(name);
	if (!result)
		result = g_twp->_actor;
	return result;
}

Color EngineDialogTarget::actorColor(const Common::String &actor) {
	Common::SharedPtr<Object> act = actorOrCurrent(actor);
	return g_twp->_hud.actorSlot(act)->verbUiColors.dialogNormal;
}

Color EngineDialogTarget::actorColorHover(const Common::String &actor) {
	Common::SharedPtr<Object> act = actorOrCurrent(actor);
	return g_twp->_hud.actorSlot(act)->verbUiColors.dialogHighlight;
}

Common::SharedPtr<Motor> EngineDialogTarget::say(const Common::String &actor, const Common::String &text) {
	debugC(kDebugDialog, "say %s: %s", actor.c_str(), text.c_str());
	Common::SharedPtr<Object> act = actorOrCurrent(actor);
	Object::say(act, {text}, act->_talkColor);
	return act->getTalking();
}

Common::SharedPtr<Motor> EngineDialogTarget::waitWhile(const Common::String &cond) {
	return Common::SharedPtr<WaitWhile>(new WaitWhile(this, cond));
}

void EngineDialogTarget::shutup() {
	g_twp->stopTalking();
}

Common::SharedPtr<Motor> EngineDialogTarget::pause(float time) {
	return Common::SharedPtr<Pause>(new Pause(time));
}

bool EngineDialogTarget::execCond(const Common::String &cond) {
	// check if the condition corresponds to an actor name
	Common::SharedPtr<Object> act = actor(cond);
	if (act) {
		// yes, so we check if the current actor is the given actor name
		Common::SharedPtr<Object> curActor = g_twp->_actor;
		return curActor && curActor->_key == act->_key;
	}

	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	// compile
	sq_pushroottable(v);
	Common::String code = Common::String::format("return %s", cond.c_str());
	if (SQ_FAILED(sq_compilebuffer(v, code.c_str(), code.size(), "condition", SQTrue))) {
		debugC(kDebugDialog, "Error executing code %s", code.c_str());
		return false;
	}

	sq_push(v, -2);
	// call
	if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue))) {
		debugC(kDebugDialog, "Error calling code %s", code.c_str());
		return false;
	}

	SQInteger condResult;
	sq_getinteger(v, -1, &condResult);
	bool result = condResult != 0;
	sq_settop(v, top);
	return result;
}

} // namespace Twp
