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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/controllers/camp_controller.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/controllers/intro_controller.h"
#include "ultima/ultima4/controllers/read_choice_controller.h"
#include "ultima/ultima4/controllers/read_dir_controller.h"
#include "ultima/ultima4/controllers/read_int_controller.h"
#include "ultima/ultima4/controllers/read_player_controller.h"
#include "ultima/ultima4/controllers/read_string_controller.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/death.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/game/script.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/views/dungeonview.h"
#include "ultima/ultima4/meta_engine.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

/*-----------------*/
/* Functions BEGIN */

/* main game functions */
void gameAdvanceLevel(PartyMember *player);
void gameInnHandler();
void gameLostEighth(Virtue virtue);
void gamePartyStarving();

void mixReagentsSuper();

/* action functions */
void wearArmor(int player = -1);

/* Functions END */
/*---------------*/

void gameSetViewMode(ViewMode newMode) {
	g_context->_location->_viewMode = newMode;
}

void gameUpdateScreen() {
	switch (g_context->_location->_viewMode) {
	case VIEW_NORMAL:
		g_screen->screenUpdate(&g_game->_mapArea, true, false);
		break;
	case VIEW_GEM:
		g_screen->screenGemUpdate();
		break;
	case VIEW_RUNE:
		g_screen->screenUpdate(&g_game->_mapArea, false, false);
		break;
	case VIEW_DUNGEON:
		g_screen->screenUpdate(&g_game->_mapArea, true, false);
		break;
	case VIEW_DEAD:
		g_screen->screenUpdate(&g_game->_mapArea, true, true);
		break;
	case VIEW_CODEX: /* the screen updates will be handled elsewhere */
		break;
	case VIEW_MIXTURES: /* still testing */
		break;
	default:
		error("invalid view mode: %d", g_context->_location->_viewMode);
	}
}

void gameSpellEffect(int spell, int player, Sound sound) {
	int time;
	Spell::SpecialEffects effect = Spell::SFX_INVERT;

	if (player >= 0)
		g_context->_stats->highlightPlayer(player);

	time = settings._spellEffectSpeed * 800 / settings._gameCyclesPerSecond;
	soundPlay(sound, false, time);

	///The following effect multipliers are not accurate
	switch (spell) {
	case 'g': /* gate */
	case 'r': /* resurrection */
		break;
	case 't': /* tremor */
		effect = Spell::SFX_TREMOR;
		break;
	default:
		/* default spell effect */
		break;
	}

	switch (effect) {
	case Spell::SFX_TREMOR:
	case Spell::SFX_INVERT:
		gameUpdateScreen();
		g_game->_mapArea.highlight(0, 0, VIEWPORT_W * TILE_WIDTH, VIEWPORT_H * TILE_HEIGHT);
		g_screen->update();
		EventHandler::sleep(time);
		g_game->_mapArea.unhighlight();
		g_screen->update();

		if (effect == Spell::SFX_TREMOR) {
			gameUpdateScreen();
			g_screen->update();
			soundPlay(SOUND_RUMBLE, false);
			g_screen->screenShake(8);
		}

		break;
	default:
		break;
	}
}

