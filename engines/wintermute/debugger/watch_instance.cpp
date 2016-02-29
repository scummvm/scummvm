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

#include "watch_instance.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script.h"
#include "engines/wintermute/debugger/watch.h"

namespace Wintermute {

WatchInstance::WatchInstance(Watch* watch, DebuggableScript* script) : _watch(watch), _script(script), _lastValue(nullptr) {}
WatchInstance::~WatchInstance() { delete _lastValue; }

void WatchInstance::evaluate() {
	if (_watch->isEnabled()) {
		if (!_watch->getFilename().compareTo(_script->_filename)) {

			if(_lastValue == nullptr) {
				_lastValue = new ScValue(_script->_gameRef);
				// ^^ This here is NULL by default
			}
			ScValue* currentValue = _script->resolveName(_watch->getSymbol());
			if(ScValue::compare(
					currentValue,
					_lastValue
					)) {
				_lastValue->copy(currentValue);
				_watch->trigger(this);
			}
			delete currentValue;
		}
	}
}
} // End of namespace Wintermute
