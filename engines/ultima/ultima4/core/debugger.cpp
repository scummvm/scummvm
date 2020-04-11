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
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Debugger *g_debugger;

Debugger::Debugger() : Shared::Debugger() {
	g_debugger = this;
	_collisionOverride = false;

	registerCmd("collisions", WRAP_METHOD(Debugger, cmdCollisions));
	registerCmd("equipment", WRAP_METHOD(Debugger, cmdEquipment));
	registerCmd("gate", WRAP_METHOD(Debugger, cmdGate));
	registerCmd("goto", WRAP_METHOD(Debugger, cmdGoto));
	registerCmd("moon", WRAP_METHOD(Debugger, cmdMoon));
	registerCmd("stats", WRAP_METHOD(Debugger, cmdStats));
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

bool Debugger::cmdCollisions(int argc, const char **argv) {
	_collisionOverride = !_collisionOverride;
	print("Collision detection %s",
		_collisionOverride ? "off" : "on");

	return isActive();
}

bool Debugger::cmdEquipment(int argc, const char **argv) {
	int i;

	for (i = ARMR_NONE + 1; i < ARMR_MAX; ++i)
		g_ultima->_saveGame->_armor[i] = 8;

	for (i = WEAP_HANDS + 1; i < WEAP_MAX; ++i) {
		const Weapon *weapon = Weapon::get(static_cast<WeaponType>(i));
		if (weapon->loseWhenUsed() || weapon->loseWhenRanged())
			g_ultima->_saveGame->_weapons[i] = 99;
		else
			g_ultima->_saveGame->_weapons[i] = 8;
	}
	
	print("All equipment given");
	return isActive();
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

	return isActive();
}

bool Debugger::cmdGoto(int argc, const char **argv) {
	Common::String dest;

	if (argc == 2) {
		dest = argv[1];
	} else if (isActive()) {
		print("teleport <destination name>");
		return true;
	} else {
		screenMessage("Goto: ");
		dest = gameGetInput(32);
		screenMessage("\n");
	}

	dest.toLowercase();

	bool found = false;
	for (unsigned p = 0; p < g_context->_location->_map->_portals.size(); p++) {
		MapId destid = g_context->_location->_map->_portals[p]->_destid;
		Common::String destNameLower = mapMgr->get(destid)->getName();
		destNameLower.toLowercase();

		if (destNameLower.find(dest) != Common::String::npos) {
			screenMessage("\n%s\n", mapMgr->get(destid)->getName().c_str());
			g_context->_location->_coords = g_context->_location->_map->_portals[p]->_coords;
			found = true;
			break;
		}
	}

	if (!found) {
		MapCoords coords = g_context->_location->_map->getLabel(dest);
		if (coords != MapCoords::nowhere) {
			print("%s", dest.c_str());
			g_context->_location->_coords = coords;
			found = true;
		}
	}

	if (found) {
		g_game->finishTurn();
		return false;
	} else {
		if (isActive())
			print("Can't find %s", dest.c_str());
		else
			print("Can't find\n%s", dest.c_str());

		return isActive();
	}
}

bool Debugger::cmdMoon(int argc, const char **argv) {
	int moonNum;

	if (argc == 2) {
		moonNum = strToInt(argv[1]);
		if (moonNum < 0 || moonNum > 7) {
			print("Invalid moon");
			return true;
		}
	} else {
		moonNum = (g_ultima->_saveGame->_trammelPhase + 1) & 7;
	}

	while (g_ultima->_saveGame->_trammelPhase != moonNum)
		g_game->updateMoons(true);
	g_game->finishTurn();

	print("Moons advanced");
	return isActive();
}

bool Debugger::cmdStats(int argc, const char **argv) {
	for (int i = 0; i < g_ultima->_saveGame->_members; i++) {
		g_ultima->_saveGame->_players[i]._str = 50;
		g_ultima->_saveGame->_players[i]._dex = 50;
		g_ultima->_saveGame->_players[i]._intel = 50;

		if (g_ultima->_saveGame->_players[i]._hpMax < 800) {
			g_ultima->_saveGame->_players[i]._xp = 9999;
			g_ultima->_saveGame->_players[i]._hpMax = 800;
			g_ultima->_saveGame->_players[i]._hp = 800;
		}
	}

	print("Full Stats given");
	return isActive();
}

} // End of namespace Ultima4
} // End of namespace Ultima
