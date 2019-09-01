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

#include "common/serializer.h"
#include "xeen/map.h"
#include "xeen/interface.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/screen.h"
#include "xeen/xeen.h"
#include "xeen/dialogs/please_wait.h"

namespace Xeen {

const int MAP_GRID_PRIOR_INDEX[] = { 0, 0, 0, 0, 1, 2, 3, 4, 0 };

const int MAP_GRID_PRIOR_DIRECTION[] = { 0, 1, 2, 3, 1, 2, 3, 0, 0 };

const int MAP_GRID_PRIOR_INDEX2[] = { 0, 0, 0, 0, 2, 3, 4, 1, 0 };

const int MAP_GRID_PRIOR_DIRECTION2[] = { 0, 1, 2, 3, 0, 1, 2, 3, 0 };

MonsterStruct::MonsterStruct() {
	_experience = 0;
	_hp = 0;
	_armorClass = 0;
	_speed = 0;
	_numberOfAttacks = 0;
	_hatesClass = CLASS_KNIGHT;
	_strikes = 0;
	_dmgPerStrike = 0;
	_attackType = DT_PHYSICAL;
	_specialAttack = SA_NONE;
	_hitChance = 0;
	_rangeAttack = 0;
	_monsterType = MONSTER_MONSTERS;
	_fireResistence = 0;
	_electricityResistence = 0;
	_coldResistence = 0;
	_poisonResistence = 0;
	_energyResistence = 0;
	_magicResistence = 0;
	_phsyicalResistence = 0;
	_field29 = 0;
	_gold = 0;
	_gems = 0;
	_itemDrop = 0;
	_flying = 0;
	_imageNumber = 0;
	_loopAnimation = 0;
	_animationEffect = 0;
	_fx = 0;
}

MonsterStruct::MonsterStruct(Common::String name, int experience, int hp, int armorClass,
		int speed, int numberOfAttacks, CharacterClass hatesClass, int strikes,
		int dmgPerStrike, DamageType attackType, SpecialAttack specialAttack,
		int hitChance, int rangeAttack, MonsterType monsterType,
		int fireResistence, int electricityResistence, int coldResistence,
		int poisonResistence, int energyResistence, int magicResistence,
		int phsyicalResistence, int field29, int gold, int gems, int itemDrop,
		bool flying, int imageNumber, int loopAnimation, int animationEffect,
		int fx, Common::String attackVoc):
		_name(name), _experience(experience), _hp(hp), _armorClass(armorClass),
		_speed(speed), _numberOfAttacks(numberOfAttacks), _hatesClass(hatesClass),
		_strikes(strikes), _dmgPerStrike(dmgPerStrike), _attackType(attackType),
		_specialAttack(specialAttack), _hitChance(hitChance), _rangeAttack(rangeAttack),
		_monsterType(monsterType), _fireResistence(fireResistence),
		_electricityResistence(electricityResistence), _coldResistence(coldResistence),
		_poisonResistence(poisonResistence), _energyResistence(energyResistence),
		_magicResistence(magicResistence), _phsyicalResistence(phsyicalResistence),
		_field29(field29), _gold(gold), _gems(gems), _itemDrop(itemDrop),
		_flying(flying), _imageNumber(imageNumber), _loopAnimation(loopAnimation),
		_animationEffect(animationEffect), _fx(fx), _attackVoc(attackVoc) {
}

void MonsterStruct::synchronize(Common::SeekableReadStream &s) {
	char name[16];
	Common::fill(name, name + 16, '\0');
	s.read(name, 16);
	name[15] = '\0';
	_name = Common::String(name);

	_experience = s.readUint32LE();
	_hp = s.readUint16LE();
	_armorClass = s.readByte();
	_speed = s.readByte();
	_numberOfAttacks = s.readByte();
	_hatesClass = (CharacterClass)s.readByte();
	_strikes = s.readUint16LE();
	_dmgPerStrike = s.readByte();
	_attackType = (DamageType)s.readByte();
	_specialAttack = (SpecialAttack)s.readByte();
	_hitChance = s.readByte();
	_rangeAttack = s.readByte();
	_monsterType = (MonsterType)s.readByte();
	_fireResistence = s.readByte();
	_electricityResistence = s.readByte();
	_coldResistence = s.readByte();
	_poisonResistence = s.readByte();
	_energyResistence = s.readByte();
	_magicResistence = s.readByte();
	_phsyicalResistence = s.readByte();
	_field29 = s.readByte();
	_gold = s.readUint16LE();
	_gems = s.readByte();
	_itemDrop = s.readByte();
	_flying = s.readByte() != 0;
	_imageNumber = s.readByte();
	_loopAnimation = s.readByte();
	_animationEffect = s.readByte();
	_fx = s.readByte();

	char attackVoc[10];
	Common::fill(attackVoc, attackVoc + 10, '\0');
	s.read(attackVoc, 9);
	attackVoc[9] = '\0';
	_attackVoc = Common::String(attackVoc);
}

MonsterData::MonsterData() {
}

void MonsterData::load(const Common::String &name) {
	File f(name);
	synchronize(f);
}

void MonsterData::synchronize(Common::SeekableReadStream &s) {
	clear();

	MonsterStruct spr;
	while (!s.eos()) {
		spr.synchronize(s);
		push_back(spr);
	}
}

/*------------------------------------------------------------------------*/

SurroundingMazes::SurroundingMazes() {
	clear();
}

void SurroundingMazes::clear() {
	_north = 0;
	_east = 0;
	_south = 0;
	_west = 0;
}

void SurroundingMazes::synchronize(XeenSerializer &s) {
	s.syncAsUint16LE(_north);
	s.syncAsUint16LE(_east);
	s.syncAsUint16LE(_south);
	s.syncAsUint16LE(_west);
}

int &SurroundingMazes::operator[](int idx) {
	switch (idx) {
	case DIR_NORTH:
		return _north;
	case DIR_EAST:
		return _east;
	case DIR_SOUTH:
		return _south;
	default:
		return _west;
	}
}

/*------------------------------------------------------------------------*/

MazeDifficulties::MazeDifficulties() {
	_unlockDoor = 0;
	_unlockBox = 0;
	_bashDoor = 0;
	_bashGrate = 0;
	_bashWall = 0;
	_wallNoPass = -1;
	_surfaceNoPass = -1;
	_chance2Run = -1;
}

void MazeDifficulties::synchronize(XeenSerializer &s) {
	s.syncAsByte(_wallNoPass);
	s.syncAsByte(_surfaceNoPass);
	s.syncAsByte(_unlockDoor);
	s.syncAsByte(_unlockBox);
	s.syncAsByte(_bashDoor);
	s.syncAsSint8(_bashGrate);
	s.syncAsSint8(_bashWall);
	s.syncAsSint8(_chance2Run);
}

/*------------------------------------------------------------------------*/

MazeData::MazeData() {
	clear();
}

void MazeData::clear() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_wallData[y][x]._data = 0;
		Common::fill(&_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH], false);
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], false);
		_wallTypes[y] = 0;
		_surfaceTypes[y] = 0;
	}
	_mazeNumber = 0;
	_surroundingMazes.clear();
	_mazeFlags = _mazeFlags2  = 0;
	_floorType = 0;
	_trapDamage = 0;
	_wallKind = 0;
	_tavernTips = 0;
	_mazeId = 0;
}

