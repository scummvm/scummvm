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
	senseNothing = 0,                   // no stimuli

	//  Situations
	senseTimePassed,                    // time passes with no occurance
	senseProximity,                     // sense proximity of protagonist

	//  Idea icons from protagonist
	senseIdeaGreeting,                  // greeting from protagonist
	senseIdeaHere,                      // talk about "here"
	senseIdeaWork,                      // talk about work
	senseIdeaFood,                      // talk about food
	senseIdeaDrink,
	senseIdeaGold,
	senseIdeaJewelry,
	senseIdeaWeapons,
	senseIdeaArmor,
	senseIdeaContainer,
	senseIdeaLeader,
	senseIdeaCrime,
	senseIdeaSelf,
	senseIdeaYou,
	senseIdeaKeys,
	senseIdeaDocument,
	senseIdeaPriest,
	senseIdeaMagicSpell,
	senseIdeaMagicItem,
	senseIdeaPotion,
	senseIdeaShip,
	senseIdeaHouse,
	senseIdeaShop,
	senseIdeaCastle,
	senseIdeaSpirit,

	//  Protagonist giving physical objects
	senseGiveWealth,                        // protag gave us gold
	senseGiveFood,                          // protag gave us food
	senseGiveIntoxicant,                    // protag gave us booze
	senseGiveWeapon,                        // protag gave us weapon
	senseGiveDefense,                       // protag gave us armor
	senseGiveMagic,                         // protag gave us magic
	senseGiveEnigmatic,                     // protag gave us strangeness

	//  Actions witnessed
	senseActionAttack,                      // protag attacked us
	senseActionAttackFaction,               // protag attacked friend
	senseActionTheft,                       // protag stole from us
	senseActionVandalism,                   // protag destroyed property
	senseActionSpellcast,                   // protag cast spell
	senseActionEnemy,                       // some foe came into sight

	//  Actions that we remember doing ourself
	senseDidAttack,                         // we attacked protag
	senseDidRun,                            // we raw away from protag
	senseDidGive,                           // we gave something to protag

	//  Number of sensory types
	senseCount

};

} // end of namespace Saga2

#endif
