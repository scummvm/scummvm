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

#include "ultima/ultima4/controllers/game_controller.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/death.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/ultima4.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

GameController *g_game = nullptr;

static const MouseArea MOUSE_AREAS[] = {
	{ 3, { { 8, 8 }, { 8, 184 }, { 96, 96 } }, MC_WEST, DIR_WEST },
	{ 3, { { 8, 8 }, { 184, 8 }, { 96, 96 } }, MC_NORTH, DIR_NORTH },
	{ 3, { { 184, 8 }, { 184, 184 }, { 96, 96 } }, MC_EAST, DIR_EAST },
	{ 3, { { 8, 184 }, { 184, 184 }, { 96, 96 } }, MC_SOUTH, DIR_SOUTH },
	{ 0, { { 0, 0 }, { 0, 0 }, { 0, 0 } }, MC_NORTH, DIR_NONE }
};

GameController::GameController() : _mapArea(BORDER_WIDTH, BORDER_HEIGHT, VIEWPORT_W, VIEWPORT_H), _paused(false), _pausedTimer(0) {
	g_game = this;
}

void GameController::initScreen() {
	Image *screen = imageMgr->get("screen")->_image;

	screen->fillRect(0, 0, screen->width(), screen->height(), 0, 0, 0);
	g_screen->update();
}

void GameController::initScreenWithoutReloadingState() {
	g_music->playMapMusic();
	imageMgr->get(BKGD_BORDERS)->_image->draw(0, 0);
	g_context->_stats->update(); /* draw the party stats */

	g_screen->screenMessage("Press Alt-h for help\n");
	g_screen->screenPrompt();

	eventHandler->pushMouseAreaSet(MOUSE_AREAS);

	eventHandler->setScreenUpdate(&gameUpdateScreen);
}

void GameController::init() {
	initScreen();

	// initialize the state of the global game context
	g_context->_line = TEXT_AREA_H - 1;
	g_context->_col = 0;
	g_context->_stats = new StatsArea();
	g_context->_moonPhase = 0;
	g_context->_windDirection = DIR_NORTH;
	g_context->_windCounter = 0;
	g_context->_windLock = false;
	g_context->_aura = new Aura();
	g_context->_horseSpeed = 0;
	g_context->_opacity = 1;
	g_context->_lastCommandTime = g_system->getMillis();
	g_context->_lastShip = nullptr;
}

void GameController::setMap(Map *map, bool saveLocation, const Portal *portal, TurnCompleter *turnCompleter) {
	int viewMode;
	LocationContext context;
	int activePlayer = g_context->_party->getActivePlayer();
	MapCoords coords;

	if (!turnCompleter)
		turnCompleter = this;

	if (portal)
		coords = portal->_start;
	else
		coords = MapCoords(map->_width / 2, map->_height / 2);

	/* If we don't want to save the location, then just return to the previous location,
	   as there may still be ones in the stack we want to keep */
	if (!saveLocation)
		exitToParentMap();

	switch (map->_type) {
	case Map::WORLD:
		context = CTX_WORLDMAP;
		viewMode = VIEW_NORMAL;
		break;
	case Map::DUNGEON:
		context = CTX_DUNGEON;
		viewMode = VIEW_DUNGEON;
		if (portal)
			g_ultima->_saveGame->_orientation = DIR_EAST;
		break;
	case Map::COMBAT:
		coords = MapCoords(-1, -1); /* set these to -1 just to be safe; we don't need them */
		context = CTX_COMBAT;
		viewMode = VIEW_NORMAL;
		activePlayer = -1; /* different active player for combat, defaults to 'None' */
		break;
	case Map::SHRINE:
		context = CTX_SHRINE;
		viewMode = VIEW_NORMAL;
		break;
	case Map::CITY:
	default:
		context = CTX_CITY;
		viewMode = VIEW_NORMAL;
		break;
	}
	g_context->_location = new Location(coords, map, viewMode, context, turnCompleter, g_context->_location);
	g_context->_location->addObserver(this);
	g_context->_party->setActivePlayer(activePlayer);
#ifdef IOS_ULTIMA4
	U4IOS::updateGameControllerContext(c->location->context);
#endif

	/* now, actually set our new tileset */
	_mapArea.setTileset(map->_tileSet);

	if (isCity(map)) {
		City *city = dynamic_cast<City *>(map);
		assert(city);
		city->addPeople();
	}
}

