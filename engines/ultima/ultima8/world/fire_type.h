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

#ifndef ULTIMA8_WORLD_FIRETYPE_H
#define ULTIMA8_WORLD_FIRETYPE_H

namespace Ultima {
namespace Ultima8 {

class Item;
struct Point3;

/**
 * A structure to hold data about the fire that comes from various weapons
 */
class FireType {
public:
	FireType(uint16 typeNo, uint16 minDamage, uint16 maxDamage, uint8 range,
			 uint8 numShots, uint16 shieldCost, uint8 shieldMask, uint8 accurate,
			 uint16 cellsPerRound, uint16 roundDuration, uint8 nearSprite);

	uint16 getTypeNo() const {
		return _typeNo;
	}

	uint16 getMinDamage() const {
		return _minDamage;
	}

	uint16 getMaxDamage() const {
		return _maxDamage;
	}

	uint8 getRange() const {
		return _range;
	}

	uint8 getNumShots() const {
		return _numShots;
	}

	uint16 getShieldCost() const {
		return _shieldCost;
	}

	uint8 getShieldMask() const {
		return _shieldMask;
	}

	uint8 getAccurate() const {
		return _accurate;
	}

	uint16 getCellsPerRound() const {
		return _cellsPerRound;
	}

	uint16 getRoundDuration() const {
		return _roundDuration;
	}

	uint8 getNearSprite() const {
		return _nearSprite;
	}

	uint16 getRandomDamage() const;

	void applySplashDamageAround(const Point3 &pt, int damage, const Item *exclude, const Item *src) const;

	void makeBulletSplashShapeAndPlaySound(int32 x, int32 y, int32 z) const;

private:
	uint16 _typeNo;
	uint16 _minDamage;
	uint16 _maxDamage;
	uint8 _range;
	uint8 _numShots;
	uint16 _shieldCost;
	uint8 _shieldMask;
	uint8 _accurate;
	uint16 _cellsPerRound;
	uint16 _roundDuration;
	uint8 _nearSprite;
};

}
}

#endif
