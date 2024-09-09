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

#include "saga2/saga2.h"
#include "saga2/gdraw.h"
#include "saga2/objects.h"
#include "saga2/grabinfo.h"
#include "saga2/contain.h"
#include "saga2/motion.h"
#include "saga2/player.h"
#include "saga2/script.h"
#include "saga2/document.h"
#include "saga2/magic.h"
#include "saga2/weapons.h"
#include "saga2/spellbuk.h"
#include "saga2/combat.h"
#include "saga2/tile.h"

#include "saga2/methods.r"
#include "saga2/pclass.r"

namespace Saga2 {

extern SpellStuff   *spellBook;
extern uint8        identityColors[256];

#ifdef __WATCOMC__
#pragma off (unreferenced);
#endif


extern ObjectSoundFXs   *objectSoundFXTable;    // the global object sound effects table

#if DEBUG
extern bool     massAndBulkCount;
#endif

/* ===================================================================== *
   Functions
 * ===================================================================== */

ObjectID ProtoObj::placeObject() {
	return 2;
}

//  Check if item can be contained by this object
bool ProtoObj::canContain(ObjectID dObj, ObjectID item) {
	return false;
}

//  Determine if the object can contain another object at the specified
//  slot
bool ProtoObj::canContainAt(
    ObjectID        dObj,
    ObjectID        item,
    const TilePoint &where) {
	return false;
}

//  Determine if this type of object is two handed
bool ProtoObj::isTwoHanded(ObjectID) {
	return false;
}

//  Determine if this type of object is a missile
bool ProtoObj::isMissile() {
	return false;
}

//Create Container Window
// ContainerWindow *ProtoObj::makeWindow( GameObject *Obj ) { return NULL; }


//  generic actions

//  Simple use command
bool ProtoObj::use(ObjectID dObj, ObjectID enactor) {
	assert(dObj != Nothing);

	int16   scriptResult;

	//  Setup use cursor, if necessary
	if (setUseCursor(dObj)) return true;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onUse,
	                        dObj, enactor, Nothing))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return useAction(dObj, enactor);
}

//  The default action is not to set up a use cursor
bool ProtoObj::setUseCursor(ObjectID dObj) {
	return false;
}

//  The virtual use action command
bool ProtoObj::useAction(ObjectID dObj, ObjectID enactor) {
	return false;
}

//  UseOn object command
bool ProtoObj::useOn(ObjectID dObj, ObjectID enactor, ObjectID item) {
	assert(dObj != Nothing);
	assert(item != Nothing);

	int16   scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onUseOn,
	                        dObj, enactor, item))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	//  If script has not aborted action call virtual useOnAction
	//  function
	return useOnAction(dObj, enactor, item);
}

//  Perform the use on action
bool ProtoObj::useOnAction(ObjectID dObj, ObjectID enactor, ObjectID item) {
	return false;
}

//  UseOn active item command
bool ProtoObj::useOn(ObjectID dObj, ObjectID enactor, ActiveItem *item) {
	assert(dObj != Nothing);
	assert(item != nullptr);

	int16   scrResult;

	scriptCallFrame scf;
	scriptResult    sResult;

	scf.invokedObject   = dObj;
	scf.enactor         = enactor;
	scf.directObject    = dObj;
	scf.indirectTAI     = item->thisID();
	scf.value           = 0;

	//  Call the SAGA script, if there is one.
	sResult = runObjectMethod(dObj, Method_GameObject_onUseOnTAI, scf);

	//  If the script actually ran, and it didn't return a code
	//  telling us to abort the action...
	if (sResult == kScriptResultFinished)
		scrResult = scf.returnVal;
	else
		scrResult = kActionResultNotDone;
	if (scrResult != kActionResultNotDone)
		return scrResult == kActionResultSuccess;

	return useOnAction(dObj, enactor, item);

}

//  Perform the use on action
bool ProtoObj::useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *item) {
	return false;
}

//  UseOn location command
bool ProtoObj::useOn(ObjectID dObj, ObjectID enactor, const Location &loc) {
	assert(dObj != Nothing);
	assert(loc != Nowhere && loc._context != Nothing);

	/*  int16   scrResult;

	    scriptCallFrame scf;
	    scriptResult    sResult=(scriptResult) kActionResultNotDone;

	    scf.invokedObject   = dObj;
	    scf.enactor         = enactor;
	    scf.directObject    = dObj;
	    //scf.indirectTAI       = item->thisID();
	//  scf.indirectObject  = indirectObj;
	    scf.value           = 0;

	        //  Call the SAGA script, if there is one.
	//  sResult = runObjectMethod( dObj, Method_GameObject_onUseOnTAI, scf );

	        //  If the script actually ran, and it didn't return a code
	        //  telling us to abort the action...
	    if ( sResult == kScriptResultFinished )
	        scrResult=scf.returnVal;
	    else
	        scrResult=kActionResultNotDone;
	*//*
        //  Handle object script in a standard fashion
    if (    ( scriptResult =    stdActionScript(
                                    Method_GameObject_useOnTAI,
                                    dObj, enactor, item->thisID() ) )
                !=  kActionResultNotDone )
        return scriptResult == kActionResultSuccess;

        //  If script has not aborted action call virtual useOnAction
        //  function
*//*
    if ( scrResult != kActionResultNotDone )
        return scrResult == kActionResultSuccess;
*/
	return useOnAction(dObj, enactor, loc);
}

//  Perform the use on action
bool ProtoObj::useOnAction(ObjectID dObj, ObjectID enactor, const Location &loc) {
	return false;
}

//-----------------------------------------------------------------------
//	Returns true if object in continuous use.

bool ProtoObj::isObjectBeingUsed(GameObject *) {
	return false;
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool ProtoObj::useSlotAvailable(GameObject *, Actor *) {
	return false;
}

//  Open this object
bool ProtoObj::open(ObjectID dObj, ObjectID enactor) {
	assert(dObj != Nothing);

	int16   scriptResult;

	if (!canOpen(dObj, enactor)) return false;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onOpen,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return openAction(dObj, enactor);
}

//  Virtual function to determine if this object can be opened
bool ProtoObj::canOpen(ObjectID, ObjectID) {
	return false;
}

//  Virtual function to actually open the object
bool ProtoObj::openAction(ObjectID, ObjectID) {
	return false;
}

//  Close this object
bool ProtoObj::close(ObjectID dObj, ObjectID enactor) {
	assert(dObj != Nothing);

	int16           scriptResult;
	GameObject      *dObjPtr = GameObject::objectAddress(dObj);

	if (!dObjPtr->isOpen()) return false;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onClose,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return closeAction(dObj, enactor);
}

//  Virtual function to actually close the object
bool ProtoObj::closeAction(ObjectID, ObjectID) {
	return false;
}

//  Take this object
bool ProtoObj::take(ObjectID dObj, ObjectID enactor, int16 num) {
	assert(dObj != Nothing);
	assert(g_vm->_mouseInfo->getObjectId() == Nothing);

	// >>> this needs to be dynamic!
	if (mass > 200 || bulk > 200) return false;

	int16           scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onTake,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return takeAction(dObj, enactor, num);
}

//  Virtual function to take the object
bool ProtoObj::takeAction(ObjectID, ObjectID) {
	return false;
}

//  Virtual function to take the object
bool ProtoObj::takeAction(ObjectID, ObjectID, int16 num) {
	return false;
}


//  Drop this object at the specified location
bool ProtoObj::drop(ObjectID dObj, ObjectID enactor, const Location &loc, int16 num) {
	assert(dObj != Nothing);

	if (!canDropAt(dObj, enactor, loc)) return false;

	//  Handle object script in a non-standard fashion
	scriptCallFrame scf;
	scriptResult    sResult;

	scf.invokedObject   = dObj;
	scf.enactor         = enactor;
	scf.directObject    = dObj;
	scf.indirectObject  = loc._context;
	scf.coords          = loc;
	scf.value           = 0;

	//  Call the SAGA script, if there is one.
	sResult = runObjectMethod(dObj, Method_GameObject_onDrop, scf);

	//  If the script actually ran, and it didn't return a code
	//  telling us to abort the action...
	if (sResult == kScriptResultFinished
	        &&  scf.returnVal != kActionResultNotDone)
		return scf.returnVal == kActionResultSuccess;

	return dropAction(dObj, enactor, loc, num);
}

//  Virtual function to determine if this object can be dropped at the
//  specified location
bool ProtoObj::canDropAt(ObjectID, ObjectID, const Location &) {
	return false;
}

//  Virtual function to drop the object
bool ProtoObj::dropAction(ObjectID, ObjectID, const Location &, int16) {
	return false;
}

//  drop an object onto another object and handle the result.
bool ProtoObj::dropOn(ObjectID dObj, ObjectID enactor, ObjectID target, int16 count) {
	assert(dObj != Nothing);

	// this prevents objects from being dropped on themselves
	if (target == dObj) return true;

	int16       scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onDropOn,
	                        dObj, enactor, target, count))
	        !=  kActionResultNotDone) {
		return scriptResult == kActionResultSuccess;
	}

	//  At this point we should probably _split_ the object...

	return dropOnAction(dObj, enactor, target, count);
}

//  Do the actual drop
bool ProtoObj::dropOnAction(ObjectID dObj, ObjectID enactor, ObjectID target, int count) {
	ProtoObj        *targetProto = GameObject::protoAddress(target);

	//  For now, we just re-send the drop message to the object
	//  being dropped on...

	return targetProto->acceptDrop(target, enactor, dObj, count);
}

//  drop this object onto a TAG
bool ProtoObj::dropOn(
    ObjectID        dObj,
    ObjectID        enactor,
    ActiveItem      *target,
    const Location  &loc,
    int16           num) {
	assert(dObj != Nothing);
	assert(target != nullptr);
	assert(isWorld(loc._context));

	return dropOnAction(dObj, enactor, target, loc, num);
}

//  Virtual function to drop an object on a TAG
bool ProtoObj::dropOnAction(
    ObjectID,
    ObjectID,
    ActiveItem *,
    const Location &,
    int16) {
	return false;
}

