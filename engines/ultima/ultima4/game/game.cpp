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
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/map/camp.h"
#include "ultima/ultima4/game/cheat.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/combat.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/death.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game/intro.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/progress_bar.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/game/script.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/map/dungeonview.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

extern bool quit;
GameController *g_game = NULL;

/*-----------------*/
/* Functions BEGIN */

/* main game functions */
void gameAdvanceLevel(PartyMember *player);
void gameInnHandler(void);
void gameLostEighth(Virtue virtue);
void gamePartyStarving(void);
time_t gameTimeSinceLastCommand(void);
int gameSave(void);

/* spell functions */
void gameCastSpell(unsigned int spell, int caster, int param);
bool gameSpellMixHowMany(int spell, int num, Ingredients *ingredients);

void mixReagents();
bool mixReagentsForSpellU4(int spell);
bool mixReagentsForSpellU5(int spell);
void mixReagentsSuper();
void newOrder();

/* conversation functions */
bool talkAt(const Coords &coords);
void talkRunConversation(Conversation &conv, Person *talker, bool showPrompt);

/* action functions */
bool attackAt(const Coords &coords);
bool destroyAt(const Coords &coords);
bool getChestTrapHandler(int player);
bool jimmyAt(const Coords &coords);
bool openAt(const Coords &coords);
void wearArmor(int player = -1);
void ztatsFor(int player = -1);

/* checking functions */
void gameLordBritishCheckLevels(void);

/* creature functions */
void gameDestroyAllCreatures(void);
void gameFixupObjects(Map *map);
void gameCreatureAttack(Creature *obj);

/* Functions END */
/*---------------*/

//extern Object *party[8];
Context *g_context = NULL;

Debug gameDbg("debug/game.txt", "Game");

static const MouseArea mouseAreas[] = {
	{ 3, { { 8, 8 }, { 8, 184 }, { 96, 96 } }, MC_WEST, { U4_ENTER, 0, U4_LEFT } },
	{ 3, { { 8, 8 }, { 184, 8 }, { 96, 96 } }, MC_NORTH, { U4_ENTER, 0, U4_UP }  },
	{ 3, { { 184, 8 }, { 184, 184 }, { 96, 96 } }, MC_EAST, { U4_ENTER, 0, U4_RIGHT } },
	{ 3, { { 8, 184 }, { 184, 184 }, { 96, 96 } }, MC_SOUTH, { U4_ENTER, 0, U4_DOWN } },
	{ 0, { { 0, 0 }, { 0, 0 }, { 0, 0 } }, MC_NORTH, { 0, 0, 0 } }
};

ReadPlayerController::ReadPlayerController() : ReadChoiceController("12345678 \033\n") {
#ifdef IOS
	U4IOS::beginCharacterChoiceDialog();
#endif
}

ReadPlayerController::~ReadPlayerController() {
#ifdef IOS
	U4IOS::endCharacterChoiceDialog();
#endif
}

bool ReadPlayerController::keyPressed(int key) {
	bool valid = ReadChoiceController::keyPressed(key);
	if (valid) {
		if (_value < '1' ||
		        _value > ('0' + g_context->_saveGame->_members))
			_value = '0';
	} else {
		_value = '0';
	}
	return valid;
}

int ReadPlayerController::getPlayer() {
	return _value - '1';
}

int ReadPlayerController::waitFor() {
	ReadChoiceController::waitFor();
	return getPlayer();
}

bool AlphaActionController::keyPressed(int key) {
	if (Common::isLower(key))
		key = toupper(key);

	if (key >= 'A' && key <= toupper(_lastValidLetter)) {
		_value = key - 'A';
		doneWaiting();
	} else if (key == U4_SPACE || key == U4_ESC || key == U4_ENTER) {
		screenMessage("\n");
		_value = -1;
		doneWaiting();
	} else {
		screenMessage("\n%s", _prompt.c_str());
		screenRedrawScreen();
		return KeyHandler::defaultHandler(key, NULL);
	}
	return true;
}

int AlphaActionController::get(char lastValidLetter, const Common::String &prompt, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	AlphaActionController ctrl(lastValidLetter, prompt);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

GameController::GameController() : _mapArea(BORDER_WIDTH, BORDER_HEIGHT, VIEWPORT_W, VIEWPORT_H), _paused(false), _pausedTimer(0) {
	g_game = this;
}

void GameController::initScreen() {
	Image *screen = imageMgr->get("screen")->_image;

	screen->fillRect(0, 0, screen->width(), screen->height(), 0, 0, 0);
	screenRedrawScreen();
}

void GameController::initScreenWithoutReloadingState() {
	musicMgr->play();
	imageMgr->get(BKGD_BORDERS)->_image->draw(0, 0);
	g_context->_stats->update(); /* draw the party stats */

	screenMessage("Press Alt-h for help\n");
	screenPrompt();

	eventHandler->pushMouseAreaSet(mouseAreas);

	eventHandler->setScreenUpdate(&gameUpdateScreen);
}


void GameController::init() {
	Common::InSaveFile *saveGameFile, *monstersFile;

	TRACE(gameDbg, "gameInit() running.");

	initScreen();

	ProgressBar pb((320 / 2) - (200 / 2), (200 / 2), 200, 10, 0, 4);
	pb.setBorderColor(240, 240, 240);
	pb.setBorderWidth(1);
	pb.setColor(0, 0, 128);

	screenTextAt(13, 11, "%s", "Loading Game...");

	/* initialize the global game context */
	g_context = new Context;
	g_context->_saveGame = new SaveGame;

	TRACE_LOCAL(gameDbg, "Global context initialized.");

	/* initialize conversation and game state variables */
	g_context->_line = TEXT_AREA_H - 1;
	g_context->col = 0;
	g_context->_stats = new StatsArea();
	g_context->_moonPhase = 0;
	g_context->_windDirection = DIR_NORTH;
	g_context->_windCounter = 0;
	g_context->_windLock = false;
	g_context->_aura = new Aura();
	g_context->_horseSpeed = 0;
	g_context->_opacity = 1;
	g_context->_lastCommandTime = g_system->getMillis();
	g_context->_lastShip = NULL;

	/* load in the save game */
	saveGameFile = g_system->getSavefileManager()->openForLoading(PARTY_SAV_BASE_FILENAME);
	if (saveGameFile) {
		g_context->_saveGame->read(saveGameFile);
		delete saveGameFile;
	} else {
		errorFatal("no savegame found!");
	}

	TRACE_LOCAL(gameDbg, "Save game loaded.");
	++pb;

	/* initialize our party */
	g_context->_party = new Party(g_context->_saveGame);
	g_context->_party->addObserver(this);

	/* set the map to the world map by default */
	setMap(mapMgr->get(MAP_WORLD), 0, NULL);
	g_context->_location->_map->clearObjects();

	TRACE_LOCAL(gameDbg, "World map set.");
	++pb;

	/* initialize our start location */
	Map *map = mapMgr->get(MapId(g_context->_saveGame->_location));
	TRACE_LOCAL(gameDbg, "Initializing start location.");

	/* if our map is not the world map, then load our map */
	if (map->_type != Map::WORLD)
		setMap(map, 1, NULL);
	else
		/* initialize the moons (must be done from the world map) */
		initMoons();


	/**
	 * Translate info from the savegame to something we can use
	 */
	if (g_context->_location->_prev) {
		g_context->_location->_coords = MapCoords(g_context->_saveGame->_x, g_context->_saveGame->_y, g_context->_saveGame->_dngLevel);
		g_context->_location->_prev->_coords = MapCoords(g_context->_saveGame->_dngX, g_context->_saveGame->_dngY);
	} else g_context->_location->_coords = MapCoords(g_context->_saveGame->_x, g_context->_saveGame->_y, (int)g_context->_saveGame->_dngLevel);
	g_context->_saveGame->_orientation = (Direction)(g_context->_saveGame->_orientation + DIR_WEST);

	/**
	 * Fix the coordinates if they're out of bounds.  This happens every
	 * time on the world map because (z == -1) is no longer valid.
	 * To maintain compatibility with u4dos, this value gets translated
	 * when the game is saved and loaded
	 */
	if (MAP_IS_OOB(g_context->_location->_map, g_context->_location->_coords))
		g_context->_location->_coords.putInBounds(g_context->_location->_map);

	TRACE_LOCAL(gameDbg, "Loading monsters.");
	++pb;

	/* load in creatures.sav */
	monstersFile = g_system->getSavefileManager()->openForLoading(MONSTERS_SAV_BASE_FILENAME);
	if (monstersFile) {
		saveGameMonstersRead(g_context->_location->_map->_monsterTable, monstersFile);
		delete monstersFile;
	}
	gameFixupObjects(g_context->_location->_map);

	/* we have previous creature information as well, load it! */
	if (g_context->_location->_prev) {
		monstersFile = g_system->getSavefileManager()->openForLoading(OUTMONST_SAV_BASE_FILENAME);
		if (monstersFile) {
			saveGameMonstersRead(g_context->_location->_prev->_map->_monsterTable, monstersFile);
			delete monstersFile;
		}
		gameFixupObjects(g_context->_location->_prev->_map);
	}

	spellSetEffectCallback(&gameSpellEffect);
	itemSetDestroyAllCreaturesCallback(&gameDestroyAllCreatures);

	++pb;

	TRACE_LOCAL(gameDbg, "Settings up reagent menu.");
	g_context->_stats->resetReagentsMenu();

	/* add some observers */
	g_context->_aura->addObserver(g_context->_stats);
	g_context->_party->addObserver(g_context->_stats);
#ifdef IOS
	c->aura->addObserver(U4IOS::IOSObserver::sharedInstance());
	c->party->addObserver(U4IOS::IOSObserver::sharedInstance());
#endif



	initScreenWithoutReloadingState();
	TRACE(gameDbg, "gameInit() completed successfully.");
}

/**
 * Saves the game state into party.sav and creatures.sav.
 */
int gameSave() {
	Common::OutSaveFile *saveGameFile, *monstersFile, *dngMapFile;
	SaveGame save = *g_context->_saveGame;

	/*************************************************/
	/* Make sure the savegame struct is accurate now */

	if (g_context->_location->_prev) {
		save._x = g_context->_location->_coords.x;
		save._y = g_context->_location->_coords.y;
		save._dngLevel = g_context->_location->_coords.z;
		save._dngX = g_context->_location->_prev->_coords.x;
		save._dngY = g_context->_location->_prev->_coords.y;
	} else {
		save._x = g_context->_location->_coords.x;
		save._y = g_context->_location->_coords.y;
		save._dngLevel = g_context->_location->_coords.z;
		save._dngX = g_context->_saveGame->_dngX;
		save._dngY = g_context->_saveGame->_dngY;
	}
	save._location = g_context->_location->_map->_id;
	save._orientation = (Direction)(g_context->_saveGame->_orientation - DIR_WEST);

	/* Done making sure the savegame struct is accurate */
	/****************************************************/

	saveGameFile = g_system->getSavefileManager()->openForSaving(PARTY_SAV_BASE_FILENAME);
	if (!saveGameFile) {
		screenMessage("Error opening " PARTY_SAV_BASE_FILENAME "\n");
		return 0;
	}

	if (!save.write(saveGameFile)) {
		screenMessage("Error writing to " PARTY_SAV_BASE_FILENAME "\n");
		delete saveGameFile;
		return 0;
	}
	delete saveGameFile;

	monstersFile = g_system->getSavefileManager()->openForSaving(MONSTERS_SAV_BASE_FILENAME);
	if (!monstersFile) {
		screenMessage("Error opening %s\n", MONSTERS_SAV_BASE_FILENAME);
		return 0;
	}

	/* fix creature animations so they are compatible with u4dos */
	g_context->_location->_map->resetObjectAnimations();
	g_context->_location->_map->fillMonsterTable(); /* fill the monster table so we can save it */

	if (!saveGameMonstersWrite(g_context->_location->_map->_monsterTable, monstersFile)) {
		screenMessage("Error opening creatures.sav\n");
		delete monstersFile;
		return 0;
	}
	delete monstersFile;

	/**
	 * Write dungeon info
	 */
	if (g_context->_location->_context & CTX_DUNGEON) {
		unsigned int x, y, z;

		typedef Std::map<const Creature *, int, Std::PointerHash> DngCreatureIdMap;
		static DngCreatureIdMap id_map;

		/**
		 * Map creatures to u4dos dungeon creature Ids
		 */
		if (id_map.size() == 0) {
			id_map[creatureMgr->getById(RAT_ID)]          = 1;
			id_map[creatureMgr->getById(BAT_ID)]          = 2;
			id_map[creatureMgr->getById(GIANT_SPIDER_ID)] = 3;
			id_map[creatureMgr->getById(GHOST_ID)]        = 4;
			id_map[creatureMgr->getById(SLIME_ID)]        = 5;
			id_map[creatureMgr->getById(TROLL_ID)]        = 6;
			id_map[creatureMgr->getById(GREMLIN_ID)]      = 7;
			id_map[creatureMgr->getById(MIMIC_ID)]        = 8;
			id_map[creatureMgr->getById(REAPER_ID)]       = 9;
			id_map[creatureMgr->getById(INSECT_SWARM_ID)] = 10;
			id_map[creatureMgr->getById(GAZER_ID)]        = 11;
			id_map[creatureMgr->getById(PHANTOM_ID)]      = 12;
			id_map[creatureMgr->getById(ORC_ID)]          = 13;
			id_map[creatureMgr->getById(SKELETON_ID)]     = 14;
			id_map[creatureMgr->getById(ROGUE_ID)]        = 15;
		}

		dngMapFile = g_system->getSavefileManager()->openForSaving("dngmap.sav");
		if (!dngMapFile) {
			screenMessage("Error opening dngmap.sav\n");
			return 0;
		}

		for (z = 0; z < g_context->_location->_map->_levels; z++) {
			for (y = 0; y < g_context->_location->_map->_height; y++) {
				for (x = 0; x < g_context->_location->_map->_width; x++) {
					unsigned char tile = g_context->_location->_map->translateToRawTileIndex(*g_context->_location->_map->getTileFromData(MapCoords(x, y, z)));
					Object *obj = g_context->_location->_map->objectAt(MapCoords(x, y, z));

					/**
					 * Add the creature to the tile
					 */
					if (obj && obj->getType() == Object::CREATURE) {
						const Creature *m = dynamic_cast<Creature *>(obj);
						DngCreatureIdMap::iterator m_id = id_map.find(m);
						if (m_id != id_map.end())
							tile |= m_id->_value;
					}

					// Write the tile
					dngMapFile->writeByte(tile);
				}
			}
		}

		delete dngMapFile;

		/**
		 * Write outmonst.sav
		 */

		monstersFile = g_system->getSavefileManager()->openForSaving(
		                   OUTMONST_SAV_BASE_FILENAME);
		if (!monstersFile) {
			screenMessage("Error opening %s\n", OUTMONST_SAV_BASE_FILENAME);
			return 0;
		}

		/* fix creature animations so they are compatible with u4dos */
		g_context->_location->_prev->_map->resetObjectAnimations();
		g_context->_location->_prev->_map->fillMonsterTable(); /* fill the monster table so we can save it */

		if (!saveGameMonstersWrite(g_context->_location->_prev->_map->_monsterTable, monstersFile)) {
			screenMessage("Error opening %s\n", OUTMONST_SAV_BASE_FILENAME);
			delete monstersFile;
			return 0;
		}
		delete monstersFile;
	}

	return 1;
}

/**
 * Sets the view mode.
 */
void gameSetViewMode(ViewMode newMode) {
	g_context->_location->_viewMode = newMode;
}

void gameUpdateScreen() {
	switch (g_context->_location->_viewMode) {
	case VIEW_NORMAL:
		screenUpdate(&g_game->_mapArea, true, false);
		break;
	case VIEW_GEM:
		screenGemUpdate();
		break;
	case VIEW_RUNE:
		screenUpdate(&g_game->_mapArea, false, false);
		break;
	case VIEW_DUNGEON:
		screenUpdate(&g_game->_mapArea, true, false);
		break;
	case VIEW_DEAD:
		screenUpdate(&g_game->_mapArea, true, true);
		break;
	case VIEW_CODEX: /* the screen updates will be handled elsewhere */
		break;
	case VIEW_MIXTURES: /* still testing */
		break;
	default:
		ASSERT(0, "invalid view mode: %d", g_context->_location->_viewMode);
	}
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
			g_context->_saveGame->_orientation = DIR_EAST;
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
#ifdef IOS
	U4IOS::updateGameControllerContext(c->location->context);
#endif

	/* now, actually set our new tileset */
	_mapArea.setTileset(map->_tileset);

	if (isCity(map)) {
		City *city = dynamic_cast<City *>(map);
		city->addPeople();
	}
}

/**
 * Exits the current map and location and returns to its parent location
 * This restores all relevant information from the previous location,
 * such as the map, map position, etc. (such as exiting a city)
 **/

int GameController::exitToParentMap() {
	if (!g_context->_location)
		return 0;

	if (g_context->_location->_prev != NULL) {
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
		_mapArea.setTileset(g_context->_location->_map->_tileset);
#ifdef IOS
		U4IOS::updateGameControllerContext(c->location->context);
#endif

		return 1;
	}
	return 0;
}

/**
 * Terminates a game turn.  This performs the post-turn housekeeping
 * tasks like adjusting the party's food, incrementing the number of
 * moves, etc.
 */
void GameController::finishTurn() {
	g_context->_lastCommandTime = g_system->getMillis();
	Creature *attacker = NULL;

	while (1) {

		/* adjust food and moves */
		g_context->_party->endTurn();

		/* count down the aura, if there is one */
		g_context->_aura->passTurn();

		gameCheckHullIntegrity();

		/* update party stats */
		//c->stats->setView(STATS_PARTY_OVERVIEW);

		screenUpdate(&this->_mapArea, true, false);
		screenWait(1);

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
			deathStart(0);
			return;
		} else {
			screenMessage("Zzzzzz\n");
			screenWait(4);
		}
	}

	if (g_context->_location->_context == CTX_DUNGEON) {
		Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
		if (g_context->_party->getTorchDuration() <= 0)
			screenMessage("It's Dark!\n");
		else g_context->_party->burnTorch();

		/* handle dungeon traps */
		if (dungeon->currentToken() == DUNGEON_TRAP) {
			dungeonHandleTrap((TrapType)dungeon->currentSubToken());
			// a little kludgey to have a second test for this
			// right here.  But without it you can survive an
			// extra turn after party death and do some things
			// that could cause a crash, like Hole up and Camp.
			if (g_context->_party->isDead()) {
				deathStart(0);
				return;
			}
		}
	}


	/* draw a prompt */
	screenPrompt();
	//screenRedrawTextArea(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_W, TEXT_AREA_H);
}

