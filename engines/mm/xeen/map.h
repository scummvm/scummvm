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

#ifndef XEEN_MAP_H
#define XEEN_MAP_H

#include "common/stream.h"
#include "common/array.h"
#include "common/rect.h"
#include "mm/xeen/combat.h"
#include "mm/xeen/files.h"
#include "mm/xeen/party.h"
#include "mm/xeen/scripts.h"
#include "mm/xeen/sprites.h"

namespace MM {
namespace Xeen {

#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define TOTAL_SURFACES 16
#define INVALID_CELL 0x8888

class XeenEngine;

enum MonsterType {
	MONSTER_MONSTERS = 0, MONSTER_ANIMAL = 1, MONSTER_INSECT = 2,
	MONSTER_HUMANOID = 3, MONSTER_UNDEAD = 4, MONSTER_GOLEM = 5,
	MONSTER_DRAGON = 6
};

enum MapId {
	XEEN_CASTLE1 = 75, XEEN_CASTLE4 = 78
};

class MonsterStruct {
public:
	Common::String _name;
	int _experience;
	int _hp;
	int _armorClass;
	int _speed;
	int _numberOfAttacks;
	int _hatesClass;
	int _strikes;
	int _dmgPerStrike;
	DamageType _attackType;
	SpecialAttack _specialAttack;
	int _hitChance;
	int _rangeAttack;
	MonsterType _monsterType;
	int _fireResistance;
	int _electricityResistance;
	int _coldResistance;
	int _poisonResistance;
	int _energyResistance;
	int _magicResistance;
	int _phsyicalResistance;
	int _field29;
	int _gold;
	int _gems;
	int _itemDrop;
	bool _flying;
	int _imageNumber;
	int _loopAnimation;
	int _animationEffect;
	int _fx;
	Common::String _attackVoc;
public:
	MonsterStruct();
	MonsterStruct(Common::String name, int experience, int hp, int accuracy,
		int speed, int numberOfAttacks, CharacterClass hatesClass, int strikes,
		int dmgPerStrike, DamageType attackType, SpecialAttack specialAttack,
		int hitChance, int rangeAttack, MonsterType monsterType,
		int fireResistance, int electricityResistance, int coldResistance,
		int poisonResistance, int energyResistance, int magicResistance,
		int phsyicalResistance, int field29, int gold, int gems, int itemDrop,
		bool flying, int imageNumber, int loopAnimation, int animationEffect,
		int field32, Common::String attackVoc);

	void synchronize(Common::SeekableReadStream &s);
};

class MonsterData : public Common::Array<MonsterStruct> {
private:
	void synchronize(Common::SeekableReadStream &s);
public:
	MonsterData();

	void load(const Common::Path &name);
};

class SurroundingMazes {
public:
	int _north;
	int _east;
	int _south;
	int _west;
public:
	SurroundingMazes();

	void clear();

	void synchronize(XeenSerializer &s);

	int &operator[](int idx);
};

class MazeDifficulties {
public:
	int _wallNoPass;
	int _surfaceNoPass;
	int _unlockDoor;
	int _unlockBox;
	int _bashDoor;
	int _bashGrate;
	int _bashWall;
	int _chance2Run;
public:
	MazeDifficulties();

	/**
	 * Synchronizes data for the item
	 */
	void synchronize(XeenSerializer &s);
};

enum MazeFlags {
	OUTFLAG_GRATE = 0x80, OUTFLAG_DRAIN = 0x20, OUTFLAG_OBJECT_EXISTS = 0x08,
	INFLAG_INSIDE = 0x08, FLAG_AUTOEXECUTE_EVENT = 0x10,
	RESTRICTION_ETHERIALIZE = 0x40, RESTRICTION_80 = 0x80,
	RESTRICTION_TOWN_PORTAL = 0x100, RESTRICTION_SUPER_SHELTER = 0x200,
	RESTRICTION_TIME_DISTORTION = 0x400, RESTRICTION_LLOYDS_BEACON = 0x800,
	RESTRICTION_TELPORT = 0x1000, RESTRICTION_2000 = 0x2000,
	RESTRICTION_REST = 0x4000, RESTRICTION_SAVE = 0x8000,

