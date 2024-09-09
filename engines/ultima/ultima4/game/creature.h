/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA4_GAME_CREATURE_H
#define ULTIMA4_GAME_CREATURE_H

#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

class CombatController;
class ConfigElement;
class Tile;

typedef unsigned short CreatureId;
typedef Common::HashMap<CreatureId, class Creature *> CreatureMap;
typedef Std::vector<class Creature *> CreatureVector;

#define MAX_CREATURES 128

/* Creatures on world map */

#define MAX_CREATURES_ON_MAP 4
#define MAX_CREATURE_DISTANCE 16

/* Creature ids */

typedef enum {
	HORSE1_ID       = 0,
	HORSE2_ID       = 1,

	MAGE_ID         = 2,
	BARD_ID         = 3,
	FIGHTER_ID      = 4,
	DRUID_ID        = 5,
	TINKER_ID       = 6,
	PALADIN_ID      = 7,
	RANGER_ID       = 8,
	SHEPHERD_ID     = 9,

	GUARD_ID        = 10,
	VILLAGER_ID     = 11,
	SINGINGBARD_ID  = 12,
	JESTER_ID       = 13,
	BEGGAR_ID       = 14,
	CHILD_ID        = 15,
	BULL_ID         = 16,
	LORDBRITISH_ID  = 17,

	PIRATE_ID       = 18,
	NIXIE_ID        = 19,
	GIANT_SQUID_ID  = 20,
	SEA_SERPENT_ID  = 21,
	SEAHORSE_ID     = 22,
	WHIRLPOOL_ID    = 23,
	STORM_ID        = 24,
	RAT_ID          = 25,
	BAT_ID          = 26,
	GIANT_SPIDER_ID = 27,
	GHOST_ID        = 28,
	SLIME_ID        = 29,
	TROLL_ID        = 30,
	GREMLIN_ID      = 31,
	MIMIC_ID        = 32,
	REAPER_ID       = 33,
	INSECT_SWARM_ID = 34,
	GAZER_ID        = 35,
	PHANTOM_ID      = 36,
	ORC_ID          = 37,
	SKELETON_ID     = 38,
	ROGUE_ID        = 39,
	PYTHON_ID       = 40,
	ETTIN_ID        = 41,
	HEADLESS_ID     = 42,
	CYCLOPS_ID      = 43,
	WISP_ID         = 44,
	EVILMAGE_ID     = 45,
	LICH_ID         = 46,
	LAVA_LIZARD_ID  = 47,
	ZORN_ID         = 48,
	DAEMON_ID       = 49,
	HYDRA_ID        = 50,
	DRAGON_ID       = 51,
	BALRON_ID       = 52
} CreatureType;

typedef enum {
	MATTR_STEALFOOD     = 0x1,
	MATTR_STEALGOLD     = 0x2,
	MATTR_CASTS_SLEEP   = 0x4,
	MATTR_UNDEAD        = 0x8,
	MATTR_GOOD          = 0x10,
	MATTR_WATER         = 0x20,
	MATTR_NONATTACKABLE = 0x40,
	MATTR_NEGATE        = 0x80,
	MATTR_CAMOUFLAGE    = 0x100,
	MATTR_NOATTACK      = 0x200,
	MATTR_AMBUSHES      = 0x400,
	MATTR_RANDOMRANGED  = 0x800,
	MATTR_INCORPOREAL   = 0x1000,
	MATTR_NOCHEST       = 0x2000,
	MATTR_DIVIDES       = 0x4000,
	MATTR_SPAWNSONDEATH = 0x8000,
	MATTR_FORCE_OF_NATURE = 0x10000
} CreatureAttrib;

typedef enum {
	MATTR_STATIONARY        = 0x1,
	MATTR_WANDERS           = 0x2,
	MATTR_SWIMS             = 0x4,
	MATTR_SAILS             = 0x8,
	MATTR_FLIES             = 0x10,
	MATTR_TELEPORT          = 0x20,
	MATTR_CANMOVECREATURES  = 0x40,
	MATTR_CANMOVEAVATAR     = 0x80
} CreatureMovementAttrib;

