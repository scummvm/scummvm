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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/magic.h"
#include "saga2/effects.h"
#include "saga2/spellbuk.h"
#include "saga2/tcoords.h"
#include "saga2/rect.h"
#include "saga2/spellio.h"
#include "saga2/spelshow.h"
#include "saga2/savefile.h"

namespace Saga2 {

const ChunkID   spellInstCountID = MakeID('S', 'P', 'E', 'L');

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern SpellDisplayList         activeSpells;

/* ===================================================================== *
   partial SpellDisplayPrototype implementation
 * ===================================================================== */

// ------------------------------------------------------------------
// ctor

SpellDisplayPrototype::SpellDisplayPrototype(ResourceSpellItem *rsi) {
	effect = rsi->effect;                // Effect ID
	effParm1 = 0;                        //   effect setting 1
	effParm2 = 0;                        //   effect setting 1
	effParm3 = 0;                        //   effect setting 1
	effParm4 = 0;                        //   effect setting 1
	scatter = diFlagZero;                // direction init mode
	elasticity = (effectCollisionCont) rsi->effectronElasticity; // collision flags
	maxAge = rsi->maxAge;                // auto self-destruct age
	implementAge = rsi->implAge;         // auto self-destruct age
	primarySpriteID = rsi->baseSprite; // RES_ID(x, y, z, 0) to get sprites
	primarySpriteNo = rsi->spriteCount; // sprites available
	secondarySpriteID = rsi->baseSprite2; // RES_ID(x, y, z, 0) to get sprites
	secondarySpriteNo = rsi->spriteCount2; // sprites available
	//effCount=0;            // effectrons to allocate
	colorMap[0] = rsi->cm0;
	colorMap[1] = rsi->cm1;
	colorMap[2] = 0;
	colorMap[3] = 0;
}

/* ===================================================================== *
   partial SpellStuff implementation
 * ===================================================================== */

// ------------------------------------------------------------------
// init from res file

void SpellStuff::setupFromResource(ResourceSpellItem *rsi) {
	master = (SpellID) rsi->spell;
	display = (SpellID) rsi->spell;
	targetableTypes = (SpellTargetingTypes) rsi->targs;
	targetTypes = (SpellApplicationTypes) rsi->applys;
	manaType = (SpellManaID) rsi->manaType;
	manaUse = rsi->manaAmount;
	shape = (effectAreas) rsi->effect;
	size = 0;
	sound = rsi->soundID;
}

// ------------------------------------------------------------------
// add spell internal effect

void SpellStuff::addEffect(ResourceSpellEffect *rse) {
	ProtoEffect *pe = nullptr;
	assert(rse && rse->spell == master);
	switch (rse->effectGroup) {
	case effectNone     :
		return;
	case effectAttrib   :
		pe = new ProtoEnchantment(
		         makeEnchantmentID(
		             rse->effectGroup,
		             rse->effectType,
		             rse->attribModifier),
		         rse->enchTimeLo,
		         rse->enchTimeHi);
		break;
	case effectResist   :
	case effectImmune   :
	case effectOthers   :
	case effectNonActor :
		pe = new ProtoEnchantment(
		         makeEnchantmentID(
		             rse->effectGroup,
		             rse->effectType,
		             rse->flagSet),
		         rse->enchTimeLo,
		         rse->enchTimeHi);
		break;
	case effectDamage   :
		pe = new ProtoDamage(
		         rse->baseDice,
		         rse->diceSides ? rse->diceSides : 6,
		         rse->skillDice,
		         0,
		         (effectDamageTypes) rse->effectType,
		         0,
		         rse->targeting & spellTargCaster);
		break;
	case effectDrains   :
		pe = new ProtoDrainage(
		         rse->baseDice,
		         rse->diceSides ? rse->diceSides : 6,
		         rse->skillDice,
		         0,
		         (effectDrainsTypes) rse->effectType,
		         0,
		         rse->targeting & spellTargCaster);
		break;
	case effectTAG      :
		pe = new ProtoTAGEffect(
		         (effectTAGTypes) rse->effectType,
		         rse->flagSet,
		         rse->attribModifier);
		break;
	case effectLocation :
		pe = new ProtoLocationEffect(
		         (effectLocationTypes) rse->effectType,
		         rse->attribModifier);
		break;
	case effectSpecial  : {
		switch (rse->effectType) {
		case    specialDispellHelpfulEnch :   // clears helpful enchantments
			pe = new ProtoSpecialEffect(DispellProtections, rse->attribModifier);
			break;
		case    specialDispellHarmfulEnch :   // clears harmful enchantments
			pe = new ProtoSpecialEffect(DispellCurses,      rse->attribModifier);
			break;
		case    specialKill               :   // death spell
			pe = new ProtoSpecialEffect(DeathSpell,         rse->attribModifier);
			break;
		case    specialRessurect          :   // raise dead spell
			pe = new ProtoSpecialEffect(Resurrect,          rse->attribModifier);
			break;
		case    specialTeleport           :   // Teleportation
			pe = new ProtoSpecialEffect(TeleportToLocation, rse->attribModifier);
			break;
		case    specialCreateActor        :   // Create an actor or wall
			pe = new ProtoSpecialEffect(CreateWraith,       rse->attribModifier);
			break;
		case    specialSagaFunc           :    // calls a saga function
			pe = new ProtoSpecialEffect(SagaSpellCall,      rse->attribModifier);
			break;
		case    specialRejoin        :   // Create an actor or wall
			pe = new ProtoSpecialEffect(Rejoin,      rse->attribModifier);
			break;
		case    specialCreateWWisp  :  // calls a saga function
			pe = new ProtoSpecialEffect(CreateWWisp,      rse->attribModifier);
			break;
		case    specialCreateFWisp  :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateFWisp,      rse->attribModifier);
			break;
		case    specialCreateWraith :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateWraith,      rse->attribModifier);
			break;
		case    specialCreateFood   :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateFood,      rse->attribModifier);
			break;
		}
		break;
	}
	}
	if (pe == nullptr)
		error("failed to alloc protoEffect");

	if (effects == nullptr)
		effects = pe;
	else {
		ProtoEffect *tail;
		for (tail = effects; tail->next; tail = tail->next) ;
		tail->next = pe;
	}
}

