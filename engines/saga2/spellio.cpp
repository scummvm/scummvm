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
	_effect = rsi->effect;                // Effect ID
	_effParm1 = 0;                        //   effect setting 1
	_effParm2 = 0;                        //   effect setting 1
	_effParm3 = 0;                        //   effect setting 1
	_effParm4 = 0;                        //   effect setting 1
	_scatter = kDiFlagZero;                // direction init mode
	_elasticity = (effectCollisionCont) rsi->effectronElasticity; // collision flags
	_maxAge = rsi->maxAge;                // auto self-destruct age
	_implementAge = rsi->implAge;         // auto self-destruct age
	_primarySpriteID = rsi->baseSprite; // RES_ID(x, y, z, 0) to get sprites
	_primarySpriteNo = rsi->spriteCount; // sprites available
	_secondarySpriteID = rsi->baseSprite2; // RES_ID(x, y, z, 0) to get sprites
	_secondarySpriteNo = rsi->spriteCount2; // sprites available
	//_effCount=0;            // effectrons to allocate
	_colorMap[0] = rsi->cm0;
	_colorMap[1] = rsi->cm1;
	_colorMap[2] = 0;
	_colorMap[3] = 0;
	_ID = kSpellNone;
}

/* ===================================================================== *
   partial SpellStuff implementation
 * ===================================================================== */

// ------------------------------------------------------------------
// init from res file

void SpellStuff::setupFromResource(ResourceSpellItem *rsi) {
	_master = (SpellID) rsi->spell;
	_display = (SpellID) rsi->spell;
	_targetableTypes = (SpellTargetingTypes) rsi->targs;
	_targetTypes = (SpellApplicationTypes) rsi->applys;
	_manaType = (SpellManaID) rsi->manaType;
	_manaUse = rsi->manaAmount;
	_shape = (effectAreas) rsi->effect;
	_size = 0;
	_sound = rsi->soundID;
}

// ------------------------------------------------------------------
// add spell internal effect

