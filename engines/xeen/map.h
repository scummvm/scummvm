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

#ifndef XEEN_MAP_H
#define XEEN_MAP_H

#include "common/stream.h"
#include "common/array.h"
#include "common/rect.h"
#include "xeen/party.h"
#include "xeen/sprites.h"

namespace Xeen {

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

class XeenEngine;

enum DamageType {
	DT_PHYSICAL = 0, DT_1 = 1, DT_FIRE = 2, DT_ELECTRICAL = 3,
	DT_COLD = 4, DT_POISON = 5, DT_ENERGY = 6, DT_SLEEP = 7,
	DT_FINGEROFDEATH = 8, DT_HOLYWORD = 9, DT_MASS_DISTORTION = 10,
	DT_UNDED = 11, DT_BEASTMASTER = 12, DT_DRAGONSLEEP = 13,
	DT_GOLEMSTOPPER = 14, DT_HYPNOTIZE = 15, DT_INSECT_SPRAY = 16,
	DT_POISON_VALLEY = 17, DT_MAGIC_ARROW = 18
};

enum SpecialAttack {
	SA_NONE = 0, SA_MAGIC = 1, SA_FIRE = 2, SA_ELEC = 3, SA_COLD = 4,
	SA_POISON = 5, SA_ENERGY = 6, SA_DISEASE = 7, SA_INSANE = 8,
	SA_SLEEP = 9, SA_CURSEITEM = 10, SA_INLOVE = 11, SA_DRAINSP = 12,
	SA_CURSE = 13, SA_PARALYZE = 14, SA_UNCONSCIOUS = 15,
	SA_CONFUSE = 16, SA_BREAKWEAPON = 17, SA_WEAKEN = 18,
	SA_ERADICATE = 19, SA_AGING = 20, SA_DEATH = 21, SA_STONE = 22
};

enum MonsterType {
	MONSTER_0 = 0, MONSTER_ANIMAL = 1, MONSTER_INSECT = 2,
	MONSTER_HUMANOID = 3, MONSTER_UNDEAD = 4, MONSTER_GOLEM = 5,
	MONSTER_DRAGON = 6
};

class MonsterStruct {
public:
	Common::String _name;
	int _experience;
	int _hp;
	int _accuracy;
	int _speed;
	int _numberOfAttacks;
	CharacterClass _hatesClass;
	int _strikes;
	int _dmgPerStrike;
	DamageType _attackType;
	SpecialAttack _specialAttack;
	int _hitChance;
	int _rangeAttack;
	MonsterType _monsterType;
	int _fireResistence;
	int _electricityResistence;
	int _coldResistence;
	int _poisonResistence;
	int _energyResistence;
	int _magicResistence;
	int _phsyicalResistence;
	int _field29;
	int _gold;
	int _gems;
	int _itemDrop;
	int _flying;
	int _imageNumber;
	int _loopAnimation;
	int _animationEffect;
	int _field32;
	Common::String _attackVoc;
public:
	MonsterStruct();
	MonsterStruct(Common::String name, int experience, int hp, int accuracy,
		int speed, int numberOfAttacks, CharacterClass hatesClass, int strikes,
		int dmgPerStrike, DamageType attackType, SpecialAttack specialAttack,
		int hitChance, int rangeAttack, MonsterType monsterType,
		int fireResistence, int electricityResistence, int coldResistence,
		int poisonResistence, int energyResistence, int magicResistence,
		int phsyicalResistence, int field29, int gold, int gems, int itemDrop,
		int flying, int imageNumber, int loopAnimation, int animationEffect,
		int field32, Common::String attackVoc);

	void synchronize(Common::SeekableReadStream &s);
};

class MonsterData : public Common::Array<MonsterStruct> {
private:
	void synchronize(Common::SeekableReadStream &s);
public:
	MonsterData();

	void load(const Common::String &name);
};

class SurroundingMazes {
public:
	int _north;
	int _east;
	int _south;
	int _west;
public:
	SurroundingMazes();

	void synchronize(Common::SeekableReadStream &s);

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

