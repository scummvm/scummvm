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

#include "twp/enginedialogtarget.h"
#include "twp/twp.h"

namespace Twp {

class Pause : public Motor {
public:
	Pause(float time) : _time(time) {}

	virtual void update(float elapsed) override {
		_time -= elapsed;
		if (_time <= 0)
			disable();
	}

private:
	float _time = 0.f;
};

class WaitWhile : public Motor {
public:
	WaitWhile(EngineDialogTarget* target, const Common::String& cond) : _target(target), _cond(cond) {}

	virtual void update(float elapsed) override {
		if (!_target->execCond(_cond))
    		disable();
	}

private:
	EngineDialogTarget* _target = nullptr;
	Common::String _cond;
};

static Object *actor(const Common::String &name) {
	// for (actor in gEngine.actors) {
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *actor = g_engine->_actors[i];
		if (actor->_key == name)
			return actor;
	}
	return nullptr;
}

static Object *actorOrCurrent(const Common::String &name) {
	Object *result = actor(name);
	if (!result)
		result = g_engine->_actor;
	return result;
}

Color EngineDialogTarget::actorColor(const Common::String &actor) {
	Object *act = actorOrCurrent(actor);
	return g_engine->_hud.actorSlot(act)->verbUiColors.dialogNormal;
}

Color EngineDialogTarget::actorColorHover(const Common::String &actor) {
	Object *act = actorOrCurrent(actor);
	return g_engine->_hud.actorSlot(act)->verbUiColors.dialogHighlight;
}

Motor *EngineDialogTarget::say(const Common::String &actor, const Common::String &text) {
	debug("say %s: %s", actor.c_str(), text.c_str());
	Object *act = actorOrCurrent(actor);
	act->say({text}, act->_talkColor);
	return act->getTalking();
}

Motor *EngineDialogTarget::waitWhile(const Common::String &cond) {
	return new WaitWhile(this, cond);
}

void EngineDialogTarget::shutup() {
	g_engine->stopTalking();
}

Motor *EngineDialogTarget::pause(float time) {
	return new Pause(time);
}

bool EngineDialogTarget::execCond(const Common::String &cond) {
	// check if the condition corresponds to an actor name
	Object *act = actor(cond);
	if (act) {
		// yes, so we check if the current actor is the given actor name
		Object *curActor = g_engine->_actor;
		return curActor && curActor->_key == act->_key;
	}

	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	// compile
	sq_pushroottable(v);
	Common::String code = Common::String::format("return %s", cond.c_str());
	if (SQ_FAILED(sq_compilebuffer(v, code.c_str(), code.size(), "condition", SQTrue))) {
		debug("Error executing code %s", code.c_str());
		return false;
	}

	sq_push(v, -2);
	// call
	if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue))) {
		debug("Error calling code {code}");
		return false;
	}

	SQInteger condResult;
	sq_getinteger(v, -1, &condResult);
	bool result = condResult != 0;
	sq_settop(v, top);
	return result;
}

} // namespace Twp
