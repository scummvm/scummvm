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

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/magic.h"
#include "saga2/effects.h"
#include "saga2/spellbuk.h"
#include "saga2/tcoords.h"
#include "saga2/rect.h"
#include "saga2/spellio.h"
#include "saga2/spelshow.h"
#include "saga2/saveload.h"

namespace Saga2 {

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
	ID = spellNone;
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

void initSpellState() {
}

void saveSpellState(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving SpellState");
	g_vm->_activeSpells->write(outS);
}

void loadSpellState(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading SpellState");

	g_vm->_activeSpells->read(in);
}

// ------------------------------------------------------------------
// cleanup active spells

void cleanupSpellState() {
	g_vm->_activeSpells->wipe();
}

// ------------------------------------------------------------------
// cleanup active spells

StorageSpellTarget::StorageSpellTarget(SpellTarget &st) {
	GameObject *go = nullptr;
	ActiveItem *ai;
	type = st.getType();
	loc = st.getPoint();
	if (type == SpellTarget::spellTargetObject) {
		if (type == SpellTarget::spellTargetObject)
			go = st.getObject();
		else
			go = nullptr;
	}

	if (go)
		obj = go->thisID();
	else
		obj = Nothing;

	if (type == SpellTarget::spellTargetTAG)
		ai = st.getTAG();
	else
		ai = nullptr;

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

void StorageSpellTarget::read(Common::InSaveFile *in) {
	ActiveItemID tagID;

	type = in->readSint16LE();
	loc.load(in);
	obj = in->readUint16LE();
	tagID = in->readSint16LE();
	tag = ActiveItemID(tagID);
}

void StorageSpellTarget::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeSint16LE(type);
	loc.write(out);
	out->writeUint16LE(obj);
	out->writeSint16LE(tag.val);
}

StorageSpellInstance::StorageSpellInstance() : implementAge(0), effect(0), dProto(spellNone), caster(0),
	world(0), age(0), spell(spellNone), maxAge(0), effSeq(0), eListSize(0) {
}

void StorageSpellInstance::read(Common::InSaveFile *in) {
	implementAge = in->readSint32LE();
	effect = in->readUint16LE();
	dProto = (SpellID)in->readByte();
	caster = in->readUint16LE();
	target.read(in);
	world = in->readUint16LE();
	age = in->readSint32LE();
	spell = (SpellID)in->readByte();
	maxAge = in->readSint32LE();
	effSeq = in->readSint16LE();
	eListSize = in->readSint16LE();
}

void StorageSpellInstance::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeSint32LE(implementAge);
	out->writeUint16LE(effect);
	out->writeByte(dProto);
	out->writeUint16LE(caster);
	target.write(out);
	out->writeUint16LE(world);
	out->writeSint32LE(age);
	out->writeByte(spell);
	out->writeSint32LE(maxAge);
	out->writeSint16LE(effSeq);
	out->writeSint16LE(eListSize);
}

SpellTarget::SpellTarget(StorageSpellTarget &sst) {
	type = (SpellTarget::spellTargetType) sst.type;
	loc = sst.loc;
	next = nullptr;
	if (sst.obj != Nothing)
		obj = GameObject::objectAddress(sst.obj);
	else
		obj = nullptr;
	if (sst.tag != NoActiveItem)
		tag = ActiveItem::activeItemAddress(sst.tag);
	else
		tag = nullptr;
}

SpellInstance::SpellInstance(StorageSpellInstance &ssi) {
	implementAge = ssi.implementAge; // age at which to implement the spell effects
	dProto = (*g_vm->_sdpList)[ssi.dProto];
	caster = GameObject::objectAddress(ssi.caster);
	target = new SpellTarget(ssi.target);
	GameObject *go = GameObject::objectAddress(ssi.world);
	assert(isWorld(go));
	world = (GameWorld *) go;
	age = ssi.age;
	spell = ssi.spell;
	maxAge = ssi.maxAge;
	effSeq = 0;
	effect = (*g_vm->_edpList)[ssi.effect];
	while (effSeq < ssi.effSeq)         // which effect in a sequence is being played
		effect = effect->next;
}