void MazeData::synchronize(XeenSerializer &s) {
	byte b;

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			s.syncAsUint16LE(_wallData[y][x]._data);
	}
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			if (s.isLoading()) {
				s.syncAsByte(b);
				_cells[y][x]._surfaceId = b & 7;
				_cells[y][x]._flags = b & 0xF8;
			} else {
				b = (_cells[y][x]._surfaceId & 7) | (_cells[y][x]._flags & 0xf8);
				s.syncAsByte(b);
			}
		}
	}

	s.syncAsUint16LE(_mazeNumber);
	_surroundingMazes.synchronize(s);
	s.syncAsUint16LE(_mazeFlags);
	s.syncAsUint16LE(_mazeFlags2);

	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_wallTypes[i]);
	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_surfaceTypes[i]);

	s.syncAsByte(_floorType);
	s.syncAsByte(_runPosition.x);
	_difficulties.synchronize(s);
	s.syncAsByte(_runPosition.y);
	s.syncAsByte(_trapDamage);
	s.syncAsByte(_wallKind);
	s.syncAsByte(_tavernTips);

	for (int y = 0; y < MAP_HEIGHT; ++y)
		File::syncBitFlags(s, &_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH]);
	for (int y = 0; y < MAP_HEIGHT; ++y)
		File::syncBitFlags(s, &_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH]);
}

void MazeData::setAllTilesStepped() {
	for (int y = 0; y < MAP_HEIGHT; ++y)
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], true);
}

void MazeData::clearCellSurfaces() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_cells[y][x]._surfaceId = 0;
	}
}

/*------------------------------------------------------------------------*/

MobStruct::MobStruct() {
	_id = 0;
	_direction = DIR_NORTH;
}

bool MobStruct::synchronize(XeenSerializer &s) {
	s.syncAsSint8(_pos.x);
	s.syncAsSint8(_pos.y);
	s.syncAsByte(_id);
	s.syncAsByte(_direction);

	return _id != 0xff || _pos.x != -1 || _pos.y != -1;
}

void MobStruct::endOfList() {
	_pos.x = _pos.y = -1;
	_id = 0xff;
	_direction = (Direction)-1;
}

/*------------------------------------------------------------------------*/

MazeObject::MazeObject() {
	_id = 0;
	_frame = 0;
	_spriteId = 0;
	_direction = DIR_NORTH;
	_flipped = false;
	_sprites = nullptr;
}

/*------------------------------------------------------------------------*/

MazeMonster::MazeMonster() {
	_frame = 0;
	_id = 0;
	_spriteId = 0;
	_isAttacking = false;
	_damageType = DT_PHYSICAL;
	_field9 = 0;
	_postAttackDelay = 0;
	_hp = 0;
	_effect1 = _effect2 = 0;
	_effect3 = 0;
	_sprites = nullptr;
	_attackSprites = nullptr;
	_monsterData = nullptr;
}

int MazeMonster::getTextColor() const {
	if (_hp == _monsterData->_hp)
		return 15;
	else if (_hp >= (_monsterData->_hp / 2))
		return 9;
	else
		return 32;
}

/*------------------------------------------------------------------------*/

MazeWallItem::MazeWallItem() {
	_id = 0;
	_frame = 0;
	_spriteId = 0;
	_direction = DIR_NORTH;
	_sprites = nullptr;
}

/*------------------------------------------------------------------------*/

MonsterObjectData::MonsterObjectData(XeenEngine *vm): _vm(vm) {
}

