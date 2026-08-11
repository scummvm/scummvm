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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_STIMTYPE_H
#define SAGA2_STIMTYPE_H

namespace Saga2 {

//  A list of stimuli types, used for both SAGA and C code

enum stimuliTypes {
	kSenseNothing = 0,                   // no stimuli

	//  Situations
	kSenseTimePassed,                    // time passes with no occurrence
	kSenseProximity,                     // sense proximity of protagonist

	//  Idea icons from protagonist
	kSenseIdeaGreeting,                  // greeting from protagonist
	kSenseIdeaHere,                      // talk about "here"
	kSenseIdeaWork,                      // talk about work
	kSenseIdeaFood,                      // talk about food
	kSenseIdeaDrink,
	kSenseIdeaGold,
	kSenseIdeaJewelry,
	kSenseIdeaWeapons,
	kSenseIdeaArmor,
	kSenseIdeaContainer,
	kSenseIdeaLeader,
	kSenseIdeaCrime,
	kSenseIdeaSelf,
	kSenseIdeaYou,
	kSenseIdeaKeys,
	kSenseIdeaDocument,
	kSenseIdeaPriest,
	kSenseIdeaMagicSpell,
	kSenseIdeaMagicItem,
	kSenseIdeaPotion,
	kSenseIdeaShip,
	kSenseIdeaHouse,
	kSenseIdeaShop,
	kSenseIdeaCastle,
	kSenseIdeaSpirit,

	//  Protagonist giving physical objects
	kSenseGiveWealth,                        // protag gave us gold
	kSenseGiveFood,                          // protag gave us food
	kSenseGiveIntoxicant,                    // protag gave us booze
	kSenseGiveWeapon,                        // protag gave us weapon
	kSenseGiveDefense,                       // protag gave us armor
	kSenseGiveMagic,                         // protag gave us magic
	kSenseGiveEnigmatic,                     // protag gave us strangeness

	//  Actions witnessed
	kSenseActionAttack,                      // protag attacked us
	kSenseActionAttackFaction,               // protag attacked friend
	kSenseActionTheft,                       // protag stole from us
	kSenseActionVandalism,                   // protag destroyed property
	kSenseActionSpellcast,                   // protag cast spell
	kSenseActionEnemy,                       // some foe came into sight

	//  Actions that we remember doing ourself
	kSenseDidAttack,                         // we attacked protag
	kSenseDidRun,                            // we raw away from protag
	kSenseDidGive,                           // we gave something to protag

	//  Number of sensory types
	kSenseCount
};

} // end of namespace Saga2

#endif