void SpellStuff::addEffect(ResourceSpellEffect *rse) {
	ProtoEffect *pe = nullptr;
	assert(rse && rse->spell == _master);
	switch (rse->effectGroup) {
	case kEffectNone     :
		return;
	case kEffectAttrib   :
		pe = new ProtoEnchantment(
		         makeEnchantmentID(
		             rse->effectGroup,
		             rse->effectType,
		             rse->attribModifier),
		         rse->enchTimeLo,
		         rse->enchTimeHi);
		break;
	case kEffectResist   :
	case kEffectImmune   :
	case kEffectOthers   :
	case kEffectNonActor :
		pe = new ProtoEnchantment(
		         makeEnchantmentID(
		             rse->effectGroup,
		             rse->effectType,
		             rse->flagSet),
		         rse->enchTimeLo,
		         rse->enchTimeHi);
		break;
	case kEffectDamage   :
		pe = new ProtoDamage(
		         rse->baseDice,
		         rse->diceSides ? rse->diceSides : 6,
		         rse->skillDice,
		         0,
		         (effectDamageTypes) rse->effectType,
		         0,
		         rse->targeting & kSpellTargCaster);
		break;
	case kEffectDrains   :
		pe = new ProtoDrainage(
		         rse->baseDice,
		         rse->diceSides ? rse->diceSides : 6,
		         rse->skillDice,
		         0,
		         (effectDrainsTypes) rse->effectType,
		         0,
		         rse->targeting & kSpellTargCaster);
		break;
	case kEffectTAG      :
		pe = new ProtoTAGEffect(
		         (effectTAGTypes) rse->effectType,
		         rse->flagSet,
		         rse->attribModifier);
		break;
	case kEffectLocation :
		pe = new ProtoLocationEffect(
		         (kEffectLocationTypes) rse->effectType,
		         rse->attribModifier);
		break;
	case kEffectSpecial  : {
		switch (rse->effectType) {
		case    kSpecialDispellHelpfulEnch :   // clears helpful enchantments
			pe = new ProtoSpecialEffect(DispellProtections, rse->attribModifier);
			break;
		case    kSpecialDispellHarmfulEnch :   // clears harmful enchantments
			pe = new ProtoSpecialEffect(DispellCurses,      rse->attribModifier);
			break;
		case    kSpecialKill               :   // death spell
			pe = new ProtoSpecialEffect(DeathSpell,         rse->attribModifier);
			break;
		case    kSpecialRessurect          :   // raise dead spell
			pe = new ProtoSpecialEffect(Resurrect,          rse->attribModifier);
			break;
		case    kSpecialTeleport           :   // Teleportation
			pe = new ProtoSpecialEffect(TeleportToLocation, rse->attribModifier);
			break;
		case    kSpecialCreateActor        :   // Create an actor or wall
			pe = new ProtoSpecialEffect(CreateWraith,       rse->attribModifier);
			break;
		case    kSpecialSagaFunc           :    // calls a saga function
			pe = new ProtoSpecialEffect(SagaSpellCall,      rse->attribModifier);
			break;
		case    kSpecialRejoin        :   // Create an actor or wall
			pe = new ProtoSpecialEffect(Rejoin,      rse->attribModifier);
			break;
		case    kSpecialCreateWWisp  :  // calls a saga function
			pe = new ProtoSpecialEffect(CreateWWisp,      rse->attribModifier);
			break;
		case    kSpecialCreateFWisp  :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateFWisp,      rse->attribModifier);
			break;
		case    kSpecialCreateWraith :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateWraith,      rse->attribModifier);
			break;
		case    kSpecialCreateFood   :   // calls a saga function
			pe = new ProtoSpecialEffect(CreateFood,      rse->attribModifier);
			break;
		}
		break;
	}
	}
	if (pe == nullptr)
		error("failed to alloc protoEffect");

	if (_effects == nullptr)
		_effects = pe;
	else {
		ProtoEffect *tail;
		for (tail = _effects; tail->_next; tail = tail->_next) ;
		tail->_next = pe;
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
// cleanup active _spells

StorageSpellTarget::StorageSpellTarget(SpellTarget &st) {
	GameObject *go = nullptr;
	ActiveItem *ai;
	type = st.getType();
	loc = st.getPoint();
	if (type == SpellTarget::kSpellTargetObject) {
		if (type == SpellTarget::kSpellTargetObject)
			go = st.getObject();
		else
			go = nullptr;
	}

	if (go)
		obj = go->thisID();
	else
		obj = Nothing;

	if (type == SpellTarget::kSpellTargetTAG)
		ai = st.getTAG();
	else
		ai = nullptr;

	if (ai)
		tag = ai->thisID();
	else
		tag = NoActiveItem;
}

StorageSpellInstance::StorageSpellInstance(SpellInstance &si) {
	implementAge = si._implementAge; // age at which to implement the spell effects
	effect = si._effect->thisID();      // effect prototype of the current effect
	dProto = si._dProto->thisID();      // effect prototype of the current effect
	caster = si._caster->thisID();
	target = StorageSpellTarget(*si._target);
	world = si._world->thisID();
	age = si._age;
	spell = si._spell;
	maxAge = si._maxAge;
	effSeq = si._effSeq;         // which effect in a sequence is being played
	eListSize = si._eList._count;
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

StorageSpellInstance::StorageSpellInstance() : implementAge(0), effect(0), dProto(kSpellNone), caster(0),
	world(0), age(0), spell(kSpellNone), maxAge(0), effSeq(0), eListSize(0) {
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
	_type = (SpellTarget::spellTargetType) sst.type;
	_loc = sst.loc;
	_next = nullptr;
	if (sst.obj != Nothing)
		_obj = GameObject::objectAddress(sst.obj);
	else
		_obj = nullptr;
	if (sst.tag != NoActiveItem)
		_tag = ActiveItem::activeItemAddress(sst.tag);
	else
		_tag = nullptr;
}

SpellInstance::SpellInstance(StorageSpellInstance &ssi) {
	_implementAge = ssi.implementAge; // age at which to implement the spell effects
	_dProto = (*g_vm->_sdpList)[ssi.dProto];
	_caster = GameObject::objectAddress(ssi.caster);
	_target = new SpellTarget(ssi.target);
	GameObject *go = GameObject::objectAddress(ssi.world);
	assert(isWorld(go));
	_world = (GameWorld *) go;
	_age = ssi.age;
	_spell = ssi.spell;
	_maxAge = ssi.maxAge;
	_effSeq = 0;
	_effect = (*g_vm->_edpList)[ssi.effect];
	while (_effSeq < ssi.effSeq)         // which effect in a sequence is being played
		_effect = _effect->_next;
}

size_t SpellDisplayList::saveSize() {
	size_t total = 0;

	total += sizeof(_count);
	if (_count) {
		for (int i = 0; i < _count; i++)
			total += _spells[i]->saveSize();
	}
	return total;
}

void SpellDisplayList::write(Common::OutSaveFile *outS) {
	outS->write("SPEL", 4);
	CHUNK_BEGIN;
	out->writeUint16LE(_count);

	debugC(3, kDebugSaveload, "... count = %d", _count);

	if (_count) {
		for (int i = 0; i < _count; i++) {
			debugC(3, kDebugSaveload, "Saving Spell Instance %d", i);
			StorageSpellInstance ssi = StorageSpellInstance(*_spells[i]);
			ssi.write(out);
			_spells[i]->writeEffect(out);
		}
	}
	CHUNK_END;
}

void SpellDisplayList::read(Common::InSaveFile *in) {
	uint16 tCount;

	tCount = in->readUint16LE();

	debugC(3, kDebugSaveload, "... count = %d", tCount);

	assert(tCount < _maxCount);
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
	assert(tCount == _count);
}

void SpellDisplayList::wipe() {
	for (int i = 0; i < _maxCount; i++)
		if (_spells[i]) {
			delete _spells[i];
			_spells[i] = nullptr;
			_count--;
		}

	assert(_count == 0);
}

size_t SpellInstance::saveSize() {
	size_t total = 0;
	total += sizeof(StorageSpellInstance);
	if (_eList._count)
		for (int32 i = 0; i < _eList._count; i++) {
			total += sizeof(StorageEffectron);
		}
	return total;
}

void SpellInstance::writeEffect(Common::MemoryWriteStreamDynamic *out) {
	if (_eList._count > 0 && !(_maxAge > 0 && (_age + 1) > _maxAge))
		for (int32 i = 0; i < _eList._count; i++) {
			StorageEffectron se = StorageEffectron(*_eList._displayList[i]._efx);
			se.write(out);
		}
}

void SpellInstance::readEffect(Common::InSaveFile *in, uint16 eListSize) {
	assert(eListSize == _effect->_nodeCount);
	_eList._count = _effect->_nodeCount; //sdp->effCount;
	if (_eList._count)
		for (int32 i = 0; i < _eList._count; i++) {
			StorageEffectron se;
			se.read(in);
			Effectron *e = new Effectron(se, this);
			_eList._displayList[i]._efx = e;
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
	flags =         e._flags;
	size =          e._size;
	hitBox =        e._hitBox;
	screenCoords =  e._screenCoords;
	partno =        e._partno;
	start =         e._start;
	finish =        e._finish;
	current =       e._current;
	velocity =      e._velocity;
	acceleration =  e._acceleration;
	totalSteps =    e._totalSteps;
	stepNo =        e._stepNo;
	hgt =           e._hgt;
	brd =           e._brd;
	pos =           e._pos;
	spr =           e._spr;
	age =           e._age;
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
	_flags =         se.flags;
	_size =          se.size;
	_hitBox =        se.hitBox;
	_screenCoords =  se.screenCoords;
	_partno =        se.partno;
	_start =         se.start;
	_finish =        se.finish;
	_current =       se.current;
	_velocity =      se.velocity;
	_acceleration =  se.acceleration;
	_totalSteps =    se.totalSteps;
	_stepNo =        se.stepNo;
	_hgt =           se.hgt;
	_brd =           se.brd;
	_pos =           se.pos;
	_spr =           se.spr;
	_age =           se.age;
	_parent = si;
}

} // end of namespace Saga2