int GameController::exitToParentMap() {
	if (!g_context->_location)
		return 0;

	if (g_context->_location->_prev != nullptr) {
		// Create the balloon for Hythloth
		if (g_context->_location->_map->_id == MAP_HYTHLOTH)
			createBalloon(g_context->_location->_prev->_map);

		// free map info only if previous location was on a different map
		if (g_context->_location->_prev->_map != g_context->_location->_map) {
			g_context->_location->_map->_annotations->clear();
			g_context->_location->_map->clearObjects();

			/* quench the torch of we're on the world map */
			if (g_context->_location->_prev->_map->isWorldMap())
				g_context->_party->quenchTorch();
		}
		locationFree(&g_context->_location);

		// restore the tileset to the one the current map uses
		_mapArea.setTileset(g_context->_location->_map->_tileSet);
#ifdef IOS_ULTIMA4
		U4IOS::updateGameControllerContext(c->location->context);
#endif

		return 1;
	}
	return 0;
}

void GameController::finishTurn() {
	g_context->_lastCommandTime = g_system->getMillis();
	Creature *attacker = nullptr;

	while (1) {

		/* adjust food and moves */
		g_context->_party->endTurn();

		/* count down the aura, if there is one */
		g_context->_aura->passTurn();

		gameCheckHullIntegrity();

		/* update party stats */
		//c->stats->setView(STATS_PARTY_OVERVIEW);

		g_screen->screenUpdate(&this->_mapArea, true, false);
		g_screen->screenWait(1);

		/* Creatures cannot spawn, move or attack while the avatar is on the balloon */
		if (!g_context->_party->isFlying()) {

			// apply effects from tile avatar is standing on
			g_context->_party->applyEffect(g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS)->getEffect());

			// Move creatures and see if something is attacking the avatar
			attacker = g_context->_location->_map->moveObjects(g_context->_location->_coords);

			// Something's attacking!  Start combat!
			if (attacker) {
				gameCreatureAttack(attacker);
				return;
			}

			// cleanup old creatures and spawn new ones
			creatureCleanup();
			checkRandomCreatures();
			checkBridgeTrolls();
		}

		/* update map annotations */
		g_context->_location->_map->_annotations->passTurn();

		if (!g_context->_party->isImmobilized())
			break;

		if (g_context->_party->isDead()) {
			g_death->start(0);
			return;
		} else {
			g_screen->screenMessage("Zzzzzz\n");
			g_screen->screenWait(4);
		}
	}

	if (g_context->_location->_context == CTX_DUNGEON) {
		Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
		assert(dungeon);

		if (g_context->_party->getTorchDuration() <= 0)
			g_screen->screenMessage("It's Dark!\n");
		else
			g_context->_party->burnTorch();

		/* handle dungeon traps */
		if (dungeon->currentToken() == DUNGEON_TRAP) {
			dungeonHandleTrap((TrapType)dungeon->currentSubToken());
			// a little kludgey to have a second test for this
			// right here.  But without it you can survive an
			// extra turn after party death and do some things
			// that could cause a crash, like Hole up and Camp.
			if (g_context->_party->isDead()) {
				g_death->start(0);
				return;
			}
		}
	}


	/* draw a prompt */
	g_screen->screenPrompt();
	//g_screen->screenRedrawTextArea(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_W, TEXT_AREA_H);
}

void GameController::flashTile(const Coords &coords, MapTile tile, int frames) {
	g_context->_location->_map->_annotations->add(coords, tile, true);

	g_screen->screenTileUpdate(&g_game->_mapArea, coords);

	g_screen->screenWait(frames);
	g_context->_location->_map->_annotations->remove(coords, tile);

	g_screen->screenTileUpdate(&g_game->_mapArea, coords, false);
}