/**
 * Show an attack flash at x, y on the current map.
 * This is used for 'being hit' or 'being missed'
 * by weapons, cannon fire, spells, etc.
 */
void GameController::flashTile(const Coords &coords, MapTile tile, int frames) {
	g_context->_location->_map->_annotations->add(coords, tile, true);

	screenTileUpdate(&g_game->_mapArea, coords);

	screenWait(frames);
	g_context->_location->_map->_annotations->remove(coords, tile);

	screenTileUpdate(&g_game->_mapArea, coords, false);
}

void GameController::flashTile(const Coords &coords, const Common::String &tilename, int timeFactor) {
	Tile *tile = g_context->_location->_map->_tileset->getByName(tilename);
	ASSERT(tile, "no tile named '%s' found in tileset", tilename.c_str());
	flashTile(coords, tile->getId(), timeFactor);
}


/**
 * Provide feedback to user after a party event happens.
 */
void GameController::update(Party *party, PartyEvent &event) {
	int i;

	switch (event._type) {
	case PartyEvent::LOST_EIGHTH:
		// inform a player he has lost zero or more eighths of avatarhood.
		screenMessage("\n %cThou hast lost\n  an eighth!%c\n", FG_YELLOW, FG_WHITE);
		break;
	case PartyEvent::ADVANCED_LEVEL:
		screenMessage("\n%c%s\nThou art now Level %d%c\n", FG_YELLOW, event._player->getName().c_str(), event._player->getRealLevel(), FG_WHITE);
		gameSpellEffect('r', -1, SOUND_MAGIC); // Same as resurrect spell
		break;
	case PartyEvent::STARVING:
		screenMessage("\n%cStarving!!!%c\n", FG_YELLOW, FG_WHITE);
		/* FIXME: add sound effect here */

		// 2 damage to each party member for starving!
		for (i = 0; i < g_context->_saveGame->_members; i++)
			g_context->_party->member(i)->applyDamage(2);
		break;
	default:
		break;
	}
}

/**
 * Provide feedback to user after a movement event happens.
 */
void GameController::update(Location *location, MoveEvent &event) {
	switch (location->_map->_type) {
	case Map::DUNGEON:
		avatarMovedInDungeon(event);
		break;
	case Map::COMBAT:
		// FIXME: let the combat controller handle it
		dynamic_cast<CombatController *>(eventHandler->getController())->movePartyMember(event);
		break;
	default:
		avatarMoved(event);
		break;
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
	case Spell::SFX_NONE:
		break;
	case Spell::SFX_TREMOR:
	case Spell::SFX_INVERT:
		gameUpdateScreen();
		g_game->_mapArea.highlight(0, 0, VIEWPORT_W * TILE_WIDTH, VIEWPORT_H * TILE_HEIGHT);
		EventHandler::sleep(time);
		g_game->_mapArea.unhighlight();
		screenRedrawScreen();

		if (effect == Spell::SFX_TREMOR) {
			gameUpdateScreen();
			soundPlay(SOUND_RUMBLE, false);
			screenShake(8);

		}

		break;
	}
}

void gameCastSpell(unsigned int spell, int caster, int param) {
	SpellCastError spellError;
	Common::String msg;

	if (!spellCast(spell, caster, param, &spellError, true)) {
		msg = spellGetErrorMessage(spell, spellError);
		if (!msg.empty())
			screenMessage("%s", msg.c_str());
	}
}

/**
 * The main key handler for the game.  Interpretes each key as a
 * command - 'a' for attack, 't' for talk, etc.
 */