Common::String gameGetInput(int maxlen) {
	g_screen->screenEnableCursor();
	g_screen->screenShowCursor();
#ifdef IOS_ULTIMA4
	U4IOS::IOSConversationHelper helper;
	helper.beginConversation(U4IOS::UIKeyboardTypeDefault);
#endif

	return ReadStringController::get(maxlen, TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
}

int gameGetPlayer(bool canBeDisabled, bool canBeActivePlayer) {
	int player;
	if (g_ultima->_saveGame->_members <= 1) {
		player = 0;
	} else {
		if (canBeActivePlayer && (g_context->_party->getActivePlayer() >= 0)) {
			player = g_context->_party->getActivePlayer();
		} else {
			ReadPlayerController readPlayerController;
			eventHandler->pushController(&readPlayerController);
			player = readPlayerController.waitFor();
		}

		if (player == -1) {
			g_screen->screenMessage("None\n");
			return -1;
		}
	}

	g_context->_col--;// display the selected character name, in place of the number
	if ((player >= 0) && (player < 8)) {
		g_screen->screenMessage("%s\n", g_ultima->_saveGame->_players[player]._name); //Write player's name after prompt
	}

	if (!canBeDisabled && g_context->_party->member(player)->isDisabled()) {
		g_screen->screenMessage("%cDisabled!%c\n", FG_GREY, FG_WHITE);
		return -1;
	}

	assertMsg(player < g_context->_party->size(), "player %d, but only %d members\n", player, g_context->_party->size());
	return player;
}

Direction gameGetDirection() {
	ReadDirController dirController;

	g_screen->screenMessage("Dir?");
#ifdef IOS_ULTIMA4
	U4IOS::IOSDirectionHelper directionPopup;
#endif

	eventHandler->pushController(&dirController);
	Direction dir = dirController.waitFor();

	g_screen->screenMessage("\b\b\b\b");

	if (dir == DIR_NONE) {
		g_screen->screenMessage("    \n");
		return dir;
	} else {
		g_screen->screenMessage("%s\n", getDirectionName(dir));
		return dir;
	}
}

bool fireAt(const Coords &coords, bool originAvatar) {
	bool validObject = false;
	bool hitsAvatar = false;
	bool objectHit = false;

	Object *obj = nullptr;


	MapTile tile(g_context->_location->_map->_tileSet->getByName("miss_flash")->getId());
	GameController::flashTile(coords, tile, 1);

	obj = g_context->_location->_map->objectAt(coords);
	Creature *m = dynamic_cast<Creature *>(obj);

	if (obj && obj->getType() == Object::CREATURE && m && m->isAttackable())
		validObject = true;
	/* See if it's an object to be destroyed (the avatar cannot destroy the balloon) */
	else if (obj &&
	         (obj->getType() == Object::UNKNOWN) &&
	         !(obj->getTile().getTileType()->isBalloon() && originAvatar))
		validObject = true;

	/* Does the cannon hit the avatar? */
	if (coords == g_context->_location->_coords) {
		validObject = true;
		hitsAvatar = true;
	}

	if (validObject) {
		/* always displays as a 'hit' though the object may not be destroyed */

		/* Is is a pirate ship firing at US? */
		if (hitsAvatar) {
			GameController::flashTile(coords, "hit_flash", 4);

			if (g_context->_transportContext == TRANSPORT_SHIP)
				gameDamageShip(-1, 10);
			else gameDamageParty(10, 25); /* party gets hurt between 10-25 damage */
		}
		/* inanimate objects get destroyed instantly, while creatures get a chance */
		else if (obj->getType() == Object::UNKNOWN) {
			GameController::flashTile(coords, "hit_flash", 4);
			g_context->_location->_map->removeObject(obj);
		}

		/* only the avatar can hurt other creatures with cannon fire */
		else if (originAvatar) {
			GameController::flashTile(coords, "hit_flash", 4);
			if (xu4_random(4) == 0) /* reverse-engineered from u4dos */
				g_context->_location->_map->removeObject(obj);
		}

		objectHit = true;
	}

	return objectHit;
}

bool gamePeerCity(int city, void *data) {
	Map *peerMap;

	peerMap = mapMgr->get((MapId)(city + 1));

	if (peerMap != nullptr) {
		g_game->setMap(peerMap, 1, nullptr);
		g_context->_location->_viewMode = VIEW_GEM;
		g_game->_paused = true;
		g_game->_pausedTimer = 0;

		g_screen->screenDisableCursor();
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationChoiceHelper continueHelper;
		continueHelper.updateChoices(" ");
		continueHelper.fullSizeChoicePanel();
#endif
		ReadChoiceController::get("\015 \033");

		g_game->exitToParentMap();
		g_screen->screenEnableCursor();
		g_game->_paused = false;

		return true;
	}
	return false;
}

void peer(bool useGem) {
	if (useGem) {
		if (g_ultima->_saveGame->_gems <= 0) {
			g_screen->screenMessage("%cPeer at What?%c\n", FG_GREY, FG_WHITE);
			return;
		}

		g_ultima->_saveGame->_gems--;
		g_screen->screenMessage("Peer at a Gem!\n");
	}

	g_game->_paused = true;
	g_game->_pausedTimer = 0;
	g_screen->screenDisableCursor();

	g_context->_location->_viewMode = VIEW_GEM;
#ifdef IOS_ULTIMA4
	U4IOS::IOSConversationChoiceHelper continueHelper;
	continueHelper.updateChoices(" ");
	continueHelper.fullSizeChoicePanel();
#endif
	ReadChoiceController::get("\015 \033");

	g_screen->screenEnableCursor();
	g_context->_location->_viewMode = VIEW_NORMAL;
	g_game->_paused = false;
}

void gameCheckHullIntegrity() {
	int i;

	bool killAll = false;
	/* see if the ship has sunk */
	if ((g_context->_transportContext == TRANSPORT_SHIP) && g_ultima->_saveGame->_shipHull <= 0) {
		g_screen->screenMessage("\nThy ship sinks!\n\n");
		killAll = true;
	}


	if (!g_debugger->_collisionOverride && g_context->_transportContext == TRANSPORT_FOOT &&
	        g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS)->isSailable() &&
	        !g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS)->isShip() &&
	        !g_context->_location->_map->getValidMoves(g_context->_location->_coords, g_context->_party->getTransport())) {
		g_screen->screenMessage("\nTrapped at sea without thy ship, thou dost drown!\n\n");
		killAll = true;
	}

	if (killAll) {
		for (i = 0; i < g_context->_party->size(); i++) {
			g_context->_party->member(i)->setHp(0);
			g_context->_party->member(i)->setStatus(STAT_DEAD);
		}

		g_screen->update();
		g_death->start(5);
	}
}

