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

#ifndef ULTIMA3_CONSOLE_H
#define ULTIMA3_CONSOLE_H

#include "common/array.h"
#include "common/rect.h"
#include "gui/debugger.h"

namespace Ultima {
namespace Ultima3 {

class Console : public GUI::Debugger {
private:
	/**
	 * Works out which party members a command applies to from an optional
	 * position given as an argument, defaulting to all of them
	 * @returns		False if the argument isn't a position in the party
	 */
	bool getTargets(int argc, const char **argv, int index, int &first, int &last);

	/**
	 * Makes sure there's a party and a world to be in
	 */
	void ensureGame();

	/**
	 * Switches to the view for wherever the party is now
	 */
	void restartGame();

	bool cmdMap(int argc, const char **argv);
	bool cmdTiles(int argc, const char **argv);
	bool cmdTeleport(int argc, const char **argv);
	bool cmdLocations(int argc, const char **argv);
	bool cmdIntangible(int argc, const char **argv);
	bool cmdEnemy(int argc, const char **argv);
	bool cmdHP(int argc, const char **argv);
	bool cmdFood(int argc, const char **argv);
	bool cmdGold(int argc, const char **argv);
	bool cmdInventory(int argc, const char **argv);
	bool cmdLoad(int argc, const char **argv);
	bool cmdSave(int argc, const char **argv);

public:
	Console();
	~Console() override;
};

} // namespace Ultima3
} // namespace Ultima

#endif