bool GameController::keyPressed(int key) {
	bool valid = true;
	int endTurn = 1;
	Object *obj;
	MapTile *tile;

	/* Translate context-sensitive action key into a useful command */
	if (key == U4_ENTER && settings._enhancements && settings._enhancementsOptions._smartEnterKey) {
		/* Attempt to guess based on the character's surroundings etc, what
		   action they want */

		/* Do they want to board something? */
		if (g_context->_transportContext == TRANSPORT_FOOT) {
			obj = g_context->_location->_map->objectAt(g_context->_location->_coords);
			if (obj && (obj->getTile().getTileType()->isShip() ||
			            obj->getTile().getTileType()->isHorse() ||
			            obj->getTile().getTileType()->isBalloon()))
				key = 'b';
		}
		/* Klimb/Descend Balloon */
		else if (g_context->_transportContext == TRANSPORT_BALLOON) {
			if (g_context->_party->isFlying())
				key = 'd';
			else {
#ifdef IOS
				U4IOS::IOSSuperButtonHelper superHelper;
				key = ReadChoiceController::get("xk \033\n");
#else
				key = 'k';
#endif
			}
		}
		/* X-it transport */
		else key = 'x';

		/* Klimb? */
		if ((g_context->_location->_map->portalAt(g_context->_location->_coords, ACTION_KLIMB) != NULL))
			key = 'k';
		/* Descend? */
		else if ((g_context->_location->_map->portalAt(g_context->_location->_coords, ACTION_DESCEND) != NULL))
			key = 'd';

		if (g_context->_location->_context == CTX_DUNGEON) {
			Dungeon *dungeon = static_cast<Dungeon *>(g_context->_location->_map);
			bool up = dungeon->ladderUpAt(g_context->_location->_coords);
			bool down = dungeon->ladderDownAt(g_context->_location->_coords);
			if (up && down) {
#ifdef IOS
				U4IOS::IOSClimbHelper climbHelper;
				key = ReadChoiceController::get("kd \033\n");
#else
				key = 'k'; // This is consistent with the previous code. Ideally, I would have a UI here as well.
#endif
			} else if (up) {
				key = 'k';
			} else {
				key = 'd';
			}
		}

		/* Enter? */
		if (g_context->_location->_map->portalAt(g_context->_location->_coords, ACTION_ENTER) != NULL)
			key = 'e';

		/* Get Chest? */
		if (!g_context->_party->isFlying()) {
			tile = g_context->_location->_map->tileAt(g_context->_location->_coords, WITH_GROUND_OBJECTS);

			if (tile->getTileType()->isChest()) key = 'g';
		}

		/* None of these? Default to search */
		if (key == U4_ENTER) key = 's';
	}

	if ((g_context->_location->_context & CTX_DUNGEON) && strchr("abefjlotxy", key))
		screenMessage("%cNot here!%c\n", FG_GREY, FG_WHITE);
	else
		switch (key) {

		case U4_UP:
		case U4_DOWN:
		case U4_LEFT:
		case U4_RIGHT: {
			/* move the avatar */
			Common::String previous_map = g_context->_location->_map->_fname;
			MoveResult retval = g_context->_location->move(keyToDirection(key), true);

			/* horse doubles speed (make sure we're on the same map as the previous move first) */
			if (retval & (MOVE_SUCCEEDED | MOVE_SLOWED) &&
			        (g_context->_transportContext == TRANSPORT_HORSE) && g_context->_horseSpeed) {
				gameUpdateScreen(); /* to give it a smooth look of movement */
				if (previous_map == g_context->_location->_map->_fname)
					g_context->_location->move(keyToDirection(key), false);
			}

			endTurn = (retval & MOVE_END_TURN); /* let the movement handler decide to end the turn */
		}

		break;

		case U4_FKEY:
		case U4_FKEY+1:
		case U4_FKEY+2:
		case U4_FKEY+3:
		case U4_FKEY+4:
		case U4_FKEY+5:
		case U4_FKEY+6:
		case U4_FKEY+7:
			/* teleport to dungeon entrances! */
			if (settings._debug && (g_context->_location->_context & CTX_WORLDMAP) && (g_context->_transportContext & TRANSPORT_FOOT_OR_HORSE)) {
				int portal = 16 + (key - U4_FKEY); /* find dungeon portal */
				g_context->_location->_coords = g_context->_location->_map->_portals[portal]->_coords;
			} else valid = false;
			break;

		case U4_FKEY+8:
			if (settings._debug && (g_context->_location->_context & CTX_WORLDMAP)) {
				setMap(mapMgr->get(MAP_DECEIT), 1, NULL);
				g_context->_location->_coords = MapCoords(1, 0, 7);
				g_context->_saveGame->_orientation = DIR_SOUTH;
			} else valid = false;
			break;

		case U4_FKEY+9:
			if (settings._debug && (g_context->_location->_context & CTX_WORLDMAP)) {
				setMap(mapMgr->get(MAP_DESPISE), 1, NULL);
				g_context->_location->_coords = MapCoords(3, 2, 7);
				g_context->_saveGame->_orientation = DIR_SOUTH;
			} else valid = false;
			break;

		case U4_FKEY+10:
			if (settings._debug && (g_context->_location->_context & CTX_WORLDMAP)) {
				setMap(mapMgr->get(MAP_DESTARD), 1, NULL);
				g_context->_location->_coords = MapCoords(7, 6, 7);
				g_context->_saveGame->_orientation = DIR_SOUTH;
			} else valid = false;
			break;

		case U4_FKEY+11:
			if (settings._debug) {
				screenMessage("Torch: %d\n", g_context->_party->getTorchDuration());
				screenPrompt();
			} else valid = false;
			break;

		case 3:                     /* ctrl-C */
			if (settings._debug) {
				screenMessage("Cmd (h = help):");
				CheatMenuController cheatMenuController(this);
				eventHandler->pushController(&cheatMenuController);
				cheatMenuController.waitFor();
			} else valid = false;
			break;

		case 4:                     /* ctrl-D */
			if (settings._debug) {
				destroy();
			} else valid = false;
			break;

		case 8:                     /* ctrl-H */
			if (settings._debug) {
				screenMessage("Help!\n");
				screenPrompt();

				/* Help! send me to Lord British (who conveniently is right around where you are)! */
				setMap(mapMgr->get(100), 1, NULL);
				g_context->_location->_coords.x = 19;
				g_context->_location->_coords.y = 8;
				g_context->_location->_coords.z = 0;
			} else valid = false;
			break;

		case 22: {                  /* ctrl-V */
			if (settings._debug && g_context->_location->_context == CTX_DUNGEON) {
				screenMessage("3-D view %s\n", DungeonViewer.toggle3DDungeonView() ? "on" : "off");
				endTurn = 0;
			} else valid = false;
		}
		break;

		case ' ':
			screenMessage("Pass\n");
			break;

		case '+':
		case '-':
		case U4_KEYPAD_ENTER: {
			int old_cycles = settings._gameCyclesPerSecond;
			if (key == '+' && ++settings._gameCyclesPerSecond > MAX_CYCLES_PER_SECOND)
				settings._gameCyclesPerSecond = MAX_CYCLES_PER_SECOND;
			else if (key == '-' && --settings._gameCyclesPerSecond == 0)
				settings._gameCyclesPerSecond = 1;
			else if (key == U4_KEYPAD_ENTER)
				settings._gameCyclesPerSecond = DEFAULT_CYCLES_PER_SECOND;

			if (old_cycles != settings._gameCyclesPerSecond) {
				eventTimerGranularity = (1000 / settings._gameCyclesPerSecond);
				eventHandler->getTimer()->reset(eventTimerGranularity);

				if (settings._gameCyclesPerSecond == DEFAULT_CYCLES_PER_SECOND)
					screenMessage("Speed: Normal\n");
				else if (key == '+')
					screenMessage("Speed Up (%d)\n", settings._gameCyclesPerSecond);
				else screenMessage("Speed Down (%d)\n", settings._gameCyclesPerSecond);
			} else if (settings._gameCyclesPerSecond == DEFAULT_CYCLES_PER_SECOND)
				screenMessage("Speed: Normal\n");
		}

		endTurn = false;
		break;

		/* handle music volume adjustments */
		case ',':
			// decrease the volume if possible
			screenMessage("Music: %d%s\n", musicMgr->decreaseMusicVolume(), "%");
			endTurn = false;
			break;
		case '.':
			// increase the volume if possible
			screenMessage("Music: %d%s\n", musicMgr->increaseMusicVolume(), "%");
			endTurn = false;
			break;

		/* handle sound volume adjustments */
		case '<':
			// decrease the volume if possible
			screenMessage("Sound: %d%s\n", musicMgr->decreaseSoundVolume(), "%");
			soundPlay(SOUND_FLEE);
			endTurn = false;
			break;
		case '>':
			// increase the volume if possible
			screenMessage("Sound: %d%s\n", musicMgr->increaseSoundVolume(), "%");
			soundPlay(SOUND_FLEE);
			endTurn = false;
			break;

		case 'a':
			attack();
			break;

		case 'b':
			board();
			break;

		case 'c':
			castSpell();
			break;

		case 'd': {
			// unload the map for the second level of Lord British's Castle. The reason
			// why is that Lord British's farewell is dependent on the number of party members.
			// Instead of just redoing the dialog, it's a bit severe, but easier to unload the
			// whole level.
			bool cleanMap = (g_context->_party->size() == 1 && g_context->_location->_map->_id == 100);
			if (!usePortalAt(g_context->_location, g_context->_location->_coords, ACTION_DESCEND)) {
				if (g_context->_transportContext == TRANSPORT_BALLOON) {
					screenMessage("Land Balloon\n");
					if (!g_context->_party->isFlying())
						screenMessage("%cAlready Landed!%c\n", FG_GREY, FG_WHITE);
					else if (g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_OBJECTS)->canLandBalloon()) {
						g_context->_saveGame->_balloonstate = 0;
						g_context->_opacity = 1;
					} else screenMessage("%cNot Here!%c\n", FG_GREY, FG_WHITE);
				} else screenMessage("%cDescend what?%c\n", FG_GREY, FG_WHITE);
			} else {
				if (cleanMap)
					mapMgr->unloadMap(100);
			}
			break;
		}

		case 'e':
			if (!usePortalAt(g_context->_location, g_context->_location->_coords, ACTION_ENTER)) {
				if (!g_context->_location->_map->portalAt(g_context->_location->_coords, ACTION_ENTER))
					screenMessage("%cEnter what?%c\n", FG_GREY, FG_WHITE);
			} else endTurn = 0; /* entering a portal doesn't end the turn */
			break;

		case 'f':
			fire();
			break;

		case 'g':
			getChest();
			break;

		case 'h':
			holeUp();
			break;

		case 'i':
			screenMessage("Ignite torch!\n");
			if (g_context->_location->_context == CTX_DUNGEON) {
				if (!g_context->_party->lightTorch())
					screenMessage("%cNone left!%c\n", FG_GREY, FG_WHITE);
			} else screenMessage("%cNot here!%c\n", FG_GREY, FG_WHITE);
			break;

		case 'j':
			jimmy();
			break;

		case 'k':
			if (!usePortalAt(g_context->_location, g_context->_location->_coords, ACTION_KLIMB)) {
				if (g_context->_transportContext == TRANSPORT_BALLOON) {
					g_context->_saveGame->_balloonstate = 1;
					g_context->_opacity = 0;
					screenMessage("Klimb altitude\n");
				} else
					screenMessage("%cKlimb what?%c\n", FG_GREY, FG_WHITE);
			}
			break;

		case 'l':
			/* can't use sextant in dungeon or in combat */
			if (g_context->_location->_context & ~(CTX_DUNGEON | CTX_COMBAT)) {
				if (g_context->_saveGame->_sextants >= 1)
					screenMessage("Locate position\nwith sextant\n Latitude: %c'%c\"\nLongitude: %c'%c\"\n",
					              g_context->_location->_coords.y / 16 + 'A', g_context->_location->_coords.y % 16 + 'A',
					              g_context->_location->_coords.x / 16 + 'A', g_context->_location->_coords.x % 16 + 'A');
				else
					screenMessage("%cLocate position with what?%c\n", FG_GREY, FG_WHITE);
			} else screenMessage("%cNot here!%c\n", FG_GREY, FG_WHITE);
			break;

		case 'm':
			mixReagents();
#ifdef IOS
			// The iOS MixSpell dialog needs control of the event loop, so it is its
			// job to complete the turn.
			endTurn = false;
#endif
			break;

		case 'n':
			newOrder();
			break;

		case 'o':
			opendoor();
			break;

		case 'p':
			peer();
			break;

		case 'q':
			screenMessage("Quit & Save...\n%d moves\n", g_context->_saveGame->_moves);
			if (g_context->_location->_context & CTX_CAN_SAVE_GAME) {
				gameSave();
				screenMessage("Press Alt-x to quit\n");
			} else screenMessage("%cNot here!%c\n", FG_GREY, FG_WHITE);

			break;

		case 'r':
			readyWeapon();
			break;

		case 's':
			if (g_context->_location->_context == CTX_DUNGEON)
				dungeonSearch();
			else if (g_context->_party->isFlying())
				screenMessage("Searching...\n%cDrift only!%c\n", FG_GREY, FG_WHITE);
			else {
				screenMessage("Searching...\n");

				const ItemLocation *item = itemAtLocation(g_context->_location->_map, g_context->_location->_coords);
				if (item) {
					if (*item->_isItemInInventory != NULL && (*item->_isItemInInventory)(item->_data))
						screenMessage("%cNothing Here!%c\n", FG_GREY, FG_WHITE);
					else {
						if (item->_name)
							screenMessage("You find...\n%s!\n", item->_name);
						(*item->_putItemInInventory)(item->_data);
					}
				} else
					screenMessage("%cNothing Here!%c\n", FG_GREY, FG_WHITE);
			}

			break;

		case 't':
			talk();
			break;

		case 'u': {
			screenMessage("Use which item:\n");
			if (settings._enhancements) {
				/* a little xu4 enhancement: show items in inventory when prompted for an item to use */
				g_context->_stats->setView(STATS_ITEMS);
			}
#ifdef IOS
			U4IOS::IOSConversationHelper::setIntroString("Use which item?");
#endif
			itemUse(gameGetInput().c_str());
			break;
		}

		case 'v':
			if (musicMgr->toggle())
				screenMessage("Volume On!\n");
			else
				screenMessage("Volume Off!\n");
			endTurn = false;
			break;

		case 'w':
			wearArmor();
			break;

		case 'x':
			if ((g_context->_transportContext != TRANSPORT_FOOT) && !g_context->_party->isFlying()) {
				obj = g_context->_location->_map->addObject(g_context->_party->getTransport(), g_context->_party->getTransport(), g_context->_location->_coords);
				if (g_context->_transportContext == TRANSPORT_SHIP)
					g_context->_lastShip = obj;

				Tile *avatar = g_context->_location->_map->_tileset->getByName("avatar");
				ASSERT(avatar, "no avatar tile found in tileset");
				g_context->_party->setTransport(avatar->getId());
				g_context->_horseSpeed = 0;
				screenMessage("X-it\n");
			} else
				screenMessage("%cX-it What?%c\n", FG_GREY, FG_WHITE);
			break;

		case 'y':
			screenMessage("Yell ");
			if (g_context->_transportContext == TRANSPORT_HORSE) {
				if (g_context->_horseSpeed == 0) {
					screenMessage("Giddyup!\n");
					g_context->_horseSpeed = 1;
				} else {
					screenMessage("Whoa!\n");
					g_context->_horseSpeed = 0;
				}
			} else
				screenMessage("%cWhat?%c\n", FG_GREY, FG_WHITE);
			break;

		case 'z':
			ztatsFor();
			break;

		case 'c' + U4_ALT:
			if (settings._debug && g_context->_location->_map->isWorldMap()) {
				/* first teleport to the abyss */
				g_context->_location->_coords.x = 0xe9;
				g_context->_location->_coords.y = 0xe9;
				setMap(mapMgr->get(MAP_ABYSS), 1, NULL);
				/* then to the final altar */
				g_context->_location->_coords.x = 7;
				g_context->_location->_coords.y = 7;
				g_context->_location->_coords.z = 7;
			}
			break;

		case 'h' + U4_ALT: {
#ifdef IOS
			U4IOS::IOSHideActionKeysHelper hideActionKeys;
#endif
			ReadChoiceController pauseController("");

			screenMessage("Key Reference:\n"
			              "Arrow Keys: Move\n"
			              "a: Attack\n"
			              "b: Board\n"
			              "c: Cast Spell\n"
			              "d: Descend\n"
			              "e: Enter\n"
			              "f: Fire Cannons\n"
			              "g: Get Chest\n"
			              "h: Hole up\n"
			              "i: Ignite torch\n"
			              "(more)");

			eventHandler->pushController(&pauseController);
			pauseController.waitFor();

			screenMessage("\n"
			              "j: Jimmy lock\n"
			              "k: Klimb\n"
			              "l: Locate\n"
			              "m: Mix reagents\n"
			              "n: New Order\n"
			              "o: Open door\n"
			              "p: Peer at Gem\n"
			              "q: Quit & Save\n"
			              "r: Ready weapon\n"
			              "s: Search\n"
			              "t: Talk\n"
			              "(more)");

			eventHandler->pushController(&pauseController);
			pauseController.waitFor();

			screenMessage("\n"
			              "u: Use Item\n"
			              "v: Volume On/Off\n"
			              "w: Wear armour\n"
			              "x: eXit\n"
			              "y: Yell\n"
			              "z: Ztats\n"
			              "Space: Pass\n"
			              ",: - Music Vol\n"
			              ".: + Music Vol\n"
			              "<: - Sound Vol\n"
			              ">: + Sound Vol\n"
			              "(more)");

			eventHandler->pushController(&pauseController);
			pauseController.waitFor();

			screenMessage("\n"
			              "Alt-Q: Main Menu\n"
			              "Alt-V: Version\n"
			              "Alt-X: Quit\n"
			              "\n"
			              "\n"
			              "\n"
			              "\n"
			              "\n"
			              "\n"
			              "\n"
			              "\n"
			             );
			screenPrompt();
			break;
		}

		case 'q' + U4_ALT: {
			// TODO - implement loop in main() and let quit fall back to there
			// Quit to the main menu
			endTurn = false;

			screenMessage("Quit to menu?");
			char choice = ReadChoiceController::get("yn \n\033");
			screenMessage("%c", choice);
			if (choice != 'y') {
				screenMessage("\n");
				break;
			}

			eventHandler->setScreenUpdate(NULL);
			eventHandler->popController();

			eventHandler->pushController(intro);

			// Fade out the music and hide the cursor
			//before returning to the menu.
			musicMgr->fadeOut(1000);
			screenHideCursor();

			intro->init();
			eventHandler->run();


			if (!quit) {
				eventHandler->setControllerDone(false);
				eventHandler->popController();
				eventHandler->pushController(this);


				if (intro->hasInitiatedNewGame()) {
					//Loads current savegame
					init();
				} else {
					//Inits screen stuff without renewing game
					initScreen();
					initScreenWithoutReloadingState();
				}

				this->_mapArea.reinit();

				intro->deleteIntro();
				eventHandler->run();
			}
		}
		break;

		// Turn sound effects on/off
		case 's' + U4_ALT:
			// FIXME: there's probably a more intuitive key combination for this
			settings._soundVol = !settings._soundVol;
			screenMessage("Sound FX %s!\n", settings._soundVol ? "on" : "off");
			endTurn = false;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (settings._enhancements && settings._enhancementsOptions._activePlayer)
				gameSetActivePlayer(key - '1');
			else screenMessage("%cBad command!%c\n", FG_GREY, FG_WHITE);

			endTurn = 0;
			break;

		default:
			valid = false;
			break;
		}

	if (valid && endTurn) {
		if (eventHandler->getController() == g_game)
			g_context->_location->_turnCompleter->finishTurn();
	} else if (!endTurn) {
		/* if our turn did not end, then manually redraw the text prompt */
		screenPrompt();
	}

	return valid || KeyHandler::defaultHandler(key, NULL);
}