typedef enum {
	MSTAT_DEAD,
	MSTAT_FLEEING,
	MSTAT_CRITICAL,
	MSTAT_HEAVILYWOUNDED,
	MSTAT_LIGHTLYWOUNDED,
	MSTAT_BARELYWOUNDED
} CreatureStatus;

/**
 * Creature Class Definition
 * @todo
 * <ul>
 *      <li>split into a CreatureType (all the settings for a
 *      particular creature e.g. orc) and Creature (a specific
 *      creature instance)</li>
 *      <li>creatures can be looked up by name, ids can probably go away</li>
 * </ul>
 */
class Creature : public Object {
	typedef Common::List<StatusType> StatusList;

public:
	/**
	 * Creature class implementation
	 */
	Creature(MapTile tile = MapTile(0));

	void load(const ConfigElement &conf);

	// Accessor methods
	virtual Common::String getName() const {
		return _name;
	}
	virtual const Common::String &getHitTile() const {
		return _rangedHitTile;
	}
	virtual const Common::String &getMissTile() const {
		return _rangedMissTile;
	}
	CreatureId getId() const {
		return _id;
	}
	CreatureId getLeader() const {
		return _leader;
	}
	virtual int getHp() const {
		return _hp;
	}
	virtual int getXp() const {
		return _xp;
	}
	virtual const Common::String &getWorldrangedtile() const {
		return _worldRangedTile;
	}
	SlowedType getSlowedType() const {
		return _slowedType;
	}
	int getEncounterSize() const {
		return _encounterSize;
	}
	byte getResists() const {
		return _resists;
	}

	// Setters
	void setName(Common::String s) {
		_name = s;
	}
	void setHitTile(const Common::String &t) {
		_rangedHitTile = t;
	}
	void setMissTile(const Common::String &t) {
		_rangedMissTile = t;
	}
	virtual void setHp(int points) {
		_hp = points;
	}

	// Query methods
	bool isGood() const {
		return _mAttr & MATTR_GOOD;
	}
	bool isEvil() const {
		return !isGood();
	}
	bool isUndead() const {
		return _mAttr & MATTR_UNDEAD;
	}
	bool leavesChest() const {
		return !isAquatic() && !(_mAttr & MATTR_NOCHEST);
	}
	bool isAquatic() const {
		return _mAttr & MATTR_WATER;
	}
	bool wanders() const {
		return _movementAttr & MATTR_WANDERS;
	}
	bool isStationary() const {
		return _movementAttr & MATTR_STATIONARY;
	}
	bool flies() const {
		return _movementAttr & MATTR_FLIES;
	}
	bool teleports() const {
		return _movementAttr & MATTR_TELEPORT;
	}
	bool swims() const {
		return _movementAttr & MATTR_SWIMS;
	}
	bool sails() const {
		return _movementAttr & MATTR_SAILS;
	}
	bool walks() const {
		return !(flies() || swims() || sails());
	}
	bool divides() const {
		return _mAttr & MATTR_DIVIDES;
	}
	bool spawnsOnDeath() const {
		return _mAttr & MATTR_SPAWNSONDEATH;
	}
	bool canMoveOntoCreatures() const {
		return _movementAttr & MATTR_CANMOVECREATURES;
	}
	bool canMoveOntoPlayer() const {
		return _movementAttr & MATTR_CANMOVEAVATAR;
	}
	bool isAttackable() const;
	bool willAttack() const {
		return !(_mAttr & MATTR_NOATTACK);
	}
	bool stealsGold() const {
		return _mAttr & MATTR_STEALGOLD;
	}
	bool stealsFood() const {
		return _mAttr & MATTR_STEALFOOD;
	}
	bool negates() const {
		return _mAttr & MATTR_NEGATE;
	}
	bool camouflages() const {
		return _mAttr & MATTR_CAMOUFLAGE;
	}
	bool ambushes() const {
		return _mAttr & MATTR_AMBUSHES;
	}
	bool isIncorporeal() const {
		return _mAttr & MATTR_INCORPOREAL;
	}
	bool hasRandomRanged() const {
		return _mAttr & MATTR_RANDOMRANGED;
	}
	bool leavesTile() const {
		return _leavesTile;
	}
	bool castsSleep() const {
		return _mAttr & MATTR_CASTS_SLEEP;
	}
	bool isForceOfNature() const {
		return _mAttr & MATTR_FORCE_OF_NATURE;
	}
	int getDamage() const;
	const Common::String &getCamouflageTile() const {
		return _camouflageTile;
	}
	void setRandomRanged();
	int setInitialHp(int hp = -1);

