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
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/camp.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/map/dungeonview.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Debugger *g_debugger;

Debugger::Debugger() : Shared::Debugger() {
	g_debugger = this;
	_collisionOverride = false;
	_dontEndTurn = false;

	registerCmd("move", WRAP_METHOD(Debugger, cmdMove));
	registerCmd("attack", WRAP_METHOD(Debugger, cmdAttack));
	registerCmd("board", WRAP_METHOD(Debugger, cmdBoard));
	registerCmd("cast", WRAP_METHOD(Debugger, cmdCastSpell));
	registerCmd("enter", WRAP_METHOD(Debugger, cmdEnter));
	registerCmd("fire", WRAP_METHOD(Debugger, cmdFire));
	registerCmd("get", WRAP_METHOD(Debugger, cmdGet));
	registerCmd("hole", WRAP_METHOD(Debugger, cmdHoleUp));
	registerCmd("ignite", WRAP_METHOD(Debugger, cmdIgnite));
	registerCmd("jimmy", WRAP_METHOD(Debugger, cmdJimmy));

	registerCmd("pass", WRAP_METHOD(Debugger, cmdPass));

	registerCmd("3d", WRAP_METHOD(Debugger, cmd3d));
	registerCmd("collisions", WRAP_METHOD(Debugger, cmdCollisions));
	registerCmd("companions", WRAP_METHOD(Debugger, cmdCompanions));
	registerCmd("destroy", WRAP_METHOD(Debugger, cmdDestroy));
	registerCmd("dungeon", WRAP_METHOD(Debugger, cmdDungeon));
	registerCmd("equipment", WRAP_METHOD(Debugger, cmdEquipment));
	registerCmd("exit", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("gate", WRAP_METHOD(Debugger, cmdGate));
	registerCmd("goto", WRAP_METHOD(Debugger, cmdGoto));
	registerCmd("help", WRAP_METHOD(Debugger, cmdHelp));
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
	registerCmd("torch", WRAP_METHOD(Debugger, cmdTorch));
	registerCmd("transport", WRAP_METHOD(Debugger, cmdTransport));
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

	printN("%s\n", str.c_str());
}

void Debugger::printN(const char *fmt, ...) {
	// Format the string
	va_list va;
	va_start(va, fmt);
	Common::String str = Common::String::vformat(fmt, va);
	va_end(va);

	if (isDebuggerActive()) {
		// Strip off any color special characters that aren't
		// relevant for showing the text in the debugger
		Common::String s;
		for (Common::String::iterator it = str.begin(); it != str.end(); ++it) {
			if (*it <= ' ' && *it != '\n')
				s += *it;
		}

		debugPrintf("%s", s.c_str());
	} else {
		screenMessage("%s", str.c_str());
	}
}

bool Debugger::handleCommand(int argc, const char **argv, bool &keepRunning) {
	bool result = Shared::Debugger::handleCommand(argc, argv, keepRunning);

	if (result && !isActive()) {
		if (!_dontEndTurn)
			g_game->finishTurn();
	}

	_dontEndTurn = false;
	return result;
}

void Debugger::getChest(int player) {
	Common::String param = Common::String::format("%d", player);
	const char *argv[2] = { "get", param.c_str() };

	cmdGet(2, argv);
}



bool Debugger::cmdMove(int argc, const char **argv) {
	Direction dir;

	if (argc == 2) {
		dir = directionFromName(argv[1]);
	} else {
		print("move <direction>");
		return isDebuggerActive();
	}

	Common::String priorMap = g_context->_location->_map->_fname;
	MoveResult retval = g_context->_location->move(dir, true);

	// horse doubles speed (make sure we're on the same map as the previous move first)
	if (retval & (MOVE_SUCCEEDED | MOVE_SLOWED) &&
		(g_context->_transportContext == TRANSPORT_HORSE) && g_context->_horseSpeed) {
		// to give it a smooth look of movement
		gameUpdateScreen();
		if (priorMap == g_context->_location->_map->_fname)
			g_context->_location->move(dir, false);
	}

	// Let the movement handler decide to end the turn
	bool endTurn = (retval & MOVE_END_TURN);
	if (!endTurn)
		dontEndTurn();

	return false;
}

bool Debugger::cmdAttack(int argc, const char **argv) {
	Direction dir;

	if (argc != 2 && isDebuggerActive()) {
		print("attack <direction>");
		return true;
	}

	printN("Attack: ");
	if (g_context->_party->isFlying()) {
		screenMessage("\n%cDrift only!%c\n", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	if (argc == 2) {
		dir = directionFromName(argv[1]);
	} else {
		dir = gameGetDirection();
	}

	if (dir == DIR_NONE) {
		if (isDebuggerActive())
			print("");
		return isDebuggerActive();
	}

	Std::vector<Coords> path = gameGetDirectionalActionPath(
		MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
		1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (attackAt(*i))
			return isDebuggerActive();
	}

	print("%cNothing to Attack!%c", FG_GREY, FG_WHITE);
	return isDebuggerActive();
}

bool Debugger::cmdBoard(int argc, const char **argv) {
	if (g_context->_transportContext != TRANSPORT_FOOT) {
		print("Board: %cCan't!%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	Object *obj = g_context->_location->_map->objectAt(g_context->_location->_coords);
	if (!obj) {
		print("%cBoard What?%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	const Tile *tile = obj->getTile().getTileType();
	if (tile->isShip()) {
		print("Board Frigate!");
		if (g_context->_lastShip != obj)
			g_context->_party->setShipHull(50);
	} else if (tile->isHorse())
		print("Mount Horse!");
	else if (tile->isBalloon())
		print("Board Balloon!");
	else {
		print("%cBoard What?%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	g_context->_party->setTransport(obj->getTile());
	g_context->_location->_map->removeObject(obj);
	return isDebuggerActive();
}

bool Debugger::cmdCastSpell(int argc, const char **argv) {
	// TODO
	return isDebuggerActive();
}

bool Debugger::cmdEnter(int argc, const char **argv) {
	if (!usePortalAt(g_context->_location, g_context->_location->_coords, ACTION_ENTER)) {
		if (!g_context->_location->_map->portalAt(g_context->_location->_coords, ACTION_ENTER))
			print("%cEnter what?%c\n", FG_GREY, FG_WHITE);
	} else {
		dontEndTurn();
	}

	return isDebuggerActive();
}

bool Debugger::cmdFire(int argc, const char **argv) {
	if (g_context->_transportContext != TRANSPORT_SHIP) {
		print("%cFire What?%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	printN("Fire Cannon!\nDir: ");
	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return isDebuggerActive();

	// can only fire broadsides
	int broadsidesDirs = dirGetBroadsidesDirs(g_context->_party->getDirection());
	if (!DIR_IN_MASK(dir, broadsidesDirs)) {
		print("%cBroadsides Only!%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	// nothing (not even mountains!) can block cannonballs
	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), broadsidesDirs, g_context->_location->_coords,
		1, 3, NULL, false);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (fireAt(*i, true))
			return isDebuggerActive();
	}

	return isDebuggerActive();
}

bool Debugger::cmdGet(int argc, const char **argv) {
	int player = 1;
	if (argc == 2)
		player = strToInt(argv[1]);

	print("Get Chest!");

	if (g_context->_party->isFlying()) {
		print("%cDrift only!%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	// first check to see if a chest exists at the current location
	// if one exists, prompt the player for the opener, if necessary
	MapCoords coords;
	g_context->_location->getCurrentPosition(&coords);
	const Tile *tile = g_context->_location->_map->tileTypeAt(coords, WITH_GROUND_OBJECTS);

	/* get the object for the chest, if it is indeed an object */
	Object *obj = g_context->_location->_map->objectAt(coords);
	if (obj && !obj->getTile().getTileType()->isChest())
		obj = NULL;

	if (tile->isChest() || obj) {
		// if a spell was cast to open this chest,
		// player will equal -2, otherwise player
		// will default to -1 or the defult character
		// number if one was earlier specified
		if (player == -1) {
			printN("Who opens? ");
			player = gameGetPlayer(false, true);
		}
		if (player == -1)
			return isDebuggerActive();

		if (obj)
			g_context->_location->_map->removeObject(obj);
		else {
			TileId newTile = g_context->_location->getReplacementTile(coords, tile);
			g_context->_location->_map->_annotations->add(coords, newTile, false, true);
		}

		// see if the chest is trapped and handle it
		getChestTrapHandler(player);

		print("The Chest Holds: %d Gold", g_context->_party->getChest());

		screenPrompt();

		if (isCity(g_context->_location->_map) && obj == NULL)
			g_context->_party->adjustKarma(KA_STOLE_CHEST);
	} else {
		print("%cNot Here!%c", FG_GREY, FG_WHITE);
	}

	return isDebuggerActive();
}

bool Debugger::cmdHoleUp(int argc, const char **argv) {
	print("Hole up & Camp!");

	if (!(g_context->_location->_context & (CTX_WORLDMAP | CTX_DUNGEON))) {
		print("%cNot here!%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	if (g_context->_transportContext != TRANSPORT_FOOT) {
		print("%cOnly on foot!%c", FG_GREY, FG_WHITE);
		return isDebuggerActive();
	}

	CombatController *cc = new CampController();
	cc->init(NULL);
	cc->begin();

	return isDebuggerActive();
}

bool Debugger::cmdIgnite(int argc, const char **argv) {
	print("Ignite torch!");
	if (g_context->_location->_context == CTX_DUNGEON) {
		if (!g_context->_party->lightTorch())
			print("%cNone left!%c", FG_GREY, FG_WHITE);
	} else {
		print("%cNot here!%c", FG_GREY, FG_WHITE);
	}

	return isDebuggerActive();
}

bool Debugger::cmdJimmy(int argc, const char **argv) {
	screenMessage("Jimmy: ");
	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return isDebuggerActive();

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
		1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (jimmyAt(*i))
			return isDebuggerActive();
	}

	print("%cJimmy what?%c", FG_GREY, FG_WHITE);
	return isDebuggerActive();
}

bool Debugger::cmdPass(int argc, const char **argv) {
	print("Pass");
	return isDebuggerActive();
}


bool Debugger::cmd3d(int argc, const char **argv) {
	if (g_context->_location->_context == CTX_DUNGEON) {
		print("3-D view %s\n", DungeonViewer.toggle3DDungeonView() ? "on" : "off");
	} else {
		print("Not here");
	}

	return isDebuggerActive();
}

bool Debugger::cmdCollisions(int argc, const char **argv) {
	_collisionOverride = !_collisionOverride;
	print("Collision detection %s",
		_collisionOverride ? "off" : "on");

	return isDebuggerActive();
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
	return isDebuggerActive();
}

bool Debugger::cmdDestroy(int argc, const char **argv) {
	Direction dir;

	if (argc == 2) {
		dir = directionFromName(argv[1]);
	} else if (isDebuggerActive()) {
		print("destroy <direction>");
		return isDebuggerActive();
	} else {
		screenMessage("Destroy Object\nDir: ");
		dir = gameGetDirection();
	}

	if (dir == DIR_NONE)
		return isDebuggerActive();

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir),
		MASK_DIR_ALL, g_context->_location->_coords, 1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (destroyAt(*i)) {
			return false;
		}
	}

	print("%cNothing there!%c\n", FG_GREY, FG_WHITE);
	return isDebuggerActive();
}

bool Debugger::cmdDungeon(int argc, const char **argv) {
	if (g_context->_location->_context & CTX_WORLDMAP) {
		if (argc == 2) {
			int dungNum = strToInt(argv[1]);

			if (dungNum >= 1 && dungNum <= 8) {
				g_context->_location->_coords = g_context->_location->_map->_portals[dungNum - 1]->_coords;
				return false;
			} else if (dungNum == 9) {
				g_game->setMap(mapMgr->get(MAP_DECEIT), 1, NULL);
				g_context->_location->_coords = MapCoords(1, 0, 7);
				g_ultima->_saveGame->_orientation = DIR_SOUTH;
			} else if (dungNum == 10) {
				g_game->setMap(mapMgr->get(MAP_DESPISE), 1, NULL);
				g_context->_location->_coords = MapCoords(3, 2, 7);
				g_ultima->_saveGame->_orientation = DIR_SOUTH;
			} else if (dungNum == 11) {
				g_game->setMap(mapMgr->get(MAP_DESTARD), 1, NULL);
				g_context->_location->_coords = MapCoords(7, 6, 7);
				g_ultima->_saveGame->_orientation = DIR_SOUTH;
			} else {
				print("Invalid dungeon");
				return isDebuggerActive();
			}

			return false;
		} else {
			print("dungeon <number>");
		}
	} else {
		print("Not here");
	}

	return isDebuggerActive();
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
	return isDebuggerActive();
}

bool Debugger::cmdExit(int argc, const char **argv) {
	if (!g_game->exitToParentMap()) {
		print("Not Here");
	} else {
		g_music->play();
		print("Exited");
	}

	return isDebuggerActive();
}

bool Debugger::cmdGate(int argc, const char **argv) {
	int gateNum = (argc == 2) ? strToInt(argv[1]) : -1;

	if (!g_context || !g_game || gateNum < 1 || gateNum > 8) {
		print("Gate <1 to 8>");
	} else {
		if (!isDebuggerActive())
			print("Gate %d!", gateNum);

		if (g_context->_location->_map->isWorldMap()) {
			const Coords *moongate = moongateGetGateCoordsForPhase(gateNum - 1);
			if (moongate) {
				g_context->_location->_coords = *moongate;
				return false;
			}
		} else {
			print("Not here!");
		}
	}

	return isDebuggerActive();
}

bool Debugger::cmdGoto(int argc, const char **argv) {
	Common::String dest;

	if (argc == 2) {
		dest = argv[1];
	} else if (isDebuggerActive()) {
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
		return false;
	} else {
		if (isDebuggerActive())
			print("Can't find %s", dest.c_str());
		else
			print("Can't find\n%s", dest.c_str());

		return isDebuggerActive();
	}
}

bool Debugger::cmdHelp(int argc, const char **argv) {
	if (!isDebuggerActive()) {
		screenMessage("Help!\n");
		screenPrompt();
	}

	/* Help! send me to Lord British (who conveniently is right around where you are)! */
	g_game->setMap(mapMgr->get(100), 1, NULL);
	g_context->_location->_coords.x = 19;
	g_context->_location->_coords.y = 8;
	g_context->_location->_coords.z = 0;

	return false;
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
	return isDebuggerActive();
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

	return isDebuggerActive();
}

bool Debugger::cmdLocation(int argc, const char **argv) {
	const MapCoords &pos = g_context->_location->_coords;

	if (isDebuggerActive()) {
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

	return isDebuggerActive();
}

bool Debugger::cmdMixtures(int argc, const char **argv) {
	for (int i = 0; i < SPELL_MAX; i++)
		g_ultima->_saveGame->_mixtures[i] = 99;

	screenMessage("All mixtures given");
	return isDebuggerActive();
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

	print("Moons advanced");
	return isDebuggerActive();
}

bool Debugger::cmdOpacity(int argc, const char **argv) {
	g_context->_opacity = !g_context->_opacity;
	screenMessage("Opacity is %s", g_context->_opacity ? "on" : "off");
	return isDebuggerActive();
}

bool Debugger::cmdPeer(int argc, const char **argv) {
	if ((g_context->_location->_viewMode == VIEW_NORMAL) || (g_context->_location->_viewMode == VIEW_DUNGEON))
		g_context->_location->_viewMode = VIEW_GEM;
	else if (g_context->_location->_context == CTX_DUNGEON)
		g_context->_location->_viewMode = VIEW_DUNGEON;
	else
		g_context->_location->_viewMode = VIEW_NORMAL;

	print("Toggle view");
	return isDebuggerActive();
}

bool Debugger::cmdReagents(int argc, const char **argv) {
	for (int i = 0; i < REAG_MAX; i++)
		g_ultima->_saveGame->_reagents[i] = 99;

	print("Reagents given");
	return isDebuggerActive();
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
	return isDebuggerActive();
}

bool Debugger::cmdSummon(int argc, const char **argv) {
	Common::String creature;

	if (argc == 2) {
		creature = argv[1];
	} else if (isDebuggerActive()) {
		print("summon <creature name>");
		return true;
	} else {
		screenMessage("Summon!\n");
		screenMessage("What?\n");
		creature = gameGetInput();
	}

	summonCreature(creature);
	return isDebuggerActive();
}

bool Debugger::cmdTorch(int argc, const char **argv) {
	print("Torch: %d\n", g_context->_party->getTorchDuration());
	if (!isDebuggerActive())
		screenPrompt();

	return isDebuggerActive();
}

bool Debugger::cmdTransport(int argc, const char **argv) {
	if (!g_context->_location->_map->isWorldMap()) {
		print("Not here!");
		return isDebuggerActive();
	}

	_horse = g_context->_location->_map->_tileset->getByName("horse")->getId();
	_ship = g_context->_location->_map->_tileset->getByName("ship")->getId();
	_balloon = g_context->_location->_map->_tileset->getByName("balloon")->getId();

	MapCoords coords = g_context->_location->_coords;
	MapTile *choice;
	Tile *tile;

	// Get the transport of choice
	char transport;
	if (argc >= 2) {
		transport = argv[1][0];
	} else if (isDebuggerActive()) {
		print("transport <transport name>");
		return isDebuggerActive();
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
		return isDebuggerActive();
	}

	tile = g_context->_location->_map->_tileset->get(choice->getId());
	Direction dir;

	if (argc == 3) {
		dir = directionFromName(argv[2]);
	} else if (isDebuggerActive()) {
		dir = DIR_NONE;
	} else {
		screenMessage("%s\n", tile->getName().c_str());

		// Get the direction in which to create the transport
		ReadDirController readDir;
		eventHandler->pushController(&readDir);

		screenMessage("Dir: ");
		dir = readDir.waitFor();
	}

	coords.move(dir, g_context->_location->_map);

	if (coords != g_context->_location->_coords) {
		bool ok = false;
		MapTile *ground = g_context->_location->_map->tileAt(coords, WITHOUT_OBJECTS);

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

	return isDebuggerActive();
}

bool Debugger::cmdUp(int argc, const char **argv) {
	if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z > 0)) {
		g_context->_location->_coords.z--;

		return false;
	} else {
		print("Leaving...");
		g_game->exitToParentMap();
		g_music->play();

		return isDebuggerActive();
	}
}

bool Debugger::cmdDown(int argc, const char **argv) {
	if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z < 7)) {
		g_context->_location->_coords.z++;
		return false;
	} else {
		print("Not here");
		return isDebuggerActive();
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

	return isDebuggerActive();
}

bool Debugger::cmdWind(int argc, const char **argv) {
	Common::String windDir;

	if (argc == 2) {
		windDir = argv[1];
	} else if (isDebuggerActive()) {
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
	} else {
		Direction dir = directionFromName(windDir);

		if (dir == DIR_NONE) {
			print("Unknown direction");
			return isDebuggerActive();
		} else {
			g_context->_windDirection = dir;
		}
	}

	return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