Common::String gameGetInput(int maxlen) {
	screenEnableCursor();
	screenShowCursor();
#ifdef IOS
	U4IOS::IOSConversationHelper helper;
	helper.beginConversation(U4IOS::UIKeyboardTypeDefault);
#endif

	return ReadStringController::get(maxlen, TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
}

int gameGetPlayer(bool canBeDisabled, bool canBeActivePlayer) {
	int player;
	if (g_context->_saveGame->_members <= 1) {
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
			screenMessage("None\n");
			return -1;
		}
	}

	g_context->col--;// display the selected character name, in place of the number
	if ((player >= 0) && (player < 8)) {
		screenMessage("%s\n", g_context->_saveGame->_players[player].name); //Write player's name after prompt
	}

	if (!canBeDisabled && g_context->_party->member(player)->isDisabled()) {
		screenMessage("%cDisabled!%c\n", FG_GREY, FG_WHITE);
		return -1;
	}

	ASSERT(player < g_context->_party->size(), "player %d, but only %d members\n", player, g_context->_party->size());
	return player;
}

Direction gameGetDirection() {
	ReadDirController dirController;

	screenMessage("Dir?");
#ifdef IOS
	U4IOS::IOSDirectionHelper directionPopup;
#endif

	eventHandler->pushController(&dirController);
	Direction dir = dirController.waitFor();

	screenMessage("\b\b\b\b");

	if (dir == DIR_NONE) {
		screenMessage("    \n");
		return dir;
	} else {
		screenMessage("%s\n", getDirectionName(dir));
		return dir;
	}
}

bool gameSpellMixHowMany(int spell, int num, Ingredients *ingredients) {
	int i;

	/* entered 0 mixtures, don't mix anything! */
	if (num == 0) {
		screenMessage("\nNone mixed!\n");
		ingredients->revert();
		return false;
	}

	/* if they ask for more than will give them 99, only use what they need */
	if (num > 99 - g_context->_saveGame->_mixtures[spell]) {
		num = 99 - g_context->_saveGame->_mixtures[spell];
		screenMessage("\n%cOnly need %d!%c\n", FG_GREY, num, FG_WHITE);
	}

	screenMessage("\nMixing %d...\n", num);

	/* see if there's enough reagents to make number of mixtures requested */
	if (!ingredients->checkMultiple(num)) {
		screenMessage("\n%cYou don't have enough reagents to mix %d spells!%c\n", FG_GREY, num, FG_WHITE);
		ingredients->revert();
		return false;
	}

	screenMessage("\nYou mix the Reagents, and...\n");
	if (spellMix(spell, ingredients)) {
		screenMessage("Success!\n\n");
		/* mix the extra spells */
		ingredients->multiply(num);
		for (i = 0; i < num - 1; i++)
			spellMix(spell, ingredients);
	} else
		screenMessage("It Fizzles!\n\n");

	return true;
}

bool ZtatsController::keyPressed(int key) {
	switch (key) {
	case U4_UP:
	case U4_LEFT:
		g_context->_stats->prevItem();
		return true;
	case U4_DOWN:
	case U4_RIGHT:
		g_context->_stats->nextItem();
		return true;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		if (g_context->_saveGame->_members >= key - '0')
			g_context->_stats->setView(StatsView(STATS_CHAR1 + key - '1'));
		return true;
	case '0':
		g_context->_stats->setView(StatsView(STATS_WEAPONS));
		return true;
	case U4_ESC:
	case U4_SPACE:
	case U4_ENTER:
		g_context->_stats->setView(StatsView(STATS_PARTY_OVERVIEW));
		doneWaiting();
		return true;
	default:
		return KeyHandler::defaultHandler(key, NULL);
	}
}

void destroy() {
	screenMessage("Destroy Object\nDir: ");

	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL,
	                           g_context->_location->_coords, 1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (destroyAt(*i))
			return;
	}

	screenMessage("%cNothing there!%c\n", FG_GREY, FG_WHITE);
}

bool destroyAt(const Coords &coords) {
	Object *obj = g_context->_location->_map->objectAt(coords);

	if (obj) {
		if (isCreature(obj)) {
			Creature *c = dynamic_cast<Creature *>(obj);
			screenMessage("%s Destroyed!\n", c->getName().c_str());
		} else {
			Tile *t = g_context->_location->_map->_tileset->get(obj->getTile()._id);
			screenMessage("%s Destroyed!\n", t->getName().c_str());
		}

		g_context->_location->_map->removeObject(obj);
		screenPrompt();

		return true;
	}

	return false;
}

void attack() {
	screenMessage("Attack: ");

	if (g_context->_party->isFlying()) {
		screenMessage("\n%cDrift only!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
	                           1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (attackAt(*i))
			return;
	}

	screenMessage("%cNothing to Attack!%c\n", FG_GREY, FG_WHITE);
}

/**
 * Attempts to attack a creature at map coordinates x,y.  If no
 * creature is present at that point, zero is returned.
 */
bool attackAt(const Coords &coords) {
	Object *under;
	const Tile *ground;
	Creature *m;

	m = dynamic_cast<Creature *>(g_context->_location->_map->objectAt(coords));
	/* nothing attackable: move on to next tile */
	if (m == NULL || !m->isAttackable())
		return false;

	/* attack successful */
	/// TODO: CHEST: Make a user option to not make chests change battlefield
	/// map (1 of 2)
	ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS);
	if (!ground->isChest()) {
		ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS);
		if ((under = g_context->_location->_map->objectAt(g_context->_location->_coords)) &&
		        under->getTile().getTileType()->isShip())
			ground = under->getTile().getTileType();
	}

	/* You're attacking a townsperson!  Alert the guards! */
	if ((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR))
		g_context->_location->_map->alertGuards();

	/* not good karma to be killing the innocent.  Bad avatar! */
	if (m->isGood() || /* attacking a good creature */
	        /* attacking a docile (although possibly evil) person in town */
	        ((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR)))
		g_context->_party->adjustKarma(KA_ATTACKED_GOOD);

	CombatController *cc = new CombatController(CombatMap::mapForTile(ground, g_context->_party->getTransport().getTileType(), m));
	cc->init(m);
	cc->begin();
	return true;
}