//  Strike another object with this object
bool ProtoObj::strike(ObjectID dObj, ObjectID enactor, ObjectID item) {
	assert(isObject(dObj) || isActor(dObj));
	assert(isObject(item) || isActor(item));

	int16   scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onStrike,
	                        dObj, enactor, item))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return strikeAction(dObj, enactor, item);
}

//  Virtual function to strike another object with this object
bool ProtoObj::strikeAction(ObjectID, ObjectID, ObjectID) {
	return false;
}

//  Damage another object with this object
bool ProtoObj::damage(ObjectID dObj, ObjectID enactor, ObjectID target) {
	assert(isObject(dObj) || isActor(dObj));
	assert(isObject(target) || isActor(target));

	int16   scriptResult;

	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onDamage,
	                        dObj, enactor, target))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return damageAction(dObj, enactor, target);
}

//  Virtual function to damage another object with this object
bool ProtoObj::damageAction(ObjectID, ObjectID, ObjectID) {
	return false;
}

//  Eat this object
bool ProtoObj::eat(ObjectID dObj, ObjectID enactor) {
	assert(dObj != Nothing);

	int16           scriptResult;

	//  Handle object script in a standard fashion
	warning("ProtoObj::eat: Method_GameObject_onEat undefined");
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onEat,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return eatAction(dObj, enactor);
}

//  Virtual function to eat this object
bool ProtoObj::eatAction(ObjectID, ObjectID) {
	return false;
}

//  Insert this object into another object
bool ProtoObj::insert(ObjectID dObj, ObjectID enactor, ObjectID item) {
	assert(dObj != Nothing);
	assert(item != Nothing);

	int16   scriptResult;

	//  Handle object script in a standard fashion
	warning("ProtoObj::insert: Method_GameObject_onInsert undefined");
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onInsert,
	                        dObj, enactor, item))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return insertAction(dObj, enactor, item);
}

//  Virtual function to insert this object into another object
bool ProtoObj::insertAction(ObjectID, ObjectID, ObjectID) {
	return false;
}

//  Remove this object from the object it is in
bool ProtoObj::remove(ObjectID dObj, ObjectID enactor) {
	assert(dObj != Nothing);

	int16           scriptResult;

	//  Handle object script in a standard fashion
	warning("ProtoObj::remove: Method_GameObject_onRemove undefined");
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onRemove,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return removeAction(dObj, enactor);
}

//  Virtual function to remove this object from the object it is in
bool ProtoObj::removeAction(ObjectID, ObjectID) {
	return false;
}

//  Handle the effects of an object being dropped on this object
bool ProtoObj::acceptDrop(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID droppedObj,
    int count) {
	assert(dObj != Nothing);
	assert(droppedObj != Nothing);

	int16   scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptDrop,
	                        dObj, enactor, droppedObj, count))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return acceptDropAction(dObj, enactor, droppedObj, count);
}

//  Virtual function to handle the effects of an object being dropped
//  on this object
bool ProtoObj::acceptDropAction(ObjectID dObj, ObjectID enactor, ObjectID droppedObj, int count) {
	return acceptInsertion(dObj, enactor, droppedObj, count);
}


bool ProtoObj::acceptDamage(
    ObjectID            dObj,
    ObjectID            enactor,
    int8                absDamage,
    effectDamageTypes   dType,
    int8                dice,
    uint8               sides,
    int8                perDieMod) {
	int16   scriptResult;

	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptDamage,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return  acceptDamageAction(
	            dObj,
	            enactor,
	            absDamage,
	            dType,
	            dice,
	            sides,
	            perDieMod);
}


//  Virtual function to damage this object directly
bool ProtoObj::acceptDamageAction(
    ObjectID,
    ObjectID,
    int8,
    effectDamageTypes,
    int8,
    uint8,
    int8) {
	return true;
}

bool ProtoObj::acceptHealing(
    ObjectID    dObj,
    ObjectID    enactor,
    int8        absDamage,
    int8        dice,
    uint8       sides,
    int8        perDieMod) {
	int8 pdm = perDieMod;
	int16 damage = 0;
	assert(dObj != Nothing);
	damage = absDamage;
	if (dice)
		for (int d = 0; d < ABS(dice); d++)
			damage += (g_vm->_rnd->getRandomNumber(sides - 1) + pdm + 1) * (dice > 0 ? 1 : -1);

	return acceptHealingAction(dObj, enactor, damage);
}


//  Virtual function to damage this object directly
bool ProtoObj::acceptHealingAction(ObjectID, ObjectID, int8) {
	return false;
}

//  Accept strike from another object (allows this object to cause
//  damage to the striking object).
bool ProtoObj::acceptStrike(
    ObjectID            dObj,
    ObjectID            enactor,
    ObjectID            strikingObj,
    uint8               skillIndex) {
	assert(dObj != Nothing);
	assert(strikingObj != Nothing);

	int16   scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptStrike,
	                        dObj, enactor, strikingObj))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return  acceptStrikeAction(
	            dObj,
	            enactor,
	            strikingObj,
	            skillIndex);
}

//  Virtual function to handle the effects of a strike on this object
bool ProtoObj::acceptStrikeAction(
    ObjectID,
    ObjectID,
    ObjectID,
    uint8) {
	return true;
}

//  Unlock or lock this object with a key.
bool ProtoObj::acceptLockToggle(
    ObjectID    dObj,
    ObjectID    enactor,
    uint8       keyCode) {
	assert(dObj != Nothing);

	int16       scriptResult;

	if (!canToggleLock(dObj, enactor, keyCode)) return false;

	//  Handle object script in a standard fashion
	warning("ProtoObj::acceptLockToggle: Method_GameObject_onAcceptLockToggle undefined");
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptLockToggle,
	                        dObj, enactor, Nothing))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return acceptLockToggleAction(dObj, enactor, keyCode);
}

//  Virtual function to determine if the lock can be toggled
bool ProtoObj::canToggleLock(ObjectID, ObjectID, uint8) {
	return false;
}

//  Virtual function to actually toggle the lock
bool ProtoObj::acceptLockToggleAction(ObjectID, ObjectID, uint8) {
	return false;
}

//  Mix this object with another.
bool ProtoObj::acceptMix(ObjectID dObj, ObjectID enactor, ObjectID mixObj) {
	assert(dObj != Nothing);
	assert(mixObj != Nothing);

	int16   scriptResult;

	//  Handle object script in a standard fashion
	warning("ProtoObj::acceptMix: Method_GameObject_onAcceptMix undefined");
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptMix,
	                        dObj, enactor, mixObj))
	        !=  kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return acceptMixAction(dObj, enactor, mixObj);
}

//  Virtual function to mix this object with another
bool ProtoObj::acceptMixAction(ObjectID, ObjectID, ObjectID) {
	return false;
}

//  Insert another object into this object.
bool ProtoObj::acceptInsertion(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID item,
    int16 count) {
	assert(dObj != Nothing);
	assert(item != Nothing);

	if (!canContain(dObj, item)) return false;

	int16           scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptInsertion,
	                        dObj, enactor, item, count))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return acceptInsertionAction(dObj, enactor, item, count);
}

//  Virtual function to insert an object into this object
bool ProtoObj::acceptInsertionAction(ObjectID, ObjectID, ObjectID, int16) {
	return false;
}

//  Insert another object into this object at a specified slot
bool ProtoObj::acceptInsertionAt(
    ObjectID        dObj,
    ObjectID        enactor,
    ObjectID        item,
    const TilePoint &where,
    int16           num) {
	assert(dObj != Nothing);
	assert(item != Nothing);

	if (!canContainAt(dObj, item, where)) return false;

	int16           scriptResult;

	//  Handle object script in a standard fashion
	if ((scriptResult = stdActionScript(
	                        Method_GameObject_onAcceptInsertion,
	                        dObj, enactor, item))
	        != kActionResultNotDone)
		return scriptResult == kActionResultSuccess;

	return acceptInsertionAtAction(dObj, enactor, item, where, num);
}

//  Virtual function to insert another object into this object at a
//  specified slot
bool ProtoObj::acceptInsertionAtAction(
    ObjectID, ObjectID, ObjectID, const TilePoint &, int16) {
	return false;
}



//  Creates a color translation table for this object
void ProtoObj::getColorTranslation(ColorTable map) {
	buildColorTable(map, colorMap, ARRAYSIZE(colorMap));
}

uint16  ProtoObj::containmentSet() {
	return 0; // the prototye object is not contained in anything
}

//  return the sprite data
ObjectSpriteInfo ProtoObj::getSprite(GameObject *obj, enum spriteTypes spr, int16 count) {
	ObjectSpriteInfo    sprInfo = { nullptr, static_cast<bool>((flags & kObjPropFlipped) != 0) };
	int16               openOffset = ((flags & kObjPropVisOpen) && obj->isOpen()) ? 1 : 0;

	switch (spr) {
	case kObjOnGround:

		//  If the object is a moving missile return the correct missile
		//  sprite
		if (obj->isMoving()
		        &&  obj->isMissile()
		        &&  obj->_data.missileFacing < 16) {
			int16   sprIndex;

			if (obj->_data.missileFacing < 8)
				sprIndex = obj->_data.missileFacing;
			else {
				sprIndex = 16 - obj->_data.missileFacing;
				sprInfo.flipped = true;
			}

			sprInfo.sp = missileSprites->sprite(sprIndex);
		} else {
			sprInfo.sp = objectSprites->sprite(groundSprite + openOffset + obj->getSprOffset(count));
			sprInfo.flipped =
			    (flags & ResourceObjectPrototype::kObjPropFlipped) != 0;
		}

		break;

	case kObjInContainerView:
	case kObjAsMousePtr:

		sprInfo.sp = objectSprites->sprite(iconSprite + openOffset + obj->getSprOffset(count));
		sprInfo.flipped =
		    (flags & ResourceObjectPrototype::kObjPropFlipped) != 0;
		break;
	}
	return sprInfo;
}

//  return the address of the sprite when held in hand
Sprite *ProtoObj::getOrientedSprite(GameObject *obj, int16 offset) {
	return nullptr;
}

