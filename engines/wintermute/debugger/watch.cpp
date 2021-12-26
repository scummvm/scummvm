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

#include "watch.h"
#include "watch_instance.h"
#include "engines/wintermute/debugger/script_monitor.h"

namespace Wintermute {

Watch::Watch(const Common::String &filename, const Common::String &symbol, ScriptMonitor* monitor) : _enabled(false), _filename(filename), _symbol(symbol), _monitor(monitor) {}

Watch::~Watch() { /* Nothing to take care of in here */ }

void Watch::trigger(WatchInstance* instance) {
	_monitor->onWatch(this, instance->_script);
}

Common::String Watch::getFilename() const { return _filename; }
Common::String Watch::getSymbol() const { return _symbol; }
bool Watch::isEnabled() const { return _enabled; }
void Watch::enable() { _enabled = true; }
void Watch::disable() { _enabled = false; }
}
