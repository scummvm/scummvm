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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef WAGE_CHR_H
#define WAGE_CHR_H

#include "wage/context.h"
 
namespace Wage {

class Chr : public Designed {
public:
	enum ChrDestination {
		RETURN_TO_STORAGE = 0,
		RETURN_TO_RANDOM_SCENE = 1,
		RETURN_TO_INITIAL_SCENE = 2
	};

	enum ChrPart {
		HEAD = 0,
		CHEST = 1,
		SIDE = 2
	};

	enum ChrArmorType {
		HEAD_ARMOR = 0,
		BODY_ARMOR = 1,
		SHIELD_ARMOR = 2
	};
	
	Chr(String name, byte *data) {}

	int _index;
	String _initialScene;
	int _gender;
	bool _nameProperNoun;
	bool _playerCharacter;
	int _maximumCarriedObjects;
	int _returnTo;
	
	int _physicalStrength;
	int _physicalHp;
	int _naturalArmor;
	int _physicalAccuracy;
	int _spiritualStength;
	int _spiritialHp;
	int _resistanceToMagic;
	int _spiritualAccuracy;
	int _runningSpeed;
	int _rejectsOffers;
	int _followsOpponent;
	
	String _initialSound;
	String _scoresHitSound;
	String _receivesHitSound;
	String _dyingSound;

	String _nativeWeapon1;
	String _operativeVerb1;
	int _weaponDamage1;
	String _weaponSound1;
	
	String _nativeWeapon2;
	String _operativeVerb2;
	int _weaponDamage2;
	String _weaponSound2;
	
	int _winningWeapons;
	int _winningMagic;
	int _winningRun;
	int _winningOffer;
	int _losingWeapons;
	int _losingMagic;
	int _losingRun;
	int _losingOffer;
	
	String _initialComment;
	String _scoresHitComment;
	String _receivesHitComment;
	String _makesOfferComment;
	String _rejectsOfferComment;
	String _acceptsOfferComment;
	String _dyingWords;

	Scene _currentScene;
	Common::List<Obj> _inventory;

	Obj _armor[3];

	Context _context;

public:
#if 0
	Weapon[] getWeapons() {
		ArrayList<Weapon> weapons = new ArrayList<Weapon>();
		if (hasNativeWeapon1()) {
			weapons.add(new Weapon() {
				String getName() {
					return _getNativeWeapon1();
				}
				String getOperativeVerb() {
					return _getOperativeVerb1();
				}
				int getType() {
					return _Obj.REGULAR_WEAPON;
				}
				int getAccuracy() {
					return _0;
				}
				int getDamage() {
					return _getWeaponDamage1();
				}
				String getSound() {
					return _getWeaponSound1();
				}
				void decrementNumberOfUses() {}
			});
		}
		if (hasNativeWeapon2()) {
			weapons.add(new Weapon() {
				String getName() {
					return _getNativeWeapon2();
				}
				String getOperativeVerb() {
					return _getOperativeVerb2();
				}
				int getType() {
					return _Obj.REGULAR_WEAPON;
				}
				int getAccuracy() {
					return _0;
				}
				int getDamage() {
					return _getWeaponDamage2();
				}
				String getSound() {
					return _getWeaponSound2();
				}
				void decrementNumberOfUses() {}
			});
		}
		for (Obj o : getInventory()) {
			switch (o.getType()) {
				case Obj.REGULAR_WEAPON:
				case Obj.THROW_WEAPON:
				case Obj.MAGICAL_OBJECT:
					weapons.add(o);
			}
		}
		return _(Weapon[]) weapons.toArray(new Weapon[0]);
	}
#endif

	bool hasNativeWeapon1() {
		return (_nativeWeapon1.size() > 0 && _operativeVerb1.size() > 0);
	}

	bool hasNativeWeapon2() {
		return (_nativeWeapon2.size() > 0 && _operativeVerb2.size() > 0);
	}
};

} // End of namespace Wage
 
#endif