int16 ProtoObj::stdActionScript(
    int             method,
    ObjectID        dObj,
    ObjectID        enactor,
    ObjectID        indirectObj) {
	scriptCallFrame scf;
	scriptResult    sResult;

	scf.invokedObject   = dObj;
	scf.enactor         = enactor;
	scf.directObject    = dObj;
	scf.indirectObject  = indirectObj;
	scf.value           = 0;

	//  Call the SAGA script, if there is one.
	sResult = runObjectMethod(dObj, method, scf);

	//  If the script actually ran, and it didn't return a code
	//  telling us to abort the action...
	if (sResult == kScriptResultFinished)
		return scf.returnVal;

	return kActionResultNotDone;
}

int16 ProtoObj::stdActionScript(
    int             method,
    ObjectID        dObj,
    ObjectID        enactor,
    ObjectID        indirectObj,
    int16           value) {
	scriptCallFrame scf;
	scriptResult    sResult;

	scf.invokedObject   = dObj;
	scf.enactor         = enactor;
	scf.directObject    = dObj;
	scf.indirectObject  = indirectObj;
	scf.value           = value;

	//  Call the SAGA script, if there is one.
	sResult = runObjectMethod(dObj, method, scf);

	//  If the script actually ran, and it didn't return a code
	//  telling us to abort the action...
	if (sResult == kScriptResultFinished)
		return scf.returnVal;

	return kActionResultNotDone;
}

//  Initiate an attack using this type of object
void ProtoObj::initiateAttack(ObjectID, ObjectID) {}

//  Initiate a defense using this type of object
void ProtoObj::initiateDefense(ObjectID, ObjectID, ObjectID) {}

//  Get projectile for missile weapons
GameObject *ProtoObj::getProjectile(ObjectID, ObjectID) {
	return nullptr;
}

//  Get spell from a magic object
GameObject *ProtoObj::getSpell(ObjectID) {
	return nullptr;
}

//  Determine if this type of object can block an attack
bool ProtoObj::canBlock() {
	return false;
}

//  Return a mask of bits indicating the directions relative to the
//  wielders facing in which this object can defend
uint8 ProtoObj::defenseDirMask() {
	return 0;
}

// ------------------------------------------------------------------------
//	Compute how much damage this defensive object will absorb

uint8 ProtoObj::adjustDamage(uint8 damage) {
	return damage;
}

// ------------------------------------------------------------------------
//	Return the fight stance approriate to this weapon

int16 ProtoObj::fightStanceAction(ObjectID actor) {
	return kActionStand;
}

// ------------------------------------------------------------------------
//	Get the value of the user's skill which applies to this  object

uint8 ProtoObj::getSkillValue(ObjectID) {
	return 0;
}

// ------------------------------------------------------------------------
//	Cause the user's associated skill to grow

void ProtoObj::applySkillGrowth(ObjectID, uint8) {}

// ------------------------------------------------------------------------
//	Given an object sound effect record, which sound should be made
//	when this object is damaged

uint8 ProtoObj::getDamageSound(const ObjectSoundFXs &) {
	//  Default is no sound
	return 0;
}

// ------------------------------------------------------------------------
//	Background update function, called once every few seconds

void ProtoObj::doBackgroundUpdate(GameObject *obj) {
	TilePoint   location = obj->getLocation();
	GameWorld   *w = obj->world();
	int u = location.u >> kSectorShift;
	int v = location.v >> kSectorShift;

	if (w == nullptr) {
		obj->deactivate();
		return;
	}

	Sector *sect = w->getSector(u, v);

	if (sect == nullptr)
		return;

	if (!sect->isActivated()) {
		obj->deactivate();
		return;
	}
}

// ------------------------------------------------------------------------

bool ProtoObj::canFitBulkwise(GameObject *, GameObject *) {
	return false;
}

// ------------------------------------------------------------------------

bool ProtoObj::canFitMasswise(GameObject *, GameObject *) {
	return false;
}

// ------------------------------------------------------------------------
//	Return the maximum mass capacity for the specified container

uint16 ProtoObj::massCapacity(GameObject *) {
	return 0;
}

// ------------------------------------------------------------------------
//	Return the maximum bulk capacity for the specified container

uint16 ProtoObj::bulkCapacity(GameObject *) {
	return 0;
}

/* ==================================================================== *
   InventoryProto class
 * ==================================================================== */

uint16 InventoryProto::containmentSet() {
	return kIsTangible;
}

bool InventoryProto::takeAction(ObjectID dObj, ObjectID enactor, int16 num) {
	g_vm->_mouseInfo->copyObject(dObj, GrabInfo::kIntDrop, num);
	return true;
}

bool InventoryProto::canDropAt(
    ObjectID,
    ObjectID        enactor,
    const Location  &loc) {
	assert(enactor != Nothing);

	//  If we're not dropping it onto a world, we're okay
	if (!isWorld(loc._context)) return true;

	GameObject      *enactorPtr = GameObject::objectAddress(enactor);

	//  If we're trying to drop it into a different world or if
	//  we're dropping it more than 4 metatile widths away from the
	//  enactor, fail
	if (enactorPtr->IDParent() != loc._context
	        || (loc - enactorPtr->getLocation()).quickHDistance()
	        >   kTileUVSize * kPlatformWidth * 4)
		return false;

	return true;
}

bool InventoryProto::dropAction(
    ObjectID        dObj,
    ObjectID        enactor,
    const Location  &loc,
    int16           num) {
	assert(loc._context != Nothing);
	assert(dObj != Nothing);
	assert(enactor != Nothing);

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	Actor       *enactorPtr = (Actor *)GameObject::objectAddress(enactor);

	/*      //  Determine if we're dropping an object from the actor's hands.
	    if ( enactor != loc._context )
	    {
	        if ( dObj == enactorPtr->rightHandObject )
	            enactorPtr->rightHandObject = Nothing;
	        if ( dObj == enactorPtr->leftHandObject )
	            enactorPtr->leftHandObject = Nothing;
	    }
	*/
	//  If this object is on a TAG release it
	if (dObjPtr->_data.currentTAG != NoActiveItem) {
		ActiveItem::activeItemAddress(dObjPtr->_data.currentTAG)->release(
		    enactor, dObj);
		dObjPtr->_data.currentTAG = NoActiveItem;
	}

	if (isWorld(loc._context)) {
		ProtoObj            *enactorProto = enactorPtr->proto();
		TilePoint           enactorLoc(enactorPtr->getLocation());
		TilePoint           vector = loc - enactorLoc;
		GameObject          *extractedObj = nullptr;

		//  Split the merged object if needed.
		if (dObjPtr->isMergeable()           //  If mergeable
		        &&  num < dObjPtr->getExtra()) {    //  And not dropping whole pile
			if (num == 0) return false;         //  If mergeing zero, then do nothing

			extractedObj = dObjPtr->extractMerged(dObjPtr->getExtra() - num);
			if (extractedObj == nullptr)
				return false;

			extractedObj->move(
			    Location(dObjPtr->getLocation(), dObjPtr->IDParent()));
		}

		if (enactorPtr->inReach(loc)) {
			dObjPtr->move(loc);

			//  Make sure the game engine knows that it may scavenge this
			//  object if necessary
			if (!dObjPtr->isImportant())
				dObjPtr->setScavengable(true);
		} else {
			int16       offsetDist = enactorProto->crossSection + crossSection;
			Direction   vectorDir = vector.quickDir();
			int16       mapNum = enactorPtr->getMapNum();
			TilePoint   startPt = Nowhere;
			int         i;

			static const int8 dirOffsetTable[] = { 0, 1, -1, 2, -2, 3, -3 };

			for (i = 0; i < ARRAYSIZE(dirOffsetTable); i++) {
				TilePoint       testPt;
				Direction       testDir;

				testDir = (vectorDir + dirOffsetTable[i]) & 0x7;
				testPt = enactorLoc + incDirTable[testDir] * offsetDist;
				testPt.z += enactorProto->height >> 1;
				if (checkBlocked(dObjPtr, mapNum, testPt) == kBlockageNone) {
					startPt = testPt;
					break;
				}
			}

			if (startPt == Nowhere) {
				if (extractedObj != nullptr)
					GameObject::mergeWith(extractedObj, dObjPtr, extractedObj->getExtra());
				return false;
			}

			dObjPtr->move(Location(startPt, loc._context));

			//  Make sure the game engine knows that it may scavenge this
			//  object if necessary
			if (!dObjPtr->isImportant())
				dObjPtr->setScavengable(true);

			MotionTask::throwObjectTo(*dObjPtr, loc);
		}
	} else {
		GameObject  *targetObj = GameObject::objectAddress(loc._context);

		return targetObj->acceptInsertionAt(enactor, dObj, loc, num);
	}

	return true;
}

bool InventoryProto::dropOnAction(
    ObjectID        dObj,
    ObjectID        enactor,
    ActiveItem      *target,
    const Location  &loc,
    int16           num) {
	assert(dObj != Nothing);
	assert(target != nullptr);
	assert(isWorld(loc._context));

	if (drop(dObj, enactor, loc, num)) {
		GameObject  *dObjPtr = GameObject::objectAddress(dObj);

		//  If we weren't thrown, try triggering the TAG
		if (!dObjPtr->isMoving() && target->trigger(enactor, dObj))
			dObjPtr->_data.currentTAG = target->thisID();

		return true;
	}

	return false;
}

//  Virtual function to handle the effects of an object being dropped
//  on this object
bool InventoryProto::acceptDropAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID droppedObj,
    int count) {
	GameObject  *dropObject     = GameObject::objectAddress(droppedObj);
	GameObject  *targetObject   = GameObject::objectAddress(dObj);
	int         mergeState      = GameObject::canStackOrMerge(dropObject, targetObject);

	if (mergeState == kCanMerge)
		return targetObject->merge(enactor, droppedObj, count);
	else if (mergeState == kCanStack)
		return targetObject->stack(enactor, droppedObj);

	return ProtoObj::acceptDropAction(dObj, enactor, droppedObj, count);
}

bool InventoryProto::acceptStrikeAction(
    ObjectID            dObj,
    ObjectID            enactor,
    ObjectID            strikingObj,
    uint8) {
	assert(isObject(dObj) || isActor(dObj));
	assert(isActor(enactor));
	assert(isObject(strikingObj) || isActor(strikingObj));

	GameObject      *weapon = GameObject::objectAddress(strikingObj);

	return weapon->damage(enactor, dObj);
}