void MonsterObjectData::synchronize(XeenSerializer &s, MonsterData &monsterData) {
	Common::Array<MobStruct> mobStructs;
	MobStruct mobStruct;
	byte b;

	if (s.isLoading()) {
		_objectSprites.clear();
		_monsterSprites.clear();
		_monsterAttackSprites.clear();
		_wallItemSprites.clear();
		_objects.clear();
		_monsters.clear();
		_wallItems.clear();
	}

	byte objSprites[16];
	int maxSprite = 0;
	for (int i = 0; i < 16; ++i) {
		objSprites[i] = (i >= (int)_objectSprites.size()) ? 0xff : _objectSprites[i]._spriteId;
		s.syncAsByte(objSprites[i]);
		if (s.isLoading() && objSprites[i] != 0xff)
			maxSprite = i;
	}

	if (s.isLoading()) {
		for (int i = 0; i <= maxSprite; ++i) {
			if (objSprites[i] == 0xff)
				_objectSprites.push_back(SpriteResourceEntry());
			else
				_objectSprites.push_back(SpriteResourceEntry(objSprites[i]));
		}
	}

	for (uint i = 0; i < 16; ++i) {
		b = (i >= _monsterSprites.size()) ? 0xff : _monsterSprites[i]._spriteId;
		s.syncAsByte(b);
		if (s.isLoading() && b != 0xff)
			_monsterSprites.push_back(SpriteResourceEntry(b));
	}
	for (uint i = 0; i < 16; ++i) {
		b = (i >= _wallItemSprites.size()) ? 0xff : _wallItemSprites[i]._spriteId;
		s.syncAsByte(b);
		if (s.isLoading() && b != 0xff)
			_wallItemSprites.push_back(SpriteResourceEntry(b));
	}

	if (s.isSaving()) {
		// Save objects
		if (_objects.empty()) {
			mobStruct.endOfList();
			mobStruct.synchronize(s);
		} else {
			for (uint i = 0; i < _objects.size(); ++i) {
				mobStruct._pos = _objects[i]._position;
				mobStruct._id = _objects[i]._id;
				mobStruct._direction = _objects[i]._direction;
				mobStruct.synchronize(s);
			}
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

		// Save monsters
		if (_monsters.empty()) {
			mobStruct.endOfList();
			mobStruct.synchronize(s);
		} else {
			for (uint i = 0; i < _monsters.size(); ++i) {
				mobStruct._pos = _monsters[i]._position;
				mobStruct._id = _monsters[i]._id;
				mobStruct._direction = DIR_NORTH;
				mobStruct.synchronize(s);
			}
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

		// Save wall items
		for (uint i = 0; i < _wallItems.size(); ++i) {
			mobStruct._pos = _wallItems[i]._position;
			mobStruct._id = _wallItems[i]._id;
			mobStruct._direction = _wallItems[i]._direction;
			mobStruct.synchronize(s);
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

	} else {
		// Load monster/obbject data and merge together with sprite Ids
		// Load objects
		mobStruct.synchronize(s);
		if (mobStruct._id == -1 && mobStruct._pos.x == -1)
			// Empty array has a blank entry
			mobStruct.synchronize(s);

		do {
			MazeObject obj;
			obj._position = mobStruct._pos;
			obj._id = mobStruct._id;
			obj._direction = mobStruct._direction;
			obj._frame = 100;

			if (obj._id < (int)_objectSprites.size()) {
				obj._spriteId = _objectSprites[obj._id]._spriteId;
				obj._sprites = &_objectSprites[obj._id]._sprites;
			}

			_objects.push_back(obj);
			mobStruct.synchronize(s);
		} while (mobStruct._id != 255 || mobStruct._pos.x != -1);

		// Load monsters
		mobStruct.synchronize(s);
		if (mobStruct._id == -1 && mobStruct._pos.x == -1)
			// Empty array has a blank entry
			mobStruct.synchronize(s);

		while (mobStruct._id != 255 || mobStruct._pos.x != -1) {
			MazeMonster mon;
			mon._position = mobStruct._pos;
			mon._id = mobStruct._id;
			mon._frame = _vm->getRandomNumber(7);

			if (mon._id < (int)_monsterSprites.size()) {
				mon._spriteId = _monsterSprites[mon._id]._spriteId;
				mon._sprites = &_monsterSprites[mon._id]._sprites;
				mon._attackSprites = &_monsterSprites[mon._id]._attackSprites;
				mon._monsterData = &monsterData[mon._spriteId];

				MonsterStruct &md = *mon._monsterData;
				mon._hp = md._hp;
				mon._effect1 = mon._effect2 = md._animationEffect;
				if (md._animationEffect)
					mon._effect3 = _vm->getRandomNumber(7);

				_monsters.push_back(mon);
			} else {
				assert(!mon._id);
			}

			mobStruct.synchronize(s);
		}

		// Load wall items. Unlike the previous two arrays, this has no dummy entry for an empty array
		mobStruct.synchronize(s);
		while (mobStruct._id != 255 || mobStruct._pos.x != -1) {
			if (mobStruct._id < (int)_wallItemSprites.size()) {
				MazeWallItem wi;
				wi._position = mobStruct._pos;
				wi._id = mobStruct._id;
				wi._direction = mobStruct._direction;
				wi._spriteId = _wallItemSprites[wi._id]._spriteId;
				wi._sprites = &_wallItemSprites[wi._id]._sprites;

				_wallItems.push_back(wi);
			}

			mobStruct.synchronize(s);
		}
	}
}

void MonsterObjectData::clearMonsterSprites() {
	_monsterSprites.clear();
	_monsterAttackSprites.clear();
}

void MonsterObjectData::addMonsterSprites(MazeMonster &monster) {
	Map &map = *g_vm->_map;
	monster._monsterData = &map._monsterData[monster._spriteId];
	int imgNumber = monster._monsterData->_imageNumber;
	uint idx;

	// Find the sprites for the monster, loading them in if necessary
	for (idx = 0; idx < _monsterSprites.size(); ++idx) {
		if (_monsterSprites[idx]._spriteId == monster._spriteId) {
			monster._sprites = &_monsterSprites[idx]._sprites;
			break;
		}
	}
	if (idx == _monsterSprites.size()) {
		_monsterSprites.push_back(SpriteResourceEntry(monster._spriteId));
		_monsterSprites.back()._sprites.load(Common::String::format("%03u.mon", imgNumber));
		monster._sprites = &_monsterSprites.back()._sprites;
	}

	// Find the attack sprites for the monster, loading them in if necessary
	for (idx = 0; idx < _monsterAttackSprites.size(); ++idx) {
		if (_monsterAttackSprites[idx]._spriteId == monster._spriteId) {
			monster._attackSprites = &_monsterAttackSprites[idx]._sprites;
			break;
		}
	}
	if (idx == _monsterAttackSprites.size()) {
		_monsterAttackSprites.push_back(SpriteResourceEntry(monster._spriteId));
		_monsterAttackSprites.back()._sprites.load(Common::String::format("%03u.att", imgNumber));
		monster._attackSprites = &_monsterAttackSprites.back()._sprites;
	}
}

/*------------------------------------------------------------------------*/

HeadData::HeadData() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			_data[y][x]._left = _data[y][x]._right = 0;
		}
	}
}