void gameFixupObjects(Map *map) {
	int i;
	Object *obj;

	/* add stuff from the monster table to the map */
	for (i = 0; i < MONSTERTABLE_SIZE; i++) {
		SaveGameMonsterRecord *monster = &map->_monsterTable[i];
		if (monster->_prevTile != 0) {
			Coords coords(monster->_x, monster->_y);

			// tile values stored in monsters.sav hardcoded to index into base tilemap
			MapTile tile = g_tileMaps->get("base")->translate(monster->_tile),
			        oldTile = g_tileMaps->get("base")->translate(monster->_prevTile);

			if (i < MONSTERTABLE_CREATURES_SIZE) {
				const Creature *creature = creatureMgr->getByTile(tile);
				/* make sure we really have a creature */
				if (creature)
					obj = map->addCreature(creature, coords);
				else {
					warning("A non-creature object was found in the creature section of the monster table. (Tile: %s)\n", tile.getTileType()->getName().c_str());
					obj = map->addObject(tile, oldTile, coords);
				}
			} else {
				obj = map->addObject(tile, oldTile, coords);
			}

			/* set the map for our object */
			obj->setMap(map);
		}
	}
}

uint32 gameTimeSinceLastCommand() {
	return (g_system->getMillis() - g_context->_lastCommandTime) / 1000;
}

void gameCreatureAttack(Creature *m) {
	Object *under;
	const Tile *ground;

	g_screen->screenMessage("\nAttacked by %s\n", m->getName().c_str());

	/// TODO: CHEST: Make a user option to not make chests change battlefield
	/// map (2 of 2)
	ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS);
	if (!ground->isChest()) {
		ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS);
		if ((under = g_context->_location->_map->objectAt(g_context->_location->_coords)) &&
		        under->getTile().getTileType()->isShip())
			ground = under->getTile().getTileType();
	}

	CombatController *cc = new CombatController(CombatMap::mapForTile(ground, g_context->_party->getTransport().getTileType(), m));
	cc->init(m);
	cc->begin();
}

bool creatureRangeAttack(const Coords &coords, Creature *m) {
//    int attackdelay = MAX_BATTLE_SPEED - settings.battleSpeed;

	// Figure out what the ranged attack should look like
	MapTile tile(g_context->_location->_map->_tileSet->getByName((m && !m->getWorldrangedtile().empty()) ?
	             m->getWorldrangedtile() :
	             "hit_flash")->getId());

	GameController::flashTile(coords, tile, 1);

	// See if the attack hits the avatar
	Object *obj = g_context->_location->_map->objectAt(coords);
	m = dynamic_cast<Creature *>(obj);

	// Does the attack hit the avatar?
	if (coords == g_context->_location->_coords) {
		/* always displays as a 'hit' */
		GameController::flashTile(coords, tile, 3);

		/* FIXME: check actual damage from u4dos -- values here are guessed */
		if (g_context->_transportContext == TRANSPORT_SHIP)
			gameDamageShip(-1, 10);
		else
			gameDamageParty(10, 25);

		return true;
	}
	// Destroy objects that were hit
	else if (obj) {
		if ((obj->getType() == Object::CREATURE && m && m->isAttackable()) ||
		        obj->getType() == Object::UNKNOWN) {

			GameController::flashTile(coords, tile, 3);
			g_context->_location->_map->removeObject(obj);

			return true;
		}
	}
	return false;
}