// ------------------------------------------------------------------
// init spells

void initSpellState(void) {
}

// ------------------------------------------------------------------
// serialize active spells

void saveSpellState(SaveFileConstructor &saveGame) {
	activeSpells.save(saveGame);
}

// ------------------------------------------------------------------
// read serialized active spells

void loadSpellState(SaveFileReader &saveGame) {
	activeSpells.load(saveGame);
}

// ------------------------------------------------------------------
// cleanup active spells

void cleanupSpellState(void) {
	activeSpells.wipe();
}

// ------------------------------------------------------------------
// cleanup active spells

StorageSpellTarget::StorageSpellTarget(SpellTarget &st) {
	GameObject *go = NULL;
	ActiveItem *ai;
	type = st.getType();
	loc = st.getPoint();
	if (type == SpellTarget::spellTargetObject) {
		if (type == SpellTarget::spellTargetObject)
			go = st.getObject();
		else
			go = NULL;
	}

	if (go)
		obj = go->thisID();
	else
		obj = Nothing;

	if (type == SpellTarget::spellTargetTAG)
		ai = st.getTAG();
	else
		ai = NULL;

	if (ai)
		tag = ai->thisID();
	else
		tag = NoActiveItem;
}

StorageSpellInstance::StorageSpellInstance(SpellInstance &si) {
	implementAge = si.implementAge; // age at which to implement the spell effects
	effect = si.effect->thisID();      // effect prototype of the current effect
	dProto = si.dProto->thisID();      // effect prototype of the current effect
	caster = si.caster->thisID();
	target = StorageSpellTarget(*si.target);
	world = si.world->thisID();
	age = si.age;
	spell = si.spell;
	maxAge = si.maxAge;
	effSeq = si.effSeq;         // which effect in a sequence is being played
	eListSize = si.eList.count;
}

StorageSpellTarget::StorageSpellTarget() {
	type = 0;
	loc = Nowhere;
	obj = Nothing;
	tag = NoActiveItem;
}

StorageSpellInstance::StorageSpellInstance() {
}

SpellTarget::SpellTarget(StorageSpellTarget &sst) {
	type = (SpellTarget::spellTargetType) sst.type;
	loc = sst.loc;
	next = NULL;
	if (sst.obj != Nothing)
		obj = GameObject::objectAddress(sst.obj);
	else
		obj = NULL;
	if (sst.tag != NoActiveItem)
		tag = ActiveItem::activeItemAddress(sst.tag);
	else
		tag = NULL;
}

SpellInstance::SpellInstance(StorageSpellInstance &ssi) {
	implementAge = ssi.implementAge; // age at which to implement the spell effects
	dProto = SpellDisplayPrototypeList::sdpList[ssi.dProto];
	caster = GameObject::objectAddress(ssi.caster);
	target = new SpellTarget(ssi.target);
	GameObject *go = GameObject::objectAddress(ssi.world);
	assert(isWorld(go));
	world = (GameWorld *) go;
	age = ssi.age;
	spell = ssi.spell;
	maxAge = ssi.maxAge;
#if 0
	effect = EffectDisplayPrototypeList::edpList[ssi.effect];
	effSeq = ssi.effSeq;        // which effect in a sequence is being played
#else
	effSeq = 0;
	effect = EffectDisplayPrototypeList::edpList[ssi.effect];
	while (effSeq < ssi.effSeq)         // which effect in a sequence is being played
		effect = effect->next;
#endif
}

