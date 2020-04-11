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
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Debugger *g_debugger;

Debugger::Debugger() : Shared::Debugger(),
	_horse(g_context->_location->_map->_tileset->getByName("horse")->getId()),
	_ship(g_context->_location->_map->_tileset->getByName("ship")->getId()),
	_balloon(g_context->_location->_map->_tileset->getByName("balloon")->getId()) {
	g_debugger = this;
	_collisionOverride = false;

	registerCmd("collisions", WRAP_METHOD(Debugger, cmdCollisions));
	registerCmd("companions", WRAP_METHOD(Debugger, cmdCompanions));
	registerCmd("equipment", WRAP_METHOD(Debugger, cmdEquipment));
	registerCmd("exit", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("gate", WRAP_METHOD(Debugger, cmdGate));
	registerCmd("goto", WRAP_METHOD(Debugger, cmdGoto));
	registerCmd("items", WRAP_METHOD(Debugger, cmdItems));
	registerCmd("karma", WRAP_METHOD(Debugger, cmdKarma));
	registerCmd("location", WRAP_METHOD(Debugger, cmdLocation));
	registerCmd("mixtures", WRAP_METHOD(Debugger, cmdMixtures));
	registerCmd("moon", WRAP_METHOD(Debugger, cmdMoon));
	registerCmd("opacity", WRAP_METHOD(Debugger, cmdOpacity));
	registerCmd("peer", WRAP_METHOD(Debugger, cmdPeer));
	registerCmd("reagents", WRAP_METHOD(Debugger, cmdReagents));
	registerCmd("stats", WRAP_METHOD(Debugger, cmdStats));
	registerCmd("summon", WRAP_METHOD(Debugger, cmdSummon));
	registerCmd("up", WRAP_METHOD(Debugger, cmdUp));
	registerCmd("down", WRAP_METHOD(Debugger, cmdDown));
	registerCmd("virtue", WRAP_METHOD(Debugger, cmdVirtue));
	registerCmd("wind", WRAP_METHOD(Debugger, cmdWind));
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

void Debugger::summonCreature(const Common::String &name) {
	const Creature *m = NULL;
	Common::String creatureName = name;

	creatureName.trim();
	if (creatureName.empty()) {
		print("\n");
		return;
	}

	/* find the creature by its id and spawn it */
	unsigned int id = atoi(creatureName.c_str());
	if (id > 0)
		m = creatureMgr->getById(id);

	if (!m)
		m = creatureMgr->getByName(creatureName);

	if (m) {
		if (gameSpawnCreature(m))
			print("\n%s summoned!\n", m->getName().c_str());
		else
			print("\n\nNo place to put %s!\n\n", m->getName().c_str());

		return;
	}

	print("\n%s not found\n", creatureName.c_str());
}


bool Debugger::cmdCollisions(int argc, const char **argv) {
	_collisionOverride = !_collisionOverride;
	print("Collision detection %s",
		_collisionOverride ? "off" : "on");

	return isActive();
}

bool Debugger::cmdCompanions(int argc, const char **argv) {
	for (int m = g_ultima->_saveGame->_members; m < 8; m++) {
		debug("m = %d\n", m);
		debug("n = %s\n", g_ultima->_saveGame->_players[m].name);
		if (g_context->_party->canPersonJoin(g_ultima->_saveGame->_players[m].name, NULL)) {
			g_context->_party->join(g_ultima->_saveGame->_players[m].name);
		}
	}

	g_context->_stats->update();
	print("Joined by companions");
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

bool Debugger::cmdExit(int argc, const char **argv) {
	if (!g_game->exitToParentMap()) {
		print("Not Here");
	} else {
		g_music->play();
		print("Exited");
	}

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

bool Debugger::cmdItems(int argc, const char **argv) {
	SaveGame &sg = *g_ultima->_saveGame;
	sg._torches = 99;
	sg._gems = 99;
	sg._keys = 99;
	sg._sextants = 1;
	sg._items = ITEM_SKULL | ITEM_CANDLE | ITEM_BOOK | ITEM_BELL | ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T | ITEM_HORN | ITEM_WHEEL;
	sg._stones = 0xff;
	sg._runes = 0xff;
	sg._food = 999900;
	sg._gold = 9999;

	g_context->_stats->update();
	print("All items given");
	return isActive();
}

bool Debugger::cmdKarma(int argc, const char **argv) {
	print("Karma!\n");

	for (int i = 0; i < 8; ++i) {
		Common::String line = Common::String::format("%s:",
			getVirtueName(static_cast<Virtue>(i)));
		while (line.size() < 13)
			line += ' ';

		if (g_ultima->_saveGame->_karma[i] > 0)
			line += Common::String::format("%.2d", g_ultima->_saveGame->_karma[i]);
		else
			line += "--";
		print("%s", line.c_str());
	}

	return isActive();
}

bool Debugger::cmdLocation(int argc, const char **argv) {
	const MapCoords &pos = g_context->_location->_coords;

	if (isActive()) {
		if (g_context->_location->_map->isWorldMap())
			print("Location: %s x: %d, y: %d",
				"World Map", pos.x, pos.y);
		else
			print("Location: %s x: %d, y: %d, z: %d",
				g_context->_location->_map->getName().c_str(), pos.x, pos.y, pos.z);
	} else {
		if (g_context->_location->_map->isWorldMap())
			print("\nLocation:\n%s\nx: %d\ny: %d\n", "World Map",
				pos.x, pos.y);
		else
			print("\nLocation:\n%s\nx: %d\ny: %d\nz: %d\n",
				g_context->_location->_map->getName().c_str(), pos.x, pos.y, pos.z);
	}

	return isActive();
}

bool Debugger::cmdMixtures(int argc, const char **argv) {
	for (int i = 0; i < SPELL_MAX; i++)
		g_ultima->_saveGame->_mixtures[i] = 99;

	screenMessage("All mixtures given");
	return isActive();
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

bool Debugger::cmdOpacity(int argc, const char **argv) {
	g_context->_opacity = !g_context->_opacity;
	screenMessage("Opacity is %s", g_context->_opacity ? "on" : "off");
	return isActive();
}

bool Debugger::cmdPeer(int argc, const char **argv) {
	if ((g_context->_location->_viewMode == VIEW_NORMAL) || (g_context->_location->_viewMode == VIEW_DUNGEON))
		g_context->_location->_viewMode = VIEW_GEM;
	else if (g_context->_location->_context == CTX_DUNGEON)
		g_context->_location->_viewMode = VIEW_DUNGEON;
	else
		g_context->_location->_viewMode = VIEW_NORMAL;

	print("Toggle view");
	return isActive();
}

bool Debugger::cmdReagents(int argc, const char **argv) {
	for (int i = 0; i < REAG_MAX; i++)
		g_ultima->_saveGame->_reagents[i] = 99;

	print("Reagents given");
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

bool Debugger::cmdSummon(int argc, const char **argv) {
	Common::String creature;

	if (argc == 2) {
		creature = argv[1];
	} else if (isActive()) {
		print("summon <creature name>");
		return true;
	} else {
		screenMessage("Summon!\n");
		screenMessage("What?\n");
		creature = gameGetInput();
	}

	summonCreature(creature);
	return isActive();
}

bool Debugger::cmdTransport(int argc, const char **argv) {
	if (!g_context->_location->_map->isWorldMap()) {
		print("Not here!");
		return isActive();
	}

	MapCoords coords = g_context->_location->_coords;
	MapTile *choice;
	Tile *tile;

	screenMessage("Create transport!\nWhich? ");

	// Get the transport of choice
	char transport;
	if (argc == 2) {
		transport = argv[1][0];
	} else if (isActive()) {
		print("transport <transport name>");
		return isActive();
	} else {
		transport = ReadChoiceController::get("shb \033\015");
	}

	switch (transport) {
	case 's':
		choice = &_ship;
		break;
	case 'h':
		choice = &_horse;
		break;
	case 'b':
		choice = &_balloon;
		break;
	default:
		print("Unknown transport");
		return isActive();
	}

	if (choice) {
		ReadDirController readDir;
		tile = g_context->_location->_map->_tileset->get(choice->getId());

		screenMessage("%s\n", tile->getName().c_str());

		// Get the direction in which to create the transport
		eventHandler->pushController(&readDir);

		screenMessage("Dir: ");
		coords.move(readDir.waitFor(), g_context->_location->_map);
		if (coords != g_context->_location->_coords) {
			bool ok = false;
			MapTile *ground = g_context->_location->_map->tileAt(coords, WITHOUT_OBJECTS);

			screenMessage("%s\n", getDirectionName(readDir.getValue()));

			switch (transport) {
			case 's':
				ok = ground->getTileType()->isSailable();
				break;
			case 'h':
				ok = ground->getTileType()->isWalkable();
				break;
			case 'b':
				ok = ground->getTileType()->isWalkable();
				break;
			default:
				break;
			}

			if (choice && ok) {
				g_context->_location->_map->addObject(*choice, *choice, coords);
				print("%s created!", tile->getName().c_str());
			} else if (!choice) {
				print("Invalid transport!");
			} else {
				print("Can't place %s there!", tile->getName().c_str());
			}
		}
	}

	return isActive();
}

bool Debugger::cmdUp(int argc, const char **argv) {
	if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z > 0)) {
		g_context->_location->_coords.z--;
		g_game->finishTurn();

		return false;
	} else {
		print("Leaving...");
		g_game->exitToParentMap();
		g_music->play();

		return isActive();
	}
}

bool Debugger::cmdDown(int argc, const char **argv) {
	if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z < 7)) {
		g_context->_location->_coords.z++;
		return false;
	} else {
		print("Not here");
		return isActive();
	}
}

bool Debugger::cmdVirtue(int argc, const char **argv) {
	if (argc == 1) {
		for (int i = 0; i < 8; i++)
			g_ultima->_saveGame->_karma[i] = 0;

		g_context->_stats->update();
		screenMessage("Full virtues");
	} else {
		int virtue = strToInt(argv[1]);

		if (virtue <= 0 || virtue >= VIRT_MAX) {
			print("Invalid virtue");
		} else {
			print("Improved %s", getVirtueName((Virtue)virtue));

			if (g_ultima->_saveGame->_karma[virtue] == 99)
				g_ultima->_saveGame->_karma[virtue] = 0;
			else if (g_ultima->_saveGame->_karma[virtue] != 0)
				g_ultima->_saveGame->_karma[virtue] += 10;
			if (g_ultima->_saveGame->_karma[virtue] > 99)
				g_ultima->_saveGame->_karma[virtue] = 99;
			g_context->_stats->update();
		}
	}

	return isActive();
}

bool Debugger::cmdWind(int argc, const char **argv) {
	Common::String windDir;

	if (argc == 2) {
		windDir = argv[1];
	} else if (isActive()) {
		print("wind <direction or 'lock'>");
		return true;
	} else {
		print("Wind Dir ('l' to lock)");
		windDir = gameGetInput();
	}

	windDir.toLowercase();
	if (windDir == "lock" || windDir == "l") {
		g_context->_windLock = !g_context->_windLock;
		print("Wind direction is %slocked",
			g_context->_windLock ? "" : "un");
	} else if (windDir == "up" || windDir == "north") {
		g_context->_windDirection = DIR_NORTH;
	} else if (windDir == "down" || windDir == "south") {
		g_context->_windDirection = DIR_SOUTH;
	} else if (windDir == "right" || windDir == "east") {
		g_context->_windDirection = DIR_EAST;
	} else if (windDir == "left" || windDir == "west") {
		g_context->_windDirection = DIR_WEST;
	} else {
		print("Unknown direction");
		return isActive();
	}

	return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