void board() {
	if (g_context->_transportContext != TRANSPORT_FOOT) {
		screenMessage("Board: %cCan't!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	Object *obj = g_context->_location->_map->objectAt(g_context->_location->_coords);
	if (!obj) {
		screenMessage("%cBoard What?%c\n", FG_GREY, FG_WHITE);
		return;
	}

	const Tile *tile = obj->getTile().getTileType();
	if (tile->isShip()) {
		screenMessage("Board Frigate!\n");
		if (g_context->_lastShip != obj)
			g_context->_party->setShipHull(50);
	} else if (tile->isHorse())
		screenMessage("Mount Horse!\n");
	else if (tile->isBalloon())
		screenMessage("Board Balloon!\n");
	else {
		screenMessage("%cBoard What?%c\n", FG_GREY, FG_WHITE);
		return;
	}

	g_context->_party->setTransport(obj->getTile());
	g_context->_location->_map->removeObject(obj);
}


void castSpell(int player) {
	if (player == -1) {
		screenMessage("Cast Spell!\nPlayer: ");
		player = gameGetPlayer(false, true);
	}
	if (player == -1)
		return;

	// get the spell to cast
	g_context->_stats->setView(STATS_MIXTURES);
	screenMessage("Spell: ");
	// ### Put the iPad thing too.
#ifdef IOS
	U4IOS::IOSCastSpellHelper castSpellController;
#endif
	int spell = AlphaActionController::get('z', "Spell: ");
	if (spell == -1)
		return;

	screenMessage("%s!\n", spellGetName(spell)); //Prints spell name at prompt

	g_context->_stats->setView(STATS_PARTY_OVERVIEW);

	// if we can't really cast this spell, skip the extra parameters
	if (spellCheckPrerequisites(spell, player) != CASTERR_NOERROR) {
		gameCastSpell(spell, player, 0);
		return;
	}

	// Get the final parameters for the spell
	switch (spellGetParamType(spell)) {
	case Spell::PARAM_NONE:
		gameCastSpell(spell, player, 0);
		break;
	case Spell::PARAM_PHASE: {
		screenMessage("To Phase: ");
#ifdef IOS
		U4IOS::IOSConversationChoiceHelper choiceController;
		choiceController.fullSizeChoicePanel();
		choiceController.updateGateSpellChoices();
#endif
		int choice = ReadChoiceController::get("12345678 \033\n");
		if (choice < '1' || choice > '8')
			screenMessage("None\n");
		else {
			screenMessage("\n");
			gameCastSpell(spell, player, choice - '1');
		}
		break;
	}
	case Spell::PARAM_PLAYER: {
		screenMessage("Who: ");
		int subject = gameGetPlayer(true, false);
		if (subject != -1)
			gameCastSpell(spell, player, subject);
		break;
	}
	case Spell::PARAM_DIR:
		if (g_context->_location->_context == CTX_DUNGEON)
			gameCastSpell(spell, player, g_context->_saveGame->_orientation);
		else {
			screenMessage("Dir: ");
			Direction dir = gameGetDirection();
			if (dir != DIR_NONE)
				gameCastSpell(spell, player, (int) dir);
		}
		break;
	case Spell::PARAM_TYPEDIR: {
		screenMessage("Energy type? ");
#ifdef IOS
		U4IOS::IOSConversationChoiceHelper choiceController;
		choiceController.fullSizeChoicePanel();
		choiceController.updateEnergyFieldSpellChoices();
#endif
		EnergyFieldType fieldType = ENERGYFIELD_NONE;
		char key = ReadChoiceController::get("flps \033\n\r");
		switch (key) {
		case 'f':
			fieldType = ENERGYFIELD_FIRE;
			break;
		case 'l':
			fieldType = ENERGYFIELD_LIGHTNING;
			break;
		case 'p':
			fieldType = ENERGYFIELD_POISON;
			break;
		case 's':
			fieldType = ENERGYFIELD_SLEEP;
			break;
		default:
			break;
		}

		if (fieldType != ENERGYFIELD_NONE) {
			screenMessage("\n");

			Direction dir;
			if (g_context->_location->_context == CTX_DUNGEON)
				dir = (Direction)g_context->_saveGame->_orientation;
			else {
				screenMessage("Dir: ");
				dir = gameGetDirection();
			}

			if (dir != DIR_NONE) {

				/* Need to pack both dir and fieldType into param */
				int param = fieldType << 4;
				param |= (int) dir;

				gameCastSpell(spell, player, param);
			}
		} else {
			/* Invalid input here = spell failure */
			screenMessage("Failed!\n");

			/*
			 * Confirmed both mixture loss and mp loss in this situation in the
			 * original Ultima IV (at least, in the Amiga version.)
			 */
			//c->saveGame->_mixtures[castSpell]--;
			g_context->_party->member(player)->adjustMp(-spellGetRequiredMP(spell));
		}
		break;
	}
	case Spell::PARAM_FROMDIR: {
		screenMessage("From Dir: ");
		Direction dir = gameGetDirection();
		if (dir != DIR_NONE)
			gameCastSpell(spell, player, (int) dir);
		break;
	}
	}
}

void fire() {
	if (g_context->_transportContext != TRANSPORT_SHIP) {
		screenMessage("%cFire What?%c\n", FG_GREY, FG_WHITE);
		return;
	}

	screenMessage("Fire Cannon!\nDir: ");
	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	// can only fire broadsides
	int broadsidesDirs = dirGetBroadsidesDirs(g_context->_party->getDirection());
	if (!DIR_IN_MASK(dir, broadsidesDirs)) {
		screenMessage("%cBroadsides Only!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	// nothing (not even mountains!) can block cannonballs
	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), broadsidesDirs, g_context->_location->_coords,
	                           1, 3, NULL, false);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (fireAt(*i, true))
			return;
	}
}

bool fireAt(const Coords &coords, bool originAvatar) {
	bool validObject = false;
	bool hitsAvatar = false;
	bool objectHit = false;

	Object *obj = NULL;


	MapTile tile(g_context->_location->_map->_tileset->getByName("miss_flash")->getId());
	GameController::flashTile(coords, tile, 1);

	obj = g_context->_location->_map->objectAt(coords);
	Creature *m = dynamic_cast<Creature *>(obj);

	if (obj && obj->getType() == Object::CREATURE && m->isAttackable())
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

/**
 * Get the chest at the current x,y of the current context for player 'player'
 */
void getChest(int player) {
	screenMessage("Get Chest!\n");

	if (g_context->_party->isFlying()) {
		screenMessage("%cDrift only!%c\n", FG_GREY, FG_WHITE);
		return;
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
			screenMessage("Who opens? ");
			player = gameGetPlayer(false, true);
		}
		if (player == -1)
			return;

		if (obj)
			g_context->_location->_map->removeObject(obj);
		else {
			TileId newTile = g_context->_location->getReplacementTile(coords, tile);
			g_context->_location->_map->_annotations->add(coords, newTile, false , true);
		}

		// see if the chest is trapped and handle it
		getChestTrapHandler(player);

		screenMessage("The Chest Holds: %d Gold\n", g_context->_party->getChest());

		screenPrompt();

		if (isCity(g_context->_location->_map) && obj == NULL)
			g_context->_party->adjustKarma(KA_STOLE_CHEST);
	} else {
		screenMessage("%cNot Here!%c\n", FG_GREY, FG_WHITE);
	}
}

/**
 * Called by getChest() to handle possible traps on chests
 **/
bool getChestTrapHandler(int player) {
	TileEffect trapType;
	int randNum = xu4_random(4);

	/* Do we use u4dos's way of trap-determination, or the original intended way? */
	int passTest = (settings._enhancements && settings._enhancementsOptions._c64chestTraps) ?
	               (xu4_random(2) == 0) : /* xu4-enhanced */
	               ((randNum & 1) == 0); /* u4dos original way (only allows even numbers through, so only acid and poison show) */

	/* Chest is trapped! 50/50 chance */
	if (passTest) {
		/* Figure out which trap the chest has */
		switch (randNum & xu4_random(4)) {
		case 0:
			trapType = EFFECT_FIRE;
			break;   /* acid trap (56% chance - 9/16) */
		case 1:
			trapType = EFFECT_SLEEP;
			break;  /* sleep trap (19% chance - 3/16) */
		case 2:
			trapType = EFFECT_POISON;
			break; /* poison trap (19% chance - 3/16) */
		case 3:
			trapType = EFFECT_LAVA;
			break;   /* bomb trap (6% chance - 1/16) */
		default:
			trapType = EFFECT_FIRE;
			break;
		}

		/* apply the effects from the trap */
		if (trapType == EFFECT_FIRE)
			screenMessage("%cAcid%c Trap!\n", FG_RED, FG_WHITE);
		else if (trapType == EFFECT_POISON)
			screenMessage("%cPoison%c Trap!\n", FG_GREEN, FG_WHITE);
		else if (trapType == EFFECT_SLEEP)
			screenMessage("%cSleep%c Trap!\n", FG_PURPLE, FG_WHITE);
		else if (trapType == EFFECT_LAVA)
			screenMessage("%cBomb%c Trap!\n", FG_RED, FG_WHITE);

		// player is < 0 during the 'O'pen spell (immune to traps)
		//
		// if the chest was opened by a PC, see if the trap was
		// evaded by testing the PC's dex
		//
		if ((player >= 0) &&
		        (g_context->_saveGame->_players[player]._dex + 25 < xu4_random(100))) {
			if (trapType == EFFECT_LAVA) /* bomb trap */
				g_context->_party->applyEffect(trapType);
			else g_context->_party->member(player)->applyEffect(trapType);
		} else screenMessage("Evaded!\n");

		return true;
	}

	return false;
}

void holeUp() {
	screenMessage("Hole up & Camp!\n");

	if (!(g_context->_location->_context & (CTX_WORLDMAP | CTX_DUNGEON))) {
		screenMessage("%cNot here!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	if (g_context->_transportContext != TRANSPORT_FOOT) {
		screenMessage("%cOnly on foot!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	CombatController *cc = new CampController();
	cc->init(NULL);
	cc->begin();
}

/**
 * Initializes the moon state according to the savegame file. This method of
 * initializing the moons (rather than just setting them directly) is necessary
 * to make sure trammel and felucca stay in sync
 */
void GameController::initMoons() {
	int trammelphase = g_context->_saveGame->_trammelPhase,
	    feluccaphase = g_context->_saveGame->_feluccaPhase;

	ASSERT(g_context != NULL, "Game context doesn't exist!");
	ASSERT(g_context->_saveGame != NULL, "Savegame doesn't exist!");
	//ASSERT(mapIsWorldMap(c->location->map) && c->location->viewMode == VIEW_NORMAL, "Can only call gameInitMoons() from the world map!");

	g_context->_saveGame->_trammelPhase = g_context->_saveGame->_feluccaPhase = 0;
	g_context->_moonPhase = 0;

	while ((g_context->_saveGame->_trammelPhase != trammelphase) ||
	        (g_context->_saveGame->_feluccaPhase != feluccaphase))
		updateMoons(false);
}

/**
 * Updates the phases of the moons and shows
 * the visual moongates on the map, if desired
 */
void GameController::updateMoons(bool showmoongates) {
	int realMoonPhase,
	    oldTrammel,
	    trammelSubphase;
	const Coords *gate;

	if (g_context->_location->_map->isWorldMap()) {
		oldTrammel = g_context->_saveGame->_trammelPhase;

		if (++g_context->_moonPhase >= MOON_PHASES * MOON_SECONDS_PER_PHASE * 4)
			g_context->_moonPhase = 0;

		trammelSubphase = g_context->_moonPhase % (MOON_SECONDS_PER_PHASE * 4 * 3);
		realMoonPhase = (g_context->_moonPhase / (4 * MOON_SECONDS_PER_PHASE));

		g_context->_saveGame->_trammelPhase = realMoonPhase / 3;
		g_context->_saveGame->_feluccaPhase = realMoonPhase % 8;

		if (g_context->_saveGame->_trammelPhase > 7)
			g_context->_saveGame->_trammelPhase = 7;

		if (showmoongates) {
			/* update the moongates if trammel changed */
			if (trammelSubphase == 0) {
				gate = moongateGetGateCoordsForPhase(oldTrammel);
				if (gate)
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate)
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
			} else if (trammelSubphase == 1) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
				}
			} else if (trammelSubphase == 2) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
				}
			} else if (trammelSubphase == 3) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
				}
			} else if ((trammelSubphase > 3) && (trammelSubphase < (MOON_SECONDS_PER_PHASE * 4 * 3) - 3)) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 3) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x43));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 2) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x42));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
				}
			} else if (trammelSubphase == (MOON_SECONDS_PER_PHASE * 4 * 3) - 1) {
				gate = moongateGetGateCoordsForPhase(g_context->_saveGame->_trammelPhase);
				if (gate) {
					g_context->_location->_map->_annotations->remove(*gate, g_context->_location->_map->translateFromRawTileIndex(0x41));
					g_context->_location->_map->_annotations->add(*gate, g_context->_location->_map->translateFromRawTileIndex(0x40));
				}
			}
		}
	}
}

