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

#ifndef ULTIMA8_GRAPHICS_SHAPEINFO_H
#define ULTIMA8_GRAPHICS_SHAPEINFO_H

#include "ultima/ultima8/world/weapon_info.h"
#include "ultima/ultima8/world/armour_info.h"
#include "ultima/ultima8/world/damage_info.h"
#include "ultima/ultima8/world/actors/monster_info.h"

namespace Ultima {
namespace Ultima8 {

class ShapeInfo {
public:
	enum SFlags {
		SI_FIXED   = 0x0001,
		SI_SOLID   = 0x0002,
		SI_SEA     = 0x0004,
		SI_LAND    = 0x0008,
		SI_OCCL    = 0x0010,
		SI_BAG     = 0x0020,
		SI_DAMAGING = 0x0040,
		SI_NOISY   = 0x0080,
		SI_DRAW    = 0x0100,
		SI_IGNORE  = 0x0200,
		SI_ROOF    = 0x0400,
		SI_TRANSL  = 0x0800,
		SI_EDITOR  = 0x1000,
		// Note: overlapping names for the rest of the bits depending on U8 or Cru.
		SI_U8_EXPLODE  = 0x2000,
		SI_CRU_SELECTABLE = 0x2000,
		SI_UNKNOWN46      = 0x4000,
		SI_CRU_PRELOAD    = 0x4000, // we don't need this flag, we preload everything.
		SI_UNKNOWN47      = 0x8000,
		SI_CRU_SOUND      = 0x8000, // TODO: how is this used?
		SI_CRU_TARGETABLE = 0x10000,
		SI_CRU_NPC        = 0x20000,
		SI_CRU_UNK66      = 0x40000,
		SI_CRU_UNK67      = 0x80000
	};

	enum SFamily {
		SF_GENERIC     = 0,
		SF_QUALITY     = 1,
		SF_QUANTITY    = 2,
		SF_GLOBEGG     = 3,
		// "Unk" eggs are not "unknown", they are triggers for usecode
		// (unk is the source language for usecode)
		SF_UNKEGG      = 4,
		SF_BREAKABLE   = 5,
		SF_CONTAINER   = 6,
		SF_MONSTEREGG  = 7,
		SF_TELEPORTEGG = 8,
		SF_REAGENT     = 9,
		SF_CRUWEAPON   = 10, // Used in Crusader
		SF_CRUAMMO     = 11, // Used in Crusader
		SF_CRUBOMB     = 12, // Used in Crusader
		SF_CRUINVITEM  = 13, // Used in Crusader
		SF_15          = 15
	};

	enum SEquipType {
		SE_NONE   = 0,
		SE_SHIELD = 1,
		SE_ARM    = 2,
		SE_HEAD   = 3,
		SE_BODY   = 4,
		SE_LEGS   = 5,
		SE_WEAPON = 6
	};

	uint32 _flags;
	uint32 _x, _y, _z;
	uint32 _family;
	uint32 _equipType;
	uint32 _animType, _animData, _animSpeed;
	uint32 _weight, _volume;

	WeaponInfo *_weaponInfo;
	ArmourInfo *_armourInfo;
	MonsterInfo *_monsterInfo;
	DamageInfo *_damageInfo;

	inline bool is_fixed() const {
		return (_flags & SI_FIXED) != 0;
	}
	inline bool is_solid() const {
		return (_flags & SI_SOLID) != 0;
	}
	inline bool is_sea() const {
		return (_flags & SI_SEA) != 0;
	}
	inline bool is_land() const {
		return (_flags & SI_LAND) != 0;
	}
	inline bool is_occl() const {
		return (_flags & SI_OCCL) != 0;
	}
	inline bool is_bag() const {
		return (_flags & SI_BAG) != 0;
	}
	inline bool is_damaging() const {
		return (_flags & SI_DAMAGING) != 0;
	}
	inline bool is_noisy() const {
		return (_flags & SI_NOISY) != 0;
	}
	inline bool is_draw() const {
		return (_flags & SI_DRAW) != 0;
	}
	inline bool is_ignore() const {
		return (_flags & SI_IGNORE) != 0;
	}
	inline bool is_roof() const {
		return (_flags & SI_ROOF) != 0;
	}
	inline bool is_translucent() const {
		return (_flags & SI_TRANSL) != 0;
	}
	inline bool is_editor() const {
		return (_flags & SI_EDITOR) != 0;
	}
	inline bool is_u8_explode() const {
		return (_flags & SI_U8_EXPLODE) != 0;
	}

	bool hasQuantity() const {
		return (_family == SF_QUANTITY || _family == SF_REAGENT);
	}

	bool takesDamage() const {
		return (_damageInfo && _damageInfo->takesDamage());
	}

	bool getTypeFlag(int typeFlag) const;
	bool getTypeFlagU8(int typeFlag) const;
	bool getTypeFlagCrusader(int typeFlag) const;

	inline void getFootpadWorld(int32 &x, int32 &y, int32 &z, uint16 flipped) const;

	ShapeInfo() :
		_flags(0), _x(0), _y(0), _z(0),
		_family(0), _equipType(0), _animType(0), _animData(0),
		_animSpeed(0), _weight(0), _volume(0),
		_weaponInfo(nullptr), _armourInfo(nullptr),
		_monsterInfo(nullptr), _damageInfo(nullptr) { }

	~ShapeInfo() {
		delete _weaponInfo;
		delete[] _armourInfo;
		delete _monsterInfo;
		delete _damageInfo;
	}

};

inline void ShapeInfo::getFootpadWorld(int32 &x, int32 &y, int32 &z, uint16 flipped) const {
	z = _z *  8;

	if (flipped) {
		x = _y * 32;
		y = _x * 32;
	} else {
		x = _x * 32;
		y = _y * 32;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