/* ==================================================================== *
   PhysicalContainerProto class
 * ==================================================================== */

//void PhysicalContainerProto::setTotalRows(ContainerView *cv)
//{
//	cv->totalRows =
//
//	};

uint16 PhysicalContainerProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsContainer;
}

bool PhysicalContainerProto::canContain(ObjectID dObj, ObjectID item) {
	GameObject *itemPtr = GameObject::objectAddress(item);
	GameObject *pPtr;

	//  Add recursive check: Make sure that the container isn't already
	//  inside of "item". Do this by looking at all of the ancestors of
	//  dObj and make sure that none of them equal "item".
	for (pPtr = GameObject::objectAddress(dObj); pPtr; pPtr = pPtr->parent()) {
		if (pPtr == itemPtr) return false;
	}

	return      dObj != item
	            && (itemPtr->containmentSet() & ProtoObj::kIsTangible);
}

bool PhysicalContainerProto::canContainAt(
    ObjectID dObj,
    ObjectID item,
    const TilePoint &where) {
	if (canContain(dObj, item) == false) return false;
	return true;
}

bool PhysicalContainerProto::useAction(ObjectID dObj, ObjectID enactor) {
	bool          result;
	GameObject    *dObjPtr = GameObject::objectAddress(dObj);

	if (dObjPtr->_data.objectFlags & kObjectOpen)
		result = close(dObj, enactor);
	else
		result = open(dObj, enactor);

	return result;
}

//  Determine if this object can be opened
bool PhysicalContainerProto::canOpen(ObjectID dObj, ObjectID) {
	GameObject *dObjPtr = GameObject::objectAddress(dObj);

	return !dObjPtr->isLocked() && !dObjPtr->isOpen();
}

//  Open a physical container
bool PhysicalContainerProto::openAction(ObjectID dObj, ObjectID) {
	ContainerNode   *cn;

	GameObject *dObjPtr = GameObject::objectAddress(dObj);

	assert(!dObjPtr->isOpen() && !dObjPtr->isLocked());

	cn = CreateContainerNode(dObj, false);
	cn->markForShow();                                      //  Deferred open
	dObjPtr->_data.objectFlags |= kObjectOpen;         //  Set open bit;
	g_vm->_cnm->setUpdate(dObjPtr->IDParent());

	return true;
}

bool PhysicalContainerProto::closeAction(ObjectID dObj, ObjectID) {
	GameObject      *dObjPtr = GameObject::objectAddress(dObj);
	ContainerNode   *cn = g_vm->_cnm->find(dObj, ContainerNode::kPhysicalType);

	assert(dObjPtr->isOpen());
	assert(cn);

	//  Delete the container (lazy delete)
	cn->markForDelete();

	//  Clear open bit
	dObjPtr->_data.objectFlags &= ~kObjectOpen;
	g_vm->_cnm->setUpdate(dObjPtr->IDParent());

	return true;
}

//  Determine if this object's lock can be toggled
bool PhysicalContainerProto::canToggleLock(
    ObjectID    dObj,
    ObjectID    enactor,
    uint8       keyCode) {
	GameObject *dObjPtr = GameObject::objectAddress(dObj);

	return keyCode == lockType && !dObjPtr->isOpen();
}

//  Unlock or lock the physical container
bool PhysicalContainerProto::acceptLockToggleAction(
    ObjectID    dObj,
    ObjectID    enactor, uint8) {
	GameObject *dObjPtr = GameObject::objectAddress(dObj);

	//  Toggle locked bit
	dObjPtr->_data.objectFlags ^= kObjectLocked;

	return true;
}

//  Insert another object into this object
bool PhysicalContainerProto::acceptInsertionAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID item,
    int16 num) {
	assert(isObject(dObj));
	assert(isObject(item));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	GameObject  *itemPtr = GameObject::objectAddress(item);

	//  Place the object in the container (if possible)
	if ((dObjPtr->_data.objectFlags & kObjectLocked)
	        ||  !dObjPtr->placeObject(enactor, item, true, num)) {
		if (isWorld(dObjPtr->IDParent()))
			dObjPtr->dropInventoryObject(itemPtr, num);
		else {
			if (!itemPtr->dropOn(enactor, dObjPtr->IDParent(), num))
				return false;
		}
	}

	return true;
}

//  Insert another object into this object at the specified slot
bool PhysicalContainerProto::acceptInsertionAtAction(
    ObjectID        dObj,
    ObjectID,
    ObjectID        item,
    const TilePoint &where,
    int16           num) {
	assert(isObject(dObj));
	assert(isObject(item));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	GameObject  *itemPtr = GameObject::objectAddress(item);
	GameObject  *extractedObj = nullptr;
	Location    oldLoc(itemPtr->getLocation(), itemPtr->IDParent());

	//  Split the merged object if needed.
	if (itemPtr->isMergeable()           //  If mergeable
	        &&  num < itemPtr->getExtra()) {    //  And not dropping whole pile
		if (num == 0) return false;         //  If mergeing zero, then do nothing

		extractedObj = itemPtr->extractMerged(itemPtr->getExtra() - num);
		if (extractedObj == nullptr)
			return false;

		extractedObj->move(oldLoc);
	}

	itemPtr->move(Location(0, 0, 0, ImportantLimbo));
	if (dObjPtr->canFitBulkwise(itemPtr)
	        &&  dObjPtr->canFitMasswise(itemPtr)) {
		itemPtr->move(Location(where, dObj));
		return true;
	}
	itemPtr->move(oldLoc);
	if (extractedObj != nullptr)
		GameObject::mergeWith(extractedObj, itemPtr, extractedObj->getExtra());

	return false;
}

// ------------------------------------------------------------------------
//	Determine if the specified object can fit within the specified container
//	based upon bulk

bool PhysicalContainerProto::canFitBulkwise(GameObject *container, GameObject *obj) {
#if DEBUG
	if (massAndBulkCount)
#endif
	{
		uint16                  maxBulk = container->bulkCapacity();
		uint16                  totalBulk = container->totalContainedBulk();

		return totalBulk + obj->totalBulk() <= maxBulk;
	}

#if DEBUG
	return true;
#endif
}

// ------------------------------------------------------------------------
//	Determine if the specified object can fit within the specified container
//	based upon mass

bool PhysicalContainerProto::canFitMasswise(GameObject *container, GameObject *obj) {
#if DEBUG
	if (massAndBulkCount)
#endif
	{
		if (!isWorld(container->IDParent()))
			return container->parent()->canFitMasswise(obj);

		return true;
	}

#if DEBUG
	return true;
#endif
}

// ------------------------------------------------------------------------
//	Return the maximum mass capacity for the specified container

uint16 PhysicalContainerProto::massCapacity(GameObject *container) {
	if (!isWorld(container->IDParent()))
		return container->parent()->massCapacity();

	return unlimitedCapacity;
}

// ------------------------------------------------------------------------
//	Return the maximum bulk capacity for the specified container

uint16 PhysicalContainerProto::bulkCapacity(GameObject *) {
	return bulk * 4;
}

/* ==================================================================== *
   KeyProto class
 * ==================================================================== */

//  Put key into mouse with intention to use
bool KeyProto::setUseCursor(ObjectID dObj) {
	assert(g_vm->_mouseInfo->getObjectId() == Nothing);
	g_vm->_mouseInfo->copyObject(GameObject::objectAddress(dObj), GrabInfo::kIntUse);
	return true;
}

//  Send acceptLockToggle message to container
bool KeyProto::useOnAction(ObjectID dObj, ObjectID enactor, ObjectID withObj) {
	GameObject *container = GameObject::objectAddress(withObj);

	if (!container->acceptLockToggle(enactor, lockType)) {
		return false;
	}

	return true;
}

//  Send acceptLockToggle message to active terrain
bool KeyProto::useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *withTAI) {
	GameObject *thisKey   = GameObject::objectAddress(dObj);

	int16 keyID = thisKey->_data.massCount > 0 ? thisKey->_data.massCount : lockType;

	if (!withTAI->acceptLockToggle(enactor, keyID)) {
//		WriteStatusF( 3, "%s doesn't work", thisKey->objName() );
		return false;
	}

	return true;
}

/* ==================================================================== *
   BottleProto class
 * ==================================================================== */

uint16 BottleProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsBottle;
}

bool BottleProto::useAction(ObjectID dObj, ObjectID enactor) {
	//Set Up Empty Bottle Sprite

	return true;
}

/* ==================================================================== *
   FoodProto class
 * ==================================================================== */

uint16 FoodProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsFood;
}

bool FoodProto::useAction(ObjectID dObj, ObjectID enactor) {
	return true;
}

/* ==================================================================== *
   WearableProto class
 * ==================================================================== */

uint16 WearableProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsWearable;
}

/* ==================================================================== *
   WeaponProto class
 * ==================================================================== */

weaponID WeaponProto::getWeaponID() {
	return weaponDamage;
}

uint16 WeaponProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsWeapon;
}

//  return the address of the sprite when held in hand
Sprite *WeaponProto::getOrientedSprite(GameObject *obj, int16 offset) {
	return weaponSprites[heldSpriteBase]->sprite(offset);
}

//-----------------------------------------------------------------------
//	Returns true if object in continuous use.

bool WeaponProto::isObjectBeingUsed(GameObject *obj) {
	ObjectID        wielder = obj->possessor();

	if (wielder != Nothing) {
		Actor *a = (Actor *)GameObject::objectAddress(wielder);

		if (a->_rightHandObject == obj->thisID()
		        ||  a->_leftHandObject == obj->thisID())
			return true;
	}
	return false;
}

/* ==================================================================== *
   MeleeWeaponProto
 * ==================================================================== */

//  Place weapon into right hand
bool MeleeWeaponProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));
	assert(isActor(enactor));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);

	if (enactor != dObjPtr->IDParent()) return false;

	if (dObj == a->_rightHandObject)
		a->holdInRightHand(Nothing);
	else {
		GameObject      *leftHandObjectPtr;

		leftHandObjectPtr = a->_leftHandObject != Nothing
		                    ?   GameObject::objectAddress(a->_leftHandObject)
		                    :   nullptr;

		if (dObjPtr->proto()->isTwoHanded(enactor)
		        || (leftHandObjectPtr != nullptr
		            &&  leftHandObjectPtr->proto()->isTwoHanded(enactor)))
			a->holdInLeftHand(Nothing);

		a->holdInRightHand(dObj);
	}

	return true;
}