/**
 * Handles feedback after avatar moved during normal 3rd-person view.
 */
void GameController::avatarMoved(MoveEvent &event) {
	if (event._userEvent) {

		// is filterMoveMessages even used?  it doesn't look like the option is hooked up in the configuration menu
		if (!settings._filterMoveMessages) {
			switch (g_context->_transportContext) {
			case TRANSPORT_FOOT:
			case TRANSPORT_HORSE:
				screenMessage("%s\n", getDirectionName(event._dir));
				break;
			case TRANSPORT_SHIP:
				if (event._result & MOVE_TURNED)
					screenMessage("Turn %s!\n", getDirectionName(event._dir));
				else if (event._result & MOVE_SLOWED)
					screenMessage("%cSlow progress!%c\n", FG_GREY, FG_WHITE);
				else
					screenMessage("Sail %s!\n", getDirectionName(event._dir));
				break;
			case TRANSPORT_BALLOON:
				screenMessage("%cDrift Only!%c\n", FG_GREY, FG_WHITE);
				break;
			default:
				ASSERT(0, "bad transportContext %d in avatarMoved()", g_context->_transportContext);
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
					openAt(new_coords);
					event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
				} else if (tile->getTileType()->isLockedDoor()) {
					jimmyAt(new_coords);
					event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
				} /*else if (mapPersonAt(c->location->map, new_coords) != NULL) {
                    talkAtCoord(newx, newy, 1, NULL);
                    event.result = MOVE_SUCCEEDED | MOVE_END_TURN;
                    }*/
			}

			/* if we're still blocked */
			if ((event._result & MOVE_BLOCKED) && !settings._filterMoveMessages) {
				soundPlay(SOUND_BLOCKED, false);
				screenMessage("%cBlocked!%c\n", FG_GREY, FG_WHITE);
			}
		} else if (g_context->_transportContext == TRANSPORT_FOOT || g_context->_transportContext == TRANSPORT_HORSE) {
			/* movement was slowed */
			if (event._result & MOVE_SLOWED) {
				soundPlay(SOUND_WALK_SLOWED);
				screenMessage("%cSlow progress!%c\n", FG_GREY, FG_WHITE);
			} else {
				soundPlay(SOUND_WALK_NORMAL);
			}
		}
	}

	/* exited map */
	if (event._result & MOVE_EXIT_TO_PARENT) {
		screenMessage("%cLeaving...%c\n", FG_GREY, FG_WHITE);
		exitToParentMap();
		musicMgr->play();
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

/**
 * Handles feedback after moving the avatar in the 3-d dungeon view.
 */
void GameController::avatarMovedInDungeon(MoveEvent &event) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	Direction realDir = dirNormalize((Direction)g_context->_saveGame->_orientation, event._dir);

	if (!settings._filterMoveMessages) {
		if (event._userEvent) {
			if (event._result & MOVE_TURNED) {
				if (dirRotateCCW((Direction)g_context->_saveGame->_orientation) == realDir)
					screenMessage("Turn Left\n");
				else screenMessage("Turn Right\n");
			}
			/* show 'Advance' or 'Retreat' in dungeons */
			else screenMessage("%s\n", realDir == g_context->_saveGame->_orientation ? "Advance" : "Retreat");
		}

		if (event._result & MOVE_BLOCKED)
			screenMessage("%cBlocked!%c\n", FG_GREY, FG_WHITE);
	}

	/* if we're exiting the map, do this */
	if (event._result & MOVE_EXIT_TO_PARENT) {
		screenMessage("%cLeaving...%c\n", FG_GREY, FG_WHITE);
		exitToParentMap();
		musicMgr->play();
	}

	/* check to see if we're entering a dungeon room */
	if (event._result & MOVE_SUCCEEDED) {
		if (dungeon->currentToken() == DUNGEON_ROOM) {
			int room = (int)dungeon->currentSubToken(); /* get room number */

			/**
			 * recalculate room for the abyss -- there are 16 rooms for every 2 levels,
			 * each room marked with 0xD* where (* == room number 0-15).
			 * for levels 1 and 2, there are 16 rooms, levels 3 and 4 there are 16 rooms, etc.
			 */
			if (g_context->_location->_map->_id == MAP_ABYSS)
				room = (0x10 * (g_context->_location->_coords.z / 2)) + room;

			Dungeon *dng = dynamic_cast<Dungeon *>(g_context->_location->_map);
			dng->_currentRoom = room;

			/* set the map and start combat! */
			CombatController *cc = new CombatController(dng->_roomMaps[room]);
			cc->initDungeonRoom(room, dirReverse(realDir));
			cc->begin();
		}
	}
}

void jimmy() {
	screenMessage("Jimmy: ");
	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
	                           1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (jimmyAt(*i))
			return;
	}

	screenMessage("%cJimmy what?%c\n", FG_GREY, FG_WHITE);
}

/**
 * Attempts to jimmy a locked door at map coordinates x,y.  The locked
 * door is replaced by a permanent annotation of an unlocked door
 * tile.
 */
bool jimmyAt(const Coords &coords) {
	MapTile *tile = g_context->_location->_map->tileAt(coords, WITH_OBJECTS);

	if (!tile->getTileType()->isLockedDoor())
		return false;

	if (g_context->_saveGame->_keys) {
		Tile *door = g_context->_location->_map->_tileset->getByName("door");
		ASSERT(door, "no door tile found in tileset");
		g_context->_saveGame->_keys--;
		g_context->_location->_map->_annotations->add(coords, door->getId());
		screenMessage("\nUnlocked!\n");
	} else
		screenMessage("%cNo keys left!%c\n", FG_GREY, FG_WHITE);

	return true;
}

void opendoor() {
	///  XXX: Pressing "o" should close any open door.

	screenMessage("Open: ");

	if (g_context->_party->isFlying()) {
		screenMessage("%cNot Here!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
	                           1, 1, NULL, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (openAt(*i))
			return;
	}

	screenMessage("%cNot Here!%c\n", FG_GREY, FG_WHITE);
}

/**
 * Attempts to open a door at map coordinates x,y.  The door is
 * replaced by a temporary annotation of a floor tile for 4 turns.
 */
bool openAt(const Coords &coords) {
	const Tile *tile = g_context->_location->_map->tileTypeAt(coords, WITH_OBJECTS);

	if (!tile->isDoor() &&
	        !tile->isLockedDoor())
		return false;

	if (tile->isLockedDoor()) {
		screenMessage("%cCan't!%c\n", FG_GREY, FG_WHITE);
		return true;
	}

	Tile *floor = g_context->_location->_map->_tileset->getByName("brick_floor");
	ASSERT(floor, "no floor tile found in tileset");
	g_context->_location->_map->_annotations->add(coords, floor->getId(), false, true)->setTTL(4);

	screenMessage("\nOpened!\n");

	return true;
}

/**
 * Readies a weapon for a player.  Prompts for the player and/or the
 * weapon if not provided.
 */
void readyWeapon(int player) {

	// get the player if not provided
	if (player == -1) {
		screenMessage("Ready a weapon for: ");
		player = gameGetPlayer(true, false);
		if (player == -1)
			return;
	}

	// get the weapon to use
	g_context->_stats->setView(STATS_WEAPONS);
	screenMessage("Weapon: ");
	WeaponType weapon = (WeaponType) AlphaActionController::get(WEAP_MAX + 'a' - 1, "Weapon: ");
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	if (weapon == -1)
		return;

	PartyMember *p = g_context->_party->member(player);
	const Weapon *w = Weapon::get(weapon);


	if (!w) {
		screenMessage("\n");
		return;
	}
	switch (p->setWeapon(w)) {
	case EQUIP_SUCCEEDED:
		screenMessage("%s\n", w->getName().c_str());
		break;
	case EQUIP_NONE_LEFT:
		screenMessage("%cNone left!%c\n", FG_GREY, FG_WHITE);
		break;
	case EQUIP_CLASS_RESTRICTED: {
		Common::String indef_article;

		switch (tolower(w->getName()[0])) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'y':
			indef_article = "an";
			break;
		default:
			indef_article = "a";
			break;
		}

		screenMessage("\n%cA %s may NOT use %s %s%c\n", FG_GREY, getClassName(p->getClass()),
		              indef_article.c_str(), w->getName().c_str(), FG_WHITE);
		break;
	}
	}
}

void talk() {
	screenMessage("Talk: ");

	if (g_context->_party->isFlying()) {
		screenMessage("%cDrift only!%c\n", FG_GREY, FG_WHITE);
		return;
	}

	Direction dir = gameGetDirection();

	if (dir == DIR_NONE)
		return;

	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, g_context->_location->_coords,
	                           1, 2, &Tile::canTalkOverTile, true);
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (talkAt(*i))
			return;
	}

	screenMessage("Funny, no response!\n");
}

/**
 * Mixes reagents.  Prompts for a spell, then which reagents to
 * include in the mix.
 */