size_t SpellDisplayList::saveSize(void) {
	size_t total = 0;

	total += sizeof(count);
	if (count) {
		for (int i = 0; i < count; i++)
			total += spells[i]->saveSize();
	}
	return total;
}

void SpellDisplayList::save(SaveFileConstructor &saveGame) {
	size_t chunkSize = saveSize();

	saveGame.newChunk(spellInstCountID, chunkSize);

	saveGame.write(&count, sizeof(count));
	if (count) {
		for (int i = 0; i < count; i++) {
			StorageSpellInstance ssi = StorageSpellInstance(*spells[i]);
			saveGame.write(&ssi, sizeof(ssi));
			spells[i]->saveEffect(saveGame);
		}
	}
}

void SpellDisplayList::load(SaveFileReader &saveGame) {
	uint16 tCount;

	saveGame.read(&tCount, sizeof(tCount));
	assert(tCount < maxCount);
	if (tCount) {
		for (int i = 0; i < tCount; i++) {
			SpellInstance *si;
			StorageSpellInstance ssi;
			saveGame.read(&ssi, sizeof(ssi));
			si = new SpellInstance(ssi);
			add(si);
			si->loadEffect(saveGame, ssi.eListSize);
		}
	}
	assert(tCount == count);
}

void SpellDisplayList::wipe(void) {
	for (int i = 0; i < maxCount; i++)
		if (spells[i]) {
			delete spells[i];
			spells[i] = NULL;
			count--;
		}

	assert(count == 0);
}

size_t SpellInstance::saveSize(void) {
	size_t total = 0;
	total += sizeof(StorageSpellInstance);
	if (eList.count)
		for (int32 i = 0; i < eList.count; i++) {
			total += sizeof(StorageEffectron);
		}
	return total;
}

void SpellInstance::saveEffect(SaveFileConstructor &saveGame) {
	if (eList.count > 0 && !(maxAge > 0 && (age + 1) > maxAge))
		for (int32 i = 0; i < eList.count; i++) {
			StorageEffectron se = StorageEffectron(*eList.displayList[i].efx);
			saveGame.write(&se, sizeof(se));
		}
}

void SpellInstance::loadEffect(SaveFileReader &saveGame, uint16 eListSize) {
	assert(eListSize == effect->nodeCount);
	eList.count = effect->nodeCount; //sdp->effCount;
	if (eList.count)
		for (int32 i = 0; i < eList.count; i++) {
			StorageEffectron se;
			saveGame.read(&se, sizeof(se));
			Effectron *e = new Effectron(se, this);
			eList.displayList[i].efx = e;
		}
}

StorageEffectron::StorageEffectron() {
	flags = 0;
	size = Extent16(0, 0);
	hitBox = Rect16(0, 0, 0, 0);
	screenCoords = Point16(0, 0);
	partno = 0;
	start = Nowhere;
	finish = Nowhere;
	current = Nowhere;
	velocity = TilePoint(0, 0, 0);
	acceleration = TilePoint(0, 0, 0);
	totalSteps = 0;
	stepNo = 0;
	hgt = 0;
	brd = 0;
	pos = 0;
	spr = 0;
	age = 0;
}

StorageEffectron::StorageEffectron(Effectron &e) {
	flags =         e.flags;
	size =          e.size;
	hitBox =        e.hitBox;
	screenCoords =  e.screenCoords;
	partno =        e.partno;
	start =         e.start;
	finish =        e.finish;
	current =       e.current;
	velocity =      e.velocity;
	acceleration =  e.acceleration;
	totalSteps =    e.totalSteps;
	stepNo =        e.stepNo;
	hgt =           e.hgt;
	brd =           e.brd;
	pos =           e.pos;
	spr =           e.spr;
	age =           e.age;
}


Effectron::Effectron(StorageEffectron &se, SpellInstance *si) {
	flags =         se.flags;
	size =          se.size;
	hitBox =        se.hitBox;
	screenCoords =  se.screenCoords;
	partno =        se.partno;
	start =         se.start;
	finish =        se.finish;
	current =       se.current;
	velocity =      se.velocity;
	acceleration =  se.acceleration;
	totalSteps =    se.totalSteps;
	stepNo =        se.stepNo;
	hgt =           se.hgt;
	brd =           se.brd;
	pos =           se.pos;
	spr =           se.spr;
	age =           se.age;
	parent = si;
}

} // end of namespace Saga2