bool MeleeWeaponProto::useOnAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID item) {
	if (item == enactor)   //If Trying To Hurt Oneself Stop It!!!
		return false;


	return strike(dObj, enactor, item);
}

bool MeleeWeaponProto::strikeAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID item) {
	assert(isObject(dObj));
	assert(isActor(enactor));
	assert(isObject(item) || isActor(item));

	GameObject      *itemPtr = GameObject::objectAddress(item);
	ObjectSoundFXs  *soundFXs;
	Actor           *a = (Actor *)GameObject::objectAddress(enactor);
	Location        ol = Location(a->getWorldLocation(), a->IDParent());

	if (itemPtr->acceptStrike(enactor, dObj, getSkillValue(enactor)))
		return true;

	soundFXs = &objectSoundFXTable[soundFXClass];

	makeCombatSound(soundFXs->soundFXMissed, ol);
	return false;
}

bool MeleeWeaponProto::damageAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID target) {
	assert(isObject(dObj));
	assert(isActor(enactor));
	assert(isObject(target) || isActor(target));

	Actor           *a = (Actor *)GameObject::objectAddress(enactor);
	ActorAttributes *effStats = a->getStats();
	WeaponStuff     *ws = &getWeapon(getWeaponID());
	GameObject      *targetPtr = GameObject::objectAddress(target);
	uint8           damageSoundID;
	Location        ol = Location(a->getWorldLocation(), a->IDParent());

	damageSoundID = targetPtr->proto()->getDamageSound(
	                    objectSoundFXTable[soundFXClass]);

	if (damageSoundID != 0)
		makeCombatSound(damageSoundID, ol);

	ws->implement(
	    a,
	    targetPtr,
	    GameObject::objectAddress(dObj),
	    effStats->getSkillLevel(kSkillIDBrawn));

	return true;
}

bool MeleeWeaponProto::acceptDamageAction(
    ObjectID,
    ObjectID,
    int8,
    effectDamageTypes   dType,
    int8,
    uint8,
    int8) {
	return true;
}

//  Determine if this type of weapon must be wielded with two hands
//  for the specified actor
bool MeleeWeaponProto::isTwoHanded(ObjectID attackerID) {
	assert(isActor(attackerID));

	Actor       *attackerPtr = (Actor *)GameObject::objectAddress(attackerID);
	ActorProto  *attackerProto = (ActorProto *)attackerPtr->proto();

	//  This weapon must be wielded in two hands if its bulk is greater
	//  than a quarter of the bulk of the wielder or if the actor does not
	//  have one handed fighing animation.
	return      !attackerPtr->isActionAvailable(kActionSwingHigh)
	            ||  bulk > attackerProto->bulk / 4;
}

//  Initiate a melee weapon attack motion
void MeleeWeaponProto::initiateAttack(ObjectID attacker, ObjectID target) {
	assert(isActor(attacker));
	assert(isObject(target) || isActor(target));

	Actor       *attackerPtr = (Actor *)GameObject::objectAddress(attacker);
	GameObject  *targetPtr = GameObject::objectAddress(target);

	//  Start the attack motion
	if (isTwoHanded(attacker))
		MotionTask::twoHandedSwing(*attackerPtr, *targetPtr);
	else
		MotionTask::oneHandedSwing(*attackerPtr, *targetPtr);
}

//  Initiate a melee weapon parry motion
void MeleeWeaponProto::initiateDefense(
    ObjectID defensiveObj,
    ObjectID defender,
    ObjectID attacker) {
	assert(isObject(defensiveObj));
	assert(isActor(defender));
	assert(isActor(attacker));

	GameObject  *weapon = GameObject::objectAddress(defensiveObj);
	Actor       *defenderPtr = (Actor *)GameObject::objectAddress(defender),
	             *attackerPtr = (Actor *)GameObject::objectAddress(attacker);

	if (isTwoHanded(defender))
		MotionTask::twoHandedParry(*defenderPtr, *weapon, *attackerPtr);
	else
		MotionTask::oneHandedParry(*defenderPtr, *weapon, *attackerPtr);
}

//  Melee weapons can block an attack
bool MeleeWeaponProto::canBlock() {
	return true;
}

//  Return a mask of bits indicating the directions relative to the
//  wielders facing in which this object can defend
uint8 MeleeWeaponProto::defenseDirMask() {
	return 1 << kDirUp;
}

//-----------------------------------------------------------------------
//	Rate this weapon's goodness for a specified attack situation

uint8 MeleeWeaponProto::weaponRating(
    ObjectID weaponID_,
    ObjectID wielderID,
    ObjectID targetID) {
	assert(isActor(wielderID));
	assert(isObject(targetID) || isActor(targetID));

	Actor       *wielder = (Actor *)GameObject::objectAddress(wielderID);

	//  If the wielder is on screen yet does not have the attack frames
	//  for this weapon then this weapon is useless
	if (wielder->_appearance != nullptr
	        &&  !wielder->isActionAvailable(fightStanceAction(wielderID)))
		return 0;

	GameObject  *target = GameObject::objectAddress(targetID);
	int16       dist = (target->getLocation() - wielder->getLocation()).quickHDistance();
	uint8       rating = 0;

	if (dist < maximumRange) rating += kInRangeRatingBonus;
	//  Add in the value of the appropriate skill1
	rating += getSkillValue(wielderID);

	return rating;
}

// ------------------------------------------------------------------------
//	Return the fight stance approriate to this weapon

int16 MeleeWeaponProto::fightStanceAction(ObjectID actor) {
	return isTwoHanded(actor) ? kActionTwoHandSwingHigh : kActionSwingHigh;
}

// ------------------------------------------------------------------------
//	Given an object sound effect record, which sound should be made
//	when this object is damaged

uint8 MeleeWeaponProto::getDamageSound(const ObjectSoundFXs &soundFXs) {
	return soundFXs.soundFXParried;
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool MeleeWeaponProto::useSlotAvailable(GameObject *obj, Actor *a) {
	assert(isObject(obj) && obj->proto() == this);
	assert(isActor(a));

	if (a->_rightHandObject == Nothing) {
		if (a->_leftHandObject != Nothing) {
			assert(isObject(a->_leftHandObject));

			GameObject      *leftHandObjectPtr;

			leftHandObjectPtr = GameObject::objectAddress(a->_leftHandObject);
			return      !isTwoHanded(a->thisID())
			            &&  !leftHandObjectPtr->proto()->isTwoHanded(a->thisID());
		}
		return true;
	}
	assert(isObject(a->_rightHandObject));

	return false;
}

/* ==================================================================== *
   BludgeoningWeaponProto
 * ==================================================================== */

//-----------------------------------------------------------------------
//	Get the value of the wielder's skill which applies to this weapon

uint8 BludgeoningWeaponProto::getSkillValue(ObjectID enactor) {
	assert(isActor(enactor));

	Actor               *a;
	ActorAttributes     *effStats;

	a = (Actor *)GameObject::objectAddress(enactor);
	effStats = a->getStats();

	return effStats->getSkillLevel(kSkillIDBludgeon);
}

// ------------------------------------------------------------------------
//	Cause the user's associated skill to grow

void BludgeoningWeaponProto::applySkillGrowth(ObjectID enactor, uint8 points) {
	assert(isActor(enactor));

	PlayerActorID       playerID;

	if (actorIDToPlayerID(enactor, playerID)) {
		PlayerActor     *player = getPlayerActorAddress(playerID);

		player->skillAdvance(kSkillIDBludgeon, points);

		if (g_vm->_rnd->getRandomNumber(1))
			player->skillAdvance(kSkillIDBrawn, points);
	}
}

/* ==================================================================== *
   SlashingWeaponProto
 * ==================================================================== */

//-----------------------------------------------------------------------
//	Get the value of the wielder's skill which applies to this weapon

uint8 SlashingWeaponProto::getSkillValue(ObjectID enactor) {
	assert(isActor(enactor));

	Actor               *a;
	ActorAttributes     *effStats;

	a = (Actor *)GameObject::objectAddress(enactor);
	effStats = a->getStats();

	return effStats->getSkillLevel(kSkillIDSwordcraft);
}

// ------------------------------------------------------------------------
//	Cause the user's associated skill to grow

void SlashingWeaponProto::applySkillGrowth(ObjectID enactor, uint8 points) {
	assert(isActor(enactor));

	PlayerActorID       playerID;

	if (actorIDToPlayerID(enactor, playerID)) {
		PlayerActor     *player = getPlayerActorAddress(playerID);

		player->skillAdvance(kSkillIDSwordcraft, points);

		if (g_vm->_rnd->getRandomNumber(1))
			player->skillAdvance(kSkillIDBrawn, points);
	}
}

/* ==================================================================== *
   BowProto
 * ==================================================================== */

bool BowProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));
	assert(isActor(enactor));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);

	if (enactor != dObjPtr->IDParent()) return false;

	//  If this object is in the enactor's left hand remove it else
	//  place it into his left hand
	if (a->_leftHandObject == dObj)
		a->holdInLeftHand(Nothing);
	else {
		a->holdInRightHand(Nothing);
		a->holdInLeftHand(dObj);
	}

	return true;
}

//  Bows are two handed
bool BowProto::isTwoHanded(ObjectID) {
	return true;
}

//  Initiate the bow firing motion
void BowProto::initiateAttack(ObjectID attacker, ObjectID target) {
	assert(isActor(attacker));
	assert(isObject(target) || isActor(target));

	Actor       *attackerPtr = (Actor *)GameObject::objectAddress(attacker);
	GameObject  *targetPtr = GameObject::objectAddress(target);

	MotionTask::fireBow(*attackerPtr, *targetPtr);
}