void HeadData::synchronize(Common::SeekableReadStream &s) {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			_data[y][x]._left = s.readByte();
			_data[y][x]._right = s.readByte();
		}
	}
}

/*------------------------------------------------------------------------*/

void AnimationEntry::synchronize(Common::SeekableReadStream &s) {
	for (int i = 0; i < 4; ++i)
		_frame1._frames[i] = s.readByte();
	for (int i = 0; i < 4; ++i)
		_flipped._flags[i] = s.readByte() != 0;
	for (int i = 0; i < 4; ++i)
		_frame2._frames[i] = s.readByte();
}

void AnimationInfo::synchronize(Common::SeekableReadStream &s) {
	AnimationEntry entry;

	clear();
	while (s.pos() < s.size()) {
		entry.synchronize(s);
		push_back(entry);
	}
}

void AnimationInfo::load(const Common::String &name) {
	File f(name);
	synchronize(f);
	f.close();
}

/*------------------------------------------------------------------------*/

Map::Map(XeenEngine *vm) : _vm(vm), _mobData(vm) {
	_loadCcNum = 0;
	_sideTownPortal = 0;
	_sideObjects = 0;
	_sideMonsters = 0;
	_sidePictures = 0;
	_isOutdoors = false;
	_mazeDataIndex = 0;
	_currentSteppedOn = false;
	_currentSurfaceId = 0;
	_currentWall = 0;
	_currentTile = 0;
	_currentGrateUnlocked = false;
	_currentCantRest = false;
	_currentIsDrain = false;
	_currentIsEvent = false;
	_currentSky = 0;
	_currentMonsterFlags = 0;
}