	FLAG_GROUND_BITS = 7
};

enum MazeFlags2 {
	FLAG_IS_OUTDOORS = 0x8000, FLAG_IS_DARK = 0x4000
};

enum SurfaceType {
	SURFTYPE_DEFAULT = 0,
	SURFTYPE_WATER = 0, SURFTYPE_DIRT = 1, SURFTYPE_GRASS = 2,
	SURFTYPE_SNOW = 3, SURFTYPE_SWAMP = 4, SURFTYPE_LAVA = 5,
	SURFTYPE_DESERT = 6, SURFTYPE_ROAD = 7, SURFTYPE_DWATER = 8,
	SURFTYPE_TFLR = 9, SURFTYPE_SKY = 10, SURFTYPE_CROAD = 11,
	SURFTYPE_SEWER = 12, SURFTYPE_CLOUD = 13, SURFTYPE_SCORCH = 14,
	SURFTYPE_SPACE = 15
};

union MazeWallLayers {
	struct MazeWallIndoors {
		uint _wallNorth : 4;
		uint _wallEast : 4;
		uint _wallSouth : 4;
		uint _wallWest : 4;
	} _indoors;
	struct MazeWallOutdoors {
		uint _surfaceId : 4;		// SurfaceType, but needs to be unsigned
		uint _iMiddle : 4;
		uint _iTop : 4;
		uint _iOverlay : 4;
	} _outdoors;
	uint16 _data;
};

struct MazeCell {
	int _flags;
	int _surfaceId;
	MazeCell() : _flags(0), _surfaceId(0) {
	}
};

class MazeData {
public:
	// Resource fields
	MazeWallLayers _wallData[MAP_HEIGHT][MAP_WIDTH];
	MazeCell _cells[MAP_HEIGHT][MAP_WIDTH];
	int _mazeNumber;
	SurroundingMazes _surroundingMazes;
	int _mazeFlags;
	int _mazeFlags2;
	int _wallTypes[16];
	int _surfaceTypes[16];
	int _floorType;
	Common::Point _runPosition;
	MazeDifficulties _difficulties;
	int _trapDamage;
	int _wallKind;
	int _tavernTips;
	bool _seenTiles[MAP_HEIGHT][MAP_WIDTH];
	bool _steppedOnTiles[MAP_HEIGHT][MAP_WIDTH];
	Common::Array<bool> _objectsPresent;

	// Misc fields
	int _mazeId;
public:
	MazeData();

	void clear();

	/**
	 * Synchronize data for the maze data
	 */
	void synchronize(XeenSerializer &s);

	/**
	 * Flags all tiles for the map as having been stepped on
	 */
	void setAllTilesStepped();

	void clearCellSurfaces();
};

class MobStruct {
public:
	Common::Point _pos;
	int _id;
	Direction _direction;
public:
	MobStruct();

	/**
	 * Synchronizes the data for the item
	 */
	bool synchronize(XeenSerializer &s);

	/**
	 * Sets up the entry as an end of list marker
	 */
	void endOfList();
};

struct MazeObject {
public:
	Common::Point _position;
	int _id;
	int _frame;
	int _spriteId;
	Direction _direction;
	bool _flipped;
	SpriteResource *_sprites;

	MazeObject();
};

struct MazeMonster {
	Common::Point _position;
	int _frame;
	int _id;
	int _spriteId;
	bool _isAttacking;
	DamageType _damageType;
	int _field9;
	int _postAttackDelay;
	int _hp;
	int _effect1, _effect2;
	int _effect3;
	SpriteResource *_sprites;
	SpriteResource *_attackSprites;
	MonsterStruct *_monsterData;

	MazeMonster();

	/**
	 * Return the text color to use when displaying the monster's name in combat
	 * to indicate how damaged they are
	 */
	int getTextColor() const;
};

class MazeWallItem {
public:
	Common::Point _position;
	int _id;
	int _frame;
	int _spriteId;
	Direction _direction;
	SpriteResource *_sprites;
public:
	MazeWallItem();
};

struct WallSprites {
	SpriteResource _surfaces[TOTAL_SURFACES];
	SpriteResource _fwl1;
	SpriteResource _fwl2;
	SpriteResource _fwl3;
	SpriteResource _fwl4;
	SpriteResource _swl;
};

class Map;

class MonsterObjectData {
	friend class Map;
public:
	struct SpriteResourceEntry {
		int _spriteId;
		SpriteResource _sprites;
		SpriteResource _attackSprites;

		SpriteResourceEntry() {
			_spriteId = -1;
		}
		SpriteResourceEntry(int spriteId) : _spriteId(spriteId) {
		}

		/**
		 * Returns true if no sprite id is specified
		 */
		bool isEmpty() const {
			return _spriteId == -1;
		}
	};
private:
	XeenEngine *_vm;
	Common::Array<SpriteResourceEntry> _monsterSprites;
	Common::Array<SpriteResourceEntry> _monsterAttackSprites;
	Common::Array<SpriteResourceEntry> _wallItemSprites;
public:
	Common::Array<MazeObject> _objects;
	Common::Array<MazeMonster> _monsters;
	Common::Array<MazeWallItem> _wallItems;
	Common::Array<SpriteResourceEntry> _objectSprites;
public:
	MonsterObjectData(XeenEngine *vm);

	/**
	 * Synchronizes the data
	 */
	void synchronize(XeenSerializer &s, MonsterData &monsterData);

	/**
	 * Clears the current list of monster sprites
	 */
	void clearMonsterSprites();