//  Grab an arrow from the actor's inventory
GameObject *BowProto::getProjectile(ObjectID weapon, ObjectID enactor) {
	assert(isObject(weapon));
	assert(isActor(enactor));

	GameObject          *obj = nullptr,
	                    *arrow = nullptr;
	Actor               *a = (Actor *)GameObject::objectAddress(enactor);
	TilePoint           bestSlot(maxint16, maxint16, 0);
	ContainerIterator   iter(a);

	while (iter.next(&obj) != Nothing) {
		//  Look for objects which are arrows
		if (obj->proto()->classType == protoClassArrow) {
			TilePoint   objSlot = obj->getLocation();

			if (objSlot.u < bestSlot.u
			        || (objSlot.u == bestSlot.u
			            &&  objSlot.v < bestSlot.v)) {
				bestSlot = objSlot;
				arrow = obj;
			}
		}
	}

	return arrow;
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool BowProto::useSlotAvailable(GameObject *obj, Actor *a) {
	assert(isObject(obj) && obj->proto() == this);
	assert(isActor(a));

	return a->_leftHandObject == Nothing && a->_rightHandObject == Nothing;
}

//-----------------------------------------------------------------------
//	Rate this weapon's goodness for a specified attack situation

uint8 BowProto::weaponRating(
    ObjectID weaponID_,
    ObjectID wielderID,
    ObjectID targetID) {
	assert(isActor(wielderID));
	assert(isObject(targetID) || isActor(targetID));

	if (getProjectile(weaponID_, wielderID) == nullptr) return 0;

	Actor       *wielder = (Actor *)GameObject::objectAddress(wielderID);

	//  If the wielder is on screen yet does not have the attack frames
	//  for this weapon then this weapon is useless
	if (wielder->_appearance != nullptr
	        &&  !wielder->isActionAvailable(fightStanceAction(wielderID)))
		return 0;

	GameObject  *target = GameObject::objectAddress(targetID);
	int16       dist = (target->getLocation() - wielder->getLocation()).quickHDistance();
	uint8       rating = 0;

	if (dist < maximumRange && !wielder->inReach(target->getLocation()))
		rating += kInRangeRatingBonus;
	rating += wielder->getStats()->getSkillLevel(kSkillIDArchery);

	return rating;
}

// ------------------------------------------------------------------------
//	Return the fight stance approriate to this weapon

int16 BowProto::fightStanceAction(ObjectID actor) {
	return kActionFireBow;
}

/* ==================================================================== *
   WeaponWandProto
 * ==================================================================== */

bool WeaponWandProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));
	assert(isActor(enactor));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);

	if (enactor != dObjPtr->IDParent()) return false;

	//  If this object is in the enactor's left hand remove it else
	//  place it into his left hand
	if (a->_leftHandObject == dObj)
		a->holdInLeftHand(Nothing);
	else {
		a->holdInRightHand(Nothing);
		a->holdInLeftHand(dObj);
	}

	return true;
}

//  Wands are two handed
bool WeaponWandProto::isTwoHanded(ObjectID) {
	return true;
}

//  Initiate the use wand motion
void WeaponWandProto::initiateAttack(ObjectID attacker, ObjectID target) {
	assert(isActor(attacker));
	assert(isObject(target) || isActor(target));

	Actor       *attackerPtr = (Actor *)GameObject::objectAddress(attacker);
	GameObject  *targetPtr = GameObject::objectAddress(target);

	MotionTask::useWand(*attackerPtr, *targetPtr);
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool WeaponWandProto::useSlotAvailable(GameObject *obj, Actor *a) {
	assert(isObject(obj) && obj->proto() == this);
	assert(isActor(a));

	return a->_leftHandObject == Nothing && a->_rightHandObject == Nothing;
}

//-----------------------------------------------------------------------
//	Rate this weapon's goodness for a specified attack situation

uint8 WeaponWandProto::weaponRating(
    ObjectID weaponID_,
    ObjectID wielderID,
    ObjectID targetID) {
	assert(isObject(weaponID_) || isActor(weaponID_));
	assert(isActor(wielderID));
	assert(isObject(targetID) || isActor(targetID));

	Actor       *wielder = (Actor *)GameObject::objectAddress(wielderID);

	//  If the wielder is on screen yet does not have the attack frames
	//  for this weapon then this weapon is useless
	if (wielder->_appearance != nullptr
	        &&  !wielder->isActionAvailable(fightStanceAction(wielderID)))
		return 0;

	GameObject  *weapon = GameObject::objectAddress(weaponID_),
	             *target = GameObject::objectAddress(targetID);
	int16       dist = (target->getLocation() - wielder->getLocation()).quickHDistance();
	uint8       rating = 0;

	if (weapon->IDChild() != Nothing) {
		SkillProto  *spellProto = (SkillProto *)GameObject::protoAddress(weapon->IDChild());
		SpellStuff  *spellStuff = &spellBook[spellProto->getSpellID()];
		ActorManaID manaType = (ActorManaID)spellStuff->getManaType();
		uint16      manaAmount = spellStuff->getManaAmt();

		if (!weapon->hasCharge(manaType, manaAmount)) return 0;
	} else
		return 0;

	if (dist < maximumRange && !wielder->inReach(target->getLocation()))
		rating += kInRangeRatingBonus;
	rating += wielder->getStats()->getSkillLevel(kSkillIDSpellcraft);

	return rating;
}

// ------------------------------------------------------------------------
//	Return the fight stance approriate to this weapon

int16 WeaponWandProto::fightStanceAction(ObjectID actor) {
	return kActionUseWand;
}

/* ==================================================================== *
   ProjectileProto class
 * ==================================================================== */

//-----------------------------------------------------------------------
//	return the address of the sprite when held in hand

Sprite *ProjectileProto::getOrientedSprite(GameObject *, int16) {
	return nullptr;
}

//-----------------------------------------------------------------------
//	Returns true if object in continuous use.

bool ProjectileProto::isObjectBeingUsed(GameObject *) {
	return false;
}

//-----------------------------------------------------------------------
//	Rate this weapon's goodness for a specified attack situation

uint8 ProjectileProto::weaponRating(
    ObjectID weaponID_,
    ObjectID wielderID,
    ObjectID targetID) {
	//  Projectiles are worthless as far as wieldable weapons
	return 0;
}

//  Projectiles are missiles
bool ProjectileProto::isMissile() {
	return true;
}

/* ==================================================================== *
   ArrowProto class
 * ==================================================================== */

bool ArrowProto::useOnAction(ObjectID dObj, ObjectID enactor, ObjectID item) {
	//  Reuse the MeleeWeaponProto's useOnAction() function.
	return ((MeleeWeaponProto *)this)->MeleeWeaponProto::useOnAction(
	           dObj, enactor, item);
}

bool ArrowProto::strikeAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID item) {
	assert(isObject(dObj));
	assert(isActor(enactor));
	assert(isObject(item) || isActor(item));

	Actor           *a = (Actor *)GameObject::objectAddress(enactor);
	GameObject      *itemPtr = GameObject::objectAddress(item);

	return  itemPtr->acceptStrike(
	            enactor,
	            dObj,
	            a->getStats()->getSkillLevel(kSkillIDArchery));
}

bool ArrowProto::damageAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID target) {
	assert(isObject(dObj));
	assert(isActor(enactor));
	assert(isObject(target) || isActor(target));

	Actor           *a = (Actor *)GameObject::objectAddress(enactor);
	ActorAttributes *effStats = a->getStats();
	WeaponStuff     *ws = &getWeapon(getWeaponID());
	GameObject      *targetPtr = GameObject::objectAddress(target);
	uint8           damageSoundID;
	Location        al = Location(a->getLocation(), a->IDParent());

	damageSoundID = targetPtr->proto()->getDamageSound(
	                    objectSoundFXTable[soundFXClass]);

	if (damageSoundID != 0)
		makeCombatSound(damageSoundID, al);

	ws->implement(
	    a,
	    GameObject::objectAddress(target),
	    GameObject::objectAddress(dObj),
	    effStats->getSkillLevel(kSkillIDBrawn));

	return true;
}

// ------------------------------------------------------------------------
//	Cause the user's associated skill to grow

void ArrowProto::applySkillGrowth(ObjectID enactor, uint8 points) {
	assert(isActor(enactor));

	PlayerActorID       playerID;

	if (actorIDToPlayerID(enactor, playerID)) {
		PlayerActor     *player = getPlayerActorAddress(playerID);

		player->skillAdvance(kSkillIDArchery, points);

		if (g_vm->_rnd->getRandomNumber(1))
			player->skillAdvance(kSkillIDBrawn, points);
	}
}

/* ==================================================================== *
   ArmorProto class
 * ==================================================================== */

uint16 ArmorProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsWearable | kIsArmor;
}

//  Compute how much damage this defensive object will absorb
uint8 ArmorProto::adjustDamage(uint8 damage) {
	//  Apply damage divider
	if (damageDivider != 0) damage /= damageDivider;

	//  Apply damage absorption
	if (damageAbsorbtion < damage)
		damage -= damageAbsorbtion;
	else
		damage = 0;

	return damage;
}

//-----------------------------------------------------------------------
//	Returns true if object in continuous use.

bool ArmorProto::isObjectBeingUsed(GameObject *obj) {
	ObjectID    aID = obj->possessor();

	if (aID != Nothing) {
		Actor       *a = (Actor *)GameObject::objectAddress(aID);
		ObjectID    id = obj->thisID();

		for (int i = 0; i < ARMOR_COUNT; i++) {
			if (a->_armorObjects[i] == id) return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool ArmorProto::useSlotAvailable(GameObject *obj, Actor *a) {
	assert(isObject(obj) || obj->proto() == this);
	assert(isActor(a));

	return a->_armorObjects[whereWearable] == Nothing;
}

//-----------------------------------------------------------------------
//	"Wear" a piece of armor.

bool ArmorProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));
	assert(isActor(enactor));

	Actor       *a = (Actor *)GameObject::objectAddress(enactor);
	GameObject  *obj = GameObject::objectAddress(dObj);

	assert(obj->proto() == this);

	if (enactor != obj->IDParent()) return false;

	int16       slot = whereWearable;

	if (a->_armorObjects[slot] == dObj)
		a->wear(Nothing, slot);
	else
		a->wear(dObj, slot);

	g_vm->_cnm->setUpdate(obj->IDParent());

	return true;
}

