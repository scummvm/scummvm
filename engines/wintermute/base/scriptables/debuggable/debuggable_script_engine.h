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

#ifndef DEBUGGABLE_SCRIPT_ENGINE_H_
#define DEBUGGABLE_SCRIPT_ENGINE_H_
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/coll_templ.h"
#include "common/algorithm.h"
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script.h"

namespace Wintermute {

class Breakpoint;
class Watch;
class DebuggableScript;
class DebuggableScEngine;
class ScriptMonitor;

class PublisherWArray : private Common::Array<Watch *> {
	Common::Array<DebuggableScript *> _subscribers;
	void notifySubscribers() {
		for (uint i = 0; i < _subscribers.size(); i++) {
			_subscribers[i]->updateWatches();
		}
	}
public:
	void subscribe(DebuggableScript *script) {
		if (Common::find(_subscribers.begin(), _subscribers.end(), script) == _subscribers.end()) {
			// If not already contained
			_subscribers.push_back(script);
		}
	}

	void unsubscribe(DebuggableScript *script) {
		int location = -1;
		for (uint i = 0; i < _subscribers.size() && location == -1; i++) {
			if (_subscribers[i] == script) {
				location = i;
			}
		}
		if (location >= 0) {
			_subscribers.remove_at(location);
		} else {
			// TODO: If this happens... it's funny. Some script out there forgot to subscribe.
		}
	}

	void push_back(Watch *newElement) {
		Common::Array<Watch *>::push_back(newElement);
		notifySubscribers();
	}

	size_type size() {
		return Common::Array<Watch *>::size();
	}

	iterator       begin() {
		return Common::Array<Watch *>::begin();
	}

	iterator       end() {
		return Common::Array<Watch *>::end();
	}

	Watch *&operator[](size_type idx) {
		return Common::Array<Watch *>::operator[](idx);
	}
	Watch *remove_at(size_type idx) {
		Watch *res = Common::Array<Watch *>::remove_at(idx);
		notifySubscribers();
		return res;
	}
};

class DebuggableScEngine : public ScEngine {
	Common::Array<Breakpoint *> _breakpoints;
	PublisherWArray _watches;
	ScriptMonitor *_monitor;
public:
	DebuggableScEngine(BaseGame *inGame);
	void attachMonitor(ScriptMonitor *);

	friend class DebuggerController;
	friend class DebuggableScript;
	friend class ScScript;
	friend class WatchableScriptArray;
};

} // End of namespace Wintermute

#endif /* DEBUGGABLE_SCRIPT_ENGINE_H_ */
