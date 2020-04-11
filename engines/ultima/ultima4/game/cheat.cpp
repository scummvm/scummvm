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

#include "ultima/ultima4/game/cheat.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/sound/music.h"
#include "common/debug.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima4 {

CheatMenuController::CheatMenuController(GameController *gc) : _game(gc) {
}

bool CheatMenuController::keyPressed(int key) {
	int i;
	bool valid = true;

	switch (key) {
	case 'h': {
		screenMessage("Help:\n"
		              "F1-F8 - +Virtue\n"
		              "i - Items\n"
		              "k - Show Karma\n"
		              "(more)");

		ReadChoiceController pauseController("");
		eventHandler->pushController(&pauseController);
		pauseController.waitFor();

		screenMessage("\n"
		              "l - Location\n"
		              "m - Mixtures\n"
		              "o - Opacity\n"
		              "p - Peer\n"
		              "r - Reagents\n"
		              "s - Summon\n"
		              "t - Transports\n"
		              "v - Full Virtues\n"
		              "w - Change Wind\n"
		              "x - Exit Map\n"
		              "y - Y-up\n"
		              "(more)");

		eventHandler->pushController(&pauseController);
		pauseController.waitFor();

		screenMessage("\n"
		              "z - Z-down\n"
		             );
		break;
	}

	case 'i':
		screenMessage("Items!\n");
		g_ultima->_saveGame->_torches = 99;
		g_ultima->_saveGame->_gems = 99;
		g_ultima->_saveGame->_keys = 99;
		g_ultima->_saveGame->_sextants = 1;
		g_ultima->_saveGame->_items = ITEM_SKULL | ITEM_CANDLE | ITEM_BOOK | ITEM_BELL | ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T | ITEM_HORN | ITEM_WHEEL;
		g_ultima->_saveGame->_stones = 0xff;
		g_ultima->_saveGame->_runes = 0xff;
		g_ultima->_saveGame->_food = 999900;
		g_ultima->_saveGame->_gold = 9999;
		g_context->_stats->update();
		break;

	case 'j':
		screenMessage("Joined by companions!\n");
		for (int m = g_ultima->_saveGame->_members; m < 8; m++) {
			debug("m = %d\n", m);
			debug("n = %s\n", g_ultima->_saveGame->_players[m].name);
			if (g_context->_party->canPersonJoin(g_ultima->_saveGame->_players[m].name, NULL)) {
				g_context->_party->join(g_ultima->_saveGame->_players[m].name);
			}
		}
		g_context->_stats->update();
		break;

	case 'k':
		screenMessage("Karma!\n\n");
		for (i = 0; i < 8; i++) {
			unsigned int j;
			screenMessage("%s:", getVirtueName(static_cast<Virtue>(i)));
			for (j = 13; j > strlen(getVirtueName(static_cast<Virtue>(i))); j--)
				screenMessage(" ");
			if (g_ultima->_saveGame->_karma[i] > 0)
				screenMessage("%.2d\n", g_ultima->_saveGame->_karma[i]);
			else screenMessage("--\n");
		}
		break;

	case 'l':
		if (g_context->_location->_map->isWorldMap())
			screenMessage("\nLocation:\n%s\nx: %d\ny: %d\n", "World Map", g_context->_location->_coords.x, g_context->_location->_coords.y);
		else
			screenMessage("\nLocation:\n%s\nx: %d\ny: %d\nz: %d\n", g_context->_location->_map->getName().c_str(), g_context->_location->_coords.x, g_context->_location->_coords.y, g_context->_location->_coords.z);
		break;

	case 'm':
		screenMessage("Mixtures!\n");
		for (i = 0; i < SPELL_MAX; i++)
			g_ultima->_saveGame->_mixtures[i] = 99;
		break;

	case 'o':
		g_context->_opacity = !g_context->_opacity;
		screenMessage("Opacity %s!\n", g_context->_opacity ? "on" : "off");
		break;

	case 'p':
		if ((g_context->_location->_viewMode == VIEW_NORMAL) || (g_context->_location->_viewMode == VIEW_DUNGEON))
			g_context->_location->_viewMode = VIEW_GEM;
		else if (g_context->_location->_context == CTX_DUNGEON)
			g_context->_location->_viewMode = VIEW_DUNGEON;
		else
			g_context->_location->_viewMode = VIEW_NORMAL;

		screenMessage("\nToggle View!\n");
		break;

	case 'r':
		screenMessage("Reagents!\n");
		for (i = 0; i < REAG_MAX; i++)
			g_ultima->_saveGame->_reagents[i] = 99;
		break;

	case 's':
		screenMessage("Summon!\n");
		screenMessage("What?\n");
		summonCreature(gameGetInput());
		break;

	case 't':
		if (g_context->_location->_map->isWorldMap()) {
			MapCoords coords = g_context->_location->_coords;
			static MapTile horse = g_context->_location->_map->_tileset->getByName("horse")->getId(),
			               ship = g_context->_location->_map->_tileset->getByName("ship")->getId(),
			               balloon = g_context->_location->_map->_tileset->getByName("balloon")->getId();
			MapTile *choice;
			Tile *tile;

			screenMessage("Create transport!\nWhich? ");

			// Get the transport of choice
			char transport = ReadChoiceController::get("shb \033\015");
			switch (transport) {
			case 's':
				choice = &ship;
				break;
			case 'h':
				choice = &horse;
				break;
			case 'b':
				choice = &balloon;
				break;
			default:
				choice = NULL;
				break;
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
						screenMessage("%s created!\n", tile->getName().c_str());
					} else if (!choice)
						screenMessage("Invalid transport!\n");
					else screenMessage("Can't place %s there!\n", tile->getName().c_str());
				}
			} else screenMessage("None!\n");
		}
		break;

	case 'v':
		screenMessage("\nFull Virtues!\n");
		for (i = 0; i < 8; i++)
			g_ultima->_saveGame->_karma[i] = 0;
		g_context->_stats->update();
		break;

	case 'w': {
		screenMessage("Wind Dir ('l' to lock):\n");
		WindCmdController ctrl;
		eventHandler->pushController(&ctrl);
		ctrl.waitFor();
		break;
	}

	case 'x':
		screenMessage("\nX-it!\n");
		if (!g_game->exitToParentMap())
			screenMessage("Not Here!\n");
		g_music->play();
		break;

	case 'y':
		screenMessage("Y-up!\n");
		if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z > 0))
			g_context->_location->_coords.z--;
		else {
			screenMessage("Leaving...\n");
			g_game->exitToParentMap();
			g_music->play();
		}
		break;

	case 'z':
		screenMessage("Z-down!\n");
		if ((g_context->_location->_context & CTX_DUNGEON) && (g_context->_location->_coords.z < 7))
			g_context->_location->_coords.z++;
		else screenMessage("Not Here!\n");
		break;

	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
	case Common::KEYCODE_F7:
	case Common::KEYCODE_F8:
		screenMessage("Improve %s!\n", getVirtueName(static_cast<Virtue>(key - Common::KEYCODE_F1)));
		if (g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] == 99)
			g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] = 0;
		else if (g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] != 0)
			g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] += 10;
		if (g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] > 99)
			g_ultima->_saveGame->_karma[key - Common::KEYCODE_F1] = 99;
		g_context->_stats->update();
		break;

	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_SPACE:
		screenMessage("Nothing\n");
		break;

	default:
		valid = false;
		break;
	}

	if (valid) {
		doneWaiting();
		screenPrompt();
	}

	return valid;
}

void CheatMenuController::summonCreature(const Common::String &name) {
	const Creature *m = NULL;
	Common::String creatureName = name;

	trim(creatureName);
	if (creatureName.empty()) {
		screenMessage("\n");
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
			screenMessage("\n%s summoned!\n", m->getName().c_str());
		else screenMessage("\n\nNo place to put %s!\n\n", m->getName().c_str());

		return;
	}

	screenMessage("\n%s not found\n", creatureName.c_str());
}

bool WindCmdController::keyPressed(int key) {
	switch (key) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
		g_context->_windDirection = keyToDirection(key);
		screenMessage("Wind %s!\n", getDirectionName(static_cast<Direction>(g_context->_windDirection)));
		doneWaiting();
		return true;

	case 'l':
		g_context->_windLock = !g_context->_windLock;
		screenMessage("Wind direction is %slocked!\n", g_context->_windLock ? "" : "un");
		doneWaiting();
		return true;
	}

	return KeyHandler::defaultHandler(key, NULL);
}

} // End of namespace Ultima4
} // End of namespace Ultima