	/**
	 * Load the normal and attack sprites for a given monster
	 */
	void addMonsterSprites(MazeMonster &monster);
};

class HeadData {
public:
	struct HeadEntry {
		int _left;
		int _right;
	};
	HeadEntry _data[MAP_HEIGHT][MAP_WIDTH];
public:
	HeadData();

	void synchronize(Common::SeekableReadStream &s);

	HeadEntry *operator[](int y) {
		return &_data[y][0];
	}
};

struct AnimationFrame {
	int _front, _left, _back, _right;
};
struct AnimationFlipped {
	bool _front, _left, _back, _right;
};
struct AnimationEntry {
	union {
		AnimationFrame _positions;
		int _frames[4];
	} _frame1;
	union {
		AnimationFlipped _positions;
		bool _flags[4];
	} _flipped;
	union {
		AnimationFrame _positions;
		int _frames[4];
	} _frame2;

	/**
	 * Synchronize data for an animation entry
	 */
	void synchronize(Common::SeekableReadStream &s);
};

class AnimationInfo : public Common::Array<AnimationEntry> {
public:
	/**
	 * Synchronize data for object animations within the game
	 */
	void synchronize(Common::SeekableReadStream &s);

	/**
	 * Load the animation info objects in the game
	 */
	void load(const Common::Path &name);
};

class Map {
private:
	XeenEngine *_vm;
	MazeData _mazeData[9];
	SpriteResource _wallPicSprites;
	int _sidePictures;
	int _sideObjects;
	int _sideMonsters;
	int _mazeDataIndex;

	/**
	 * Load the events for a new map
	 */
	void loadEvents(int mapId, int ccNum);

	/**
	 * Save the events for a map
	 */
	void saveEvents();

	/**
	 * Save the monster data for a map
	 */
	void saveMonsters();

	/**
	 * Save the map data
	 */
	void saveMap();

	/**
	 * Finds a map in the array that contains the currently active and the surrounding
	 * maps in the eight cardinal directions
	 */
	void findMap(int mapId = -1);
public:
	Common::String _mazeName;
	bool _isOutdoors;
	MonsterObjectData _mobData;
	MonsterData _monsterData;
	MazeEvents _events;
	HeadData _headData;
	AnimationInfo _animationInfo;
	SpriteResource _skySprites[2];
	SpriteResource _groundSprites;
	SpriteResource _tileSprites;
	SpriteResource _surfaceSprites[TOTAL_SURFACES];
	WallSprites _wallSprites;
	bool _currentGrateUnlocked;
	bool _currentCantRest;
	bool _currentIsDrain;
	bool _currentIsEvent;
	int _currentSky;
	int _currentMonsterFlags;
	int _currentWall;
	int _currentTile;
	int _currentSurfaceId;
	bool _currentSteppedOn;
	int _loadCcNum;
	int _sideTownPortal;
public:
	Map(XeenEngine *vm);

	/**
	 * Loads a specified map
	 */
	void load(int mapId);

	int mazeLookup(const Common::Point &pt, int layerShift, int wallMask = 0xf);

	void cellFlagLookup(const Common::Point &pt);

	/**
	 * Sets the surface flags for a given position
	 */
	void setCellSurfaceFlags(const Common::Point &pt, int bits);

	/**
	 * Sets the value for the wall in a given direction from a given point
	 */
	void setWall(const Common::Point &pt, Direction dir, int v);

	/**
	 * Saves all changeable maze data to the in-memory save state
	 */
	void saveMaze();

	/**
	 * Clears the current maze. This is used during savegame loads so that
	 * the previous games maze data isn't saved as the new map is loaded
	 */
	void clearMaze();

	/**
	 * Gets the data for a map position at one of the relative indexes
	 * surrounding the current position
	 */
	int getCell(int idx);

	/**
	 * Returns the data for the primary active map
	 */
	MazeData &mazeData() {
		return _mazeData[0];
	}

	/**
	 * Returns the data for the primary active map
	 */
	const MazeData *mazeDataSurrounding() {
		return _mazeData;
	}

	/**
	 * Returns the data for the currently indexed map
	 */
	MazeData &mazeDataCurrent() {
		return _mazeData[_mazeDataIndex];
	}

	/**
	 * Loads the sprites needed for rendering the skyline
	 */
	void loadSky();

	/**
	 * Tests the current position, and if it's moved beyond the valid (0,0) to (15,15)
	 * range for a map, loads in the correct surrounding map, and adjusts the
	 * position to the relative position on the new map
	 */
	void getNewMaze();

	/**
	 * Return the name of a specified maze
	 * @param mapId		Map Id
	 * @param ccNum		Cc file number. If -1, uses the current C
	 */
	static Common::String getMazeName(int mapId, int ccNum = -1);
};

} // End of namespace Xeen
} // End of namespace MM

#endif