void GameController::flashTile(const Coords &coords, const Common::String &tilename, int timeFactor) {
	Tile *tile = g_context->_location->_map->_tileSet->getByName(tilename);
	assertMsg(tile, "no tile named '%s' found in tileset", tilename.c_str());
	flashTile(coords, tile->getId(), timeFactor);
}

void GameController::update(Party *party, PartyEvent &event) {
	int i;

	switch (event._type) {
	case PartyEvent::LOST_EIGHTH:
		// inform a player he has lost zero or more eighths of avatarhood.
		g_screen->screenMessage("\n %cThou hast lost\n  an eighth!%c\n", FG_YELLOW, FG_WHITE);
		break;
	case PartyEvent::ADVANCED_LEVEL:
		g_screen->screenMessage("\n%c%s\nThou art now Level %d%c\n", FG_YELLOW, event._player->getName().c_str(), event._player->getRealLevel(), FG_WHITE);
		gameSpellEffect('r', -1, SOUND_MAGIC); // Same as resurrect spell
		break;
	case PartyEvent::STARVING:
		g_screen->screenMessage("\n%cStarving!!!%c\n", FG_YELLOW, FG_WHITE);
		/* FIXME: add sound effect here */

		// 2 damage to each party member for starving!
		for (i = 0; i < g_ultima->_saveGame->_members; i++)
			g_context->_party->member(i)->applyDamage(2);
		break;
	default:
		break;
	}
}

void GameController::update(Location *location, MoveEvent &event) {
	switch (location->_map->_type) {
	case Map::DUNGEON:
		avatarMovedInDungeon(event);
		break;
	case Map::COMBAT: {
		// FIXME: let the combat controller handle it
		CombatController *ctl = dynamic_cast<CombatController *>(eventHandler->getController());
		assert(ctl);
		ctl->movePartyMember(event);
		break;
	}
	default:
		avatarMoved(event);
		break;
	}
}

void GameController::setActive() {
	// The game controller has the keybindings enabled
	MetaEngine::setKeybindingMode(KBMODE_NORMAL);
}

void GameController::keybinder(KeybindingAction action) {
	MetaEngine::executeAction(action);
}

bool GameController::mousePressed(const Common::Point &mousePos) {
	const MouseArea *area = eventHandler->mouseAreaForPoint(mousePos.x, mousePos.y);

	if (area) {
		keybinder(KEYBIND_INTERACT);
		return true;
	}

	return false;
}

void GameController::initMoons() {
	int trammelphase = g_ultima->_saveGame->_trammelPhase,
	    feluccaphase = g_ultima->_saveGame->_feluccaPhase;

	assertMsg(g_context != nullptr, "Game context doesn't exist!");
	assertMsg(g_ultima->_saveGame != nullptr, "Savegame doesn't exist!");
	//assertMsg(mapIsWorldMap(c->location->map) && c->location->viewMode == VIEW_NORMAL, "Can only call gameInitMoons() from the world map!");

	g_ultima->_saveGame->_trammelPhase = g_ultima->_saveGame->_feluccaPhase = 0;
	g_context->_moonPhase = 0;

	while ((g_ultima->_saveGame->_trammelPhase != trammelphase) ||
	        (g_ultima->_saveGame->_feluccaPhase != feluccaphase))
		updateMoons(false);
}

