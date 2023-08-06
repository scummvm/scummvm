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

#ifndef MM_MM1_CONSOLE_H
#define MM_MM1_CONSOLE_H

#include "gui/debugger.h"
#include "mm/mm1/game/spell_casting.h"

namespace MM {
namespace MM1 {

class Console : public GUI::Debugger, public MM1::Game::SpellCasting {
protected:
	/**
	 * Used to dump a map's code and data
	 */
	bool cmdDumpMap(int argc, const char **argv);

	/**
	 * Dumps the monster list
	 */
	bool cmdDumpMonsters(int argc, const char **argv);

	/**
	 * Dumps the item list
	 */
	bool cmdDumpItems(int argc, const char **argv);

	/**
	 * Prints a string from within a map's data segment
	 */
	bool cmdMapString(int argc, const char **argv);

	/**
	 * Toggles intangible mode, allowing walking through walls
	 */
	bool cmdIntangible(int argc, const char **argv);

	/**
	 * Jumps to a given map, and optionally a given position
	 */
	bool cmdMap(int argc, const char **argv);

	/**
	 * Changes the party's position in the current map
	 */
	bool cmdPos(int argc, const char **argv);

	/**
	 * Casts a spell
	 */
	bool cmdCast(int argc, const char **argv);

	/**
	 * Enables/disables casting any spell
	 */
	bool cmdSpellsAll(int argc, const char **argv);

	/**
	 * Trigger an encounter
	 */
	bool cmdEncounter(int argc, const char **argv);

	/**
	 * Turns encounters on or off
	 */
	bool cmdEncounters(int argc, const char **argv);

	/**
	 * List the special cells in the current map
	 */
	bool cmdSpecials(int argc, const char **argv);

	/**
	 * Trigger a special in the current map
	 */
	bool cmdSpecial(int argc, const char **argv);

	/**
	 * Add a specific view
	 */
	bool cmdView(int argc, const char **argv);

	/**
	 * Dump the roster in the original roster.dat format
	 */
	bool cmdDumpRoster(int argc, const char **argv);

public:
	Console();
	~Console() override {}
};

} // namespace MM1
} // namespace MM

#endif
