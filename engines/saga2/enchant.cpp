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

//#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/cmisc.h"
#include "saga2/player.h"
#include "saga2/enchant.h"

namespace Saga2 {

extern int16        objectProtoCount;       // object prototype count

int enchantmentProto = -1;

void setEnchantmentDisplay();

//-------------------------------------------------------------------
//	Enchantment Creation Function

ObjectID EnchantObject(
    ObjectID        target,
    int             enchantmentType,
    int             duration) {
	GameObject      *obj = GameObject::objectAddress(target);
	GameObject      *ench;
	ProtoObj        *enchProto;
	TilePoint       slot;

	assert(enchantmentProto >= 0);
	assert(enchantmentProto <  objectProtoCount);

	enchProto = g_vm->_objectProtos[enchantmentProto];

	ench = GameObject::newObject(); //Create Enchantment
	if (ench == nullptr) return Nothing;

	//  Fill in the enchantment object. Note that the 'hitpoints'
	//  of an enchantment are actually the duration of it's life
	//  (in 10-second background cycles).
	//
	//  Also note the use of the 'enchantment type' field to
	//  indicate the effects of the enchantment. This is to
	//  avoid having to create 50 new classes, representing
	//  50 new enchantments.
	ench->setScript(0);
	ench->setFlags(0, (uint8) - 1);
	ench->setHitPoints(duration);
	ench->setExtra(enchantmentType);
	ench->setProtoNum(enchantmentProto);

	//  Put in object's container
	if (obj->getAvailableSlot(ench, &slot))
		ench->move(Location(slot, target));

	//  Now, change the object base on enchantments
	obj->evalEnchantments();
	assert(enchProto->containmentSet() & ProtoObj::isEnchantment);
	assert((ench->protoAddress(ench->thisID()))->containmentSet() & ProtoObj::isEnchantment);
	return ench->thisID();
}

//-------------------------------------------------------------------
//	Function to deliberately dispel an enchantment

bool DispelObjectEnchantment(
    ObjectID        target,
    int             enchantmentType) {
	ObjectID        enchID;

	enchID = FindObjectEnchantment(target, enchantmentType);

	if (enchID != Nothing) {
		GameObject      *ench = GameObject::objectAddress(enchID);
		GameObject      *obj  = GameObject::objectAddress(target);

		//  Remove the enchantment and it's effects
		ench->deleteObject();
		obj->evalEnchantments();
		return true;
	}

	return false;
}

//-------------------------------------------------------------------
//	Function to locate an enchantment on an object

ObjectID FindObjectEnchantment(
    ObjectID        target,
    int             enchantmentType) {
	GameObject          *obj = GameObject::objectAddress(target);
	GameObject          *containedObj;
	ObjectID            objID;
	ContainerIterator   iter(obj);

	while ((objID = iter.next(&containedObj)) != Nothing) {
		ProtoObj *proto = containedObj->proto();

		if ((proto->containmentSet() & ProtoObj::isEnchantment)
		        && ((containedObj->getExtra() & 0xFF00) == (enchantmentType & 0xFF00))) {
			return objID;
		}
	}

	return Nothing;
}


void clearEnchantments(Actor *a) {
	ActorAttributes *ea = a->getStats();
	ActorAttributes *ba = a->getBaseStats();

	ea->archery     = ba->archery;
	ea->swordcraft  = ba->swordcraft;
	ea->shieldcraft = ba->shieldcraft;
	ea->bludgeon    = ba->bludgeon;
	ea->throwing    = ba->throwing;
	ea->spellcraft  = ba->spellcraft;
	ea->stealth     = ba->stealth;
	ea->agility     = ba->agility;
	ea->brawn       = ba->brawn;
	ea->lockpick    = ba->lockpick;
	ea->pilfer      = ba->pilfer;
	ea->firstAid    = ba->firstAid;
	ea->spotHidden  = ba->spotHidden;

	a->_enchantmentFlags     = a->getBaseEnchantmentEffects();
	a->_effectiveResistance  = a->getBaseResistance();
	a->_effectiveImmunity    = a->getBaseImmunity();
	a->_recPointsPerUpdate       = a->getBaseRecovery();
}

void addEnchantment(Actor *a, uint16 enchantmentID) {
	ActorAttributes *ea = a->getStats();
	uint8 *stats = &ea->archery;
	uint16 eType = getEnchantmentType(enchantmentID);
	uint16 eSubType = getEnchantmentSubType(enchantmentID);
	int16  eAmount = getEnchantmentAmount(enchantmentID);

	switch (eType) {
	case effectAttrib:
		stats[eSubType] = clamp(0, stats[eSubType] + eAmount, 100);
		break;
	case effectResist:
		a->setResist((effectResistTypes) eSubType, eAmount);
		break;
	case effectImmune:
		a->setImmune((effectImmuneTypes) eSubType, eAmount);
		break;
	case effectOthers:
		a->setEffect((effectOthersTypes) eSubType, eAmount);
		break;
	case effectSpecial:           // damage shouldn't be an enchantment
	// Special code needed
	case effectDamage:           // damage shouldn't be an enchantment
	case effectNone:
		break;
	}

}

//-------------------------------------------------------------------
//	Function to eval the enchantments on an actor

void evalActorEnchantments(Actor *a) {
	GameObject          *obj = nullptr;
	ObjectID            id;
	PlayerActorID       playerID;
	EnchantmentIterator iter(a);
	ContainerIterator   cIter(a);

	clearEnchantments(a);

	for (id = iter.first(&obj); id != Nothing; id = iter.next(&obj)) {
		ProtoObj *proto = obj->proto();

		if (proto->containmentSet() & ProtoObj::isEnchantment) {
			uint16 enchantmentID = obj->getExtra();
			addEnchantment(a, enchantmentID);
		}
	}

	while (cIter.next(&obj)) {
		ProtoObj        *proto = obj->proto();
		uint16          cSet = proto->containmentSet();

		if ((cSet & (ProtoObj::isArmor | ProtoObj::isWeapon | ProtoObj::isWearable))
		        &&  proto->isObjectBeingUsed(obj)) {
			a->_effectiveResistance  |= proto->resistance;
			a->_effectiveImmunity    |= proto->immunity;
		}
	}

	if (actorToPlayerID(a, playerID))
		recalcPortraitType(playerID);

	if (a->thisID() == getCenterActorID())
		setEnchantmentDisplay();
}

//-------------------------------------------------------------------
//	Function to eval the enchantments on an actor

void evalObjectEnchantments(GameObject *obj) {
	//  The only enchantment that currently works
	//  on objects is the invisibility bit.
	//
	//  If more enchantment types are added, then we'll
	//  have to do this a bit differently...

	if (FindObjectEnchantment(obj->thisID(), makeEnchantmentID(effectNonActor, objectInvisible, true)))
		obj->setFlags((uint8) - 1, objectInvisible);
	else
		obj->setFlags(0, objectInvisible);
	if (FindObjectEnchantment(obj->thisID(), makeEnchantmentID(effectNonActor, objectLocked, false)))
		obj->setFlags((uint8) - 1, objectLocked);
}

//-------------------------------------------------------------------
//	Enchantment iterator class

EnchantmentIterator::EnchantmentIterator(GameObject *container) {
	//  Get the ID of the 1st object in the sector list
	baseObject = container;
	wornObject = nullptr;
	nextID = Nothing;
}

ObjectID EnchantmentIterator::first(GameObject **obj) {
	nextID = baseObject->IDChild();

	return next(obj);
}

ObjectID EnchantmentIterator::next(GameObject **obj) {
	GameObject          *object;
	ObjectID            id;

	for (;;) {
		id = nextID;

		if (id == Nothing) {
			//  If we were searching a 'worn' object, then pop up a level
			if (wornObject) {
				nextID = wornObject->IDNext();
				wornObject = nullptr;
				continue;
			}

			return Nothing;
		}

		//  Get address of next object
		object = GameObject::objectAddress(id);

		ProtoObj        *proto = object->proto();
		uint16          cSet = proto->containmentSet();

		if ((cSet & (ProtoObj::isArmor | ProtoObj::isWeapon | ProtoObj::isWearable))
		        &&  wornObject == nullptr
		        &&  proto->isObjectBeingUsed(object)) {
			wornObject = object;
			nextID = object->IDChild();
			continue;
		}

		nextID = object->IDNext();

		if (cSet & ProtoObj::isEnchantment) break;
	}

	if (obj) *obj = object;
	return id;
}

} // end of namespace Saga2