/* ==================================================================== *
   ShieldProto class
 * ==================================================================== */

uint16 ShieldProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsWearable | kIsArmor;
}

//  Place shield into left hand
bool ShieldProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));
	assert(isActor(enactor));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);

	if (enactor != dObjPtr->IDParent()) return false;

	if (a->_rightHandObject != Nothing) {
		assert(isObject(a->_rightHandObject));
		GameObject  *rightHandObjectPtr =
		    GameObject::objectAddress(a->_rightHandObject);

		if (rightHandObjectPtr->proto()->isTwoHanded(enactor))
			return false;
	}

	a->holdInLeftHand(dObj != a->_leftHandObject ? dObj : Nothing);

	return true;
}

bool ShieldProto::acceptDamageAction(
    ObjectID,
    ObjectID,
    int8,
    effectDamageTypes   dType,
    int8,
    uint8,
    int8) {
	return true;
}

Sprite *ShieldProto::getOrientedSprite(GameObject *obj, int16 offset) {
	return weaponSprites[heldSpriteBase]->sprite(offset);
}

//  Initiate a shield parrying motion
void ShieldProto::initiateDefense(
    ObjectID defensiveObj,
    ObjectID defender,
    ObjectID attacker) {
	assert(isObject(defensiveObj));
	assert(isActor(defender));
	assert(isActor(attacker));

	GameObject  *shield = GameObject::objectAddress(defensiveObj);
	Actor       *defenderPtr = (Actor *)GameObject::objectAddress(defender),
	             *attackerPtr = (Actor *)GameObject::objectAddress(attacker);

	MotionTask::shieldParry(*defenderPtr, *shield, *attackerPtr);
}


//  Shields can block an attack
bool ShieldProto::canBlock() {
	return true;
}

//  Return a mask of bits indicating the directions relative to the
//  wielders facing in which this object can defend
uint8 ShieldProto::defenseDirMask() {
	return (1 << kDirUp) | (1 << kDirUpLeft);
}

//-----------------------------------------------------------------------
//	Returns true if object in continuous use.

bool ShieldProto::isObjectBeingUsed(GameObject *obj) {
	ObjectID        wielder = obj->possessor();

	if (wielder != Nothing) {
		Actor *a = (Actor *)GameObject::objectAddress(wielder);

		if (a->_rightHandObject == obj->thisID()
		        ||  a->_leftHandObject == obj->thisID())
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------
//	Determine if the specified object's 'use' slot is available within the
//	specified actor

bool ShieldProto::useSlotAvailable(GameObject *obj, Actor *a) {
	assert(isObject(obj) || obj->proto() == this);
	assert(isActor(a));

	if (a->_leftHandObject == Nothing) {
		if (a->_rightHandObject != Nothing) {
			assert(isObject(a->_rightHandObject));

			GameObject      *rightHandObjectPtr;

			rightHandObjectPtr = GameObject::objectAddress(a->_rightHandObject);
			return !rightHandObjectPtr->proto()->isTwoHanded(a->thisID());
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Get the value of the user's skill which applies to this object

uint8 ShieldProto::getSkillValue(ObjectID enactor) {
	assert(isActor(enactor));

	Actor               *a;
	ActorAttributes     *effStats;

	a = (Actor *)GameObject::objectAddress(enactor);
	effStats = a->getStats();

	return effStats->getSkillLevel(kSkillIDShieldcraft);
}

// ------------------------------------------------------------------------
//	Cause the user's associated skill to grow

void ShieldProto::applySkillGrowth(ObjectID enactor, uint8 points) {
	assert(isActor(enactor));

	PlayerActorID       playerID;

	if (actorIDToPlayerID(enactor, playerID)) {
		PlayerActor     *player = getPlayerActorAddress(playerID);

		player->skillAdvance(kSkillIDShieldcraft, points);

		if (g_vm->_rnd->getRandomNumber(1))
			player->skillAdvance(kSkillIDBrawn, points);
	}
}

// ------------------------------------------------------------------------
//	Given an object sound effect record, which sound should be made
//	when this object is damaged

uint8 ShieldProto::getDamageSound(const ObjectSoundFXs &soundFXs) {
	return soundFXs.soundFXParried;
}

/* ==================================================================== *
   ToolProto class
 * ==================================================================== */

//  Put tool into mouse with intention to use
bool ToolProto::setUseCursor(ObjectID dObj) {
	assert(g_vm->_mouseInfo->getObjectId() == Nothing);
	g_vm->_mouseInfo->copyObject(GameObject::objectAddress(dObj), GrabInfo::kIntUse);
	return true;
}

bool ToolProto::useOnAction(ObjectID, ObjectID, ObjectID) {
	return true;
}

/* ==================================================================== *
   DocumentProto class
 * ==================================================================== */

uint16 DocumentProto::containmentSet() {
	return InventoryProto::containmentSet() | kIsDocument;
}

/* ==================================================================== *
   BookProto class
 * ==================================================================== */

bool BookProto::useAction(ObjectID dObj, ObjectID enactor) {
	GameObject  *bookObj = GameObject::objectAddress(dObj);

	// open the modal book with text filled in with the previously run script
	switch (appearanceType) {
	case 0:
		openBook(bookObj->getExtra());
		break;
	case 1:
		openScroll(bookObj->getExtra());
		break;
	case 2:
		openParchment(bookObj->getExtra());
		break;
	}

	return true;
}

/* ==================================================================== *
   ScrollProto class
 * ==================================================================== */

bool ScrollProto::useAction(ObjectID dObj, ObjectID enactor) {
	GameObject  *bookObj = GameObject::objectAddress(dObj);

	openScroll(bookObj->getExtra());     // open the modal book with text filled in with
	// the previously run script
	return true;
}

/* ==================================================================== *
   AutoMapProto class
 * ==================================================================== */

bool AutoMapProto::openAction(ObjectID, ObjectID) {
	return false;
}

/* ==================================================================== *
   IntangibleObjProto class
 * ==================================================================== */

uint16 IntangibleObjProto::containmentSet() {
	return kIsIntangible;
}

bool IntangibleObjProto::useAction(ObjectID dObj, ObjectID enactor) {
	assert(isObject(dObj));

	/*  GameObject *obj = GameObject::objectAddress(dObj);

	    if(obj->isAlias()) //This Tells Whether Its A Copy Or Not
	        grabTangible(dObj);//Set To Use If Its The Copy
	*/
	return false;
}

bool IntangibleObjProto::takeAction(ObjectID dObj, ObjectID enactor, int16) {
	assert(isObject(dObj));
	assert(g_vm->_mouseInfo->getObjectId() == Nothing);

	GameObject      *dObjPtr = GameObject::objectAddress(dObj);

	if (dObjPtr->isTrueSkill())
		return false;

	g_vm->_mouseInfo->copyObject(dObj);

	return true;
}

bool IntangibleObjProto::canDropAt(
    ObjectID,
    ObjectID,
    const Location  &loc) {
	//  We can try dropping this object anywhere, except within a world
	return !isWorld(loc._context);
}

bool IntangibleObjProto::dropAction(
    ObjectID        dObj,
    ObjectID        enactor,
    const Location  &loc,
    int16) {
	assert(isObject(dObj));
	assert(loc._context != Nothing);
	assert(!isWorld(loc._context));

	GameObject  *container = GameObject::objectAddress(loc._context);

	if (container->canContain(dObj)) {
		GameObject  *dObjPtr = GameObject::objectAddress(dObj);
		ObjectID    newObj;

		if ((newObj = dObjPtr->makeAlias(Location(dObjPtr->getLocation(), dObjPtr->IDParent()))) != Nothing) {
			if (container->acceptInsertionAt(enactor, newObj, loc))
				return true;
			else
				GameObject::objectAddress(newObj)->deleteObject();
		}
	}

	return false;
}

bool IntangibleObjProto::acceptDropAction(
    ObjectID dObj,
    ObjectID enactor,
    ObjectID droppedObj,
    int) {
	assert(isObject(dObj));

	GameObject  *dObjPtr = GameObject::objectAddress(dObj);

	if (dObjPtr->isAlias()) {
		GameObject      *droppedObjPtr =
		    GameObject::objectAddress(droppedObj);
		Location        loc(dObjPtr->getLocation(), dObjPtr->IDParent());

		dObjPtr->deleteObject();
		return droppedObjPtr->drop(enactor, loc);
	}

	return false;
}

ObjectID IntangibleObjProto::placeObject() {
	//return Container That It Inserted Itself Into
	return 2;
}

//  Builds an identity color translation table for intagible objects
void IntangibleObjProto::getColorTranslation(ColorTable map) {
	memcpy(map, identityColors, sizeof(ColorTable));
	/*
	    uint8 color = 0;

	    do {
	        map[color] = color;
	    } while ( ++color != 0 );
	*/
}

//  Return the sprite data
ObjectSpriteInfo IntangibleObjProto::getSprite(
    GameObject *obj,
    enum spriteTypes spr,
    int16) {
	ObjectSpriteInfo    sprInfo = { nullptr, false };

	switch (spr) {
	case kObjOnGround:
		sprInfo.sp = mentalSprites->sprite(groundSprite);
		break;

	case kObjInContainerView:
	case kObjAsMousePtr:
		sprInfo.sp = mentalSprites->sprite(iconSprite);
	}
	return sprInfo;
}


/* ==================================================================== *
   IdeaProto class
 * ==================================================================== */

uint16 IdeaProto::containmentSet() {
	//Maybe I Could Use This ID And Call IntanobjProt For Setting IsIntangible
	return kIsConcept | kIsIntangible;
}

/* ==================================================================== *
   MemoryProto class
 * ==================================================================== */

uint16 MemoryProto::containmentSet() {
	//Maybe I Could Use This ID And Call IntanobjProt For Setting IsIntangible
	return kIsConcept | kIsIntangible;
}

/* ==================================================================== *
   PsychProto class
 * ==================================================================== */

uint16 PsychProto::containmentSet() {
	//Maybe I Could Use This ID And Call IntanobjProt For Setting IsIntangible
	return kIsPsych | kIsIntangible;
}

/* ==================================================================== *
   SkillProto class
 * ==================================================================== */


uint16 SkillProto::containmentSet() {
	//Maybe I Could Use This ID And Call IntanobjProt For Setting IsIntangible
	return kIsSkill | kIsIntangible;
}

bool SkillProto::useAction(ObjectID dObj, ObjectID enactor) {
	if (nonUsable(this))
		return false;

	if (nonTargeted(this)) {
		Actor   *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
		return castUntargetedSpell(attackerPtr, this);
	}
	g_vm->_mouseInfo->copyObject(dObj, GrabInfo::kIntUse);
	return true;
}

// cast a skill at various things

bool SkillProto::useOnAction(ObjectID dObj, ObjectID enactor, ObjectID withObj) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	GameObject  *targetPtr = GameObject::objectAddress(withObj);
	//              *spellPtr = GameObject::objectAddress( dObj );
	return castSpell(attackerPtr, targetPtr, this);
}

bool SkillProto::useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *item) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	//GameObject    *spellPtr = GameObject::objectAddress( dObj );
	return castSpell(attackerPtr, item, this);
}

bool SkillProto::useOnAction(ObjectID dObj, ObjectID enactor, const Location &loc) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	Location    l = loc;
	//GameObject    *spellPtr = GameObject::objectAddress( dObj );
	return castSpell(attackerPtr, l, this);
}

bool SkillProto::canDropAt(ObjectID, ObjectID, const Location &) {
	return true;
}

bool SkillProto::dropAction(ObjectID dObj,  ObjectID enactor, const Location &loc, int16 num) {
	assert(isActor(enactor));

	if (isWorld(loc._context)) {
		Actor       *enactorPtr = (Actor *)GameObject::objectAddress(enactor);

		if (validTarget(enactorPtr, nullptr, nullptr, this))
			return useOn(dObj, enactor, loc);

		return false;
	}

	return IntangibleObjProto::dropAction(dObj, enactor, loc, num);
}

bool SkillProto::dropOnAction(ObjectID dObj, ObjectID enactor, ObjectID target, int count) {
	assert(isActor(enactor));
	assert(isObject(target) || isActor(target));

	GameObject      *targetPtr = GameObject::objectAddress(target);

	if (isWorld(targetPtr->IDParent())) {
		Actor       *enactorPtr = (Actor *)GameObject::objectAddress(enactor);

		if (validTarget(enactorPtr, targetPtr, nullptr, this))
			return useOn(dObj, enactor, target);
	}

	return IntangibleObjProto::dropOnAction(dObj, enactor, target, count);
}

bool SkillProto::dropOnAction(
    ObjectID        dObj,
    ObjectID        enactor,
    ActiveItem      *target,
    const Location  &loc,
    int16           num) {
	assert(isActor(enactor));

	if (target != nullptr) {
		Actor       *enactorPtr = (Actor *)GameObject::objectAddress(enactor);

		if (validTarget(enactorPtr, nullptr, target, this))
			return useOn(dObj, enactor, target);
	}

	return IntangibleObjProto::dropOnAction(dObj, enactor, target, loc, num);
}

bool SkillProto::implementAction(SpellID dObj, ObjectID enactor, ObjectID withObj) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	GameObject  *targetPtr = GameObject::objectAddress(withObj);
	//              *spellPtr = GameObject::objectAddress( dObj );
	return implementSpell(attackerPtr, targetPtr, this);
}