Std::vector<Coords> gameGetDirectionalActionPath(int dirmask, int validDirections, const Coords &origin, int minDistance, int maxDistance, bool (*blockedPredicate)(const Tile *tile), bool includeBlocked) {
	Std::vector<Coords> path;
	Direction dirx = DIR_NONE,
	          diry = DIR_NONE;

	/* Figure out which direction the action is going */
	if (DIR_IN_MASK(DIR_WEST, dirmask))
		dirx = DIR_WEST;
	else if (DIR_IN_MASK(DIR_EAST, dirmask))
		dirx = DIR_EAST;
	if (DIR_IN_MASK(DIR_NORTH, dirmask))
		diry = DIR_NORTH;
	else if (DIR_IN_MASK(DIR_SOUTH, dirmask))
		diry = DIR_SOUTH;

	/*
	 * try every tile in the given direction, up to the given range.
	 * Stop when the the range is exceeded, or the action is blocked.
	 */

	MapCoords t_c(origin);
	if ((dirx <= 0 || DIR_IN_MASK(dirx, validDirections)) &&
	        (diry <= 0 || DIR_IN_MASK(diry, validDirections))) {
		for (int distance = 0; distance <= maxDistance;
		        distance++, t_c.move(dirx, g_context->_location->_map), t_c.move(diry, g_context->_location->_map)) {

			if (distance >= minDistance) {
				/* make sure our action isn't taking us off the map */
				if (MAP_IS_OOB(g_context->_location->_map, t_c))
					break;

				const Tile *tile = g_context->_location->_map->tileTypeAt(t_c, WITH_GROUND_OBJECTS);

				/* should we see if the action is blocked before trying it? */
				if (!includeBlocked && blockedPredicate &&
				        !(*(blockedPredicate))(tile))
					break;

				path.push_back(t_c);

				/* see if the action was blocked only if it did not succeed */
				if (includeBlocked && blockedPredicate &&
				        !(*(blockedPredicate))(tile))
					break;
			}
		}
	}

	return path;
}

void gameDamageParty(int minDamage, int maxDamage) {
	int i;
	int damage;
	int lastdmged = -1;

	for (i = 0; i < g_context->_party->size(); i++) {
		if (xu4_random(2) == 0) {
			damage = ((minDamage >= 0) && (minDamage < maxDamage)) ?
			         xu4_random((maxDamage + 1) - minDamage) + minDamage :
			         maxDamage;
			g_context->_party->member(i)->applyDamage(damage);
			g_context->_stats->highlightPlayer(i);
			lastdmged = i;
			EventHandler::sleep(50);
		}
	}

	g_screen->screenShake(1);

	// Un-highlight the last player
	if (lastdmged != -1) g_context->_stats->highlightPlayer(lastdmged);
}

void gameDamageShip(int minDamage, int maxDamage) {
	int damage;

	if (g_context->_transportContext == TRANSPORT_SHIP) {
		damage = ((minDamage >= 0) && (minDamage < maxDamage)) ?
		         xu4_random((maxDamage + 1) - minDamage) + minDamage :
		         maxDamage;

		g_screen->screenShake(1);

		g_context->_party->damageShip(damage);
		gameCheckHullIntegrity();
	}
}

void gameSetActivePlayer(int player) {
	if (player == -1) {
		g_context->_party->setActivePlayer(-1);
		g_screen->screenMessage("Set Active Player: None!\n");
	} else if (player < g_context->_party->size()) {
		g_screen->screenMessage("Set Active Player: %s!\n", g_context->_party->member(player)->getName().c_str());
		if (g_context->_party->member(player)->isDisabled())
			g_screen->screenMessage("Disabled!\n");
		else
			g_context->_party->setActivePlayer(player);
	}
}