	/**
	 * Performs a special action for the creature
	 * Returns true if the action takes up the creatures
	 * whole turn (i.e. it can't move afterwards)
	 */
	bool specialAction();

	/**
	 * Performs a special effect for the creature
	 * Returns true if something special happened,
	 * or false if nothing happened
	 */
	bool specialEffect();

	/* combat methods */
	void act(CombatController *controller);

	/**
	 * Add status effects to the creature, in order of importance
	 */
	virtual void addStatus(StatusType status);
	void applyTileEffect(TileEffect effect);
	virtual int getAttackBonus() const;
	virtual int getDefense() const;
	bool divide();
	bool spawnOnDeath();
	virtual CreatureStatus getState() const;
	StatusType getStatus() const;
	bool isAsleep() const;

	/**
	 * Hides or shows a camouflaged creature, depending on its distance from
	 * the nearest opponent
	 */
	bool hideOrShow();

	Creature *nearestOpponent(int *dist, bool ranged);
	virtual void putToSleep();
	virtual void removeStatus(StatusType status);
	virtual void setStatus(StatusType status);
	virtual void wakeUp();

	/**
	 * Applies damage to the creature.
	 * Returns true if the creature still exists after the damage has been applied
	 * or false, if the creature was destroyed
	 *
	 * If byplayer is false (when a monster is killed by walking through
	 * fire or poison, or as a result of jinx) we don't report experience
	 * on death
	 */
	virtual bool applyDamage(int damage, bool byplayer = true);
	virtual bool dealDamage(Creature *m, int damage);

	// Properties
protected:
	Common::String _name;
	Common::String _rangedHitTile;
	Common::String _rangedMissTile;
	CreatureId     _id;
	Common::String _camouflageTile;
	CreatureId     _leader;
	int            _baseHp;
	int            _hp;
	StatusList     _status;
	int            _xp;
	byte  _ranged;
	Common::String _worldRangedTile;
	bool           _leavesTile;
	CreatureAttrib _mAttr;
	CreatureMovementAttrib _movementAttr;
	SlowedType     _slowedType;
	int            _encounterSize;
	byte  _resists;
	CreatureId     _spawn;
};

/**
 * CreatureMgr Class Definition
 */
class CreatureMgr {
public:
	static CreatureMgr *getInstance();

	void loadAll();

	/**
	 * Returns a creature using a tile to find which one to create
	 * or nullptr if a creature with that tile cannot be found
	 */
	Creature *getByTile(MapTile tile);

	/**
	 * Returns the creature that has the corresponding id
	 * or returns nullptr if no creature with that id could
	 * be found.
	 */
	Creature *getById(CreatureId id);

	/**
	 * Returns the creature that has the corresponding name
	 * or returns nullptr if no creature can be found with
	 * that name (case insensitive)
	 */
	Creature *getByName(Common::String name);

	/**
	 * Creates a random creature based on the tile given
	 */
	Creature *randomForTile(const Tile *tile);

	/**
	 * Creates a random creature based on the dungeon level given
	 */
	Creature *randomForDungeon(int dnglevel);

	/**
	 * Creates a random ambushing creature
	 */
	Creature *randomAmbushing();

private:
	CreatureMgr() {}

	// disallow assignments, copy construction
	CreatureMgr(const CreatureMgr &);
	const CreatureMgr &operator=(const CreatureMgr &);

	static CreatureMgr *_instance;

	CreatureMap _creatures;
};

bool isCreature(Object *punknown);

#define creatureMgr (CreatureMgr::getInstance())

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
