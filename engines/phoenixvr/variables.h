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

#ifndef PHOENIXVR_VARIABLES_H
#define PHOENIXVR_VARIABLES_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/list.h"

namespace PhoenixVR {

class Variables {
	Common::List<int> _variableValues;
	Common::HashMap<Common::String, int *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variableIndex;

public:
	Common::List<int> &values() { return _variableValues; }
	const Common::List<int> &values() const { return _variableValues; }

	int get(const Common::String &name) const;
	void set(const Common::String &name, int value);
	void declare(const Common::String &name);
	bool declared(const Common::String &name) const {
		return _variableIndex.contains(name);
	}
	void clear();
};

} // namespace PhoenixVR

#endif