void GameController::updateMoons(bool showmoongates) {
	int realMoonPhase,
	    oldTrammel,
	    trammelSubphase;
	const Coords *gate;

	if (g_context->_location->_map->isWorldMap()) {
		oldTrammel = g_ultima->_saveGame->_trammelPhase;

		if (++g_context->_moonPhase >= MOON_PHASES * MOON_SECONDS_PER_PHASE * 4)
			g_context->_moonPhase = 0;

		trammelSubphase = g_context->_moonPhase % (MOON_SECONDS_PER_PHASE * 4 * 3);
		realMoonPhase = (g_context->_moonPhase / (4 * MOON_SECONDS_PER_PHASE));

		g_ultima->_saveGame->_trammelPhase = realMoonPhase / 3;
		g_ultima->_saveGame->_feluccaPhase = realMoonPhase % 8;

		if (g_ultima->_saveGame->_trammelPhase > 7)
			g_ultima->_saveGame->_trammelPhase = 7;

		if (showmoongates) {
			/* update the moongates if trammel changed */
			if (trammelSubphase == 0) {
				gate = g_moongates->getGateCoordsForPhase(oldTrammel);
				if (gate)
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate)
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
			} else if (trammelSubphase == 1) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
				}
			} else if (trammelSubphase == 2) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
				}
			} else if (trammelSubphase == 3) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
				}
			} else if ((trammelSubphase > 3) && (trammelSubphase < (MOON_SECONDS_PER_PHASE * 4 * 3) - 3)) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 3) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 2) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 1) {
				gate = g_moongates->getGateCoordsForPhase(g_ultima->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
				}
			}
		}
	}
}

void GameController::avatarMoved(MoveEvent &event) {
	if (event._userEvent) {

		// is filterMoveMessages even used?  it doesn't look like the option is hooked up in the configuration menu
		if (!settings._filterMoveMessages) {
			switch (g_context->_transportContext) {
			case TRANSPORT_FOOT:
			case TRANSPORT_HORSE:
				g_screen->screenMessage("%s\n", getDirectionName(event._dir));
				break;
			case TRANSPORT_SHIP:
				if (event._result & MOVE_TURNED)
					g_screen->screenMessage("Turn %s!\n", getDirectionName(event._dir));
				else if (event._result & MOVE_SLOWED)
					g_screen->screenMessage("%cSlow progress!%c\n", FG_GREY, FG_WHITE);
				else
					g_screen->screenMessage("Sail %s!\n", getDirectionName(event._dir));
				break;
			case TRANSPORT_BALLOON:
				g_screen->screenMessage("%cDrift Only!%c\n", FG_GREY, FG_WHITE);
				break;
			default:
				error("bad transportContext %d in avatarMoved()", g_context->_transportContext);
			}
		}

		/* movement was blocked */
		if (event._result & MOVE_BLOCKED) {

			/* if shortcuts are enabled, try them! */
			if (settings._shortcutCommands) {
				MapCoords new_coords = g_context->_location->_coords;
				MapTile *tile;

				new_coords.move(event._dir, g_context->_location->_map);
				tile = g_context->_location->_map->tileAt(new_coords, WITH_OBJECTS);

				if (tile->getTileType()->isDoor()) {
					g_debugger->openAt(new_coords);
					event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
				} else if (tile->getTileType()->isLockedDoor()) {
					g_debugger->jimmyAt(new_coords);
					event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
				} /*else if (mapPersonAt(c->location->map, new_coords) != nullptr) {
                    talkAtCoord(newx, newy, 1, nullptr);
                    event.result = MOVE_SUCCEEDED | MOVE_END_TURN;
                    }*/
			}

			/* if we're still blocked */
			if ((event._result & MOVE_BLOCKED) && !settings._filterMoveMessages) {
				soundPlay(SOUND_BLOCKED, false);
				g_screen->screenMessage("%cBlocked!%c\n", FG_GREY, FG_WHITE);
			}
		} else if (g_context->_transportContext == TRANSPORT_FOOT || g_context->_transportContext == TRANSPORT_HORSE) {
			/* movement was slowed */
			if (event._result & MOVE_SLOWED) {
				soundPlay(SOUND_WALK_SLOWED);
				g_screen->screenMessage("%cSlow progress!%c\n", FG_GREY, FG_WHITE);
			} else {
				soundPlay(SOUND_WALK_NORMAL);
			}
		}
	}

	/* exited map */
	if (event._result & MOVE_EXIT_TO_PARENT) {
		g_screen->screenMessage("%cLeaving...%c\n", FG_GREY, FG_WHITE);
		exitToParentMap();
		g_music->playMapMusic();
	}

	/* things that happen while not on board the balloon */
	if (g_context->_transportContext & ~TRANSPORT_BALLOON)
		checkSpecialCreatures(event._dir);
	/* things that happen while on foot or horseback */
	if ((g_context->_transportContext & TRANSPORT_FOOT_OR_HORSE) &&
	        !(event._result & (MOVE_SLOWED | MOVE_BLOCKED))) {
		if (checkMoongates())
			event._result = (MoveResult)(MOVE_MAP_CHANGE | MOVE_END_TURN);
	}
}