void Map::load(int mapId) {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Patcher &patcher = *g_vm->_patcher;
	Sound &sound = *g_vm->_sound;
	IndoorDrawList &indoorList = intf._indoorList;
	OutdoorDrawList &outdoorList = intf._outdoorList;

	PleaseWait waitMsg(intf._falling);
	waitMsg.show();

	intf._objNumber = -1;
	party._stepped = true;
	party._mazeId = mapId;
	saveMaze();
	events.clearEvents();

	_sideObjects = 1;
	_sideMonsters = 1;
	_sidePictures = 1;
	if (mapId >= 113 && mapId <= 127) {
		_sideTownPortal = 0;
	} else {
		_sideTownPortal = _loadCcNum;
	}

	if (_vm->getGameID() == GType_Swords || _vm->getGameID() == GType_DarkSide) {
		_animationInfo.load("dark.dat");
		_monsterData.load((_vm->getGameID() == GType_Swords) ? "monsters.swd" : "dark.mon");
		_wallPicSprites.load("darkpic.dat");
	} else if (_vm->getGameID() == GType_Clouds) {
		_animationInfo.load("animinfo.cld");
		_monsterData.load("monsters.cld");
		_wallPicSprites.load("wallpics.cld");
	} else if (_vm->getGameID() == GType_WorldOfXeen) {
		files.setGameCc(1);

		if (!_loadCcNum) {
			_animationInfo.load("clouds.dat");
			_monsterData.load("xeen.mon");
			_wallPicSprites.load("xeenpic.dat");
			_sidePictures = 0;
			_sideMonsters = 0;
			_sideObjects = 0;
		} else {
			switch (mapId) {
			case 113:
			case 114:
			case 115:
			case 116:
			case 128:
				_animationInfo.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObjects = 0;
				break;
			case 117:
			case 118:
			case 119:
			case 120:
			case 124:
				_animationInfo.load("clouds.dat");
				_monsterData.load("xeen.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObjects = 0;
				_sideMonsters = 0;
				break;
			case 125:
			case 126:
			case 127:
				_animationInfo.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("xeenpic.dat");
				_sideObjects = 0;
				_sidePictures = 0;
				break;
			default:
				_animationInfo.load("dark.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("darkpic.dat");
				break;
			}
		}

		files.setGameCc(_loadCcNum);
	}

	// Load any events for the new map
	loadEvents(mapId, _loadCcNum);

	// Iterate through loading the given maze as well as the two successive
	// mazes in each of the four cardinal directions
	int ccNum = files._ccNum;
	MazeData *mazeDataP = &_mazeData[0];
	bool mapDataLoaded = false;

	for (int idx = 0; idx < 9; ++idx, ++mazeDataP) {
		mazeDataP->_mazeId = mapId;

		if (mapId == 0) {
			mazeDataP->clear();
		} else {
			// Load in the maze's data file
			Common::String datName = Common::String::format("maze%c%03d.dat",
				(mapId >= 100) ? 'x' : '0', mapId);
			File datFile(datName);
			XeenSerializer datSer(&datFile, nullptr);
			mazeDataP->synchronize(datSer);
			datFile.close();

			if (ccNum && mapId == 50)
				mazeDataP->setAllTilesStepped();
			if (!ccNum && party._gameFlags[0][25] &&
					(mapId == 42 || mapId == 43 || mapId == 4)) {
				mazeDataP->clearCellSurfaces();
			}

			_isOutdoors = (mazeDataP->_mazeFlags2 & FLAG_IS_OUTDOORS) != 0;

			Common::String mobName = Common::String::format("maze%c%03d.mob", (mapId >= 100) ? 'x' : '0', mapId);

			if (!mapDataLoaded) {
				// Called once for the main map being loaded
				mapDataLoaded = true;
				_mazeName = getMazeName(mapId, ccNum);

				// Load the monster/object data
				File mobFile(mobName);
				XeenSerializer sMob(&mobFile, nullptr);
				_mobData.synchronize(sMob, _monsterData);
				mobFile.close();

				Common::String headName = Common::String::format("aaze%c%03d.hed",
					(mapId >= 100) ? 'x' : '0', mapId);
				File headFile(headName);
				_headData.synchronize(headFile);
				headFile.close();

				if (!ccNum && mapId == 15) {
					if ((_mobData._monsters[0]._position.x > 31 || _mobData._monsters[0]._position.y > 31) &&
						(_mobData._monsters[1]._position.x > 31 || _mobData._monsters[1]._position.y > 31) &&
						(_mobData._monsters[2]._position.x > 31 || _mobData._monsters[2]._position.y > 31)) {
						party._gameFlags[0][56] = true;
					}
				}
			} else if (File::exists(mobName)) {
				// For surrounding maps, set up flags for whether objects are present
				// Load the monster/object data
				File mobFile(mobName);
				XeenSerializer sMob(&mobFile, nullptr);
				MonsterObjectData mobData(_vm);
				mobData.synchronize(sMob, _monsterData);
				mobFile.close();

				mazeDataP->_objectsPresent.resize(mobData._objects.size());
				for (uint objIndex = 0; objIndex < mobData._objects.size(); ++objIndex) {
					const Common::Point &pt = mobData._objects[objIndex]._position;
					mazeDataP->_objectsPresent[objIndex] = ABS(pt.x) != 128 && ABS(pt.y) != 128;
				}
			}
		}

		// Move to next surrounding maze
		MazeData *baseMaze = &_mazeData[MAP_GRID_PRIOR_INDEX[idx]];
		mapId = baseMaze->_surroundingMazes[MAP_GRID_PRIOR_DIRECTION[idx]];
		if (!mapId) {
			baseMaze = &_mazeData[MAP_GRID_PRIOR_INDEX2[idx]];
			mapId = baseMaze->_surroundingMazes[MAP_GRID_PRIOR_DIRECTION2[idx]];
		}
	}

	// Reload the monster data for the main maze that we're loading
	mapId = party._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.mob",
		(mapId >= 100) ? 'x' : '0', mapId);
	File mobFile(filename);
	XeenSerializer sMob(&mobFile, nullptr);
	_mobData.synchronize(sMob, _monsterData);
	mobFile.close();

	// Load sprites for the objects
	for (uint i = 0; i < _mobData._objectSprites.size(); ++i) {
		files.setGameCc(_sideObjects);

		if (party._cloudsCompleted && _mobData._objectSprites[i]._spriteId == 85 &&
				mapId == 27 && ccNum) {
			_mobData._objects[29]._spriteId = 0;
			_mobData._objects[29]._id = 8;
			_mobData._objectSprites[i]._sprites.clear();
		} else if (mapId == 12 && party._gameFlags[0][43] &&
			_mobData._objectSprites[i]._spriteId == 118 && !ccNum) {
			filename = "085.obj";
			_mobData._objectSprites[0]._spriteId = 85;
		} else {
			filename = Common::String::format("%03d.%cbj",
				_mobData._objectSprites[i]._spriteId,
				_mobData._objectSprites[i]._spriteId >= 100 ? '0' : 'o');
		}

		// Read in the object sprites
		if (!_mobData._objectSprites[i].isEmpty())
			_mobData._objectSprites[i]._sprites.load(filename);
	}

	// Load sprites for the monsters
	for (uint i = 0; i < _mobData._monsterSprites.size(); ++i) {
		MonsterObjectData::SpriteResourceEntry &spr = _mobData._monsterSprites[i];
		uint imgNumber = _monsterData[spr._spriteId]._imageNumber;

		files.setGameCc((spr._spriteId == 91 && _vm->getGameID() == GType_WorldOfXeen) ?
			0 : _sideMonsters);
		filename = Common::String::format("%03u.mon", imgNumber);
		_mobData._monsterSprites[i]._sprites.load(filename);

		filename = Common::String::format("%03u.att", imgNumber);
		_mobData._monsterSprites[i]._attackSprites.load(filename);
	}

	// Load wall picture sprite resources
	for (uint i = 0; i < _mobData._wallItemSprites.size(); ++i) {
		filename = Common::String::format("%03d.pic", _mobData._wallItemSprites[i]._spriteId);
		_mobData._wallItemSprites[i]._sprites.load(filename, _sidePictures);
	}

	files.setGameCc(ccNum);

	// Handle loading miscellaneous sprites for the map
	if (_isOutdoors) {
		// Start playing relevant music
		sound._musicSide = ccNum;
		Common::String musName;

		if (_vm->_files->_ccNum) {
			int randIndex = _vm->getRandomNumber(6);
			musName = Res.MUSIC_FILES2[_mazeData->_wallKind][randIndex];
		} else {
			musName = "outdoors.m";
		}
		if (musName != sound._currentMusic)
			sound.playSong(musName, 207);

		// Load sprite sets needed for scene rendering
		_groundSprites.load("water.out");
		_tileSprites.load("outdoor.til");
		outdoorList._sky1._sprites = &_skySprites[0];
		outdoorList._sky2._sprites = &_skySprites[0];
		outdoorList._groundSprite._sprites = &_groundSprites;

		for (int i = 0; i < TOTAL_SURFACES; ++i) {
			_wallSprites._surfaces[i].clear();

			if (_mazeData[0]._wallTypes[i] != 0) {
				_wallSprites._surfaces[i].load(Common::String::format("%s.wal",
					Res.OUTDOORS_WALL_TYPES[_mazeData[0]._wallTypes[i]]));
			}

			_surfaceSprites[i].clear();
			if (i != 0 && _mazeData[0]._surfaceTypes[i] != 0)
				_surfaceSprites[i].load(Res.SURFACE_NAMES[_mazeData[0]._surfaceTypes[i]]);
		}
	} else {
		if (files._ccNum && (mapId == 125 || mapId == 126 || mapId == 127))
			files.setGameCc(0);
		sound._musicSide = files._ccNum;

		// Start playing relevant music
		const int MUS_INDEXES[] = { 1, 2, 3, 4, 3, 5 };
		Common::String musName;

		if (files._ccNum) {
			int randIndex = _vm->getRandomNumber(6);
			musName = Res.MUSIC_FILES2[MUS_INDEXES[_mazeData->_wallKind]][randIndex];
		} else {
			musName = Res.MUSIC_FILES1[MUS_INDEXES[_mazeData->_wallKind]];
		}
		if (musName != sound._currentMusic)
			sound.playSong(musName, 207);

		// Load sprite sets needed for scene rendering
		_skySprites[1].load(Common::String::format("%s.sky",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_groundSprites.load(Common::String::format("%s.gnd",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_tileSprites.load(Common::String::format("%s.til",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));

		for (int i = 0; i < TOTAL_SURFACES; ++i) {
			_surfaceSprites[i].clear();

			if (_mazeData[0]._surfaceTypes[i] != 0 || i == 4)
				_surfaceSprites[i].load(Res.SURFACE_NAMES[_mazeData[0]._surfaceTypes[i]]);
		}

		for (int i = 0; i < TOTAL_SURFACES; ++i)
			_wallSprites._surfaces[i].clear();

		_wallSprites._fwl1.load(Common::String::format("f%s1.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]), _sidePictures);
		_wallSprites._fwl2.load(Common::String::format("f%s2.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]), _sidePictures);
		_wallSprites._fwl3.load(Common::String::format("f%s3.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]), _sidePictures);
		_wallSprites._fwl4.load(Common::String::format("f%s4.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]), _sidePictures);
		_wallSprites._swl.load(Common::String::format("s%s.swl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]), _sidePictures);

		// Set entries in the indoor draw list to the correct sprites
		// for drawing various parts of the background
		indoorList._swl_0F1R._sprites = &_wallSprites._swl;
		indoorList._swl_0F1L._sprites = &_wallSprites._swl;
		indoorList._swl_1F1R._sprites = &_wallSprites._swl;
		indoorList._swl_1F1L._sprites = &_wallSprites._swl;
		indoorList._swl_2F2R._sprites = &_wallSprites._swl;
		indoorList._swl_2F1R._sprites = &_wallSprites._swl;
		indoorList._swl_2F1L._sprites = &_wallSprites._swl;
		indoorList._swl_2F2L._sprites = &_wallSprites._swl;

		indoorList._swl_3F1R._sprites = &_wallSprites._swl;
		indoorList._swl_3F2R._sprites = &_wallSprites._swl;
		indoorList._swl_3F3R._sprites = &_wallSprites._swl;
		indoorList._swl_3F4R._sprites = &_wallSprites._swl;
		indoorList._swl_3F1L._sprites = &_wallSprites._swl;
		indoorList._swl_3F2L._sprites = &_wallSprites._swl;
		indoorList._swl_3F3L._sprites = &_wallSprites._swl;
		indoorList._swl_3F4L._sprites = &_wallSprites._swl;

		indoorList._swl_4F4R._sprites = &_wallSprites._swl;
		indoorList._swl_4F3R._sprites = &_wallSprites._swl;
		indoorList._swl_4F2R._sprites = &_wallSprites._swl;
		indoorList._swl_4F1R._sprites = &_wallSprites._swl;
		indoorList._swl_4F1L._sprites = &_wallSprites._swl;
		indoorList._swl_4F2L._sprites = &_wallSprites._swl;
		indoorList._swl_4F3L._sprites = &_wallSprites._swl;
		indoorList._swl_4F4L._sprites = &_wallSprites._swl;

		indoorList._fwl_4F4R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F3R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F2R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F1R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F1L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F2L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F3L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F4L._sprites = &_wallSprites._fwl4;

		indoorList._fwl_2F1R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_2F._sprites = &_wallSprites._fwl3;
		indoorList._fwl_2F1L._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F2R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F1R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F1L._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F2L._sprites = &_wallSprites._fwl3;

		indoorList._fwl_1F._sprites = &_wallSprites._fwl1;
		indoorList._fwl_1F1R._sprites = &_wallSprites._fwl1;
		indoorList._fwl_1F1L._sprites = &_wallSprites._fwl1;
		indoorList._horizon._sprites = &_wallSprites._fwl1;

		indoorList._ground._sprites = &_groundSprites;

		// Don't show horizon for certain maps
		if (_vm->_files->_ccNum) {
			if ((mapId >= 89 && mapId <= 112) || mapId == 128 || mapId == 129)
				indoorList._horizon._sprites = nullptr;
		} else {
			if (mapId >= 25 && mapId <= 27)
				indoorList._horizon._sprites = nullptr;
		}
	}

	patcher.patch();
	loadSky();

	files.setGameCc(ccNum);
}

void Map::findMap(int mapId) {
	if (mapId == -1)
		mapId = _vm->_party->_mazeId;

	_mazeDataIndex = 0;
	while (_mazeDataIndex < 9 && _mazeData[_mazeDataIndex]._mazeId != mapId)
		++_mazeDataIndex;
	if (_mazeDataIndex == 9)
		error("Could not find map %d", mapId);
}

int Map::mazeLookup(const Common::Point &pt, int layerShift, int wallMask) {
	Common::Point pos = pt;
	int mapId = _vm->_party->_mazeId;

	if (pt.x < -16 || pt.y < -16 || pt.x >= 32 || pt.y >= 32) {
		_currentWall = INVALID_CELL;
		return INVALID_CELL;
	}

	// Find the correct maze data out of the set to use
	findMap();

	// Handle map changing to the north or south as necessary
	if (pos.y & 16) {
		if (pos.y >= 0) {
			pos.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pos.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (mapId) {
			// Move to the correct map to north/south
			findMap(mapId);
		} else {
			// No map, so reached outside indoor area or outer space outdoors
			_currentSteppedOn = true;
			return _isOutdoors ? SURFTYPE_SPACE : INVALID_CELL;
		}
	}

	// Handle map changing to the east or west as necessary
	if (pos.x & 16) {
		if (pos.x >= 0) {
			pos.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pos.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		if (mapId)
			// Move to the correct map to east/west
			findMap(mapId);
	}

	if (mapId) {
		if (_isOutdoors) {
			_currentSurfaceId = _mazeData[_mazeDataIndex]._wallData[pos.y][pos.x]._outdoors._surfaceId;
		} else {
			_currentSurfaceId = _mazeData[_mazeDataIndex]._cells[pos.y][pos.x]._surfaceId;
		}

		if (mazeData()._surfaceTypes[_currentSurfaceId] == SURFTYPE_SPACE ||
				mazeData()._surfaceTypes[_currentSurfaceId] == SURFTYPE_SKY) {
			_currentSteppedOn = true;
		} else {
			_currentSteppedOn = _mazeData[_mazeDataIndex]._steppedOnTiles[pos.y][pos.x];
		}

		return (_mazeData[_mazeDataIndex]._wallData[pos.y][pos.x]._data >> layerShift) & wallMask;

	} else {
		_currentSteppedOn = _isOutdoors;
		return _isOutdoors ? SURFTYPE_SPACE : INVALID_CELL;
	}
}

void Map::loadEvents(int mapId, int ccNum) {
	// Load events
	Common::String filename = Common::String::format("maze%c%03d.evt",
		(mapId >= 100) ? 'x' : '0', mapId);
	File fEvents(filename, ccNum);
	XeenSerializer sEvents(&fEvents, nullptr);
	_events.synchronize(sEvents);
	fEvents.close();

	// Load text data
	filename = Common::String::format("aaze%c%03d.txt",
		(mapId >= 100) ? 'x' : '0', mapId);
	File fText(filename, ccNum);
	_events._text.clear();
	while (fText.pos() < fText.size())
		_events._text.push_back(fText.readString());
	fText.close();
}

void Map::saveEvents() {
	// Save eents
	int mapId = _mazeData[0]._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.evt",
		(mapId >= 100) ? 'x' : '0', mapId);
	OutFile fEvents(filename);
	XeenSerializer sEvents(nullptr, &fEvents);
	_events.synchronize(sEvents);
	fEvents.finalize();
}

void Map::saveMap() {
	FileManager &files = *g_vm->_files;
	Party &party = *g_vm->_party;
	int mapId = _mazeData[0]._mazeId;
	if (!files._ccNum && mapId == 85)
		return;

	// Save the primary maze
	Common::String datName = Common::String::format("maze%c%03d.dat", (mapId >= 100) ? 'x' : '0', mapId);
	OutFile datFile(datName);
	XeenSerializer datSer(nullptr, &datFile);
	_mazeData[0].synchronize(datSer);
	datFile.finalize();

	if (!files._ccNum && mapId == 15) {
		for (uint idx = 0; idx < MIN(_mobData._monsters.size(), (uint)3); ++idx) {
			MazeMonster &mon = _mobData._monsters[idx];
			if (mon._position.x > 31 || mon._position.y > 31) {
				party._gameFlags[0][56] = true;
				break;
			}
		}
	}

	if (!_isOutdoors) {
		// Iterate through the surrounding mazes
		for (int mazeIndex = 1; mazeIndex < 9; ++mazeIndex) {
			mapId = _mazeData[mazeIndex]._mazeId;
			if (mapId == 0)
				continue;

			datName = Common::String::format("maze%c%03d.dat", (mapId >= 100) ? 'x' : '0', mapId);
			OutFile datFile2(datName);
			XeenSerializer datSer2(nullptr, &datFile2);
			_mazeData[mazeIndex].synchronize(datSer2);
			datFile2.finalize();
		}
	}
}

void Map::saveMonsters() {
	int mapId = _mazeData[0]._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.mob",
		(mapId >= 100) ? 'x' : '0', mapId);
	OutFile fMob(filename);
	XeenSerializer sMob(nullptr, &fMob);
	_mobData.synchronize(sMob, _monsterData);
	fMob.finalize();
}

void Map::saveMaze() {
	int mazeNum = _mazeData[0]._mazeNumber;
	if (!mazeNum || (mazeNum == 85 && !_vm->_files->_ccNum))
		return;

	saveEvents();
	saveMap();
	saveMonsters();
}

void Map::clearMaze() {
	_mazeData[0]._mazeNumber = 0;
}

void Map::cellFlagLookup(const Common::Point &pt) {
	Common::Point pos = pt;
	findMap();

	int mapId = _vm->_party->_mazeId;
	findMap(mapId);

	// Handle map changing to the north or south as necessary
	if (pos.y & 16) {
		if (pos.y >= 0) {
			pos.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pos.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		findMap(mapId);
	}

	// Handle map changing to the east or west as necessary
	if (pos.x & 16) {
		if (pos.x >= 0) {
			pos.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pos.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		findMap(mapId);
	}

	// Get the cell flags
	const MazeCell &cell = _mazeData[_mazeDataIndex]._cells[pos.y][pos.x];
	_currentGrateUnlocked = cell._flags & OUTFLAG_GRATE;
	_currentCantRest = cell._flags & RESTRICTION_REST;
	_currentIsDrain = cell._flags & OUTFLAG_DRAIN;
	_currentIsEvent = cell._flags & FLAG_AUTOEXECUTE_EVENT;
	_currentSky = (cell._flags & OUTFLAG_OBJECT_EXISTS) ? 1 : 0;
	_currentMonsterFlags = cell._flags & 7;
}

void Map::setCellSurfaceFlags(const Common::Point &pt, int bits) {
	mazeLookup(pt, 0);

	Common::Point mapPos(pt.x & 15, pt.y & 15);
	MazeCell &cell = _mazeData[_mazeDataIndex]._cells[mapPos.y][mapPos.x];
	cell._flags |= bits & 0xF8;
}

void Map::setWall(const Common::Point &pt, Direction dir, int v) {
	const int XOR_MASKS[4] = { 0xFFF, 0xF0FF, 0xFF0F, 0xFFF0 };
	mazeLookup(pt, 0, 0);

	Common::Point mapPos(pt.x & 15, pt.y & 15);
	MazeWallLayers &wallLayer = _mazeData[_mazeDataIndex]._wallData[mapPos.y][mapPos.x];
	wallLayer._data &= XOR_MASKS[dir];
	wallLayer._data |= v << Res.WALL_SHIFTS[dir][2];
}

int Map::getCell(int idx) {
	Party &party = *g_vm->_party;
	int mapId = party._mazeId;
	Direction dir = _vm->_party->_mazeDirection;
	Common::Point pt(
		_vm->_party->_mazePosition.x + Res.SCREEN_POSITIONING_X[_vm->_party->_mazeDirection][idx],
		_vm->_party->_mazePosition.y + Res.SCREEN_POSITIONING_Y[_vm->_party->_mazeDirection][idx]
	);

	if (pt.x > 31 || pt.y > 31) {
		if (_vm->_files->_ccNum) {
			if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
					mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
				_currentSurfaceId = SURFTYPE_DESERT;
			} else {
				_currentSurfaceId = 0;
			}
		} else {
			_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? 7 : 0;
		}
		_currentWall = INVALID_CELL;
		return INVALID_CELL;
	}

	findMap(mapId);

	if (pt.y & 16) {
		if (pt.y >= 0) {
			pt.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pt.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (!mapId) {
			mapId = party._mazeId;

			if (_isOutdoors) {
				_currentSurfaceId = SURFTYPE_SPACE;
				_currentWall = 0;
				return 0;
			} else {
				if (_vm->_files->_ccNum) {
					if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
						mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
						_currentSurfaceId = 6;
					} else {
						_currentSurfaceId = 0;
					}
				} else {
					_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? SURFTYPE_ROAD : SURFTYPE_DEFAULT;
				}

				_currentWall = INVALID_CELL;
				return INVALID_CELL;
			}
		}

		findMap(mapId);
	}

	if (pt.x & 16) {
		if (pt.x >= 0) {
			pt.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pt.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		if (!mapId) {
			mapId = party._mazeId;

			if (_isOutdoors) {
				_currentSurfaceId = SURFTYPE_SPACE;
				_currentWall = 0;
				return 0;
			} else {
				if (_vm->_files->_ccNum) {
					if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
						mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
						_currentSurfaceId = 6;
					} else {
						_currentSurfaceId = 0;
					}
				} else {
					_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? SURFTYPE_ROAD : SURFTYPE_DEFAULT;
				}

				_currentWall = INVALID_CELL;
				return INVALID_CELL;
			}
		}

		findMap(mapId);
	}

	assert(pt.x >= 0 && pt.x < 16 && pt.y >= 0 && pt.y < 16);
	int wallData = _mazeData[_mazeDataIndex]._wallData[pt.y][pt.x]._data;
	if (_isOutdoors) {
		if (mapId) {
			_currentTile = (wallData >> 8) & 0xFF;
			_currentWall = (wallData >> 4) & 0xF;
			_currentSurfaceId = wallData & 0xF;
		} else {
			_currentSurfaceId = SURFTYPE_DEFAULT;
			_currentWall = 0;
			_currentTile = 0;
		}
	} else {
		if (!mapId)
			return 0;

		_currentSurfaceId = _mazeData[_mazeDataIndex]._cells[pt.y][pt.x]._surfaceId;
		_currentWall = wallData;
		return (_currentWall >> Res.WALL_SHIFTS[dir][idx]) & 0xF;
	}

	return _currentWall;
}

void Map::loadSky() {
	Party &party = *_vm->_party;

	party._isNight = party._minutes < (5 * 60) || party._minutes >= (21 * 60);
	_skySprites[0].load(((party._mazeId >= 89 && party._mazeId <= 112) ||
		party._mazeId == 128 || party._mazeId == 129) || !party._isNight
		? "sky.sky" : "night.sky");
}

void Map::getNewMaze() {
	Party &party = *_vm->_party;
	Common::Point pt = party._mazePosition;
	int mapId = party._mazeId;

	// Get the correct map to use from the cached list
	findMap(mapId);

	// Adjust Y and X to be in the 0-15 range, and on the correct surrounding
	// map if either value is < 0 or >= 16
	if (pt.y & 16) {
		if (pt.y >= 0) {
			pt.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pt.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (mapId)
			findMap(mapId);
	}

	if (pt.x & 16) {
		if (pt.x >= 0) {
			pt.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pt.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		if (mapId)
			findMap(mapId);
	}

	// Save the adjusted (0,0)-(15,15) position and load the given map.
	// This will make it the new center, with it's own surrounding mazees loaded
	party._mazePosition = pt;
	if (mapId)
		load(mapId);
}

Common::String Map::getMazeName(int mapId, int ccNum) {
	if (ccNum == -1)
		ccNum = g_vm->_files->_ccNum;

	if (g_vm->getGameID() == GType_Clouds) {
		return Res._cloudsMapNames[mapId];
	} else {
		Common::String txtName = Common::String::format("%s%c%03d.txt",
			ccNum ? "dark" : "xeen", mapId >= 100 ? 'x' : '0', mapId);
		File fText(txtName, 1);
		char mazeName[33];
		fText.read(mazeName, 33);
		mazeName[32] = '\0';

		Common::String name = Common::String(mazeName);
		fText.close();
		return name;
	}
}

} // End of namespace Xeen
