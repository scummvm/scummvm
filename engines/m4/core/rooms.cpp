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

#include "m4/core/rooms.h"
#include "m4/vars.h"

namespace M4 {

HotSpotRec *Section::walker_spotter(int32 x, int32 y) {
	warning("TODO: walker_spotter");
	return nullptr;
}

void Sections::global_section_constructor() {
	uint sectionNum = _G(game).new_section;
	assert(sectionNum >= 1 && sectionNum <= 9);

	_activeSection = &_sections[sectionNum - 1];
}

} // namespace M4