void GameController::avatarMovedInDungeon(MoveEvent &event) {
	Direction realDir = dirNormalize((Direction)g_ultima->_saveGame->_orientation, event._dir);
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	assert(dungeon);

	if (!settings._filterMoveMessages) {
		if (event._userEvent) {
			if (event._result & MOVE_TURNED) {
				if (dirRotateCCW((Direction)g_ultima->_saveGame->_orientation) == realDir)
					g_screen->screenMessage("Turn Left\n");
				else
					g_screen->screenMessage("Turn Right\n");
			} else {
				// Show 'Advance' or 'Retreat' in dungeons
				g_screen->screenMessage("%s\n", realDir == g_ultima->_saveGame->_orientation ? "Advance" : "Retreat");
			}
		}

		if (event._result & MOVE_BLOCKED)
			g_screen->screenMessage("%cBlocked!%c\n", FG_GREY, FG_WHITE);
	}

	// If we're exiting the map, do this
	if (event._result & MOVE_EXIT_TO_PARENT) {
		g_screen->screenMessage("%cLeaving...%c\n", FG_GREY, FG_WHITE);
		exitToParentMap();
		g_music->playMapMusic();
	}

	// Check to see if we're entering a dungeon room
	if (event._result & MOVE_SUCCEEDED) {
		if (dungeon->currentToken() == DUNGEON_ROOM) {
			int room = (int)dungeon->currentSubToken(); // Get room number

			/**
			 * recalculate room for the abyss -- there are 16 rooms for every 2 levels,
			 * each room marked with 0xD* where (* == room number 0-15).
			 * for levels 1 and 2, there are 16 rooms, levels 3 and 4 there are 16 rooms, etc.
			 */
			if (g_context->_location->_map->_id == MAP_ABYSS)
				room = (0x10 * (g_context->_location->_coords.z / 2)) + room;

			Dungeon *dng = dynamic_cast<Dungeon *>(g_context->_location->_map);
			assert(dng);
			dng->_currentRoom = room;

			// Set the map and start combat!
			CombatController *cc = new CombatController(dng->_roomMaps[room]);
			cc->initDungeonRoom(room, dirReverse(realDir));
			cc->begin();
		}
	}
}

void GameController::timerFired() {
	if (_pausedTimer > 0) {
		_pausedTimer--;
		if (_pausedTimer <= 0) {
			_pausedTimer = 0;
			_paused = false; /* unpause the game */
		}
	}

	if (!_paused && !_pausedTimer) {
		if (++g_context->_windCounter >= MOON_SECONDS_PER_PHASE * 4) {
			if (xu4_random(4) == 1 && !g_context->_windLock)
				g_context->_windDirection = dirRandomDir(MASK_DIR_ALL);
			g_context->_windCounter = 0;
		}

		/* balloon moves about 4 times per second */
		if ((g_context->_transportContext == TRANSPORT_BALLOON) &&
		        g_context->_party->isFlying()) {
			g_context->_location->move(dirReverse((Direction) g_context->_windDirection), false);
		}

		updateMoons(true);

		g_screen->screenCycle();

		// Check for any right-button mouse movement
		KeybindingAction action = eventHandler->getAction();
		if (action != KEYBIND_NONE)
			keybinder(action);

		// Do game udpates to the screen, like tile animations
		gameUpdateScreen();

		/*
		 * force pass if no commands within last 20 seconds
		 */
		Controller *controller = eventHandler->getController();
		if (controller != nullptr && (eventHandler->getController() == g_game ||
			dynamic_cast<CombatController *>(eventHandler->getController()) != nullptr) &&
			gameTimeSinceLastCommand() > 20) {

			/* pass the turn, and redraw the text area so the prompt is shown */
			MetaEngine::executeAction(KEYBIND_PASS);
			g_screen->screenRedrawTextArea(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_W, TEXT_AREA_H);
		}
	}

}