	void synchronize(Common::SeekableReadStream &s);
};

enum MazeFlags {
	OUTFLAG_GRATE = 0x80, OUTFLAG_DRAIN = 0x20, OUTFLAG_OBJECT_EXISTS = 0x08,
	INFLAG_INSIDE = 0x08,
	FLAG_WATER = 0x40, FLAG_AUTOEXECUTE_EVENT = 0x10,
	FLAG_GROUND_BITS = 7
};

enum MazeFlags2 { FLAG_IS_OUTDOORS = 0x8000, FLAG_IS_DARK = 0x4000 };

enum SurfaceType {
	SURFTYPE_DEFAULT = 0, SURFTYPE_DIRT = 1, SURFTYPE_GRASS = 2,
	SURFTYPE_SNOW = 3, SURFTYPE_SWAMP = 4, SURFTYPE_LAVA = 5,
	SURFTYPE_DESERT = 6, SURFTYPE_ROAD = 7, SURFTYPE_WATER = 8,
	SURFTYPE_TFLR = 9, SURFTYPE_SKY = 10, SURFTYPE_CROAD = 11,
	SURFTYPE_SEWER = 12, SURFTYPE_CLOUD = 13, SURFTYPE_SCORCH = 14,
	SURFTYPE_SPACE = 15
};

union MazeWallLayers {
	struct MazeWallIndoors {
		int _wallNorth : 4;
		int _wallEast : 4;
		int _wallSouth : 4;
		int _wallWest : 4;
	} _indoors;
	struct MazeWallOutdoors {
		SurfaceType _surfaceId : 4;
		int _iMiddle : 4;
		int _iTop : 4;
		int _iOverlay : 4;
	} _outdoors;
	uint16 _data;
};

struct MazeCell {
	int _flags;
	int _surfaceId;
	MazeCell() : _flags(0), _surfaceId(0) {}
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

	// Misc fields
	int _mazeId;
public:
	MazeData();

	void synchronize(Common::SeekableReadStream &s);

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

	bool synchronize(Common::SeekableReadStream &s);
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
	int _hp;
	int _effect1, _effect2;
	int _effect3;
	SpriteResource *_sprites;
	SpriteResource *_attackSprites;

	MazeMonster();
};

class MazeWallItem {
public:
	Common::Point _position;
	int _id;
	int _spriteId;
	Direction _direction;
	SpriteResource *_sprites;
public:
	MazeWallItem();
};

class Map;

class MonsterObjectData {
	friend class Map;
public:
	struct SpriteResourceEntry {
		int _spriteId;
		SpriteResource _sprites;
		SpriteResource _attackSprites;

		SpriteResourceEntry() { _spriteId = -1; }
		SpriteResourceEntry(int spriteId): _spriteId(spriteId) { }
	};
private:
	XeenEngine *_vm;
	Common::Array<SpriteResourceEntry> _objectSprites;
	Common::Array<SpriteResourceEntry> _monsterSprites;
	Common::Array<SpriteResourceEntry> _monsterAttackSprites;
	Common::Array<SpriteResourceEntry> _wallItemSprites;
public:
	Common::Array<MazeObject> _objects;
	Common::Array<MazeMonster> _monsters;
	Common::Array<MazeWallItem> _wallItems;
public:
	MonsterObjectData(XeenEngine *vm);

	void synchronize(Common::SeekableReadStream &s, bool isOutdoors, 
		MonsterData monsterData);
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
};

class Map {
private:
	XeenEngine *_vm;
	MazeData _mazeData[9];
	Common::String _mazeName;
	HeadData _headData;
	SpriteResource _objPicSprites;
	MonsterData _monsterData;
	SpriteResource _wallPicSprites;
	int _townPortalSide;
	int _sideObj;
	int _sideMon;
	bool _stepped;
	int _mazeDataIndex;
	bool _currentSteppedOn;
	int _currentSurfaceId;

	void cellFlagLookup(const Common::Point &pt);
public:
	bool _isOutdoors;
	MonsterObjectData _mobData;
	bool _currentIsGrate;
	bool _currentCantRest;
	bool _currentIsDrain;
	bool _currentIsEvent;
	bool _currentIsObject;
	int _currentMonsterFlags;
public:
	Map(XeenEngine *vm);

	void load(int mapId);

	int mazeLookup(const Common::Point &pt, int directionLayerIndex);

	void setCellSurfaceFlags(const Common::Point &pt, int bits);
};

} // End of namespace Xeen

#endif	/* XEEN_MAP_H */
