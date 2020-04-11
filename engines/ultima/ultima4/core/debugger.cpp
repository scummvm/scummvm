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

#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Debugger *g_debugger;

Debugger::Debugger() : Shared::Debugger() {
	g_debugger = this;

	registerCmd("gate", WRAP_METHOD(Debugger, cmdGate));
}

Debugger::~Debugger() {
	g_debugger = nullptr;
}

void Debugger::print(const char *fmt, ...) {
	// Format the string
	va_list va;
	va_start(va, fmt);
	Common::String str = Common::String::vformat(fmt, va);
	va_end(va);

	if (isActive()) {
		debugPrintf("%s\n", str.c_str());
	} else {
		screenMessage("%s\n", str.c_str());
	}
}

bool Debugger::cmdGate(int argc, const char **argv) {
	int gateNum = (argc == 2) ? strToInt(argv[1]) : -1;

	if (!g_context || !g_game || gateNum < 1 || gateNum > 8) {
		print("Gate <1 to 8>");
	} else {
		if (!isActive())
			print("Gate %d!", gateNum);

		if (g_context->_location->_map->isWorldMap()) {
			const Coords *moongate = moongateGetGateCoordsForPhase(gateNum - 1);
			if (moongate) {
				g_context->_location->_coords = *moongate;
				g_game->finishTurn();
				return false;
			}
		} else {
			print("Not here!");
		}
	}

	return true;
}

} // End of namespace Ultima4
} // End of namespace Ultima
