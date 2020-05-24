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

#ifndef ULTIMA4_CONTROLLERS_COMBAT_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_COMBAT_CONTROLLER_H

#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"

namespace Ultima {
namespace Ultima4 {

#define AREA_CREATURES   16
#define AREA_PLAYERS    8

class CombatMap;
class Creature;
class MoveEvent;
class Weapon;

typedef enum {
	CA_ATTACK,
	CA_CAST_SLEEP,
	CA_ADVANCE,
	CA_RANGED,
	CA_FLEE,
	CA_TELEPORT
} CombatAction;

/**
 * CombatController class
 */
class CombatController : public Controller, public Observer<Party *, PartyEvent &>, public TurnCompleter {
protected:
	CombatController();
public:
	CombatController(CombatMap *m);
	CombatController(MapId id);
	virtual ~CombatController();

	// Accessor Methods
	bool isCombatController() const override {
		return true;
	}

	/**
	 * Called when a controller is made active
	 */
	void setActive() override;

	bool isCamping() const;
	bool isWinOrLose() const;
	Direction getExitDir() const;
	byte getFocus() const;
	CombatMap *getMap() const;
	Creature *getCreature() const;
	PartyMemberVector *getParty();
	PartyMember *getCurrentPlayer();

	void setExitDir(Direction d);
	void setCreature(Creature *);
	void setWinOrLose(bool worl = true);
	void showCombatMessage(bool show = true);

	// Methods
	/**
	 * Initializes the combat controller with combat information
	 */
	virtual void init(Creature *m);

	/**
	 * Initializes dungeon room combat
	 */
	void initDungeonRoom(int room, Direction from);

	/**
	 * Apply tile effects to all creatures depending on what they're standing on
	 */
	void applyCreatureTileEffects();

	/**
	 * Begin combat
	 */
	virtual void begin();
	virtual void end(bool adjustKarma);

	/**
	 * Fills the combat creature table with the creatures that the party will be facing.
	 * The creature table only contains *which* creatures will be encountered and
	 * *where* they are placed (by position in the table).  Information like
	 * hit points and creature status will be created when the creature is actually placed
	 */
	void fillCreatureTable(const Creature *creature);

	/**
	 * Generate the number of creatures in a group.
	 */
	int  initialNumberOfCreatures(const Creature *creature) const;

	/**
	 * Returns true if the player has won.
	 */
	bool isWon() const;

	/**
	 * Returns true if the player has lost.
	 */
	bool isLost() const;

	/**
	 * Performs all of the creature's actions
	 */
	void moveCreatures();

	/**
	 * Places creatures on the map from the creature table and from the creature_start coords
	 */
	void placeCreatures();

	/**
	 * Places the party members on the map
	 */
	void placePartyMembers();

	/**
	 * Sets the active player for combat, showing which weapon they're weilding, etc.
	 */
	bool setActivePlayer(int player);
	bool attackHit(Creature *attacker, Creature *defender);
	virtual void awardLoot();

	// attack functions
	void attack(Direction dir = DIR_NONE, int distance = 0);
	bool attackAt(const Coords &coords, PartyMember *attacker, int dir, int range, int distance);
	bool rangedAttack(const Coords &coords, Creature *attacker);
	void rangedMiss(const Coords &coords, Creature *attacker);
	bool returnWeaponToOwner(const Coords &coords, int distance, int dir, const Weapon *weapon);

	/**
	 * Static member functions
	 */
	static void attackFlash(const Coords &coords, MapTile tile, int timeFactor);
	static void attackFlash(const Coords &coords, const Common::String &tilename, int timeFactor);
	static void doScreenAnimationsWhilePausing(int timeFactor);

	void keybinder(KeybindingAction action) override;

	void finishTurn() override;

	/**
	 * Move a party member during combat and display the appropriate messages
	 */
	void movePartyMember(MoveEvent &event);
	void update(Party *party, PartyEvent &event) override;

	// Properties
protected:
	CombatMap *_map;

	PartyMemberVector _party;
	byte _focus;

	const Creature *_creatureTable[AREA_CREATURES];
	Creature *_creature;

	bool _camping;
	bool _forceStandardEncounterSize;
	bool _placePartyOnMap;
	bool _placeCreaturesOnMap;
	bool _winOrLose;
	bool _showMessage;
	Direction _exitDir;

private:
	CombatController(const CombatController &);
	const CombatController &operator=(const CombatController &);

	void init();
};

extern CombatController *g_combat;

/**
 * CombatMap class
 */
class CombatMap : public Map {
public:
	CombatMap();
	~CombatMap() override {}

	/**
	 * Returns a vector containing all of the creatures on the map
	 */
	CreatureVector getCreatures();

	/**
	 * Returns a vector containing all of the party members on the map
	 */
	PartyMemberVector getPartyMembers();

	/**
	 * Returns the party member at the given coords, if there is one,
	 * nullptr if otherwise.
	 */
	PartyMember *partyMemberAt(Coords coords);

	/**
	 * Returns the creature at the given coords, if there is one,
	 * nullptr if otherwise.
	 */
	Creature *creatureAt(Coords coords);

	/**
	 * Returns a valid combat map given the provided information
	 */
	static MapId mapForTile(const Tile *ground, const Tile *transport, Object *obj);

	// Getters
	bool isDungeonRoom() const {
		return _dungeonRoom;
	}
	bool isAltarRoom() const {
		return _altarRoom != VIRT_NONE;
	}
	bool isContextual() const {
		return _contextual;
	}
	BaseVirtue getAltarRoom() const {
		return _altarRoom;
	}

	// Setters
	void setAltarRoom(BaseVirtue ar) {
		_altarRoom = ar;
	}
	void setDungeonRoom(bool d) {
		_dungeonRoom = d;
	}
	void setContextual(bool c) {
		_contextual = c;
	}

	// Properties
protected:
	bool _dungeonRoom;
	BaseVirtue _altarRoom;
	bool _contextual;

public:
	Coords creature_start[AREA_CREATURES];
	Coords player_start[AREA_PLAYERS];
};

bool isCombatMap(Map *punknown);
CombatMap *getCombatMap(Map *punknown = nullptr);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