bool gameSpawnCreature(const Creature *m) {
	int t, i;
	const Creature *creature;
	MapCoords coords = g_context->_location->_coords;

	if (g_context->_location->_context & CTX_DUNGEON) {
		/* FIXME: for some reason dungeon monsters aren't spawning correctly */

		bool found = false;
		MapCoords new_coords;

		for (i = 0; i < 0x20; i++) {
			new_coords = MapCoords(xu4_random(g_context->_location->_map->_width), xu4_random(g_context->_location->_map->_height), coords.z);
			const Tile *tile = g_context->_location->_map->tileTypeAt(new_coords, WITH_OBJECTS);
			if (tile->isCreatureWalkable()) {
				found = true;
				break;
			}
		}

		if (!found)
			return false;

		coords = new_coords;
	} else {
		int dx = 0,
		    dy = 0;
		bool ok = false;
		int tries = 0;
		static const int MAX_TRIES = 10;

		while (!ok && (tries < MAX_TRIES)) {
			dx = 7;
			dy = xu4_random(7);

			if (xu4_random(2))
				dx = -dx;
			if (xu4_random(2))
				dy = -dy;
			if (xu4_random(2)) {
				t = dx;
				dx = dy;
				dy = t;
			}

			/* make sure we can spawn the creature there */
			if (m) {
				MapCoords new_coords = coords;
				new_coords.move(dx, dy, g_context->_location->_map);

				const Tile *tile = g_context->_location->_map->tileTypeAt(new_coords, WITHOUT_OBJECTS);
				if ((m->sails() && tile->isSailable()) ||
				        (m->swims() && tile->isSwimable()) ||
				        (m->walks() && tile->isCreatureWalkable()) ||
				        (m->flies() && tile->isFlyable()))
					ok = true;
				else
					tries++;
			} else {
				ok = true;
			}
		}

		if (ok)
			coords.move(dx, dy, g_context->_location->_map);
	}

	/* can't spawn creatures on top of the player */
	if (coords == g_context->_location->_coords)
		return false;

	/* figure out what creature to spawn */
	if (m)
		creature = m;
	else if (g_context->_location->_context & CTX_DUNGEON)
		creature = creatureMgr->randomForDungeon(g_context->_location->_coords.z);
	else
		creature = creatureMgr->randomForTile(g_context->_location->_map->tileTypeAt(coords, WITHOUT_OBJECTS));

	if (creature)
		g_context->_location->_map->addCreature(creature, coords);
	return true;
}

void gameDestroyAllCreatures(void) {
	int i;

	gameSpellEffect('t', -1, SOUND_MAGIC); /* same effect as tremor */

	if (g_context->_location->_context & CTX_COMBAT) {
		// Destroy all creatures in combat
		for (i = 0; i < AREA_CREATURES; i++) {
			CombatMap *cm = getCombatMap();
			CreatureVector creatures = cm->getCreatures();
			CreatureVector::iterator obj;

			for (obj = creatures.begin(); obj != creatures.end(); obj++) {
				if ((*obj)->getId() != LORDBRITISH_ID)
					cm->removeObject(*obj);
			}
		}
	} else {
		// Destroy all creatures on the map
		ObjectDeque::iterator current;
		Map *map = g_context->_location->_map;

		for (current = map->_objects.begin(); current != map->_objects.end();) {
			Creature *m = dynamic_cast<Creature *>(*current);

			if (m) {
				// The skull does not destroy Lord British
				if (m->getId() != LORDBRITISH_ID)
					current = map->removeObject(current);
				else
					current++;
			} else {
				current++;
			}
		}
	}

	// Alert the guards! Really, the only one left should be LB himself :)
	g_context->_location->_map->alertGuards();
}

// Colors assigned to reagents based on my best reading of them
// from the book of wisdom.  Maybe we could use BOLD to distinguish
// the two grey and the two red reagents.
const int colors[] = {
	FG_YELLOW, FG_GREY, FG_BLUE, FG_WHITE, FG_RED, FG_GREY, FG_GREEN, FG_RED
};

void showMixturesSuper(int page = 0) {
	g_screen->screenTextColor(FG_WHITE);
	for (int i = 0; i < 13; i++) {
		char buf[4];

		const Spell *s = g_spells->getSpell(i + 13 * page);
		int line = i + 8;
		g_screen->screenTextAt(2, line, "%s", s->_name);

		snprintf(buf, 4, "%3d", g_ultima->_saveGame->_mixtures[i + 13 * page]);
		g_screen->screenTextAt(6, line, "%s", buf);

		g_screen->screenShowChar(32, 9, line);
		int comp = s->_components;
		for (int j = 0; j < 8; j++) {
			g_screen->screenTextColor(colors[j]);
			g_screen->screenShowChar(comp & (1 << j) ? CHARSET_BULLET : ' ', 10 + j, line);
		}
		g_screen->screenTextColor(FG_WHITE);

		snprintf(buf, 3, "%2d", s->_mp);
		g_screen->screenTextAt(19, line, "%s", buf);
	}
}