void GameController::checkSpecialCreatures(Direction dir) {
	int i;
	Object *obj;
	static const struct {
		int x, y;
		Direction dir;
	} pirateInfo[] = {
		{ 224, 220, DIR_EAST }, /* N'M" O'A" */
		{ 224, 228, DIR_EAST }, /* O'E" O'A" */
		{ 226, 220, DIR_EAST }, /* O'E" O'C" */
		{ 227, 228, DIR_EAST }, /* O'E" O'D" */
		{ 228, 227, DIR_SOUTH }, /* O'D" O'E" */
		{ 229, 225, DIR_SOUTH }, /* O'B" O'F" */
		{ 229, 223, DIR_NORTH }, /* N'P" O'F" */
		{ 228, 222, DIR_NORTH } /* N'O" O'E" */
	};

	/*
	 * if heading east into pirates cove (O'A" N'N"), generate pirate
	 * ships
	 */
	if (dir == DIR_EAST &&
	        g_context->_location->_coords.x == 0xdd &&
	        g_context->_location->_coords.y == 0xe0) {
		for (i = 0; i < 8; i++) {
			obj = g_context->_location->_map->addCreature(creatureMgr->getById(PIRATE_ID), MapCoords(pirateInfo[i].x, pirateInfo[i].y));
			obj->setDirection(pirateInfo[i].dir);
		}
	}

	/*
	 * if heading south towards the shrine of humility, generate
	 * daemons unless horn has been blown
	 */
	if (dir == DIR_SOUTH &&
	        g_context->_location->_coords.x >= 229 &&
	        g_context->_location->_coords.x < 234 &&
	        g_context->_location->_coords.y >= 212 &&
	        g_context->_location->_coords.y < 217 &&
	        *g_context->_aura != Aura::HORN) {
		for (i = 0; i < 8; i++)
			g_context->_location->_map->addCreature(creatureMgr->getById(DAEMON_ID), MapCoords(231, g_context->_location->_coords.y + 1, g_context->_location->_coords.z));
	}
}

bool GameController::checkMoongates() {
	Coords dest;

	if (g_moongates->findActiveGateAt(g_ultima->_saveGame->_trammelPhase, g_ultima->_saveGame->_feluccaPhase, g_context->_location->_coords, dest)) {

		gameSpellEffect(-1, -1, SOUND_MOONGATE); // Default spell effect (screen inversion without 'spell' sound effects)

		if (g_context->_location->_coords != dest) {
			g_context->_location->_coords = dest;
			gameSpellEffect(-1, -1, SOUND_MOONGATE); // Again, after arriving
		}

		if (g_moongates->isEntryToShrineOfSpirituality(g_ultima->_saveGame->_trammelPhase, g_ultima->_saveGame->_feluccaPhase)) {
			Shrine *shrine_spirituality;

			shrine_spirituality = dynamic_cast<Shrine *>(mapMgr->get(MAP_SHRINE_SPIRITUALITY));
			assert(shrine_spirituality);

			if (!g_context->_party->canEnterShrine(VIRT_SPIRITUALITY))
				return true;

			setMap(shrine_spirituality, 1, nullptr);
			g_music->playMapMusic();

			shrine_spirituality->enter();
		}

		return true;
	}

	return false;
}

void GameController::creatureCleanup() {
	ObjectDeque::iterator i;
	Map *map = g_context->_location->_map;

	for (i = map->_objects.begin(); i != map->_objects.end();) {
		Object *obj = *i;
		MapCoords o_coords = obj->getCoords();

		if ((obj->getType() == Object::CREATURE) && (o_coords.z == g_context->_location->_coords.z) &&
		        o_coords.distance(g_context->_location->_coords, g_context->_location->_map) > MAX_CREATURE_DISTANCE) {

			/* delete the object and remove it from the map */
			i = map->removeObject(i);
		} else {
			i++;
		}
	}
}

