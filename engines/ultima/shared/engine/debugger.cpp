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

#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/maps/map.h"

namespace Ultima {
namespace Shared {

Debugger::Debugger() : GUI::Debugger() {
	registerCmd("continue",	     WRAP_METHOD(Debugger, cmdExit));
	registerCmd("spell", WRAP_METHOD(Debugger, cmdSpell));
}

int Debugger::strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::cmdSpell(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("spell <spell number>\n");
		return true;
	} else {
		int spellId = strToInt(argv[1]);
		Shared::Game *game = dynamic_cast<Shared::Game *>(g_vm->_game);
		assert(game);

		game->_map->castSpell(spellId);
		return false;
	}
}

} // End of namespace Shared
} // End of namespace Ultima
