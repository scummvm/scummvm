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

#ifndef ULTIMA4_GAME_H
#define ULTIMA4_GAME_H

#include "ultima/ultima4/events/controller.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/observer.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/graphics/tileview.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

class Map;
struct Portal;
class Creature;
class Location;
class MoveEvent;
class Party;
class PartyEvent;
class PartyMember;

typedef enum {
	VIEW_NORMAL,
	VIEW_GEM,
	VIEW_RUNE,
	VIEW_DUNGEON,
	VIEW_DEAD,
	VIEW_CODEX,
	VIEW_MIXTURES
} ViewMode;

/**
 * A controller to read a player number.
 */
class ReadPlayerController : public ReadChoiceController {
public:
	ReadPlayerController();
	~ReadPlayerController();
	virtual bool keyPressed(int key);

	int getPlayer();
	int waitFor();
};

/**
 * A controller to handle input for commands requiring a letter
 * argument in the range 'a' - lastValidLetter.
 */
class AlphaActionController : public WaitableController<int> {
public:
	AlphaActionController(char letter, const Common::String &p) : _lastValidLetter(letter), _prompt(p) {}
	bool keyPressed(int key);

	static int get(char lastValidLetter, const Common::String &prompt, EventHandler *eh = NULL);

private:
	char _lastValidLetter;
	Common::String _prompt;
};

/**
 * Controls interaction while Ztats are being displayed.
 */
class ZtatsController : public WaitableController<void *> {
public:
	bool keyPressed(int key);
};

class TurnCompleter {
public:
	virtual ~TurnCompleter() {}
	virtual void finishTurn() = 0;
};

/**
 * The main game controller that handles basic game flow and keypresses.
 *
 * @todo
 *  <ul>
 *      <li>separate the dungeon specific stuff into another class (subclass?)</li>
 *  </ul>
 */
class GameController : public Controller, public Observer<Party *, PartyEvent &>, public Observer<Location *, MoveEvent &>,
	public TurnCompleter {
public:
	GameController();

	/* controller functions */
	virtual bool keyPressed(int key);
	virtual void timerFired();

	/* main game functions */
	void init();
	void initScreen();
	void initScreenWithoutReloadingState();
	void setMap(Map *map, bool saveLocation, const Portal *portal, TurnCompleter *turnCompleter = NULL);
	int exitToParentMap();
	virtual void finishTurn();

	virtual void update(Party *party, PartyEvent &event);
	virtual void update(Location *location, MoveEvent &event);

	void initMoons();
	void updateMoons(bool showmoongates);

	static void flashTile(const Coords &coords, MapTile tile, int timeFactor);
	static void flashTile(const Coords &coords, const Common::String &tilename, int timeFactor);
	static void doScreenAnimationsWhilePausing(int timeFactor);

	TileView _mapArea;
	bool _paused;
	int _pausedTimer;

private:
	void avatarMoved(MoveEvent &event);
	void avatarMovedInDungeon(MoveEvent &event);

	void creatureCleanup();
	void checkBridgeTrolls();
	void checkRandomCreatures();
	void checkSpecialCreatures(Direction dir);
	bool checkMoongates();

	bool createBalloon(Map *map);
};

extern GameController *g_game;

/* map and screen functions */
void gameSetViewMode(ViewMode newMode);
void gameUpdateScreen(void);

/* spell functions */
void castSpell(int player = -1);
void gameSpellEffect(int spell, int player, Sound sound);

/* action functions */
void destroy();
void attack();
void board();
void fire();
void getChest(int player = -1);
void holeUp();
void jimmy();
void opendoor();
bool gamePeerCity(int city, void *data);
void peer(bool useGem = true);
void talk();
bool fireAt(const Coords &coords, bool originAvatar);
Direction gameGetDirection();
void readyWeapon(int player = -1);

/* checking functions */
void gameCheckHullIntegrity(void);

/* creature functions */
bool creatureRangeAttack(const Coords &coords, Creature *m);
void gameCreatureCleanup(void);
bool gameSpawnCreature(const class Creature *m);

/* etc */
Common::String gameGetInput(int maxlen = 32);
int gameGetPlayer(bool canBeDisabled, bool canBeActivePlayer);
void gameGetPlayerForCommand(bool (*commandFn)(int player), bool canBeDisabled, bool canBeActivePlayer);
void gameDamageParty(int minDamage, int maxDamage);
void gameDamageShip(int minDamage, int maxDamage);
void gameSetActivePlayer(int player);
Std::vector<Coords> gameGetDirectionalActionPath(int dirmask, int validDirections, const Coords &origin, int minDistance, int maxDistance, bool (*blockedPredicate)(const Tile *tile), bool includeBlocked);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