size_t SpellDisplayList::saveSize() {
	size_t total = 0;

	total += sizeof(count);
	if (count) {
		for (int i = 0; i < count; i++)
			total += spells[i]->saveSize();
	}
	return total;
}

void SpellDisplayList::write(Common::OutSaveFile *outS) {
	outS->write("SPEL", 4);
	CHUNK_BEGIN;
	out->writeUint16LE(count);

	debugC(3, kDebugSaveload, "... count = %d", count);

	if (count) {
		for (int i = 0; i < count; i++) {
			debugC(3, kDebugSaveload, "Saving Spell Instance %d", i);
			StorageSpellInstance ssi = StorageSpellInstance(*spells[i]);
			ssi.write(out);
			spells[i]->writeEffect(out);
		}
	}
	CHUNK_END;
}

void SpellDisplayList::read(Common::InSaveFile *in) {
	uint16 tCount;

	tCount = in->readUint16LE();

	debugC(3, kDebugSaveload, "... count = %d", tCount);

	assert(tCount < maxCount);
	if (tCount) {
		for (int i = 0; i < tCount; i++) {
			debugC(3, kDebugSaveload, "Loading Spell Instance %d", i);
			SpellInstance *si;
			StorageSpellInstance ssi;
			ssi.read(in);
			si = new SpellInstance(ssi);
			add(si);
			si->readEffect(in, ssi.eListSize);
		}
	}
	assert(tCount == count);
}

void SpellDisplayList::wipe() {
	for (int i = 0; i < maxCount; i++)
		if (spells[i]) {
			delete spells[i];
			spells[i] = nullptr;
			count--;
		}

	assert(count == 0);
}

size_t SpellInstance::saveSize() {
	size_t total = 0;
	total += sizeof(StorageSpellInstance);
	if (eList.count)
		for (int32 i = 0; i < eList.count; i++) {
			total += sizeof(StorageEffectron);
		}
	return total;
}

void SpellInstance::writeEffect(Common::MemoryWriteStreamDynamic *out) {
	if (eList.count > 0 && !(maxAge > 0 && (age + 1) > maxAge))
		for (int32 i = 0; i < eList.count; i++) {
			StorageEffectron se = StorageEffectron(*eList.displayList[i].efx);
			se.write(out);
		}
}

void SpellInstance::readEffect(Common::InSaveFile *in, uint16 eListSize) {
	assert(eListSize == effect->nodeCount);
	eList.count = effect->nodeCount; //sdp->effCount;
	if (eList.count)
		for (int32 i = 0; i < eList.count; i++) {
			StorageEffectron se;
			se.read(in);
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

void StorageEffectron::read(Common::InSaveFile *in) {
	flags = in->readUint32LE();
	size.load(in);
	hitBox.read(in);
	partno = in->readSint16LE();
	screenCoords.load(in);
	start.load(in);
	finish.load(in);
	current.load(in);
	velocity.load(in);
	acceleration.load(in);
	totalSteps = in->readUint16LE();
	stepNo = in->readUint16LE();
	hgt = in->readSint16LE();
	brd = in->readSint16LE();
	pos = in->readSint32LE();
	spr = in->readSint32LE();
	age = in->readSint32LE();
}

void StorageEffectron::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint32LE(flags);
	size.write(out);
	hitBox.write(out);
	out->writeSint16LE(partno);
	screenCoords.write(out);
	start.write(out);
	finish.write(out);
	current.write(out);
	velocity.write(out);
	acceleration.write(out);
	out->writeUint16LE(totalSteps);
	out->writeUint16LE(stepNo);
	out->writeSint16LE(hgt);
	out->writeSint16LE(brd);
	out->writeSint32LE(pos);
	out->writeSint32LE(spr);
	out->writeSint32LE(age);
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
