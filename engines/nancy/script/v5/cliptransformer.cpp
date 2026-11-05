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

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"

#include "engines/nancy/script/v5/cliptransformer.h"

namespace Nancy {
namespace Script {

void ClipTransformerRecord::readTable(int tableIndex) {
	TransformerRecord::readTable(tableIndex);
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "clip")) {
		_clip = checkRect(L, -1);
		_hasClip = true;
		lua_pop(L, 1);
	}

	// The table can give the record a transform of its own
	getNode()->setTransform(getLocalTransform());
}

// The record's own transform moves the node, which the stages make when there
// are any. Every record moves its stages on by itself.
void ClipTransformerRecord::onStagesChanged() {
	Record::onStagesChanged();
}

void ClipTransformerRecord::onLocalTransformChanged() {
	getNode()->setTransform(getLocalTransform());
}

// What else a Transformer of the earlier games has, every record has here
bool ClipTransformerRecord::getField(const Common::String &key) {
	if (key == "Attach") {
		return TransformerRecord::getField(key);
	} else if (key == "clip") {
		if (_hasClip) {
			pushRect(getLua(), _clip);
		} else {
			lua_pushnil(getLua());
		}

		return true;
	}

	return false;
}

bool ClipTransformerRecord::setField(const Common::String &key) {
	if (key != "clip") {
		return false;
	}

	lua_State *L = getLua();
	_hasClip = !lua_isnil(L, 3);
	if (_hasClip) {
		_clip = checkRect(L, 3);
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