void mixReagents() {

	/*  uncomment this line to activate new spell mixing code */
	//   return mixReagentsSuper();
	bool done = false;

	while (!done) {
		screenMessage("Mix reagents\n");
#ifdef IOS
		U4IOS::beginMixSpellController();
		return; // Just return, the dialog takes control from here.
#endif

		// Verify that there are reagents remaining in the inventory
		bool found = false;
		for (int i = 0; i < 8; i++) {
			if (g_context->_saveGame->_reagents[i] > 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			screenMessage("%cNone Left!%c", FG_GREY, FG_WHITE);
			done = true;
		} else {
			screenMessage("For Spell: ");
			g_context->_stats->setView(STATS_MIXTURES);

			int choice = ReadChoiceController::get("abcdefghijklmnopqrstuvwxyz \033\n\r");
			if (choice == ' ' || choice == '\033' || choice == '\n' || choice == '\r')
				break;

			int spell = choice - 'a';
			screenMessage("%s\n", spellGetName(spell));

			// ensure the mixtures for the spell isn't already maxed out
			if (g_context->_saveGame->_mixtures[spell] == 99) {
				screenMessage("\n%cYou cannot mix any more of that spell!%c\n", FG_GREY, FG_WHITE);
				break;
			}

			// Reset the reagent spell mix menu by removing
			// the menu highlight from the current item, and
			// hiding reagents that you don't have
			g_context->_stats->resetReagentsMenu();

			g_context->_stats->setView(MIX_REAGENTS);
			if (settings._enhancements && settings._enhancementsOptions._u5spellMixing)
				done = mixReagentsForSpellU5(spell);
			else
				done = mixReagentsForSpellU4(spell);
		}
	}

	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	screenMessage("\n\n");
}

/**
 * Prompts for spell reagents to mix in the traditional Ultima IV
 * style.
 */
bool mixReagentsForSpellU4(int spell) {
	Ingredients ingredients;

	screenMessage("Reagent: ");

	while (1) {
		int choice = ReadChoiceController::get("abcdefgh\n\r \033");

		// done selecting reagents? mix it up and prompt to mix
		// another spell
		if (choice == '\n' || choice == '\r' || choice == ' ') {
			screenMessage("\n\nYou mix the Reagents, and...\n");

			if (spellMix(spell, &ingredients))
				screenMessage("Success!\n\n");
			else
				screenMessage("It Fizzles!\n\n");

			return false;
		}

		// escape: put ingredients back and quit mixing
		if (choice == '\033') {
			ingredients.revert();
			return true;
		}

		screenMessage("%c\n", toupper(choice));
		if (!ingredients.addReagent((Reagent)(choice - 'a')))
			screenMessage("%cNone Left!%c\n", FG_GREY, FG_WHITE);
		screenMessage("Reagent: ");
	}

	return true;
}

/**
 * Prompts for spell reagents to mix with an Ultima V-like menu.
 */
bool mixReagentsForSpellU5(int spell) {
	Ingredients ingredients;

	screenDisableCursor();

	g_context->_stats->getReagentsMenu()->reset(); // reset the menu, highlighting the first item
	ReagentsMenuController getReagentsController(g_context->_stats->getReagentsMenu(), &ingredients, g_context->_stats->getMainArea());
	eventHandler->pushController(&getReagentsController);
	getReagentsController.waitFor();

	g_context->_stats->getMainArea()->disableCursor();
	screenEnableCursor();

	screenMessage("How many? ");

	int howmany = ReadIntController::get(2, TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
	gameSpellMixHowMany(spell, howmany, &ingredients);

	return true;
}

/**
 * Exchanges the position of two players in the party.  Prompts the
 * user for the player numbers.
 */
void newOrder() {
	screenMessage("New Order!\nExchange # ");

	int player1 = gameGetPlayer(true, false);

	if (player1 == -1)
		return;

	if (player1 == 0) {
		screenMessage("%s, You must lead!\n", g_context->_party->member(0)->getName().c_str());
		return;
	}

	screenMessage("    with # ");

	int player2 = gameGetPlayer(true, false);

	if (player2 == -1)
		return;

	if (player2 == 0) {
		screenMessage("%s, You must lead!\n", g_context->_party->member(0)->getName().c_str());
		return;
	}

	if (player1 == player2) {
		screenMessage("%cWhat?%c\n", FG_GREY, FG_WHITE);
		return;
	}

	g_context->_party->swapPlayers(player1, player2);
}

/**
 * Peers at a city from A-P (Lycaeum telescope) and functions like a gem
 */
bool gamePeerCity(int city, void *data) {
	Map *peerMap;

	peerMap = mapMgr->get((MapId)(city + 1));

	if (peerMap != NULL) {
		g_game->setMap(peerMap, 1, NULL);
		g_context->_location->_viewMode = VIEW_GEM;
		g_game->_paused = true;
		g_game->_pausedTimer = 0;

		screenDisableCursor();
#ifdef IOS
		U4IOS::IOSConversationChoiceHelper continueHelper;
		continueHelper.updateChoices(" ");
		continueHelper.fullSizeChoicePanel();
#endif
		ReadChoiceController::get("\015 \033");

		g_game->exitToParentMap();
		screenEnableCursor();
		g_game->_paused = false;

		return true;
	}
	return false;
}

/**
 * Peers at a gem
 */
void peer(bool useGem) {

	if (useGem) {
		if (g_context->_saveGame->_gems <= 0) {
			screenMessage("%cPeer at What?%c\n", FG_GREY, FG_WHITE);
			return;
		}

		g_context->_saveGame->_gems--;
		screenMessage("Peer at a Gem!\n");
	}

	g_game->_paused = true;
	g_game->_pausedTimer = 0;
	screenDisableCursor();

	g_context->_location->_viewMode = VIEW_GEM;
#ifdef IOS
	U4IOS::IOSConversationChoiceHelper continueHelper;
	continueHelper.updateChoices(" ");
	continueHelper.fullSizeChoicePanel();
#endif
	ReadChoiceController::get("\015 \033");

	screenEnableCursor();
	g_context->_location->_viewMode = VIEW_NORMAL;
	g_game->_paused = false;
}

/**
 * Begins a conversation with the NPC at map coordinates x,y.  If no
 * NPC is present at that point, zero is returned.
 */
bool talkAt(const Coords &coords) {
	extern int personIsVendor(const Person * person);
	City *city;

	/* can't have any conversations outside of town */
	if (!isCity(g_context->_location->_map)) {
		screenMessage("Funny, no response!\n");
		return true;
	}

	city = dynamic_cast<City *>(g_context->_location->_map);
	Person *talker = city->personAt(coords);

	/* make sure we have someone we can talk with */
	if (!talker || !talker->canConverse())
		return false;

	/* No response from alerted guards... does any monster both
	   attack and talk besides Nate the Snake? */
	if (talker->getMovementBehavior() == MOVEMENT_ATTACK_AVATAR &&
	        talker->getId() != PYTHON_ID)
		return false;

	/* if we're talking to Lord British and the avatar is dead, LB resurrects them! */
	if (talker->getNpcType() == NPC_LORD_BRITISH &&
	        g_context->_party->member(0)->getStatus() == STAT_DEAD) {
		screenMessage("%s, Thou shalt live again!\n", g_context->_party->member(0)->getName().c_str());

		g_context->_party->member(0)->setStatus(STAT_GOOD);
		g_context->_party->member(0)->heal(HT_FULLHEAL);
		gameSpellEffect('r', -1, SOUND_LBHEAL);
	}

	Conversation conv;
	TRACE_LOCAL(gameDbg, "Setting up script information providers.");
	conv.script->addProvider("party", g_context->_party);
	conv.script->addProvider("context", g_context);

	conv.state = Conversation::INTRO;
	conv.reply = talker->getConversationText(&conv, "");
	conv.playerInput.clear();
	talkRunConversation(conv, talker, false);

	return true;
}

/**
 * Executes the current conversation until it is done.
 */
void talkRunConversation(Conversation &conv, Person *talker, bool showPrompt) {
	while (conv.state != Conversation::DONE) {
		// TODO: instead of calculating linesused again, cache the
		// result in person.cpp somewhere.
		int linesused = linecount(conv.reply.front(), TEXT_AREA_W);
		screenMessage("%s", conv.reply.front().c_str());
		conv.reply.pop_front();

		/* if all chunks haven't been shown, wait for a key and process next chunk*/
		int size = conv.reply.size();
		if (size > 0) {
#ifdef IOS
			U4IOS::IOSConversationChoiceHelper continueDialog;
			continueDialog.updateChoices(" ");
#endif
			ReadChoiceController::get("");
			continue;
		}

		/* otherwise, clear current reply and proceed based on conversation state */
		conv.reply.clear();

		/* they're attacking you! */
		if (conv.state == Conversation::ATTACK) {
			conv.state = Conversation::DONE;
			talker->setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
		}

		if (conv.state == Conversation::DONE)
			break;

		/* When Lord British heals the party */
		else if (conv.state == Conversation::FULLHEAL) {
			int i;

			for (i = 0; i < g_context->_party->size(); i++) {
				g_context->_party->member(i)->heal(HT_CURE);        // cure the party
				g_context->_party->member(i)->heal(HT_FULLHEAL);    // heal the party
			}
			gameSpellEffect('r', -1, SOUND_MAGIC); // same spell effect as 'r'esurrect

			conv.state = Conversation::TALK;
		}
		/* When Lord British checks and advances each party member's level */
		else if (conv.state == Conversation::ADVANCELEVELS) {
			gameLordBritishCheckLevels();
			conv.state = Conversation::TALK;
		}

		if (showPrompt) {
			Common::String prompt = talker->getPrompt(&conv);
			if (!prompt.empty()) {
				if (linesused + linecount(prompt, TEXT_AREA_W) > TEXT_AREA_H) {
#ifdef IOS
					U4IOS::IOSConversationChoiceHelper continueDialog;
					continueDialog.updateChoices(" ");
#endif
					ReadChoiceController::get("");
				}

				screenMessage("%s", prompt.c_str());
			}
		}

		int maxlen;
		switch (conv.getInputRequired(&maxlen)) {
		case Conversation::INPUT_STRING: {
			conv.playerInput = gameGetInput(maxlen);
#ifdef IOS
			screenMessage("%s", conv.playerInput.c_str()); // Since we put this in a different window, we need to show it again.
#endif
			conv.reply = talker->getConversationText(&conv, conv.playerInput.c_str());
			conv.playerInput.clear();
			showPrompt = true;
			break;
		}
		case Conversation::INPUT_CHARACTER: {
			char message[2];
#ifdef IOS
			U4IOS::IOSConversationChoiceHelper yesNoHelper;
			yesNoHelper.updateChoices("yn ");
#endif
			int choice = ReadChoiceController::get("");


			message[0] = choice;
			message[1] = '\0';

			conv.reply = talker->getConversationText(&conv, message);
			conv.playerInput.clear();

			showPrompt = true;
			break;
		}

		case Conversation::INPUT_NONE:
			conv.state = Conversation::DONE;
			break;
		}
	}
	if (conv.reply.size() > 0)
		screenMessage("%s", conv.reply.front().c_str());
}

/**
 * Changes a player's armor.  Prompts for the player and/or the armor
 * type if not provided.
 */
void wearArmor(int player) {

	// get the player if not provided
	if (player == -1) {
		screenMessage("Wear Armour\nfor: ");
		player = gameGetPlayer(true, false);
		if (player == -1)
			return;
	}

	g_context->_stats->setView(STATS_ARMOR);
	screenMessage("Armour: ");
	ArmorType armor = (ArmorType) AlphaActionController::get(ARMR_MAX + 'a' - 1, "Armour: ");
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	if (armor == -1)
		return;

	const Armor *a = Armor::get(armor);
	PartyMember *p = g_context->_party->member(player);

	if (!a) {
		screenMessage("\n");
		return;
	}
	switch (p->setArmor(a)) {
	case EQUIP_SUCCEEDED:
		screenMessage("%s\n", a->getName().c_str());
		break;
	case EQUIP_NONE_LEFT:
		screenMessage("%cNone left!%c\n", FG_GREY, FG_WHITE);
		break;
	case EQUIP_CLASS_RESTRICTED:
		screenMessage("\n%cA %s may NOT use %s%c\n", FG_GREY, getClassName(p->getClass()), a->getName().c_str(), FG_WHITE);
		break;
	}
}

/**
 * Called when the player selects a party member for ztats
 */
void ztatsFor(int player) {
	// get the player if not provided
	if (player == -1) {
		screenMessage("Ztats for: ");
		player = gameGetPlayer(true, false);
		if (player == -1)
			return;
	}

	// Reset the reagent spell mix menu by removing
	// the menu highlight from the current item, and
	// hiding reagents that you don't have
	g_context->_stats->resetReagentsMenu();

	g_context->_stats->setView(StatsView(STATS_CHAR1 + player));
#ifdef IOS
	U4IOS::IOSHideActionKeysHelper hideExtraControls;
#endif
	ZtatsController ctrl;
	eventHandler->pushController(&ctrl);
	ctrl.waitFor();
}

/**
 * This function is called every quarter second.
 */
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

		screenCycle();

		/*
		 * refresh the screen only if the timer queue is empty --
		 * i.e. drop a frame if another timer event is about to be fired
		 */
		if (eventHandler->timerQueueEmpty())
			gameUpdateScreen();

		/*
		 * force pass if no commands within last 20 seconds
		 */
		Controller *controller = eventHandler->getController();
		if (controller != NULL && (eventHandler->getController() == g_game || dynamic_cast<CombatController *>(eventHandler->getController()) != NULL) &&
		        gameTimeSinceLastCommand() > 20) {

			/* pass the turn, and redraw the text area so the prompt is shown */
			controller->keyPressed(U4_SPACE);
			screenRedrawTextArea(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_W, TEXT_AREA_H);
		}
	}

}

/**
 * Checks the hull integrity of the ship and handles
 * the ship sinking, if necessary
 */
void gameCheckHullIntegrity() {
	int i;

	bool killAll = false;
	/* see if the ship has sunk */
	if ((g_context->_transportContext == TRANSPORT_SHIP) && g_context->_saveGame->_shipHull <= 0) {
		screenMessage("\nThy ship sinks!\n\n");
		killAll = true;
	}


	if (!collisionOverride && g_context->_transportContext == TRANSPORT_FOOT &&
	        g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS)->isSailable() &&
	        !g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS)->isShip() &&
	        !g_context->_location->_map->getValidMoves(g_context->_location->_coords, g_context->_party->getTransport())) {
		screenMessage("\nTrapped at sea without thy ship, thou dost drown!\n\n");
		killAll = true;
	}

	if (killAll) {
		for (i = 0; i < g_context->_party->size(); i++) {
			g_context->_party->member(i)->setHp(0);
			g_context->_party->member(i)->setStatus(STAT_DEAD);
		}

		screenRedrawScreen();
		deathStart(5);
	}
}

/**
 * Checks for valid conditions and handles
 * special creatures guarding the entrance to the
 * abyss and to the shrine of spirituality
 */
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

/**
 * Checks for and handles when the avatar steps on a moongate
 */
bool GameController::checkMoongates() {
	Coords dest;

	if (moongateFindActiveGateAt(g_context->_saveGame->_trammelPhase, g_context->_saveGame->_feluccaPhase, g_context->_location->_coords, dest)) {

		gameSpellEffect(-1, -1, SOUND_MOONGATE); // Default spell effect (screen inversion without 'spell' sound effects)

		if (g_context->_location->_coords != dest) {
			g_context->_location->_coords = dest;
			gameSpellEffect(-1, -1, SOUND_MOONGATE); // Again, after arriving
		}

		if (moongateIsEntryToShrineOfSpirituality(g_context->_saveGame->_trammelPhase, g_context->_saveGame->_feluccaPhase)) {
			Shrine *shrine_spirituality;

			shrine_spirituality = dynamic_cast<Shrine *>(mapMgr->get(MAP_SHRINE_SPIRITUALITY));

			if (!g_context->_party->canEnterShrine(VIRT_SPIRITUALITY))
				return true;

			setMap(shrine_spirituality, 1, NULL);
			musicMgr->play();

			shrine_spirituality->enter();
		}

		return true;
	}

	return false;
}

