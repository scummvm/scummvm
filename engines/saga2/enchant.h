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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ENCHANT_H
#define SAGA2_ENCHANT_H

namespace Saga2 {

//  Enchantments for actors
enum actorEnchantments {

	//  Object-enchantments
	kEnchNone = 0,
	kEnchInvisible,

	kEnchWarded,

	//  Actor skill boosters
	kEnchAttackFast,
	kEnchEnhanceBrawn,
	kEnchEnhanceAgility,
	kEnchEnhanceStealth,
	kEnchEnhanceArmor,
	kEnchEnhanceHitChance,

	//  Actor Immunities
	kEnchImmunePhysical,
	kEnchImmuneProjectile,
	kEnchImmuneHandToHand,
	kEnchImmuneMagicMissile,
	kEnchImmuneFire,
	kEnchImmuneFireMagic,
	kEnchImmuneLava,
	kEnchImmuneCold,
	kEnchImmuneMental,
	kEnchImmuneDirectMagic,
	kEnchImmuneLifeDrain,

	//  Actor Movement enchantents
	kEnchLandWalking,
	kEnchDesolidified,
	kEnchFloating,
	kEnchFallSlowly,
	kEnchLevitating,
	kEnchWaterWalking,
	kEnchFlying,

	//  Curses
	kEnchBlind,
	kEnchPanic,
	kEnchParalyzed,
	kEnchMoveFast,
	kEnchMoveSlow,
	kEnchAttackSlow,
	kEnchAsleep,
	kEnchReduceHitCchance,

	//  Changes display algorithms
	kEnchSoulSight,
	kEnchClairvoyant,
	kEnchDetectPoison,

	//  Changes NPC behavior
	kEnchHasNoSmell
};

//-----------------------------------------------------------------------
//	Iterates through all active enchantments on an object or actor

class EnchantmentIterator {
	ObjectID        _nextID;             //  Pointer to ID of next object.

public:
	GameObject      *_baseObject,        //  Base obj we're searching for enchantments
	                *_wornObject;        //  An object 'worn' by the base object.

	//  Constructor
	EnchantmentIterator(GameObject *container);

	//  Iteration function
	ObjectID        first(GameObject **obj);
	ObjectID        next(GameObject **obj);
};

//-----------------------------------------------------------------------
// Global Enchantments

enum worldEnchantments {
	kWETimeStop = 0,
	kWECount
};

} // end of namespace Saga2

#endif