void mixReagentsSuper() {
	g_screen->screenMessage("Mix reagents\n");

	static int page = 0;

	struct ReagentShop {
		const char *name;
		int price[6];
	};
	ReagentShop shops[] = {
		{ "BuccDen", {6, 7, 9, 9, 9, 1} },
		{ "Moonglo", {2, 5, 6, 3, 6, 9} },
		{ "Paws", {3, 4, 2, 8, 6, 7} },
		{ "SkaraBr", {2, 4, 9, 6, 4, 8} },
	};
	const int shopcount = sizeof(shops) / sizeof(shops[0]);

	int oldlocation = g_context->_location->_viewMode;
	g_context->_location->_viewMode = VIEW_MIXTURES;
	g_screen->screenUpdate(&g_game->_mapArea, true, true);

	g_screen->screenTextAt(16, 2, "%s", "<-Shops");

	g_context->_stats->setView(StatsView(STATS_REAGENTS));
	g_screen->screenTextColor(FG_PURPLE);
	g_screen->screenTextAt(2, 7, "%s", "SPELL # Reagents MP");

	for (int i = 0; i < shopcount; i++) {
		int line = i + 1;
		ReagentShop *s = &shops[i];
		g_screen->screenTextColor(FG_WHITE);
		g_screen->screenTextAt(2, line, "%s", s->name);
		for (int j = 0; j < 6; j++) {
			g_screen->screenTextColor(colors[j]);
			g_screen->screenShowChar('0' + s->price[j], 10 + j, line);
		}
	}

	for (int i = 0; i < 8; i++) {
		g_screen->screenTextColor(colors[i]);
		g_screen->screenShowChar('A' + i, 10 + i, 6);
	}

	bool done = false;
	while (!done) {
		showMixturesSuper(page);
		g_screen->screenMessage("For Spell: ");

		int spell = ReadChoiceController::get("abcdefghijklmnopqrstuvwxyz \033\n\r");
		if (spell < 'a' || spell > 'z') {
			g_screen->screenMessage("\nDone.\n");
			done = true;
		} else {
			spell -= 'a';
			const Spell *s = g_spells->getSpell(spell);
			g_screen->screenMessage("%s\n", s->_name);
			page = (spell >= 13);
			showMixturesSuper(page);

			// how many can we mix?
			int mixQty = 99 - g_ultima->_saveGame->_mixtures[spell];
			int ingQty = 99;
			int comp = s->_components;
			for (int i = 0; i < 8; i++) {
				if (comp & 1 << i) {
					int reagentQty = g_ultima->_saveGame->_reagents[i];
					if (reagentQty < ingQty)
						ingQty = reagentQty;
				}
			}
			g_screen->screenMessage("You can make %d.\n", (mixQty > ingQty) ? ingQty : mixQty);
			g_screen->screenMessage("How many? ");

			int howmany = ReadIntController::get(2, TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);

			if (howmany == 0) {
				g_screen->screenMessage("\nNone mixed!\n");
			} else if (howmany > mixQty) {
				g_screen->screenMessage("\n%cYou cannot mix that much more of that spell!%c\n", FG_GREY, FG_WHITE);
			} else if (howmany > ingQty) {
				g_screen->screenMessage("\n%cYou don't have enough reagents to mix %d spells!%c\n", FG_GREY, howmany, FG_WHITE);
			} else {
				g_ultima->_saveGame->_mixtures[spell] += howmany;
				for (int i = 0; i < 8; i++) {
					if (comp & 1 << i) {
						g_ultima->_saveGame->_reagents[i] -= howmany;
					}
				}
				g_screen->screenMessage("\nSuccess!\n\n");
			}
		}
		g_context->_stats->setView(StatsView(STATS_REAGENTS));
	}

	g_context->_location->_viewMode = oldlocation;
	return;
}

} // End of namespace Ultima4
} // End of namespace Ultima