void GameController::checkRandomCreatures() {
	int canSpawnHere = g_context->_location->_map->isWorldMap() || g_context->_location->_context & CTX_DUNGEON;
#ifdef IOS_ULTIMA4
	int spawnDivisor = c->location->context & CTX_DUNGEON ? (53 - (c->location->coords.z << 2)) : 53;
#else
	int spawnDivisor = g_context->_location->_context & CTX_DUNGEON ? (32 - (g_context->_location->_coords.z << 2)) : 32;
#endif

	/* If there are too many creatures already,
	   or we're not on the world map, don't worry about it! */
	if (!canSpawnHere ||
	        g_context->_location->_map->getNumberOfCreatures() >= MAX_CREATURES_ON_MAP ||
	        xu4_random(spawnDivisor) != 0)
		return;

	// If combat is turned off, then don't spawn any creator
	if (g_debugger->_disableCombat)
		return;

	gameSpawnCreature(nullptr);
}

void GameController::checkBridgeTrolls() {
	const Tile *bridge = g_context->_location->_map->_tileSet->getByName("bridge");
	if (!bridge)
		return;

	// TODO: CHEST: Make a user option to not make chests block bridge trolls
	if (!g_context->_location->_map->isWorldMap() ||
	        g_context->_location->_map->tileAt(g_context->_location->_coords, WITH_OBJECTS)->_id != bridge->getId() ||
	        xu4_random(8) != 0)
		return;

	g_screen->screenMessage("\nBridge Trolls!\n");

	Creature *m = g_context->_location->_map->addCreature(creatureMgr->getById(TROLL_ID), g_context->_location->_coords);
	CombatController *cc = new CombatController(MAP_BRIDGE_CON);
	cc->init(m);
	cc->begin();
}

bool GameController::createBalloon(Map *map) {
	ObjectDeque::iterator i;

	/* see if the balloon has already been created (and not destroyed) */
	for (i = map->_objects.begin(); i != map->_objects.end(); i++) {
		Object *obj = *i;
		if (obj->getTile().getTileType()->isBalloon())
			return false;
	}

	const Tile *balloon = map->_tileSet->getByName("balloon");
	assertMsg(balloon, "no balloon tile found in tileset");
	map->addObject(balloon->getId(), balloon->getId(), map->getLabel("balloon"));
	return true;
}

void GameController::attack(Direction dir) {
	g_screen->screenMessage("Attack: ");
	if (g_context->_party->isFlying()) {
		g_screen->screenMessage("\n%cDrift only!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	if (dir == DIR_NONE)
		dir = gameGetDirection();

	if (dir == DIR_NONE) {
		g_screen->screenMessage("\n");
		return;
	}

	Std::vector<Coords> path = gameGetDirectionalActionPath(
		MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
		1, 1, nullptr, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (attackAt(*i))
			return;
	}

	g_screen->screenMessage("%cNothing to Attack!%c\n", FG_GREY, FG_WHITE);
}

bool GameController::attackAt(const Coords &coords) {
	Object *under;
	const Tile *ground;
	Creature *m;

	m = dynamic_cast<Creature *>(g_context->_location->_map->objectAt(coords));
	// Nothing attackable: move on to next tile
	if (m == nullptr || !m->isAttackable())
		return false;

	// Attack successful
	/// TODO: CHEST: Make a user option to not make chests change battlefield
	/// map (1 of 2)
	ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS);
	if (!ground->isChest()) {
		ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS);
		if ((under = g_context->_location->_map->objectAt(g_context->_location->_coords)) &&
			under->getTile().getTileType()->isShip())
			ground = under->getTile().getTileType();
	}

	// You're attacking a townsperson!  Alert the guards!
	if ((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR))
		g_context->_location->_map->alertGuards();

	// Not good karma to be killing the innocent.  Bad avatar!
	if (m->isGood() || /* attacking a good creature */
			/* attacking a docile (although possibly evil) person in town */
		((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR)))
		g_context->_party->adjustKarma(KA_ATTACKED_GOOD);

	CombatController *cc = new CombatController(CombatMap::mapForTile(ground, g_context->_party->getTransport().getTileType(), m));
	cc->init(m);
	cc->begin();
	return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
