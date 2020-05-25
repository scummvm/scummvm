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

#ifndef ULTIMA4_GAME_WEAPON_H
#define ULTIMA4_GAME_WEAPON_H

#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Weapons;

class Weapon {
	friend class Weapons;
public:
	/**< Flags affecting weapon's behavior. @see Weapon::flags */
	enum Flags {
		WEAP_LOSE                   = 0x0001,   /**< lost when used */
		WEAP_LOSEWHENRANGED         = 0x0002,   /**< lost when used for ranged attack */
		WEAP_CHOOSEDISTANCE         = 0x0004,   /**< allows player to choose attack distance */
		WEAP_ALWAYSHITS             = 0x0008,   /**< always hits it's target */
		WEAP_MAGIC                  = 0x0010,   /**< is magical */
		WEAP_ATTACKTHROUGHOBJECTS   = 0x0040,   /**< can attack through solid objects */
		WEAP_ABSOLUTERANGE          = 0x0080,   /**< range is absolute (only works at specific distance) */
		WEAP_RETURNS                = 0x0100,   /**< returns to user after used/thrown */
		WEAP_DONTSHOWTRAVEL         = 0x0200    /**< do not show animations when attacking */
	};

public:
	WeaponType getType() const {
		return _type;
	}
	const Common::String &getName() const {
		return _name;
	}
	const Common::String &getAbbrev() const {
		return _abbr;
	}
	bool canReady(ClassType klass) const {
		return (_canUse & (1 << klass)) != 0;
	}
	int getRange() const {
		return _range;
	}
	int getDamage() const {
		return _damage;
	}
	const Common::String &getHitTile() const {
		return _hitTile;
	}
	const Common::String &getMissTile() const {
		return _missTile;
	}
	const Common::String &leavesTile() const {
		return _leaveTile;
	}
	unsigned short getFlags() const {
		return _flags;
	}

	bool loseWhenUsed() const {
		return _flags & WEAP_LOSE;
	}
	bool loseWhenRanged() const {
		return _flags & WEAP_LOSEWHENRANGED;
	}
	bool canChooseDistance() const {
		return _flags & WEAP_CHOOSEDISTANCE;
	}
	bool alwaysHits() const {
		return _flags & WEAP_ALWAYSHITS;
	}
	bool isMagic() const {
		return _flags & WEAP_MAGIC;
	}
	bool canAttackThroughObjects() const {
		return _flags & WEAP_ATTACKTHROUGHOBJECTS;
	}
	bool rangeAbsolute() const {
		return _flags & WEAP_ABSOLUTERANGE;
	}
	bool returns() const {
		return _flags & WEAP_RETURNS;
	}
	bool showTravel() const {
		return !(_flags & WEAP_DONTSHOWTRAVEL);
	}

private:
	Weapon(WeaponType weaponType, const ConfigElement &conf);

	WeaponType _type;
	Common::String _name;
	Common::String _abbr;            /**< abbreviation for the weapon */
	byte _canUse;             /**< bitmask of classes that can use weapon */
	int _range;              /**< range of weapon */
	int _damage;             /**< damage of weapon */
	Common::String _hitTile;         /**< tile to display a hit */
	Common::String _missTile;        /**< tile to display a miss */
	Common::String _leaveTile;       /**< if the weapon leaves a tile, the tile #, zero otherwise */
	unsigned short _flags;
};

class Weapons : public Common::Array<Weapon *> {
private:
	bool _confLoaded;

	void loadConf();
public:
	/**
	 * Constructor
	 */
	Weapons();

	/**
	 * Destructor
	 */
	~Weapons();

	/**
	 * Returns weapon by WeaponType.
	 */
	const Weapon *get(WeaponType w);

	/**
	 * Returns weapon that has the given name
	 */
	const Weapon *get(const Common::String &name);
};

extern Weapons *g_weapons;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
