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

#include "backends/keymapper/action.h"

#include "backends/keymapper/keymap.h"

namespace Common {

Action::Action(const char *i, const U32String &des) :
		id(i),
		description(des),
		_shouldTriggerOnKbdRepeats(false) {
	assert(i);
}

void Action::addDefaultInputMapping(const String &hwId) {
	if (hwId.empty()) {
		return;
	}

	// Don't allow an input to map to the same action multiple times
	Array<String>::const_iterator found = find(_defaultInputMapping.begin(), _defaultInputMapping.end(), hwId);
	if (found == _defaultInputMapping.end()) {
		_defaultInputMapping.push_back(hwId);
	}
}

} // End of namespace Common
