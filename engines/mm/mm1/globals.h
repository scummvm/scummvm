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

#ifndef MM1_GLOBALS_H
#define MM1_GLOBALS_H

#include "graphics/font.h"
#include "mm/utils/strings_data.h"
#include "mm/mm1/utils/roster.h"

namespace MM {
namespace MM1 {

class Globals {
public:
	StringsData _strings;
	Roster _roster;
	const Graphics::Font *_font = nullptr;
	RosterEntry *_rosterEntry = nullptr;
public:
	Globals();
	virtual ~Globals();

	/**
	 * Loads data for the globals
	 */
	bool load();
};

extern Globals *g_globals;

#define STRING (g_globals->_strings)

} // namespace MM1
} // namespace MM

#endif
