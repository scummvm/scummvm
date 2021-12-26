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

#ifndef SAGA2_SPELLIO_H
#define SAGA2_SPELLIO_H

namespace Saga2 {

class SpellTarget;
class Effectron;

class EffectDisplayPrototype;
class EffectDisplayPrototypeList;

class SpellDisplayPrototype;
class SpellDisplayPrototypeList;

class SpellStuff;

class SpellInstance;
class SpellDisplayList;

struct ResourceSpellItem ;
struct ResourceSpellEffect;
struct ResourceItemEffect;

struct StorageSpellTarget;
struct StorageSpellInstance;
struct StorageEffectron;

//-------------------------------------------------------------------
// ResourceSpellItem
//   Spell configuration info is loaded from the resfile in these

struct ResourceSpellItem {
	int16   spell;                  // Spell ID
	int16   effect;                 // Effect ID
	int16   effectronElasticity;    // Effectron elasticity
	int16   cm1;             // effectron count
	int16   maxAge;                 // max age
	int16   implAge;                // implementation age
	int16   baseSprite;             // sprite ID
	int16   spriteCount;            // sprite count
	int16   baseSprite2;             // sprite ID
	int16   spriteCount2;            // sprite count
	int16   cm0;                    // color map b0
	int16   soundID;                    // color map b1
	int16   targs;                  // targeting bits
	int16   applys;                 // apply bits
	int16   manaType;               // mana ID
	int16   manaAmount;             // mana count
};

//-------------------------------------------------------------------
// ResourceSpellEffect
//   Since spells can have multiple effects, they are stored
//   separately

struct ResourceSpellEffect {
	int16 spell;                // spell ID
	int16 effectGroup;                // effect group
	int16 effectType;                // effect ID
	int16 targeting;                // targeting
	int16 baseDice;                // base dice
	int16 skillDice;                // skill dice
	int16 attribModifier;                // attrib change
	int16 flagSet;                // Set/clear
	int16 enchTimeLo;                // min enchant
	int16 enchTimeHi;                // max enchant
	int16 diceSides;
};

//-------------------------------------------------------------------
// Similar struct for weapon effects

struct ResourceItemEffect {
	int16 item;                // spell ID
	int16 effectGroup;                // effect group
	int16 effectType;                // effect ID
	int16 targeting;                // targeting
	int16 baseDice;                // base dice
	int16 skillDice;                // skill dice
	int16 baseDamage;                // attrib change
	int16 skillDamage;                // attrib change
	int16 reserved0;                // min enchant
	int16 reserved1;                // min enchant
	int16 diceSides;
};

//-------------------------------------------------------------------
// Serialized spell target

struct StorageSpellTarget {
	int16           type;

	TilePoint       loc;
	ObjectID        obj;
	ActiveItemID    tag;

	StorageSpellTarget();
	StorageSpellTarget(SpellTarget &st);

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);
};

//-------------------------------------------------------------------
// Serialized spell instance

struct StorageSpellInstance {
	int32                   implementAge;   // age at which to implement the spell effects
	uint16                  effect;        // effect prototype of the current effect
	SpellID                 dProto;        // effect prototype of the current effect
	ObjectID                caster;

	StorageSpellTarget      target;
	ObjectID                world;
	int32                   age;
	SpellID                 spell;
	int32                   maxAge;
	int16                   effSeq;         // which effect in a sequence is being played
	int16                   eListSize;

	StorageSpellInstance();
	StorageSpellInstance(SpellInstance &si);

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);
};

//-------------------------------------------------------------------
// Serialized spell sprite

struct StorageEffectron {
	uint32                  flags;
	Extent16                size;
	Rect16                  hitBox;           // hitbox for clicking this item

	int16                   partno;           // Which effectron in a group this represents
	Point16                 screenCoords;     // screen coordinates last drawn at

	TilePoint               start,            // travelling from
	                        finish,           // travelling to
	                        current,          // current position
	                        velocity,         // current velocity
	                        acceleration;     // current acceleration
	uint16                  totalSteps,       // discrete jumps in the path
	                        stepNo;           // current jump

	int16                   hgt;              // collision detection stuff
	int16                   brd;

	int32                   pos;              // These three are part of an old way of
	int32                   spr;              // updating effectrons
	int32                   age;

	StorageEffectron();
	StorageEffectron(Effectron &e);

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);
};

} // end of namespace Saga2

#endif
