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

#include "phoenixvr/variables.h"
#include "common/debug.h"
#include "phoenixvr/console.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

void Variables::declare(const Common::String &name) {
	if (!_variableIndex.contains(name)) {
		_variableValues.push_back(0);
		auto *varPtr = &_variableValues.back();
		_variableIndex.setVal(name, varPtr);
		static_cast<Console *>(g_engine->getDebugger())->registerVar(name, varPtr);
	}
}

int Variables::get(const Common::String &name) const {
	auto it = _variableIndex.find(name);
	if (it == _variableIndex.end()) {
		warning("get %s - variable was not declared", name.c_str());
		return 0;
	}
	return *it->_value;
}

void Variables::set(const Common::String &name, int value) {
	auto it = _variableIndex.find(name);
	if (it == _variableIndex.end()) {
		debug("set %s %d - ignored, variable was not declared", name.c_str(), value);
		return;
	}
	debug("set %s %d", name.c_str(), value);
	*it->_value = value;
}

void Variables::clear() {
	_variableIndex.clear();
	_variableValues.clear();
	static_cast<Console *>(g_engine->getDebugger())->clearVars();
}

} // namespace PhoenixVR
