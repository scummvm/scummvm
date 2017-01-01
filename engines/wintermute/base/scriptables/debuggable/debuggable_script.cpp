/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/tokenizer.h"
#include "debuggable_script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script_engine.h"
#include "engines/wintermute/debugger/breakpoint.h"
#include "engines/wintermute/debugger/script_monitor.h"
#include "engines/wintermute/debugger/watch_instance.h"

namespace Wintermute {

DebuggableScript::DebuggableScript(BaseGame *inGame, DebuggableScEngine *engine) : ScScript(inGame, engine), _engine(engine), _stepDepth(kDefaultStepDepth) {
	_engine->_watches.subscribe(this);
	for (uint i = 0; i < _engine->_watches.size(); i++) {
		_watchInstances.push_back(new WatchInstance(_engine->_watches[i], this));
	}
}

DebuggableScript::~DebuggableScript() {
	for (uint i = 0; i < _watchInstances.size(); i++) {
		delete _watchInstances[i];
	}
	_engine->_watches.unsubscribe(this);
}
void DebuggableScript::preInstHook(uint32 inst) {}

void DebuggableScript::postInstHook(uint32 inst) {
	if (inst == II_DBG_LINE) {
		for (uint j = 0; j < _engine->_breakpoints.size(); j++) {
			_engine->_breakpoints[j]->evaluate(this);
		}

		if (_callStack->_sP <= _stepDepth) {
			_engine->_monitor->notifyStep(this);
		}
	}

	for (uint i = 0; i < _watchInstances.size(); i++) {
		this->_watchInstances[i]->evaluate();
	}

}

void DebuggableScript::setStepDepth(int depth) {
	_stepDepth = depth;
}

void DebuggableScript::step() {
	setStepDepth(_callStack->_sP);
	// TODO double check
}

void DebuggableScript::stepContinue() {
	setStepDepth(kDefaultStepDepth);
}

void DebuggableScript::stepFinish() {
	setStepDepth(_callStack->_sP - 1);
}

ScValue *DebuggableScript::resolveName(const Common::String &name) {

	Common::String trimmed = name;
	trimmed.trim();
	Common::StringTokenizer st = Common::StringTokenizer(trimmed.c_str(), ".");
	Common::String nextToken;

	nextToken = st.nextToken();


	char cstr[256]; // TODO not pretty
	Common::strlcpy(cstr, nextToken.c_str(), nextToken.size() + 1);
	cstr[255] = '\0'; // We 0-terminate it just in case it's > 256 chars.

	ScValue *value = getVar(cstr);
	ScValue *res = new ScValue(_gameRef);

	if (value == nullptr) {
		return res;
	}

	nextToken = st.nextToken();

	while (nextToken.size() > 0 && (value->isObject() || value->isNative())) {
		value = value->getProp(nextToken.c_str());
		nextToken = st.nextToken();
		if (value == nullptr) {
			return res;
		}
	}

	res->copy(value);

	return res;
}

uint DebuggableScript::dbgGetLine() const {
	return _currentLine;
}

Common::String DebuggableScript::dbgGetFilename() const {
	return _filename;
}

void DebuggableScript::updateWatches() {
	// We drop obsolete watches
	for (uint i = 0; i < _watchInstances.size(); i++) {
		Watch *findMe = _watchInstances[i]->_watch;
		if (Common::find(_engine->_watches.begin(), _engine->_watches.end(), findMe) == _engine->_watches.end()) {
			// Not found on engine-wide list, must have been removed from watches. Must remove it from local list.
			delete _watchInstances[i];
			_watchInstances.remove_at(i);
		}
	}

	// We add any new watches
	for (uint i = 0; i < _engine->_watches.size(); i++) {
		Watch *findMe = _engine->_watches[i];
		if (Common::find(_engine->_watches.begin(), _engine->_watches.end(), findMe) == _engine->_watches.end()) {
			// Not found on local list, must be a new one.
			_watchInstances.push_back(new WatchInstance(_engine->_watches[i], this));
		}
	}
}
} // End of namespace Wintermute

