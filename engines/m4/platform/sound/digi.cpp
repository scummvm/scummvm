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

#include "m4/platform/sound/digi.h"
#include "m4/vars.h"

namespace M4 {
namespace Sound {

void Digi::preload_sounds(const char **names) {
	if (!_names.empty()) {
		unload_sounds();
		_names.clear();
	}

	if (names) {
		for (; *names; ++names) {
			_names.push_back(*names);
			preload(*names, NOWHERE);
		}
	}
}

void Digi::unload_sounds() {
	for (uint i = 0; i < _names.size(); ++i)
		unload(_names[i]);
}

void Digi::preload(const Common::String &name, int roomNum) {
	warning("TODO: Digi::preload - %s, %d", name.c_str(), roomNum);
}

void Digi::unload(const Common::String &name) {
	warning("TODO: Digi::unload");
}

} // namespace Sound
} // namespace M4
