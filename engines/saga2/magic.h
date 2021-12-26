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

#ifndef SAGA2_MAGIC_H
#define SAGA2_MAGIC_H

#include "saga2/spells.h"

namespace Saga2 {

class ActiveItem;
class Location;
class SkillProto;
class GameObject;

// ------------------------------------------------------------------
// This is an include file for the spellcasting interface. This file
// is meant to publish the main API functions for spell casting.
// The internal workings of the spell stuff is defined elsewhere:
//    SPELLS.H    has spell ID numbers
//    EFFECTS.H   what spells are capable of doing to things
//    SPELSHOW.H  is the portion of the interface needed by DISPNODE
//    SPELLBUK.H  defines the global spell list
//


// ------------------------------------------------------------------
//  Prototypes for spell / skill use

// see if the mouse pointer is pointing at a viable target for a
//   selected spell
bool validTarget(GameObject *enactor,
                 GameObject *target,
                 ActiveItem *tag,
                 SkillProto *skill);

// test for untargeted skill / spell that gets cast immediately
bool nonTargeted(SkillProto *spell);

// test for skill / spell that cannot be used
bool nonUsable(SkillProto *spell);

// This call looks up a spells object prototype. It can accept either
//   an object ID or a spell ID
SkillProto *skillProtoFromID(int16 spellOrObjectID);

// Initialization call to map the Spell Object prototypes
//   into the main spell database
void initializeSkill(SkillProto *oNo, SpellID sNo);

//
//    Cast: This call makes the actor do his spell casting dance. The
//      motion task calls the implement routine at the appropriate
//      time. If the caster is not an actor the implement routine is
//      chained directly.
//
//    Implement: This call actually causes the spell to 'fire'. The
//      display effect is started, and the internal effects are
//      implemented.
//

// check for available mana
bool canCast(GameObject *enactor, SkillProto *spell);

// cast untargeted spell
bool castUntargetedSpell(GameObject *enactor, SkillProto *spell);

// global cast spell routines at assorted targets
bool castSpell(GameObject *enactor, Location   &target, SkillProto *spell);
bool castSpell(GameObject *enactor, ActiveItem *target, SkillProto *spell);
bool castSpell(GameObject *enactor, GameObject *target, SkillProto *spell);

// global cast spell routines at assorted targets
bool implementSpell(GameObject *enactor, Location   &target, SkillProto *spell);
bool implementSpell(GameObject *enactor, ActiveItem *target, SkillProto *spell);
bool implementSpell(GameObject *enactor, GameObject *target, SkillProto *spell);

// spell saving & loading
void initSpellState();
void saveSpellState(Common::OutSaveFile *outS);
void loadSpellState(Common::InSaveFile *in);
void cleanupSpellState();

} // end of namespace Saga2

#endif