/**
 * Fixes objects initially loaded by saveGameMonstersRead,
 * and alters movement behavior accordingly to match the creature
 */
void gameFixupObjects(Map *map) {
	int i;
	Object *obj;

	/* add stuff from the monster table to the map */
	for (i = 0; i < MONSTERTABLE_SIZE; i++) {
		SaveGameMonsterRecord *monster = &map->_monsterTable[i];
		if (monster->_prevTile != 0) {
			Coords coords(monster->_x, monster->_y);

			// tile values stored in monsters.sav hardcoded to index into base tilemap
			MapTile tile = TileMap::get("base")->translate(monster->_tile),
			        oldTile = TileMap::get("base")->translate(monster->_prevTile);

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

time_t gameTimeSinceLastCommand() {
	return g_system->getMillis() - g_context->_lastCommandTime;
}

/**
 * Handles what happens when a creature attacks you
 */
void gameCreatureAttack(Creature *m) {
	Object *under;
	const Tile *ground;

	screenMessage("\nAttacked by %s\n", m->getName().c_str());

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

/**
 * Performs a ranged attack for the creature at x,y on the world map
 */
bool creatureRangeAttack(const Coords &coords, Creature *m) {
//    int attackdelay = MAX_BATTLE_SPEED - settings.battleSpeed;

	// Figure out what the ranged attack should look like
	MapTile tile(g_context->_location->_map->_tileset->getByName((m && !m->getWorldrangedtile().empty()) ?
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
		else gameDamageParty(10, 25);

		return true;
	}
	// Destroy objects that were hit
	else if (obj) {
		if ((obj->getType() == Object::CREATURE && m->isAttackable()) ||
		        obj->getType() == Object::UNKNOWN) {

			GameController::flashTile(coords, tile, 3);
			g_context->_location->_map->removeObject(obj);

			return true;
		}
	}
	return false;
}

/**
 * Gets the path of coordinates for an action.  Each tile in the
 * direction specified by dirmask, between the minimum and maximum
 * distances given, is included in the path, until blockedPredicate
 * fails.  If a tile is blocked, that tile is included in the path
 * only if includeBlocked is true.
 */
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

/**
 * Deals an amount of damage between 'minDamage' and 'maxDamage'
 * to each party member, with a 50% chance for each member to
 * avoid the damage.  If (minDamage == -1) or (minDamage >= maxDamage),
 * deals 'maxDamage' damage to each member.
 */
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

	screenShake(1);

	// Un-highlight the last player
	if (lastdmged != -1) g_context->_stats->highlightPlayer(lastdmged);
}

/**
 * Deals an amount of damage between 'minDamage' and 'maxDamage'
 * to the ship.  If (minDamage == -1) or (minDamage >= maxDamage),
 * deals 'maxDamage' damage to the ship.
 */
void gameDamageShip(int minDamage, int maxDamage) {
	int damage;

	if (g_context->_transportContext == TRANSPORT_SHIP) {
		damage = ((minDamage >= 0) && (minDamage < maxDamage)) ?
		         xu4_random((maxDamage + 1) - minDamage) + minDamage :
		         maxDamage;

		screenShake(1);

		g_context->_party->damageShip(damage);
		gameCheckHullIntegrity();
	}
}

/**
 * Sets (or unsets) the active player
 */
void gameSetActivePlayer(int player) {
	if (player == -1) {
		g_context->_party->setActivePlayer(-1);
		screenMessage("Set Active Player: None!\n");
	} else if (player < g_context->_party->size()) {
		screenMessage("Set Active Player: %s!\n", g_context->_party->member(player)->getName().c_str());
		if (g_context->_party->member(player)->isDisabled())
			screenMessage("Disabled!\n");
		else
			g_context->_party->setActivePlayer(player);
	}
}

/**
 * Removes creatures from the current map if they are too far away from the avatar
 */
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
		} else i++;
	}
}

/**
 * Checks creature conditions and spawns new creatures if necessary
 */
void GameController::checkRandomCreatures() {
	int canSpawnHere = g_context->_location->_map->isWorldMap() || g_context->_location->_context & CTX_DUNGEON;
#ifdef IOS
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

	gameSpawnCreature(NULL);
}

/**
 * Handles trolls under bridges
 */
void GameController::checkBridgeTrolls() {
	const Tile *bridge = g_context->_location->_map->_tileset->getByName("bridge");
	if (!bridge)
		return;

	// TODO: CHEST: Make a user option to not make chests block bridge trolls
	if (!g_context->_location->_map->isWorldMap() ||
	        g_context->_location->_map->tileAt(g_context->_location->_coords, WITH_OBJECTS)->_id != bridge->getId() ||
	        xu4_random(8) != 0)
		return;

	screenMessage("\nBridge Trolls!\n");

	Creature *m = g_context->_location->_map->addCreature(creatureMgr->getById(TROLL_ID), g_context->_location->_coords);
	CombatController *cc = new CombatController(MAP_BRIDGE_CON);
	cc->init(m);
	cc->begin();
}

/**
 * Check the levels of each party member while talking to Lord British
 */
void gameLordBritishCheckLevels() {
	bool advanced = false;

	for (int i = 0; i < g_context->_party->size(); i++) {
		PartyMember *player = g_context->_party->member(i);
		if (player->getRealLevel() <
		        player->getMaxLevel())

			// add an extra space to separate messages
			if (!advanced) {
				screenMessage("\n");
				advanced = true;
			}

		player->advanceLevel();
	}

	screenMessage("\nWhat would thou\nask of me?\n");
}

/**
 * Spawns a creature (m) just offscreen of the avatar.
 * If (m==NULL) then it finds its own creature to spawn and spawns it.
 */
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
				else tries++;
			} else ok = true;
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

/**
 * Destroys all creatures on the current map.
 */
void gameDestroyAllCreatures(void) {
	int i;

	gameSpellEffect('t', -1, SOUND_MAGIC); /* same effect as tremor */

	if (g_context->_location->_context & CTX_COMBAT) {
		/* destroy all creatures in combat */
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
		/* destroy all creatures on the map */
		ObjectDeque::iterator current;
		Map *map = g_context->_location->_map;

		for (current = map->_objects.begin(); current != map->_objects.end();) {
			Creature *m = dynamic_cast<Creature *>(*current);

			if (m) {
				/* the skull does not destroy Lord British */
				if (m->getId() != LORDBRITISH_ID)
					current = map->removeObject(current);
				else current++;
			} else current++;
		}
	}

	/* alert the guards! Really, the only one left should be LB himself :) */
	g_context->_location->_map->alertGuards();
}

/**
 * Creates the balloon near Hythloth, but only if the balloon doesn't already exists somewhere
 */
bool GameController::createBalloon(Map *map) {
	ObjectDeque::iterator i;

	/* see if the balloon has already been created (and not destroyed) */
	for (i = map->_objects.begin(); i != map->_objects.end(); i++) {
		Object *obj = *i;
		if (obj->getTile().getTileType()->isBalloon())
			return false;
	}

	const Tile *balloon = map->_tileset->getByName("balloon");
	ASSERT(balloon, "no balloon tile found in tileset");
	map->addObject(balloon->getId(), balloon->getId(), map->getLabel("balloon"));
	return true;
}

// Colors assigned to reagents based on my best reading of them
// from the book of wisdom.  Maybe we could use BOLD to distinguish
// the two grey and the two red reagents.
const int colors[] = {
	FG_YELLOW, FG_GREY, FG_BLUE, FG_WHITE, FG_RED, FG_GREY, FG_GREEN, FG_RED
};

void
showMixturesSuper(int page = 0) {
	screenTextColor(FG_WHITE);
	for (int i = 0; i < 13; i++) {
		char buf[4];

		const Spell *s = getSpell(i + 13 * page);
		int line = i + 8;
		screenTextAt(2, line, "%s", s->_name);

		snprintf(buf, 4, "%3d", g_context->_saveGame->_mixtures[i + 13 * page]);
		screenTextAt(6, line, "%s", buf);

		screenShowChar(32, 9, line);
		int comp = s->_components;
		for (int j = 0; j < 8; j++) {
			screenTextColor(colors[j]);
			screenShowChar(comp & (1 << j) ? CHARSET_BULLET : ' ', 10 + j, line);
		}
		screenTextColor(FG_WHITE);

		snprintf(buf, 3, "%2d", s->_mp);
		screenTextAt(19, line, "%s", buf);
	}
}

void
mixReagentsSuper() {

	screenMessage("Mix reagents\n");

	static int page = 0;

	struct ReagentShop {
		const char *name;
		int price[6];
	};
	ReagentShop shops[] = {
		{ "BuccDen", {6, 7, 9, 9, 9, 1} },
		{ "Moonglo", {2, 5, 6, 3, 6, 9} },
		{ "Paws",    {3, 4, 2, 8, 6, 7} },
		{ "SkaraBr", {2, 4, 9, 6, 4, 8} },
	};
	const int shopcount = sizeof(shops) / sizeof(shops[0]);

	int oldlocation = g_context->_location->_viewMode;
	g_context->_location->_viewMode = VIEW_MIXTURES;
	screenUpdate(&g_game->_mapArea, true, true);

	screenTextAt(16, 2, "%s", "<-Shops");

	g_context->_stats->setView(StatsView(STATS_REAGENTS));
	screenTextColor(FG_PURPLE);
	screenTextAt(2, 7, "%s", "SPELL # Reagents MP");

	for (int i = 0; i < shopcount; i++) {
		int line = i + 1;
		ReagentShop *s = &shops[i];
		screenTextColor(FG_WHITE);
		screenTextAt(2, line, "%s", s->name);
		for (int j = 0; j < 6; j++) {
			screenTextColor(colors[j]);
			screenShowChar('0' + s->price[j], 10 + j, line);
		}
	}

	for (int i = 0; i < 8; i++) {
		screenTextColor(colors[i]);
		screenShowChar('A' + i, 10 + i, 6);
	}

	bool done = false;
	while (!done) {
		showMixturesSuper(page);
		screenMessage("For Spell: ");

		int spell = ReadChoiceController::get("abcdefghijklmnopqrstuvwxyz \033\n\r");
		if (spell < 'a' || spell > 'z') {
			screenMessage("\nDone.\n");
			done = true;
		} else {
			spell -= 'a';
			const Spell *s = getSpell(spell);
			screenMessage("%s\n", s->_name);
			page = (spell >= 13);
			showMixturesSuper(page);

			// how many can we mix?
			int mixQty = 99 - g_context->_saveGame->_mixtures[spell];
			int ingQty = 99;
			int comp = s->_components;
			for (int i = 0; i < 8; i++) {
				if (comp & 1 << i) {
					int reagentQty = g_context->_saveGame->_reagents[i];
					if (reagentQty < ingQty)
						ingQty = reagentQty;
				}
			}
			screenMessage("You can make %d.\n", (mixQty > ingQty) ? ingQty : mixQty);
			screenMessage("How many? ");

			int howmany = ReadIntController::get(2, TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);

			if (howmany == 0) {
				screenMessage("\nNone mixed!\n");
			} else if (howmany > mixQty) {
				screenMessage("\n%cYou cannot mix that much more of that spell!%c\n", FG_GREY, FG_WHITE);
			} else if (howmany > ingQty) {
				screenMessage("\n%cYou don't have enough reagents to mix %d spells!%c\n", FG_GREY, howmany, FG_WHITE);
			} else {
				g_context->_saveGame->_mixtures[spell] += howmany;
				for (int i = 0; i < 8; i++) {
					if (comp & 1 << i) {
						g_context->_saveGame->_reagents[i] -= howmany;
					}
				}
				screenMessage("\nSuccess!\n\n");
			}
		}
		g_context->_stats->setView(StatsView(STATS_REAGENTS));
	}

	g_context->_location->_viewMode = oldlocation;
	return;
}

} // End of namespace Ultima4
} // End of namespace Ultima