bool SkillProto::implementAction(SpellID dObj, ObjectID enactor, ActiveItem *item) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	//GameObject    *spellPtr = GameObject::objectAddress( dObj );
	return implementSpell(attackerPtr, item, this);
}

bool SkillProto::implementAction(SpellID dObj, ObjectID enactor, Location &loc) {
	Actor       *attackerPtr = (Actor *) GameObject::objectAddress(enactor);
	//GameObject    *spellPtr = GameObject::objectAddress( dObj );
	return implementSpell(attackerPtr, loc, this);
}


/* ==================================================================== *
   EnchantmentProto class
 * ==================================================================== */

uint16 EnchantmentProto::containmentSet() {
	return kIsEnchantment;
}

// ------------------------------------------------------------------------
//	Background update function, called once every few seconds

void EnchantmentProto::doBackgroundUpdate(GameObject *obj) {
	int16       hitPoints   = obj->getHitPoints();  // get hitpoints of enchant
	GameObject  *parentObj  = obj->parent();        // get parent of enchantment

	assert(parentObj);

	// if this is a poison enchantment
	// then hurt the victim
	if (parentObj && isActor(parentObj)) {
		// get the enchantment type
		uint16 flgs = obj->getExtra();
		uint16  type = getEnchantmentType(flgs),
		        subType = getEnchantmentSubType(flgs);

		if (type == kEffectOthers && subType == kActorPoisoned) {
			// get the damage amount for this poison
			int16 damage = getEnchantmentAmount(flgs);

			// apply the damage
			parentObj->acceptDamage(obj->thisID(), damage, kDamagePoison);
		}
	}

	// if the enchantment does not hemorage
	// away mana, it does'nt go away ( oh no mr.bill! )
	if (hitPoints == Forever) return;

	hitPoints -= 1;
	if (hitPoints <= 0) {
		// delete the now dead enchantment
		obj->deleteObject();

		if (parentObj) {
			parentObj->evalEnchantments();
		}
	} else {
		obj->setHitPoints(hitPoints);
	}
}

/* ======================================================================== *
   GeneratorProto
 * ======================================================================== */

uint16 GeneratorProto::containmentSet() {
	return kIsIntangible;
}

/* ======================================================================== *
   MonsterGeneratorProto
 * ======================================================================== */

/* ======================================================================== *
   EncounterGeneratorProto
 * ======================================================================== */

//  Generate an encounter at approx. 10-second intervals
void EncounterGeneratorProto::doBackgroundUpdate(GameObject *obj) {
	Actor           *centerActor = getCenterActor();
	PlayerActor     *playerActor;

	Location        generatorLoc;           //  World position of this object
	LivingPlayerActorIterator iter;

	//  REM: Add code to disable this generator if there are too many
	//  hostiles on the screen

	//  Get the world that the object is in, and the location of the object
	//  within the world.
	obj->getWorldLocation(generatorLoc);

	//  Iterate through all living player actors
	for (playerActor = iter.first(); playerActor; playerActor = iter.next()) {
		Actor       *a = playerActor->getActor();
		Location    actorLoc;

		a->getWorldLocation(actorLoc);

		//  If actor is in the same world as the generator
		if (actorLoc._context == generatorLoc._context) {
			int32   dist,
			        mtRadius = obj->getHitPoints(),// Radius in metatiles
			        ptRadius = mtRadius * kTileUVSize * kPlatformWidth,
			        prob = obj->getExtra() * (256 * 256) / 100;

			TilePoint   diff = (TilePoint)actorLoc - (TilePoint)generatorLoc;

			//  Compute the distance between the actor and the generator
			dist = diff.quickHDistance();

			//  Determine distance from center of
//			prob = (ptRadius - dist) * prob / (ptRadius * mtRadius);

			// avoid divde by zeros
			if (ptRadius == 0) {
				return;
			}

			// prevent negtive distance from generator
			if (dist > ptRadius) {
				return;
			}

			prob = (ptRadius - dist) * prob / ptRadius;

			if (a == centerActor) prob /= 2;
			else prob /= 4;

			//  Now, roll to see if we got an encounter!

			if (g_vm->_rnd->getRandomNumber(0xffff) < (uint)prob) {
				scriptCallFrame scf;

#if DEBUG
				WriteStatusF(8, "Encounter %d Triggered!", obj->thisID());
#endif

				scf.invokedObject   = obj->thisID();
				scf.enactor         = a->thisID();
				scf.directObject    = scf.invokedObject;
				scf.indirectObject  = Nothing;
				scf.value           = dist / kTileUVSize;

				//  Call the SAGA script, if there is one.
				runObjectMethod(obj->thisID(), Method_GameObject_onTrigger, scf);
				return;
			}
		}
	}
}

/* ======================================================================== *
   MissionGeneratorProto
 * ======================================================================== */

//  Generate an encounter at approx. 10-second intervals
void MissionGeneratorProto::doBackgroundUpdate(GameObject *obj) {
}

/* ==================================================================== *
   IntangibleContainerProto class
 * ==================================================================== */

bool IntangibleContainerProto::canContain(ObjectID dObj, ObjectID item) {
	assert(isObject(item));

	GameObject      *itemPtr = GameObject::objectAddress(item);

	return (itemPtr->containmentSet() & (kIsSkill | kIsConcept)) != 0;
}

bool IntangibleContainerProto::useAction(ObjectID dObj, ObjectID enactor) {
	bool          result;
	GameObject    *dObjPtr = GameObject::objectAddress(dObj);

	if (dObjPtr->_data.objectFlags & kObjectOpen)
		result = close(dObj, enactor);
	else
		result = open(dObj, enactor);

	return result;
}


//  Determine if this intagible container can be opened
bool IntangibleContainerProto::canOpen(ObjectID dObj, ObjectID) {
	GameObject *dObjPtr = GameObject::objectAddress(dObj);

	return !dObjPtr->isOpen();
}

//  Open a intangible container
bool IntangibleContainerProto::openAction(ObjectID dObj, ObjectID enactor) {
	ContainerNode       *cn;

	//  Perform appropriate opening tasks
	cn = CreateContainerNode(enactor, false);
	cn->markForShow();

	return true;
}

bool IntangibleContainerProto::closeAction(ObjectID dObj, ObjectID) {
	ContainerNode *cn = g_vm->_cnm->find(dObj, ContainerNode::kMentalType);

	assert(cn);

	//  Mark container for lazy deletion
	cn->markForDelete();

	return true;
}

uint16 IntangibleContainerProto::containmentSet() {
	return kIsContainer | kIsIntangible;
}
/* ==================================================================== *
   IdeaContainerProto class
 * ==================================================================== */

/* ==================================================================== *
   MemoryContainerProto class
 * ==================================================================== */

/* ==================================================================== *
   PsychContainerProto class
 * ==================================================================== */

/* ==================================================================== *
   SkillContainerProto class
 * ==================================================================== */

/* ==================================================================== *
   MindContainerProto class
 * ==================================================================== */

} // end of namespace Saga2
