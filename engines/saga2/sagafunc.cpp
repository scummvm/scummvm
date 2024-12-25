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

#include "common/debug.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/script.h"
#include "saga2/actor.h"
#include "saga2/speech.h"
#include "saga2/assign.h"
#include "saga2/intrface.h"
#include "saga2/document.h"
#include "saga2/motion.h"
#include "saga2/sensor.h"
#include "saga2/magic.h"
#include "saga2/uidialog.h"
#include "saga2/mission.h"
#include "saga2/band.h"
#include "saga2/tromode.h"
#include "saga2/automap.h"
#include "saga2/videobox.h"
#include "saga2/display.h"
#include "saga2/transit.h"
#include "saga2/contain.h"
#include "saga2/tile.h"
#include "saga2/tilemode.h"

void drawMainDisplay();

#define MONOLOG(s) {debugC(2, kDebugScripts, "cfunc: " #s );}
#define OBJLOG(s) {debugC(2, kDebugScripts, "cfunc: [%s]." #s , (((ObjectData *)thisThread->_thisObject)->obj)->objName() );}

namespace Saga2 {

extern WorldMapData         *mapList;           //  master map data array

/* ============================================================================ *
   CFunc helper functions
 * ============================================================================ */

//-----------------------------------------------------------------------
//	Convert string ID to string text

inline char *STRING(int strNum) {
	return (char *)thisThread->strAddress(strNum);
}

//-----------------------------------------------------------------------
//	SPrintf-like formatting of strings, except that it works on
//	SAGA arglists.

int stringf(char *buffer, long maxlen, int formatStr, int16 *args) {
	char        *fmt = STRING(formatStr);
	char        *bufEnd = buffer + maxlen - 1;
	char        dbuf[16],             // temp buffer for digits
	            *dptr;

	//  While there is room in the buffer
	while (buffer < bufEnd && *fmt != '\0') {
		//  Format string character
		if (*fmt == '%') {
			if (fmt[1] == 'd') {      //  If it's %d
				//  Convert numeric argument to string
				snprintf(dbuf, 15, "%d", *args++);

				//  Copy string to buffer (if it fits)
				for (dptr = dbuf; *dptr && buffer < bufEnd;) {
					*buffer++ = *dptr++;
				}
			} else if (fmt[1] == 'x') { //  If it's %x
				//  Convert numeric argument to string
				snprintf(dbuf, 15, "%x", *args++);

				//  Copy string to buffer (if it fits)
				for (dptr = dbuf; *dptr && buffer < bufEnd;) {
					*buffer++ = *dptr++;
				}
			} else if (fmt[1] == 's') { //  If it's %s
				//  Obtain SAGA string, copy to buffer (if it fits)
				for (dptr = STRING(*args++); *dptr && buffer < bufEnd;) {
					*buffer++ = *dptr++;
				}
			} else if (fmt[1] == 'n') { //  If it's %n (object name)
				GameObject  *obj = GameObject::objectAddress(*args++);

				//  Obtain SAGA string, copy to buffer (if it fits)
				for (const char *dptr1 = obj->objName(); *dptr1 && buffer < bufEnd;) {
					*buffer++ = *dptr1++;
				}
			} else {
				//  Write the character after the '%' to the buffer
				*buffer++ = fmt[1];
			}
			fmt += 2;
		} else *buffer++ = *fmt++;
	}

	*buffer++ = '\0';

	//  Return the number of characters written to the buffer
	return buffer - (bufEnd - (maxlen - 1));
}

/* ============================================================================ *
   Script C-Function call tables
 * ============================================================================ */

//-----------------------------------------------------------------------
//	Returns the id of this object
//		GameObject id "c" thisID( void );

int16 scriptGameObjectThisID(int16 *args) {
	OBJLOG(ThisID);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->thisID();
}

//-----------------------------------------------------------------------
//	Recharges this object
//		void "c" recharge( void );

int16 scriptGameObjectRecharge(int16 *args) {
	OBJLOG(Recharge);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->recharge();
	return 0;
}

//-----------------------------------------------------------------------
//	this returns the type of charge this item should have
//  none, red, violet, etc...
//		int "c" getChargeType( void );

int16 scriptGameObjectGetChargeType(int16 *args) {
	OBJLOG(GetChargeType);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->getChargeType();
}

//-----------------------------------------------------------------------
//	Move an object to a new location. (optional 4th parameter
//	is for actor facing, only used by actors)
//		void "c" move( int u, int v, int z, ... );

int16 scriptActorMove(int16 *args) {
	OBJLOG(Move);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	//  Move the object to a new location
	obj->move(TilePoint(args[0], args[1], args[2]));

	//  If optional 4th parameter is present, then set actor facing
	if (thisThread->_argCount > 3 && isActor(obj)) {
		Actor       *a = (Actor *)obj;

		a->_currentFacing = args[3];
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Move object relative to another object (optional 4th
//	parameter is for actor facing, only used by actors)
//		void "c" moveRel( GameObject id baseObj, int angle, int distance, ... );

extern const StaticTilePoint dirTable[8];

int16 scriptActorMoveRel(int16 *args) {
	OBJLOG(MoveRel);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj,
	                 *baseObj = GameObject::objectAddress(args[0]);
	Location        l;
	TilePoint       tp;

	l._context   = baseObj->IDParent();
	tp          = baseObj->getLocation();

	//  Add offset for angle and distance
	tp += (dirTable[args[1] & 7] * args[2]) / 3;

	//  Move the object to a new location
	*(TilePoint *)&l = tp;
	obj->move(l);

	//  If optional 4th parameter is present, then set actor facing
	if (thisThread->_argCount > 3 && isActor(obj)) {
		Actor       *a = (Actor *)obj;

		a->_currentFacing = args[3];
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Transfer object to a new context. (optional 4th parameter
//	is for actor facing, only used by actors)
//		void "c" transfer( GameObject id context, int u, int v, int z, ... );

int16 scriptActorTransfer(int16 *args) {
	OBJLOG(Transfer);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	//  Move the object to a new location
	if ((isObject(args[0])
	        && (GameObject::protoAddress(args[0])->containmentSet()
	            &   ProtoObj::kIsContainer))
	        ||  isActor(args[0])) {
		ObjectID        targetID = args[0];
		GameObject      *target = GameObject::objectAddress(targetID);
		TilePoint       targetSlot;

		//  If it's an actor we're moving to, then "place" the object rather than
		//  using the explicit coordinates specified
		if (target->getAvailableSlot(obj, &targetSlot)) {
			uint16      cSet = target->proto()->containmentSet();

			obj->move(Location(targetSlot, targetID));
			if ((cSet & (ProtoObj::kIsIntangible | ProtoObj::kIsContainer))
			        == (ProtoObj::kIsIntangible | ProtoObj::kIsContainer))
				g_vm->_cnm->setUpdate(targetID);
		}
	} else {
		obj->move(Location(args[1], args[2], args[3], args[0]));
	}

	//  If optional 5th parameter is present, then set actor facing
	if (thisThread->_argCount > 4 && isActor(obj)) {
		Actor       *a = (Actor *)obj;

		a->_currentFacing = args[4];
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Pick a random location for an object within an area
//		void "c" moveRandom( int u, int v, int z, int distance );

int16 scriptMoveRandom(int16 *args) {
	OBJLOG(MoveRandom);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	TilePoint       tpMin, tpMax;
	int16           distance = args[3];

	tpMin.u = args[0] - distance;
	tpMin.v = args[1] - distance;
	tpMin.z = 0;

	tpMax.u = args[0] + distance;
	tpMax.v = args[1] + distance;
	tpMax.z = 100;

	obj->moveRandom(tpMin, tpMax);
	return 0;
}

//-----------------------------------------------------------------------
//	Return Name ID of actor
//		int "c" name( void );

int16 scriptActorGetName(int16 *) {
	OBJLOG(GetName);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	int16           oldName = obj->getNameIndex();

	return oldName;
}

//-----------------------------------------------------------------------
//	Set Name ID of actor
//		int "c" setName( int nameID );

int16 scriptActorSetName(int16 *args) {
	OBJLOG(SetName);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	int16           oldName = obj->getNameIndex();

	obj->setNameIndex(args[0]);

	return oldName;
}

//-----------------------------------------------------------------------
//	return current object prototype
//		int "c" proto( void );

int16 scriptActorGetProto(int16 *) {
	OBJLOG(GetProto);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	return obj->getProtoNum();
}

//-----------------------------------------------------------------------
// Set object prototype and return old proto
//		int "c" setProto( int protoID );

int16 scriptActorSetProto(int16 *args) {
	OBJLOG(SetProto);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	int16           oldProto = obj->getProtoNum();

	if (isActor(obj) && (((Actor *)obj)->_flags & Actor::kAFTemporary)) {
		decTempActorCount(oldProto);
		incTempActorCount(args[0]);
	}

	obj->setProtoNum(args[0]);

	return oldProto;
}

//-----------------------------------------------------------------------
//	return current object prototype class
//		int "c" protoClass( void );

int16 scriptActorGetProtoClass(int16 *) {
	OBJLOG(GetProtoClass);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	ProtoObj        *objProto = obj->proto();

	return objProto->classType;
}

//-----------------------------------------------------------------------
//	return current object prototype
//		int "c" getScript( void );

int16 scriptActorGetScript(int16 *) {
	OBJLOG(GetScript);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	return obj->getScript();
}

//-----------------------------------------------------------------------
// Set object script index and return old script
//		int "c" setScript( int protoID );

int16 scriptActorSetScript(int16 *args) {
	OBJLOG(SetScript);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	int16           oldScript = obj->getScript();

	obj->setScript(args[0]);

	return oldScript;
}

//-----------------------------------------------------------------------
//	Use this object
//		int "c" use( GameObject id enactor );

int16 scriptGameObjectUse(int16 *args) {
	OBJLOG(Use);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->use(args[0]);
}

//-----------------------------------------------------------------------
//	Use this object on another object
//		int "c" useOn( GameObject id enactor, GameObject id target );

int16 scriptGameObjectUseOn(int16 *args) {
	OBJLOG(UseOn);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->useOn(args[0], args[1]);
}

//-----------------------------------------------------------------------
//	Use this object on a TAI
//		int "c" useOnTAI( GameObject id enactor, TileActivityInstance id target );

int16 scriptGameObjectUseOnTAI(int16 *args) {
	OBJLOG(UseOnTAI);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->useOn(args[0], ActiveItem::activeItemAddress(args[1]));
}

//-----------------------------------------------------------------------
//	Drop this object
//		int "c" drop(
//			GameObject id  enactor,
//			GameObject id  context,
//			int                u,
//			int                v,
//			int                z );

int16 scriptGameObjectDrop(int16 *args) {
	OBJLOG(Drop);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return  obj->drop(
	            args[0],
	            Location(args[2], args[3], args[4], args[1 ]));
}

//-----------------------------------------------------------------------
//	Drop this object on another object
//		int "c" dropOn( GameObject id enactor, GameObject id target );

int16 scriptGameObjectDropOn(int16 *args) {
	OBJLOG(DropOn);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->dropOn(args[0], args[1]);
}

//-----------------------------------------------------------------------
//	Drop this object on another object.  For mergeable objects only
//		int "c" dropMergeableOn(
//			GameObject id  enactor,
//			GameObject id  target,
//			int                count );

int16 scriptGameObjectDropMergeableOn(int16 *args) {
	OBJLOG(DropMergeableOn);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->dropOn(args[0], args[1], args[2]);
}

//-----------------------------------------------------------------------
//	Drop this object on a TAI
//		int "c" dropOnTAI(
//			GameObject id          enactor,
//			TileActivityInstance id target,
//			GameObject id          context,
//			int                        u,
//			int                        v,
//			int                        z );

int16 scriptGameObjectDropOnTAI(int16 *args) {
	OBJLOG(DropOnTAI);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return  obj->dropOn(
	            args[0],
	            ActiveItem::activeItemAddress(args[1]),
	            Location(args[3], args[4], args[5], args[2 ]));
}

//-----------------------------------------------------------------------
//	Causes actor (or object) to speak...
//		void "c" say( int flags, int sampleID, string, ... );

int16 scriptActorSay(int16 *args) {
	OBJLOG(Say);
	//  Actor speech enums -- move these to include file
	// MOVED TO SPEECH.H - EO
//	enum {
//		kSpeakContinued  = (1<<0),           // Append next speech
//		kSpeakNoAnimate  = (1<<1),           // Don't animate speaking
//		kSpeakWait       = (1<<2),           // wait until speech finished
//		kSpeakLock       = (1<<3),           // LockUI while speaking
//	};

	//  'obj' is the actor doing the speaking.
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	uint16          flags = args[0];
	Speech          *sp;

	if (isActor(obj)) {
		Actor       *a = (Actor *)obj;

		if (a->isDead()) return 0;
	}

	//  Determine if a speech record is being built for this actor.
	//  If so, then retrieve it. If not, then build a new one and
	//  retrieve it.
	sp = speechList.findSpeech(obj->thisID());
	if (sp == nullptr) {
		uint16  spFlags = 0;

		if (flags & kSpeakNoAnimate) spFlags |= Speech::kSpNoAnimate;
		if (flags & kSpeakLock)      spFlags |= Speech::kSpLock;

		sp = speechList.newTask(obj->thisID(), spFlags);

		if (sp == nullptr) return 0;
	}

	//  Loop through each of the arguments.
	//  REM: Might want to do some range checking on the arguments.
	for (int i = 1; i < thisThread->_argCount; i += 2) {
		uint16      sampleNum = args[i];
		char        *speechText = STRING(args[i + 1]);

		debugC(2, kDebugScripts, "Speech Text: - %s", speechText);
		sp->append(speechText, sampleNum);
	}

	//  If we're ready to activate the speech
	if (!(flags & kSpeakContinued)) {
		//  If we're going to wait for it synchronously
		if (flags & kSpeakWait) {
			thisThread->waitForEvent(Thread::kWaitOther, nullptr);
			sp->setWakeUp(getThreadID(thisThread));
		}

		//  Move speech to active queue
		sp->activate();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Appends a string of text (no samples) to speech buffer, but does not
//	begin the speech
//		void "c" sayText( string txt, ... );

int16 scriptActorSayText(int16 *args) {
	OBJLOG(SayText);
	//  'obj' is the actor doing the speaking.
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	Speech          *sp;
	char            buffer[256];

	//  Determine if a speech record is being built for this actor.
	//  If so, then retrieve it. If not, then fail.
	sp = speechList.findSpeech(obj->thisID());
	if (sp == nullptr) return 0;

	stringf(buffer, sizeof buffer, args[0], &args[1]);
	sp->append(buffer, 0);
	return 1;
}

//-----------------------------------------------------------------------
//	Returns overall type of object
//		int "c" objectType( void );

int16 scriptActorObjectType(int16 *) {
	OBJLOG(ObjectType);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return (int16)(obj->containmentSet());
}

//-----------------------------------------------------------------------
//	Returns overall type of object
//		GameObject id "c" copyObject( void );

int16 scriptActorCopyObject(int16 *) {
	OBJLOG(CopyObject);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	Location        l(0, 0, 0, Nothing);

	return (int16)(obj->copy(l));
}

//-----------------------------------------------------------------------
//	Determine if this object is activated
//		int "c" isActivated( void );

int16 scriptGameObjectIsActivated(int16 *args) {
	OBJLOG(IsActivated);
	return (((ObjectData *)thisThread->_thisObject)->obj)->isActivated();
}

//-----------------------------------------------------------------------
//	Object flags access functions

int16 scriptActorGetOpen(int16 *) {
	OBJLOG(GetOpen);
	return (((ObjectData *)thisThread->_thisObject)->obj)->isOpen();
}

int16 scriptActorGetLocked(int16 *) {
	OBJLOG(GetLocked);
	return (((ObjectData *)thisThread->_thisObject)->obj)->isLocked();
}

int16 scriptActorGetImportant(int16 *) {
	OBJLOG(GetImportant);
	return (((ObjectData *)thisThread->_thisObject)->obj)->isImportant();
}

int16 scriptActorGetScavengable(int16 *) {
	OBJLOG(GetScavengable);
	return (((ObjectData *)thisThread->_thisObject)->obj)->isScavengable();
}

/*
int16 scriptActorSetOpen( int16 *args )
{
    (((ObjectData *)thisThread->_thisObject)->obj)->setFlags(
        args[0] ? 0xffff : 0,
        kObjectOpen );
    return 0;
}

int16 scriptActorSetLocked( int16 *args )
{
    (((ObjectData *)thisThread->_thisObject)->obj)->setFlags(
        args[0] ? 0xffff : 0,
        kObjectLocked );
    return 0;
}
*/

int16 scriptActorSetImportant(int16 *args) {
	OBJLOG(SetImportant);
	(((ObjectData *)thisThread->_thisObject)->obj)->setFlags(
	    args[0] ? (int16) 0xffff : (int16) 0,
	    kObjectImportant);
	return 0;
}

int16 scriptActorSetScavengable(int16 *args) {
	OBJLOG(SetScavengable);
	(((ObjectData *)thisThread->_thisObject)->obj)->setFlags(
	    args[0] ? (int16) 0xffff : (int16) 0,
	    kObjectScavengable);
	return 0;
}

//-----------------------------------------------------------------------
//	Create a new timer for this object
//		int "c" addTimer( int timerID, int frameInterval );

int16 scriptGameObjectAddTimer(int16 *args) {
	OBJLOG(AddTimer);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addTimer(args[0], args[1]);
}

//-----------------------------------------------------------------------
//	Create a new timer for this object using the standard frame
//	interval (5 frames)
//		int "c" addStdTimer( int timerID, int frameInterval );

int16 scriptGameObjectAddStdTimer(int16 *args) {
	OBJLOG(AddStdTimer);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addTimer(args[0]);
}

//-----------------------------------------------------------------------
//	Delete a specific timer
//		void "c" removeTimer( int timerID );

int16 scriptGameObjectRemoveTimer(int16 *args) {
	OBJLOG(RemoveTimer);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->removeTimer(args[0]);

	return 0;
}

//-----------------------------------------------------------------------
//	Delete all timers for this object
//		void "c" removeAllTimers( void );

int16 scriptGameObjectRemoveAllTimers(int16 *args) {
	OBJLOG(RemoveAllTimers);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->removeAllTimers();

	return 0;
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect the proximity of a
//	protagonist
//		int "c" addProtaganistSensor( int sensorID, int range );

int16 scriptGameObjectAddProtaganistSensor(int16 *args) {
	OBJLOG(AddProtaganistSensor);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addProtaganistSensor(args[0], args[1]);
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect the proximity of a
//	specified actor
//		int "c" addSpecificActorSensor(
//			int            sensorID,
//			int            range,
//			Actor id   actor );

int16 scriptGameObjectAddSpecificActorSensor(int16 *args) {
	OBJLOG(AddSpecificActorSensor);
	assert(isActor(args[2]));

	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return  obj->addSpecificActorSensor(
	            args[0],
	            args[1],
	            (Actor *)GameObject::objectAddress(args[2]));
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect the proximity of a
//	specified object
//		int "c" addSpecificObjectSensor(
//			int                sensorID,
//			int                range,
//			GameObject id  obj );

int16 scriptGameObjectAddSpecificObjectSensor(int16 *args) {
	OBJLOG(AddSpecificObjectSensor);
	assert(isObject(args[2]) || isActor(args[2]));

	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addSpecificObjectSensor(args[0], args[1], args[2]);
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect the proximity of an
//	actor with a specified property
//		int "c" addActorPropertySensor(
//			int sensorID,
//			int range,
//			int actorProperty );

int16 scriptGameObjectAddActorPropertySensor(int16 *args) {
	OBJLOG(AddActorPropertySensor);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addActorPropertySensor(args[0], args[1], args[2]);
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect the proximity of an
//	object with a specified property
//		int "c" addObjectPropertySensor(
//			int sensorID,
//			int range,
//			int objectProperty );

int16 scriptGameObjectAddObjectPropertySensor(int16 *args) {
	OBJLOG(AddObjectPropertySensor);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->addObjectPropertySensor(args[0], args[1], args[2]);
}

//-----------------------------------------------------------------------
//	Create a sensor for this object to detect a specified event
//	within this actor's proximity
//		int "c" addEventSensor( int sensorID, int range, int eventType );

int16 scriptGameObjectAddEventSensor(int16 *args) {
	OBJLOG(AddEventSensor);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return  obj->addEventSensor(
	            args[0],
	            args[1],
	            args[2]);
}

//-----------------------------------------------------------------------
//	Delete a specified sensor
//		void "c" removeSensor( int sensorID );

int16 scriptGameObjectRemoveSensor(int16 *args) {
	OBJLOG(RemoveSensor);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->removeSensor(args[0]);

	return 0;
}

//-----------------------------------------------------------------------
//	Delete every sensor for this object
//		void "c" removeAllSensors( void );

int16 scriptGameObjectRemoveAllSensors(int16 *args) {
	OBJLOG(RemoveAllSensors);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->removeAllSensors();

	return 0;
}

//-----------------------------------------------------------------------
//	Determine if this object can sense the proximity of a protagonist
//		int "c" canSenseProtaganist( int range );

int16 scriptGameObjectCanSenseProtaganist(int16 *args) {
	OBJLOG(CanSenseProtaganist);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	SenseInfo       info;

	if (obj->canSenseProtaganist(info, args[0])) {
		scriptCallFrame     &scf = thisThread->_threadArgs;

		scf.enactor = obj->thisID();
		scf.directObject = info.sensedObject->thisID();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Determine if this object can sense the proximity of a specific
//	actor
//		int "c" canSenseSpecificActor( int range, Actor id actor );

int16 scriptGameObjectCanSenseSpecificActor(int16 *args) {
	OBJLOG(CanSenseSpecificActor);
	assert(isActor(args[1]));

	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	SenseInfo       info;

	if (obj->canSenseSpecificActor(
	            info,
	            args[0],
	            (Actor *)GameObject::objectAddress(args[1]))) {
		scriptCallFrame     &scf = thisThread->_threadArgs;

		scf.enactor = obj->thisID();
		scf.directObject = info.sensedObject->thisID();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Determine if this object can sense the proximity of a specific
//	object
//		int "c" canSenseSpecificObject( int range, GameObject id obj );

int16 scriptGameObjectCanSenseSpecificObject(int16 *args) {
	OBJLOG(CanSenseSpecificObject);
	assert(isObject(args[1]) || isActor(args[1]));

	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	SenseInfo       info;

	if (obj->canSenseSpecificObject(info, args[0], args[1])) {
		scriptCallFrame     &scf = thisThread->_threadArgs;

		scf.enactor = obj->thisID();
		scf.directObject = info.sensedObject->thisID();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Determine if this object can sense the proximity of an actor
//	with a specified property
//		int "c" canSenseActorProperty( int range, int actorProperty );

int16 scriptGameObjectCanSenseActorProperty(int16 *args) {
	OBJLOG(CanSenseActorProperty);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	SenseInfo       info;

	if (obj->canSenseActorProperty(info, args[0], args[1])) {
		scriptCallFrame     &scf = thisThread->_threadArgs;

		scf.enactor = obj->thisID();
		scf.directObject = info.sensedObject->thisID();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Determine if this object can sense the proximity of an object
//	with a specified property
//		int "c" canSenseObjectProperty( int range, int objectProperty );

int16 scriptGameObjectCanSenseObjectProperty(int16 *args) {
	OBJLOG(CanSenseObjectProperty);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	SenseInfo       info;

	if (obj->canSenseObjectProperty(info, args[0], args[1])) {
		scriptCallFrame     &scf = thisThread->_threadArgs;

		scf.enactor = obj->thisID();
		scf.directObject = info.sensedObject->thisID();

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	returns object script, or proto's script if has none
//		int "c" getActualScript( void );

int16 scriptGameObjectGetActualScript(int16 *) {
	OBJLOG(GetActualScript);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	int16           script;

	script = obj->getScript();
	if (script == 0)
		script = obj->proto()->script;
	return script;
}

//-----------------------------------------------------------------------
//	returns the script of the prototype
//		int "c" getProtoScript( void );

int16 scriptGameObjectGetProtoScript(int16 *) {
	OBJLOG(GetProtoScript);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->proto()->script;
}

//-----------------------------------------------------------------------
//	Get and set mass count -- mergeable objects only
//		int "c" getMass( void );

int16 scriptGameObjectGetMass(int16 *) {
	OBJLOG(GetMass);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return (obj->proto()->flags & ResourceObjectPrototype::kObjPropMergeable)
	       ? obj->getExtra() : 1;
}

//-----------------------------------------------------------------------
//	Get and set mass count -- mergeable objects only
//		int "c" setMass( int newMass );

int16 scriptGameObjectSetMass(int16 *args) {
	OBJLOG(SetMass);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	if (obj->proto()->flags & ResourceObjectPrototype::kObjPropMergeable) {
		obj->setExtra(args[0]);
		if (obj->proto()->flags & ResourceObjectPrototype::kObjPropMergeable) {
			g_vm->_cnm->setUpdate(obj->IDParent());
		}
		return true;
	} else return false;
}

//-----------------------------------------------------------------------
//	Get the extra scratch var
//		int "c" getExtra( void );

int16 scriptGameObjectGetExtra(int16 *) {
	OBJLOG(GetExtra);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->getExtra();
}

//-----------------------------------------------------------------------
//	Set the extra scratch var
//		void "c" setExtra( int newVal );

int16 scriptGameObjectSetExtra(int16 *args) {
	OBJLOG(SetExtra);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->setExtra(args[0]);

	return 0;
}

//-----------------------------------------------------------------------
//	Make a copy of this object and all sub-objects
//		GameObject id "c" deepCopy( void );

int16 deepCopy(GameObject *src, ObjectID parentID, TilePoint tp) {
	OBJLOG(DeepCopy);
	Location    l;
	int16       newID, childID;
	GameObject  *childObj = nullptr;

	l.u = tp.u;
	l.v = tp.v;
	l.z = tp.z;
	l._context = parentID;

	//  Make a copy of this object, and place it in the parent container we spec'd
	newID = src->copy(l);
	if (newID == Nothing) return 0; //  REM: How to send error message to script?

	//  Now, recursively copy all the children of this object.
	ContainerIterator   iter(src);
	while ((childID = iter.next(&childObj)))
		deepCopy(childObj, newID, childObj->getLocation());

	//  Return the ID of the object just copied.
	return newID;
}

int16 scriptGameObjectDeepCopy(int16 *args) {
	OBJLOG(DeepCopy);
	ObjectID        newParentID = args[0];
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj,
	                 *newParent = GameObject::objectAddress(newParentID),
	                  *newObj;
	ObjectID        id;

	id = deepCopy(obj, Nothing, TilePoint(0, 0, 0));

	newObj = GameObject::objectAddress(id);
	if (newParentID != Nothing) {
		TilePoint       slot;

		if (newParent->getAvailableSlot(newObj, &slot))
			newObj->move(Location(slot, newParentID));
	}

	return id;
}

//-----------------------------------------------------------------------
//	Add an enchantment to the object or actor
//		GameObject id "c" addEnchantment(   int majorType,
//											int minorType,
//											int amount,
//											int duration );

int16 scriptGameObjectAddEnchantment(int16 *args) {
	OBJLOG(Enchant);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return EnchantObject(obj->thisID(),
	                     makeEnchantmentID(args[0], args[1], args[2]),
	                     args[3]);
}

//-----------------------------------------------------------------------
//	Add an enchantment to the object or actor
//		GameObject id "c" dispelEnchantment( int majorType, int minorType );

int16 scriptGameObjectRemoveEnchantment(int16 *args) {
	OBJLOG(Disenchant);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return DispelObjectEnchantment(obj->thisID(),
	                               makeEnchantmentID(args[0], args[1], 0));
}

//-----------------------------------------------------------------------
//	Locates an enchantment object within the actor
//		GameObject id "c" findEnchantment( int majorType, int minorType );

int16 scriptGameObjectFindEnchantment(int16 *args) {
	OBJLOG(FindEnchantment);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return FindObjectEnchantment(obj->thisID(),
	                             makeEnchantmentID(args[0], args[1], 0));
}

//-----------------------------------------------------------------------
//  Determines if this object is currently in use.
//    int "c" inUse( void );

int16 scriptGameObjectInUse(int16 *) {
	OBJLOG(InUse);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	return obj->proto()->isObjectBeingUsed(obj);
}

//-----------------------------------------------------------------------
//	Get the specified scratch variable value
//		int "c" getScratchVar( int index );

int16 scriptActorGetScratchVar(int16 *args) {
	OBJLOG(GetScratchVar);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->_scriptVar[args[0]];
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the specified scratch variable to a new value
//		int "c" setScratchVar( int index, int newVal );

int16 scriptActorSetScratchVar(int16 *args) {
	OBJLOG(SetScratchVar);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		int16       oldVal = a->_scriptVar[args[0]];

		a->_scriptVar[args[0]] = args[1];

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Return this actor's disposition
//		int "c" getDisposition( void );

int16 scriptActorGetDisposition(int16 *args) {
	OBJLOG(GetDisposition);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getDisposition();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set this actor's disposition
//		int "c" setDisposition( int newDisp );

int16 scriptActorSetDisposition(int16 *args) {
	OBJLOG(SetDisposition);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->setDisposition(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of a specified skill
//		int "c" getSkill( int skillID );

int16 scriptActorGetSkill(int16 *args) {
	OBJLOG(GetSkill);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getStats()->skill(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set a specified skill to a specified value
//		int "c" setSkill( int skillID, int newVal );

int16 scriptActorSetSkill(int16 *args) {
	OBJLOG(SetSkill);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		uint8       &skillRef = a->getStats()->skill(args[0]);
		uint8       oldVal = skillRef;

		skillRef = args[1];

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of a specified base skill
//		int "c" getBaseSkill( int skillID );

int16 scriptActorGetBaseSkill(int16 *args) {
	OBJLOG(GetBaseSkill);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getBaseStats()->skill(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set a specified base skill to a specified value
//		int "c" setBaseSkill( int skillID, int newVal );

int16 scriptActorSetBaseSkill(int16 *args) {
	OBJLOG(SetBaseSkill);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		uint8       &skillRef = a->getBaseStats()->skill(args[0]);
		uint8       oldVal = skillRef;


		//  If not a player actor, do nothing
		if (isPlayerActor(a)) skillRef = args[1];

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of the actor's vitality
//		int "c" getVitality( void );

int16 scriptActorGetVitality(int16 *) {
	OBJLOG(GetVitality);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		debugC(2, kDebugScripts, " - value = %d", a->getStats()->vitality);
		return a->getStats()->vitality;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the actor's vitality to a specified value
//		int "c" setVitality( int newVal );

int16 scriptActorSetVitality(int16 *args) {
	OBJLOG(SetVitality);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (a->_godmode)
			return 0;

		int16       &vitalityRef = a->getStats()->vitality;
		int16       oldVal = vitalityRef;
		PlayerActorID   pID;
		debugC(2, kDebugScripts, " - value = %d", args[0]);

		vitalityRef = args[0];
		if (actorToPlayerID(a, pID)) updateBrotherControls(pID);

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of the actor's base vitality
//		int "c" getBaseVitality( void );

int16 scriptActorGetBaseVitality(int16 *) {
	OBJLOG(GetBaseVitality);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getBaseStats()->vitality;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the actor's base vitality to a specified value
//		int "c" setBaseVitality( int newVal );

int16 scriptActorSetBaseVitality(int16 *args) {
	OBJLOG(SetBaseVitality);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		int16       &vitalityRef = a->getBaseStats()->vitality;
		int16       oldVal = vitalityRef;
		PlayerActorID   pID;

		//  If not a player actor, do nothing
		if (actorToPlayerID(a, pID)) {
			vitalityRef = args[0];
			updateBrotherControls(pID);
		}

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of the specified color of mana
//		int "c" getMana( int manaID );

int16 scriptActorGetMana(int16 *args) {
	OBJLOG(GetMana);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getStats()->mana(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the specified mana to the specified value
//		int "c" setMana( int manaID, int newVal );

int16 scriptActorSetMana(int16 *args) {
	OBJLOG(SetMana);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		int16       &manaRef = a->getStats()->mana(args[0]);
		int16       oldVal = manaRef;
		PlayerActorID   pID;

		manaRef = args[1];
		if (actorToPlayerID(a, pID)) updateBrotherControls(pID);

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Get the value of the specified color of base mana
//		int "c" getBaseMana( int manaID );

int16 scriptActorGetBaseMana(int16 *args) {
	OBJLOG(GetBaseMana);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->getBaseStats()->mana(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the specified base mana to the specified value
//		int "c" setBaseMana( int manaID, int newVal );

int16 scriptActorSetBaseMana(int16 *args) {
	OBJLOG(SetBaseMana);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		int16       &manaRef = a->getBaseStats()->mana(args[0]);
		int16       oldVal = manaRef;
		PlayerActorID   pID;

		//  If not a player actor, do nothing
		if (isPlayerActor(a)) manaRef = args[1];
		if (actorToPlayerID(a, pID)) updateBrotherControls(pID);

		return oldVal;
	}

	return 0;
}

//-----------------------------------------------------------------------
//  Get actor's schedule
//		int "c" getSchedule( void );

int16 scriptActorGetSchedule(int16 *args) {
	OBJLOG(GetSchedule);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->_schedule;
	}

	return 0;
}

//-----------------------------------------------------------------------
//  Set actor's schedule -- also returns old schedule
//		int "c" setSchedule( int schedule );

int16 scriptActorSetSchedule(int16 *args) {
	OBJLOG(SetSchedule);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		uint16      oldSchedule = a->_schedule;

		a->_schedule = (uint16)args[0];

		if (a->getAssignment() != nullptr)
			delete a->getAssignment();

		return (int16)oldSchedule;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Lobotomize the actor
//		void "c" lobotomize( void );

int16 scriptActorLobotomize(int16 *args) {
	OBJLOG(Lobotomize);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		a->lobotomize();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	De-lobotomize the actor
//		void "c" delobotomize( void );

int16 scriptActorDelobotomize(int16 *args) {
	OBJLOG(Delobotomize);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		a->delobotomize();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Determine if an action is available
//		int "c" isActionAvailable( int action, int anyDir );

int16 scriptActorIsActionAvailable(int16 *args) {
	OBJLOG(IsActionAvailable);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->isActionAvailable(args[0], args[1]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Set the current animation for this actor.  Returns the number
//	of poses in the sequence, or 0 if there are no poses in the
//	sequence.
//		int "c" setAction( int action, int flags );

int16 scriptActorSetAction(int16 *args) {
	OBJLOG(SetAction);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->setAction(args[0], args[1]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//  Return the number of animation frames in the specified action
//  for the specified direction.
//		int "c" animationFrames( int action, int dir );

int16 scriptActorAnimationFrames(int16 *args) {
	OBJLOG(AnimationFrames);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->animationFrames(args[0], args[1]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Update the current animation sequence to the next frame.
//	Returns true if there are more animation frames in the current
//	sequence.
//		int "c" nextAnimationFrame( void );

int16 scriptActorNextAnimationFrame(int16 *args) {
	OBJLOG(NextAnimationFrame);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->nextAnimationFrame();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Causes actor to face in a new direction, and returns old actor facing
//		int "c" face( int direction );

int16 scriptActorFace(int16 *args) {
	OBJLOG(Face);
	int16           oldFacing = 0;

	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		oldFacing = a->_currentFacing;

		a->_currentFacing = args[0] & 7;
	}

	return oldFacing;
}

//-----------------------------------------------------------------------
//	Causes actor to face towards another object or actor,
//	returns old facing
//		int "c" faceTowards( GameObject id );

int16 scriptActorFaceTowards(int16 *args) {
	OBJLOG(FaceTowards);
	int16           oldFacing = 0;

	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		assert(isObject(args[0]) || isActor(args[0]));

		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		oldFacing = a->_currentFacing;

		a->_currentFacing =
		    (GameObject::objectAddress(args[0])->getLocation()
		     -   a->getLocation()).quickDir();
	}

	return oldFacing;
}

//-----------------------------------------------------------------------
//	Causes actor to turn to a new direction.
//		int "c" turn( int direction, int flags );

int16 scriptActorTurn(int16 *args) {
	OBJLOG(Turn);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (a->isDead()) return 0;

		uint16      flags = args[1];

		if (flags & kMoveWait) {
			thisThread->waitForEvent(Thread::kWaitOther, nullptr);
			MotionTask::turn(getThreadID(thisThread), *a, args[0] & 7);
		} else {
			MotionTask::turn(*a, args[0] & 7);
			return kMotionStarted;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Causes actor to turn towards another object or actor.
//		int "c" turnTowards( GameObject id, int flags );

int16 scriptActorTurnTowards(int16 *args) {
	OBJLOG(TurnTowards);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		assert(isObject(args[0]) || isActor(args[0]));

		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (a->isDead()) return 0;

		uint16      flags = args[1];
		Direction   dir;

		dir = (GameObject::objectAddress(args[0])->getLocation()
		       -   a->getLocation()).quickDir();

		if (flags & kMoveWait) {
			thisThread->waitForEvent(Thread::kWaitOther, nullptr);
			MotionTask::turn(getThreadID(thisThread), *a, dir);
		} else {
			MotionTask::turn(*a, dir);
			return kMotionStarted;
		}
	}

	return 0;
}
//-----------------------------------------------------------------------
//	Walk the actor to a point
//		int "c" walk( int u, int v, int z, int flags );

int16 scriptActorWalk(int16 *args) {
	OBJLOG(Walk);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (a->isDead()) return 0;

		TilePoint   dest(args[0], args[1], args[2]);
		uint16      flags = args[3];

		if (flags & kMoveWait) {
			thisThread->waitForEvent(Thread::kWaitOther, nullptr);
			MotionTask::walkToDirect(
			    getThreadID(thisThread), *a, dest, flags & kMoveRun);
		} else {
			MotionTask::walkToDirect(*a, dest, flags & kMoveRun);
			return kMotionStarted;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Constructs a patrol route assignment for this actor.  Optional fourth
//	parameter specifies the starting waypoint.
//		int "c" assignPatrolRoute(
//			int until,
//			int routeNo,
//			int flags,
//			... );

int16 scriptActorAssignPatrolRoute(int16 *args) {
	OBJLOG(AssignPatrolRoute);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new PatrolRouteAssignment(a,
		            (uint16)args[0]
		            *   CalendarTime::kFramesPerHour,
		            args[1],
		            (uint8)args[2],
		            thisThread->_argCount >= 4
		            ?   args[3]
		            :   -1)
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Assign a patrol route and specify the beginning and ending waypoints
//		int "c" assignPartialPatrolRoute(
//			int untilHour,
//			int routeNo,
//			int flags,
//			int startingWayPoint,
//			int endingWayPoint );

int16 scriptActorAssignPartialPatrolRoute(int16 *args) {
	OBJLOG(AssignPartialPatrolRoute);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new PatrolRouteAssignment(a,
		            (uint16)args[0]
		            *   CalendarTime::kFramesPerHour,
		            args[1],
		            (uint8)args[2],
		            args[3],
		            args[4])
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Assign this actor to hunt to be near a location.  The range
//	parameter specifies how close to be near the target.
//		int "c" assignBeNearLocation(
//			int    untilHour,
//			int    targetU,
//			int    targetV,
//			int    targetZ,
//			int    range );

int16 scriptActorAssignBeNearLocation(int16 *args) {
	OBJLOG(AssignBeNearLocation);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;
		TilePoint   targetLoc = TilePoint(args[1], args[2], args[3]);

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new HuntToBeNearLocationAssignment(a,
		            args[0],
		            targetLoc,
		            args[4])
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Assign this actor to hunt to be near another actor.
//	where the target actor is.
//		int "c" assignBeNearActor(
//			int            until,
//			Actor id   target,
//			int            range,
//			int            track );

int16 scriptActorAssignBeNearActor(int16 *args) {
	OBJLOG(AssignBeNearActor);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		assert(isActor(args[1]));

		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj,
		             *targetActor;

		targetActor = (Actor *)GameObject::objectAddress(args[1]);

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new HuntToBeNearActorAssignment(a,
		            args[0],
		            targetActor,
		            args[2],
		            args[3])
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Assign this actor to kill another actor.
//		int "c" assignKillActor( int until, Actor id target, int track );

int16 scriptActorAssignKillActor(int16 *args) {
	OBJLOG(AssignKillActor);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		assert(isActor(args[1]));

		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj,
		             *targetActor;

		targetActor = (Actor *)GameObject::objectAddress(args[1]);

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new HuntToKillAssignment(a,
		            args[0],
		            targetActor,
		            args[2])
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Constructs a tethered wander assignment for this actor
//		int "c" assignTetheredWander(
//			int until,
//			int minU,
//			int minV,
//			int maxU,
//			int maxV );

int16 scriptActorAssignTetheredWander(int16 *args) {
	OBJLOG(AssignTetheredWander);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		TileRegion  tetherReg;
		int16       &minU = args[1],
		             &minV = args[2],
		              &maxU = args[3],
		               &maxV = args[4];

		//  Normalize the coordinates
		if (maxU < minU) {
			int16   temp = maxU;
			maxU = minU;
			minU = temp;
		}
		if (maxV < minV) {
			int16   temp = maxV;
			maxV = minV;
			minV = temp;
		}

		tetherReg.min = TilePoint(minU, minV, 0);
		tetherReg.max = TilePoint(maxU, maxV, 0);

		if (new TetheredWanderAssignment(a,
		            (uint16)args[0]
		            *   CalendarTime::kFramesPerHour,
		            tetherReg)
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Constructs a patrol route assignment for this actor
//		int "c" assignAttend( int frames, GameObject id obj );

int16 scriptActorAssignAttend(int16 *args) {
	OBJLOG(AssignAttend);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		//  Delete the actor's current assignment
		if (a->getAssignment() != nullptr) delete a->getAssignment();

		if (new AttendAssignment(a,
		            (g_vm->_calendar->frameInDay()
		             + (uint16)args[0])
		            %   CalendarTime::kFramesPerDay,
		            GameObject::objectAddress(args[1]))
		        !=  nullptr)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------
//	Remove this actor's current assignment
//		void "c" removeAssignment( void );

int16 scriptActorRemoveAssignment(int16 *args) {
	OBJLOG(removeAssignment);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (a->getAssignment() != nullptr) delete a->getAssignment();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Bands this actor as a follower to the specified leader.
//		void "c" bandWith( Actor id leader );

int16 scriptActorBandWith(int16 *args) {
	OBJLOG(BandWith);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		assert(isActor(args[0]));

		a->bandWith((Actor *)GameObject::objectAddress(args[0]));
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Remove the actor from his current band.
//		void "c" disband( void );

int16 scriptActorDisband(int16 *) {
	OBJLOG(Disband);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		a->disband();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Returns this actor's leader's ID.
//		Actor id "c" getLeader( void );

int16 scriptActorGetLeader(int16 *) {
	OBJLOG(GetLeader);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->_leader != nullptr ? a->_leader->thisID() : Nothing;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Returns the number of followers this actor has.
//		int "c" numFollowers( void );

int16 scriptActorNumFollowers(int16 *) {
	OBJLOG(ActorNumFollowers);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		return a->_followers != nullptr ? a->_followers->size() : 0;
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Returns the ID of the specified follower.
//		Actor id "c" getFollower( int followerNum );

int16 scriptActorGetFollower(int16 *args) {
	OBJLOG(GetFollower);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		assert(a->_followers != nullptr);
		assert(args[0] < a->_followers->size());

		return (*a->_followers)[args[0]]->thisID();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Routine that is called by actor class to use that actor's
//	knowledge list
//		int "c" useKnowledge( GameObject id );

int16 scriptActorUseKnowledge(int16 *) {
	OBJLOG(UseKnowledge);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		a->useKnowledge(thisThread->_threadArgs);
	}

	return thisThread->_threadArgs.returnVal;
}

//-----------------------------------------------------------------------
//	add knowledge package to actor
//		void "c" addKnowledge( int kpIndex );

int16 scriptActorAddKnowledge(int16 *args) {
	OBJLOG(AddKnowledge);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		a->addKnowledge(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	delete knowledge package delete actor
//		void "c" deleteKnowledge( int kpIndex );

int16 scriptActorDeleteKnowledge(int16 *args) {
	OBJLOG(DeleteKnowledge);
	if (isActor(((ObjectData *)thisThread->_thisObject)->obj)) {
		Actor       *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

		if (args[0] == 0) a->clearKnowledge();
		else a->removeKnowledge(args[0]);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	add knowledge package to actor (lasts for duration of mission)
//		void "c" addMissionKnowledge( int missionID, int kpIndex );

int16 scriptActorAddMissionKnowledge(int16 *args) {
	OBJLOG(AddMissionKnowledge);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	ActiveMission   *am = ActiveMission::missionAddress(args[0]);

	if (isActor(obj)) {
		return am->addKnowledgeID(obj->thisID(), args[1]);
	}
	return 0;
}

//-----------------------------------------------------------------------
//	delete mission knowledge package from actor
//		void "c" removeMissionKnowledge( int missionID, int kpIndex );

int16 scriptActorDeleteMissionKnowledge(int16 *args) {
	OBJLOG(DeleteMissionKnowledge);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;
	ActiveMission   *am = ActiveMission::missionAddress(args[0]);

	if (isActor(obj)) {
		return am->removeKnowledgeID(obj->thisID(), args[1]);
	}
	return 0;
}

//-----------------------------------------------------------------------
//	Deduct gold or other payment type from actor
//		void "c" deductPayment( int paymentTypeProto, int paymentAmount );

int16 scriptActorDeductPayment(int16 *args) {
	OBJLOG(DeductPayment);
	Actor           *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

	ProtoObj    *currencyProto = g_vm->_objectProtos[args[0]];
	int32       paymentAmount = args[1];
	int32       paymentFound = 0;
	GameObject  *obj, *delObj = nullptr;
	ObjectID    id;
	bool        mergeable =
	    currencyProto->flags & ResourceObjectPrototype::kObjPropMergeable;

	RecursiveContainerIterator  iter(a);

	//  See if he has enough currency
	for (id = iter.first(&obj); id != Nothing; id = iter.next(&obj)) {
		//  For each object of appropriate currency type
		if (isObject(id) && obj->proto() == currencyProto) {
			if (mergeable) paymentFound += obj->getExtra();
			else paymentFound++;

			if (paymentFound >= paymentAmount) break;
		}
	}

	//  If he doesn't have enough, return false.
	if (paymentFound < paymentAmount) return false;

	//  Now, deduct the actual currency
	for (id = iter.first(&obj); id != Nothing; id = iter.next(&obj)) {
		//  If the object is valid currency
		if (isObject(id) && obj->proto() == currencyProto) {
			//  Mergeable object use the mass count
			if (mergeable) {
				int     massCount = obj->getExtra();

				if (massCount > paymentAmount) {
					obj->setExtra(massCount - paymentAmount);
					g_vm->_cnm->setUpdate(obj->IDParent());
					break;
				} else {
					if (delObj) {
						ObjectID    dParent = delObj->IDParent();
						delObj->deleteObject();
						g_vm->_cnm->setUpdate(dParent);
					}
					paymentAmount -= massCount;
					delObj = obj;
					if (paymentAmount == 0)
						break;
				}
			} else {
				//  Non-mergeable objects count as 1
				paymentAmount--;
				if (delObj) {
					ObjectID    dParent = delObj->IDParent();
					delObj->deleteObject();
					g_vm->_cnm->setUpdate(dParent);
				}
				delObj = obj;
			}
		}
	}

	if (delObj) {
		ObjectID    dParent = delObj->IDParent();
		delObj->deleteObject();
		g_vm->_cnm->setUpdate(dParent);
	}

	//  Payment succeeded!
	return true;
}

//-----------------------------------------------------------------------
//	Count the amount of gold or other payment type actor has
//		void "c" countPayment( int paymentTypeProto );

int16 scriptActorCountPayment(int16 *args) {
	OBJLOG(CountPayment);
	Actor           *a = (Actor *)((ObjectData *)thisThread->_thisObject)->obj;

	ProtoObj    *currencyProto = g_vm->_objectProtos[args[0]];
	int32       paymentFound = 0;
	GameObject  *obj = nullptr;
	ObjectID    id;
	bool        mergeable =
	    currencyProto->flags & ResourceObjectPrototype::kObjPropMergeable;

	RecursiveContainerIterator  iter(a);

	for (id = iter.first(&obj); id != Nothing; id = iter.next(&obj)) {
		if (isObject(id) && obj->proto() == currencyProto) {
			if (mergeable) paymentFound += obj->getExtra();
			else paymentFound++;
		}
	}

	return paymentFound;
}

//-----------------------------------------------------------------------
//	Physician, heal thyself...
//		void "c" acceptHealing( int amt );

int16 scriptActorAcceptHealing(int16 *args) {
	OBJLOG(acceptHealing);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->acceptHealing(obj->thisID(), args[0]);
	return 0;
}

//-----------------------------------------------------------------------
//	Hurt me!
//		void "c" acceptDamage( GameObject id enactor, absDamage, type );

int16 scriptActorAcceptDamage(int16 *args) {
	OBJLOG(acceptHealing);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	obj->acceptDamage(args[0], args[1], (enum effectDamageTypes)args[2]);
	return 0;
}

//-----------------------------------------------------------------------
//	Resurrect
//		void "c" imNotQuiteDead( void );

int16 scriptActorImNotQuiteDead(int16 *args) {
	OBJLOG(imNotQuiteDead);
	GameObject      *obj = ((ObjectData *)thisThread->_thisObject)->obj;

	if (isActor(obj)) {
		((Actor *)obj)->imNotQuiteDead();
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Actor script call table

C_Call *actorCFuncList[] = {
	scriptGameObjectThisID,
	scriptGameObjectRecharge,
	scriptGameObjectGetChargeType,
	scriptActorMove,
	scriptActorMoveRel,
	scriptActorTransfer,
	scriptMoveRandom,
	scriptActorGetName,
	scriptActorSetName,
	scriptActorGetProto,
	scriptActorSetProto,
	scriptActorGetProtoClass,
	scriptActorGetScript,
	scriptActorSetScript,

	scriptGameObjectUse,
	scriptGameObjectUseOn,
	scriptGameObjectUseOnTAI,
	scriptGameObjectDrop,
	scriptGameObjectDropOn,
	scriptGameObjectDropMergeableOn,
	scriptGameObjectDropOnTAI,

	scriptActorSay,
	scriptActorSayText,
	scriptActorObjectType,
	scriptActorCopyObject,

	scriptGameObjectIsActivated,

	scriptActorGetOpen,
	scriptActorGetLocked,
	scriptActorGetImportant,
	scriptActorSetImportant,
	scriptActorGetScavengable,
	scriptActorSetScavengable,

	scriptGameObjectAddTimer,
	scriptGameObjectAddStdTimer,
	scriptGameObjectRemoveTimer,
	scriptGameObjectRemoveAllTimers,

	scriptGameObjectAddProtaganistSensor,
	scriptGameObjectAddSpecificActorSensor,
	scriptGameObjectAddSpecificObjectSensor,
	scriptGameObjectAddActorPropertySensor,
	scriptGameObjectAddObjectPropertySensor,
	scriptGameObjectAddEventSensor,
	scriptGameObjectRemoveSensor,
	scriptGameObjectRemoveAllSensors,

	scriptGameObjectCanSenseProtaganist,
	scriptGameObjectCanSenseSpecificActor,
	scriptGameObjectCanSenseSpecificObject,
	scriptGameObjectCanSenseActorProperty,
	scriptGameObjectCanSenseObjectProperty,

	scriptGameObjectGetActualScript,
	scriptGameObjectGetProtoScript,

	scriptGameObjectGetMass,
	scriptGameObjectSetMass,

	scriptGameObjectGetExtra,
	scriptGameObjectSetExtra,

	scriptGameObjectDeepCopy,
	scriptGameObjectAddEnchantment,
	scriptGameObjectRemoveEnchantment,
	scriptGameObjectFindEnchantment,

	scriptGameObjectInUse,

	scriptActorGetScratchVar,
	scriptActorSetScratchVar,

	scriptActorGetDisposition,
	scriptActorSetDisposition,

	scriptActorGetSkill,
	scriptActorSetSkill,
	scriptActorGetBaseSkill,
	scriptActorSetBaseSkill,

	scriptActorGetVitality,
	scriptActorSetVitality,
	scriptActorGetBaseVitality,
	scriptActorSetBaseVitality,

	scriptActorGetMana,
	scriptActorSetMana,
	scriptActorGetBaseMana,
	scriptActorSetBaseMana,

	scriptActorGetSchedule,
	scriptActorSetSchedule,

	scriptActorLobotomize,
	scriptActorDelobotomize,
	scriptActorIsActionAvailable,
	scriptActorSetAction,
	scriptActorAnimationFrames,
	scriptActorNextAnimationFrame,
	scriptActorFace,
	scriptActorFaceTowards,
	scriptActorTurn,
	scriptActorTurnTowards,
	scriptActorWalk,

	scriptActorAssignPatrolRoute,
	scriptActorAssignPartialPatrolRoute,
	scriptActorAssignBeNearLocation,
	scriptActorAssignBeNearActor,
	scriptActorAssignKillActor,
	scriptActorAssignTetheredWander,
	scriptActorAssignAttend,
	scriptActorRemoveAssignment,

	scriptActorBandWith,
	scriptActorDisband,
	scriptActorGetLeader,
	scriptActorNumFollowers,
	scriptActorGetFollower,

	scriptActorUseKnowledge,
	scriptActorAddKnowledge,
	scriptActorDeleteKnowledge,
	scriptActorAddMissionKnowledge,
	scriptActorDeleteMissionKnowledge,

	scriptActorDeductPayment,
	scriptActorCountPayment,

	scriptActorAcceptHealing,
	scriptActorAcceptDamage,
	scriptActorImNotQuiteDead,
};

CallTable   actorCFuncs = { actorCFuncList, ARRAYSIZE(actorCFuncList), 0 };

//-----------------------------------------------------------------------
//	Return the id of this TAI
//		TileActivityInstance id "c" thisID( void );

int16 scriptTagThisID(int16 *) {
	MONOLOG(TAG::ThisID);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->thisID();
}

//-----------------------------------------------------------------------
//	Return the current state of this tag
//		int "c" getState( void );

int16 scriptTagGetState(int16 *args) {
	MONOLOG(TAG::GetState);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->getInstanceState(ai->getMapNum());
}

//-----------------------------------------------------------------------
//	Set the tag instance to a given state
//		void "c" setState( int state );

int16 scriptTagSetState(int16 *args) {
	MONOLOG(TAG::SetState);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	ai->setInstanceState(ai->getMapNum(), args[0]);

	return 0;
}

//-----------------------------------------------------------------------
//	Return the number of associations for this tag
//		int "c" numAssociations( void );

int16 scriptTagNumAssoc(int16 *args) {
	MONOLOG(TAG::NumAssoc);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->_data.numAssociations;
}

//-----------------------------------------------------------------------
//	Get the Nth association for this tag
//		TileActivityInstance id "c" assoc( int assocNum );

int16 scriptTagAssoc(int16 *args) {
	MONOLOG(TAG::Assoc);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;
	int mapNum       = ai->getMapNum();

	assert(args[0] >= 0);
	assert(args[0] <  ai->_data.numAssociations);
	assert(mapNum >= 0);
	assert(mapNum < 8);

	return (mapList[mapNum].assocList)[ai->_data.associationOffset + args[0]];
}

//-----------------------------------------------------------------------
//	Return the target U coord of this tag
//		int "c" getTargetU( void );

int16 scriptTagGetTargetU(int16 *args) {
	MONOLOG(TAG::GetTargetU);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->_data.instance.targetU;
}

//-----------------------------------------------------------------------
//	Return the target V coord of this tag
//		int "c" getTargetV( void );

int16 scriptTagGetTargetV(int16 *) {
	MONOLOG(TAG::GetTargetV);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->_data.instance.targetV;
}

//-----------------------------------------------------------------------
//	Return the target Z coord of this tag
//		int "c" getTargetU( void );

int16 scriptTagGetTargetZ(int16 *) {
	MONOLOG(TAG::GetTargetZ);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->_data.instance.targetZ;
}

//-----------------------------------------------------------------------
//	Return the target world coord of this tag
//		int "c" getTargetW( void );

int16 scriptTagGetTargetW(int16 *) {
	MONOLOG(TAG::GetTargetW);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->_data.instance.worldNum;
}

//-----------------------------------------------------------------------
//	Return the state of the "locked" bit
//		int "c" isLocked( void );

int16 scriptTagIsLocked(int16 *) {
	MONOLOG(TAG::IsLocked);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->isLocked() ? true : false;
}

//-----------------------------------------------------------------------
//	Change the state of the "locked" bit.
//		int "c" setLocked( void );


int16 scriptTagSetLocked(int16 *args) {
	MONOLOG(TAG::SetLocked);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	ai->setLocked(args[0]);

	return 0;
}

//-----------------------------------------------------------------------
//	Return the tag's key type
//		int "c" getKeyType( void );

int16 scriptTagGetKeyType(int16 *) {
	MONOLOG(TAG::GetKeyType);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return ai->lockType();
}

//-----------------------------------------------------------------------
//	Use this TAI
//		int "c" use( GameObject id enactor );

int16 scriptTagUse(int16 *args) {
	MONOLOG(TAG::Use);
	ActiveItem      *tai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	return tai->use(args[0]);
}

//-----------------------------------------------------------------------
//	Animate the tag
//		void "c" setAnimation( int flags, int targetState );

enum {
	tileAnimateWait = (1 << 0)              // wait until animation finished
};

int16 scriptTagSetAnimation(int16 *args) {
	MONOLOG(TAG::SetAnimation);
	extern uint32 parse_res_id(char IDstr[]);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;
	//TilePoint tagLoc;
	int32       soundID = parse_res_id(STRING(args[2]));
	Location    ail = ai->getInstanceLocation();

	//  Assert that the state is valid
	assert(args[1] >= 0);
	assert(args[1] < ai->getGroup()->_data.group.numStates);

	//  If soundID is not NULL, then play the sound
	if (soundID) playSoundAt(soundID, ail);

	//  If we want to wait until finished
	if (args[0] & tileAnimateWait) {
		//  Wait for the animation
		thisThread->waitForEvent(Thread::kWaitOther, nullptr);

		//  And start the tile animation
		TileActivityTask::doScript(*ai, args[1], getThreadID(thisThread));
	} else {
		//  Else just start the tile animation
		TileActivityTask::doScript(*ai, args[1], NoThread);
	}

	return ai->lockType();
}

//-----------------------------------------------------------------------
//	Wait for animation to finish.
//		void "c" waitAnimate( void );

int16 scriptTagSetWait(int16 *args) {
	MONOLOG(TAG::SetAnimation);
	extern uint32 parse_res_id(char IDstr[]);
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	if (TileActivityTask::setWait(ai, getThreadID(thisThread))) {
		//  Wait for the animation
		thisThread->waitForEvent(Thread::kWaitOther, nullptr);
	}

	return 0;
}

//-----------------------------------------------------------------------
//	Lock the tag for exclusive use, or else go to sleep
//		void "c" obtainLock( void );
//		void "c" releaseLock( void );

#if DEBUG*0
static int16 lockCount;
#endif

int16 scriptTagObtainLock(int16 *) {
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	if (ai->isExclusive() == false) {
		ai->setExclusive(true);
#if DEBUG*0
		lockCount += 1;
		WriteStatusF(15, "Locked: %d\n", lockCount);
#endif
	} else {
		thisThread->waitForEvent(Thread::kWaitTagSemaphore, ai);
#if DEBUG*0
		lockCount += 1;
		WriteStatusF(15, "Locked: %d\n", lockCount);
#endif
	}
	return 0;
}

int16 scriptTagReleaseLock(int16 *) {
	ActiveItem  *ai = ((ActiveItemData *)thisThread->_thisObject)->aItem;

	ai->setExclusive(false);
#if DEBUG*0
	lockCount -= 1;
	WriteStatusF(15, "Locked: %d\n", lockCount);
#endif
	return 0;
}

//-----------------------------------------------------------------------
//	TAG Instance script call table

C_Call *tagCFuncList[] = {
	scriptTagThisID,
	scriptTagGetState,
	scriptTagSetState,
	scriptTagSetAnimation,
	scriptTagNumAssoc,
	scriptTagAssoc,
	scriptTagGetTargetU,
	scriptTagGetTargetV,
	scriptTagGetTargetZ,
	scriptTagGetTargetW,
	scriptTagIsLocked,
	scriptTagSetLocked,
	scriptTagGetKeyType,
	scriptTagUse,
	scriptTagSetWait,
	scriptTagObtainLock,
	scriptTagReleaseLock,
};

CallTable   tagCFuncs = { tagCFuncList, ARRAYSIZE(tagCFuncList), 0 };

//-----------------------------------------------------------------------
//	Find a mission by generator id
//		void "c" deleteMission( int missionID );

int16 scriptMissionDelete(int16 *args) {
	MONOLOG(ActiveMission::Delete);
	ActiveMission       *am = ((ActiveMissionData *)thisThread->_thisObject)->aMission;

	am->cleanup();
	return 0;
}

//-----------------------------------------------------------------------
//	Create a new object
//		GameObject id "C" makeObject(   int protoID,
//										int nameIndex,
//										int scriptIndex );

int16 scriptMakeObject(int16 *args);

int16 scriptMissionMakeObject(int16 *args) {
	MONOLOG(TAG::MakeObject);
	ActiveMission       *am = ((ActiveMissionData *)thisThread->_thisObject)->aMission;
	ObjectID            id;

	//  If there's room in the mission to record the existence of the object
	if (!am->spaceForObject()) return Nothing;

	//  Call the regular make-object function
	id = scriptMakeObject(args);

	//  And record it in the mission object
	if (id != Nothing) {
		am->addObjectID(id);
	}

	return id;
}

//-----------------------------------------------------------------------
//	Create a new object
//		GameObject id "C" makeActor(    int protoID,
//										int nameIndex,
//										int scriptIndex,
//										string appearance,
//										int color );

int16 scriptMakeActor(int16 *args);

int16 scriptMissionMakeActor(int16 *args) {
	MONOLOG(ActiveMission::MakeActor);
	ActiveMission       *am = ((ActiveMissionData *)thisThread->_thisObject)->aMission;
	ObjectID            id;

	//  If there's room in the mission to record the existence of the actor
	if (!am->spaceForObject()) return Nothing;

	//  Call the regular make-actor function. Add in the "permanent"
	//  flag, since actor will be deleted at mission end.
	args[6] |= kActorPermanent;
	id = scriptMakeActor(args);

	//  And record it in the mission object
	if (id != Nothing) {
		am->addObjectID(id);
	}

	return id;
}

//-----------------------------------------------------------------------
//	ActiveMission Instance script call table

C_Call *missionCFuncList[] = {
	scriptMissionDelete,
	scriptMissionMakeObject,
	scriptMissionMakeActor,
};

CallTable   missionCFuncs = { missionCFuncList, ARRAYSIZE(missionCFuncList), 0 };

//-----------------------------------------------------------------------
//	Global functions

int16 scriptWriteMessage(int16 *args) {
	MONOLOG(WriteMessage);
	char        buffer[256];

	stringf(buffer, sizeof buffer, args[1], &args[2]);

	WriteStatusF(args[0], buffer);
	return 0;
}

//-----------------------------------------------------------------------
//	Write a message to the status line
//		void "C" status( string caption, ... );

int16 scriptStatus(int16 *args) {
	MONOLOG(Status);
	char        buffer[256];

	stringf(buffer, sizeof buffer, args[0], &args[1]);

	StatusMsg(buffer);
	return 0;
}

void writeObject(char *str) {
	warning("OBJ: %s", str);
}

int16 scriptWriteLog(int16 *args) {
	MONOLOG(WriteLog);
	char buffer[256];

	stringf(buffer, sizeof buffer, args[0], &args[1]);
	debugC(2, kDebugScripts, "%s", buffer);

	return 0;
}

int16 scriptWriteObject(int16 *args) {
	MONOLOG(WriteObject);
	char      buffer[256];

	stringf(buffer, sizeof buffer, args[0], &args[1]);
	writeObject(buffer);
	return 0;
}
//-----------------------------------------------------------------------
//	Put up an error-message dialog
//		void "C" errorDialog( string caption, ... );

int16 scriptErrorDialog(int16 *args) {
	MONOLOG(ErrorDialog);
	char        buffer[512];

	stringf(buffer, sizeof buffer, args[1], &args[2]);

//	WriteStatusF( 1, buffer );
	return 0;
}

//-----------------------------------------------------------------------
//	Put up a user-notification dialog
//		void "C" messageDialog( string caption, ... );

int16 scriptMessageDialog(int16 *args) {
	MONOLOG(MessageDialog);
	//stringf( buffer, sizeof buffer, args[1], &args[2] );

	userDialog(STRING(args[0]),
	           STRING(args[1]),
	           args[2] ? STRING(args[2]) : nullptr,
	           nullptr,
	           nullptr);

//	WriteStatusF( 1, buffer );
	return 0;
}

//-----------------------------------------------------------------------
//	Put up a multiple-choice button dialog
//		int "C" choiceDialog( string caption, string buttons, ... );

int16 scriptChoiceDialog(int16 *args) {
	MONOLOG(ChoiceDialog);
	//stringf( buffer, sizeof buffer, args[1], &args[2] );

	userDialog(STRING(args[0]),
	           STRING(args[1]),
	           args[2] ? STRING(args[2]) : nullptr,
	           args[3] ? STRING(args[3]) : nullptr,
	           args[4] ? STRING(args[4]) : nullptr);

//	WriteStatusF( 1, buffer );
	return 0;
}

//-----------------------------------------------------------------------
//	Put up a placard (stone, wood, or brass)
//		void "C" placard( int placardType, string text, ... );

// defined in uidialog.h
//enum placardTypes {
//	WOOD_TYPE,
//	STONE_TYPE,
//	BRASS_TYPE
//};

int16 scriptPlacard(int16 *args) {
	MONOLOG(Placard);
	char        buffer[256];

	stringf(buffer, sizeof buffer, args[1], &args[2]);

	placardWindow(args[0], buffer);    // plaq type, text

	//GameDisplay( buffer, 0 );
	return 0;
}

//-----------------------------------------------------------------------
//	Lock the display so that no updates occur
//		void "C" lockDIsplay( int lockState );

int16 scriptLockDisplay(int16 *args) {
	MONOLOG(LockDisplay);
//	WriteStatusF( args[0], STRING( args[1] ) );
	return 0;
}

//-----------------------------------------------------------------------
//	Set the current game mode
//		void "C" setGameMode( int modeNumber );

int16 scriptSetGameMode(int16 *args) {
	MONOLOG(SetGameMode);
	//  Mode zero is "game not running".
	if (args[0] == 0)
		endGame();
	//gameRunning = false;

	//  REM: Add other modes

	return 0;
}

//-----------------------------------------------------------------------
//	Extended-sequence functions (all these functions automatically
//	cause the script to go into extended sequnce mode).

int16 scriptWait(int16 *args) {
	MONOLOG(Wait);
	thisThread->_waitAlarm.set(args[0]);
	thisThread->waitForEvent(Thread::kWaitDelay, nullptr);
	thisThread->setExtended();
	return 0;
}

int16 scriptWaitFrames(int16 *args) {
	MONOLOG(WaitFrames);
	thisThread->_waitFrameAlarm.set(args[0]);
	thisThread->waitForEvent(Thread::kWaitFrameDelay, nullptr);
	thisThread->setExtended();
	return 0;
}

//-----------------------------------------------------------------------
//	Play a song (with optional fade-out of old song)
//		void "C" playSong( int songID, int fade );

int16 scriptPlaySong(int16 *args) {
	MONOLOG(PlaySong);
//	WriteStatusF( args[0], STRING( args[1] ) );
	return 0;
}

//-----------------------------------------------------------------------
//	Play sound effect (with optional looping) { volume = 0-127 }
//		void "C" playFX( int fxID, int volume, int looped );

int16 scriptPlayFX(int16 *args) {
	MONOLOG(PlayFX);
//	WriteStatusF( args[0], STRING( args[1] ) );
	return 0;
}

//-----------------------------------------------------------------------
//	Type-casting function
//		Actor id "C" object2Actor( GameObject id objID );

int16 scriptObject2Actor(int16 *args) {
	MONOLOG(Object2Actor);
	//return isActor(args[0]) ? args[0] : Nothing;
	return args[0];
}

//-----------------------------------------------------------------------
//	Generic script type casting function

int16 scriptGenericCast(int16 *args) {
	MONOLOG(genericCast);
	return args[0];
}

//-----------------------------------------------------------------------
//	Create a new object
//		GameObject id "C" makeObject(   int protoID,
//										int nameIndex,
//										int scriptIndex );

int16 scriptMakeObject(int16 *args) {
	MONOLOG(MakeObject);
	GameObject      *obj = GameObject::newObject();

	//  REM: We need to throw some kind of SAGA exception...?
	//  (We don't have SAGA exceptions, only the C kind...)
	if (obj == nullptr) {
		return 0;
	}

	obj->setProtoNum(args[0]);
	obj->setNameIndex(args[1]);
	obj->setScript(args[2]);

	//  If it's a mergeable object, have it's mass count default to 1.
	if (obj->proto()->flags & ResourceObjectPrototype::kObjPropMergeable)
		obj->setExtra(1);

	return obj->thisID();
}

//-----------------------------------------------------------------------
//	Delete an existing object
//		void "C" deleteObject( GameObject id objID );

int16 scriptDeleteObject(int16 *args) {
	MONOLOG(DeleteObject);
	GameObject      *obj = GameObject::objectAddress(args[0]);
	ObjectID        oldParentID;

	assert(obj);
	oldParentID = obj->IDParent();
	obj->deleteObjectRecursive();
	g_vm->_cnm->setUpdate(oldParentID);

	return 0;
}

//-----------------------------------------------------------------------
//	Create a new object
//		GameObject id "C" makeActor(    int protoID,
//										int nameIndex,
//										int scriptIndex,
//										string appearance,
//										int color );

int16 scriptMakeActor(int16 *args) {
	MONOLOG(MakeActor);
	char        *actorAppearanceName = STRING(args[3]);
	int32       actorAppearanceNum;
	Actor       *a;

	assert(actorAppearanceName);
	actorAppearanceNum = READ_BE_INT32(actorAppearanceName);

	a = Actor::newActor(
	        args[0],
	        args[1],
	        args[2],
	        actorAppearanceNum,
	        args[4],
	        args[5],
	        args[6]);

	//  REM: We need to throw some kind of SAGA exception...?
	//  (We don't have SAGA exceptions, only the C kind...)
	if (a == nullptr) {
		return 0;
	}

	return a->thisID();
}

//-----------------------------------------------------------------------
//	Get the ID of the center actor
//		Actor id "C" getCenterActor( void );

int16 scriptGetCenterActor(int16 *) {
	MONOLOG(GetCenterActor);
	return getCenterActorID();
}

//-----------------------------------------------------------------------
// SAGA Import defs
//
//  void "C" scriptPlaySound( string );
//
//  void "C" scriptPlayVoice( string );
//
//  void "C" scriptPlayLoop( string );
//
//  void "C" scriptPlayMusic( string );
//

int16 scriptPlaySound(int16 *args) {
	MONOLOG(PlaySound);
	char        *sID = STRING(args[0]);

	PlaySound(sID);

	return 0;
}

uint32 parse_res_id(char IDstr[]);

int16 scriptPlaySoundAt(int16 *args) {
	MONOLOG(PlaySoundAt);
	char        *sID = STRING(args[0]);
	args++;
	int16       u    = *args++; // << kTileUVShift;
	int16       v    = *args++; // << kTileUVShift;
	int16       h    = *args++;
	Location l = Location(TilePoint(u, v, h), Nothing);
	int32       soundID;
	soundID = parse_res_id(sID);

	if (soundID) playSoundAt(soundID, l);

	return 0;
}

int16 scriptPlaySoundFrom(int16 *args) {
	MONOLOG(PlaySoundAt);
	char        *sID = STRING(args[0]);
	int32       soundID;
	soundID = parse_res_id(sID);
	GameObject *go = GameObject::objectAddress(args[1]);
	assert(go != nullptr);
	if (soundID) playSoundAt(soundID, go->notGetWorldLocation());

	return 0;
}

int16 scriptPlayMusic(int16 *args) {
	MONOLOG(PlayMusic);
	char        *sID = STRING(args[0]);
	PlayMusic(sID);
	return 0;
}

int16 scriptPlayLoop(int16 *args) {
	MONOLOG(PlayLoop);
	return 0;
}


void PlayLoopAt(char IDstr[], Location l);

int16 scriptPlayLoopAt(int16 *args) {
	MONOLOG(PlayLoop);
	char        *sID = STRING(args[0]);
	int16       u    = *args++; // << kTileUVShift;
	int16       v    = *args++; // << kTileUVShift;
	int16       h    = *args++;
	Location l = Location(TilePoint(u, v, h), Nothing);
	PlayLoopAt(sID, l);
	return 0;
}

int16 scriptPlayVoice(int16 *args) {
	MONOLOG(PlayVoice);
	char        *sID = STRING(args[0]);
	PlayVoice(sID);
	return 0;
}

//-----------------------------------------------------------------------
//	int "c" getHour( void );

int16 scriptGetHour(int16 *) {
	MONOLOG(GetHour);
	return g_vm->_calendar->_hour;
}

//-----------------------------------------------------------------------
//	int "c" getFrameInHour( void );

int16 scriptGetFrameInHour(int16 *) {
	MONOLOG(GetFrameInHour);
	return g_vm->_calendar->_frameInHour;
}

//-----------------------------------------------------------------------
//	int "c" getRandomBetween( int,  int );

int16 scriptGetRandomBetween(int16 *args) {
	MONOLOG(GetRandomBetween);
	return (GetRandomBetween(args[0], args[1]));
}

//-----------------------------------------------------------------------
//	Check if Object contained in another object
//		bool "C" isContaining( GameObject id owner, GameObject id object);

int16 scriptIsContaining(int16 *args) {
	MONOLOG(IsContaining);
	GameObject *containerObject = GameObject::objectAddress(args[0]);
	GameObject *containedObject = GameObject::objectAddress(args[1]);

	return (containerObject->isContaining(containedObject));
}

//-----------------------------------------------------------------------
//  void "C" scriptPlayLongSound( string );

int16 scriptPlayLongSound(int16 *args) {
	MONOLOG(PlayLongSound);
	char        *sID = STRING(args[0]);

	PlayLongSound(sID);

	return 0;
}

//-----------------------------------------------------------------------
//   int "C" scriptResID( string  );  GT - 03/04/1996
//   converts a standard HRES resource ID into a 16-bit integer.  The
//   first character is interpreted as thousands, providing a total
//   possible range of between 1-25,999.
//-----------------------------------------------------------------------

int16 scriptResID(int16 *args) {
	char *sID = STRING(args[0]);
	return (sID[0] - 'A') * 1000 + atoi(&sID[1]);
}

int16 scriptWorldNum2Object(int16 *args) {
	MONOLOG(WorldNum2Object);
	assert(args[0] >= 0);
	//  REM: I can't seem to find a symbolic constant for the
	//  maximum number of worlds. I know that it's currently 8.
	assert(args[0] < 8);

	return args[0] + WorldBaseID;
}

//-----------------------------------------------------------------------
//	Append a set of strings to the book text
//		void "C" bookText( string text, ... );

int16 scriptAppendBookText(int16 *args) {
	MONOLOG(AppendBookText);
	//  If optional 4th parameter is present, then set actor facing
	for (int i = 0; i < thisThread->_argCount; i++) {
		char        *bookText = STRING(args[i]);

		appendBookText(bookText);
	}
	return 0;
}

#if 0
//-----------------------------------------------------------------------
//	Append a single string to the scroll text, with printf formatting
//		void "C" bookTextF( string text, ... );

int16 scriptAppendScrollTextF(int16 *args) {
	MONOLOG(AppendScrollTextF);
	char        buffer[256];

	stringf(buffer, sizeof buffer, args[0], &args[1]);

	appendBookText(buffer);

	return 0;
}

//-----------------------------------------------------------------------
//	Append a set of strings to the scroll text
//		void "C" bookText( string text, ... );

int16 scriptAppendScrollText(int16 *args) {
	MONOLOG(AppendScrollText);
	//  If optional 4th parameter is present, then set actor facing
	for (int i = 0; i < thisThread->_argCount; i++) {
		char        *ScrollText = STRING(args[i]);

		appendBookText(ScrollText);
	}
	return 0;
}
#endif

//-----------------------------------------------------------------------
//	Append a single string to the book text, with printf formatting
//		void "C" bookTextF( string text, ... );

int16 scriptAppendBookTextF(int16 *args) {
	MONOLOG(AppendBookTextF);
	char        buffer[256];

	stringf(buffer, sizeof buffer, args[0], &args[1]);

	appendBookText(buffer);

	return 0;
}


//-----------------------------------------------------------------------
//	Assert an event for any sensors which might be looking for it
//	void "c" assertEvent(
//		int             type,
//		GameObject id   directObject,
//		GameObject id   indirectObject );

int16 scriptAssertEvent(int16 *args) {
	MONOLOG(AssertEvent);
	GameEvent       ev;

	assert(isObject(args[1]) || isActor(args[1]));
	assert(args[2] == Nothing
	       ||  isObject(args[2])
	       ||  isActor(args[2]));

	ev.type = args[0];
	ev.directObject = GameObject::objectAddress(args[1]);
	ev.indirectObject = args[2] != Nothing
	                    ?   GameObject::objectAddress(args[2])
	                    :   nullptr;

	assertEvent(ev);

	return 0;
}

//-----------------------------------------------------------------------
//	Spell casting
//
//bool "C"  scriptCanCast    (
//				GameObject id casterID,
//				GameObject id spellID);
//
//void "C"  scriptCastSpellAtObject   (
//				GameObject id casterID,
//				GameObject id spellID,
//				GameObject id targetID);
//
//void "C"  scriptCastSpellAtActor    (
//				GameObject id casterID,
//				GameObject id spellID,
//				GameObject id targetID);
//
//void "C"  scriptCastSpellAtWorld    (
//				GameObject id casterID,
//				GameObject id spellID);
//
//void "C"  scriptCastSpellAtTAG      (
//				GameObject id casterID,
//				GameObject id spellID,
//				TileActivityInstance id target);
//
//void "C"  scriptCastSpellAtTile         (
//				GameObject id casterID,
//				GameObject id spellID,
//				int u, int v, int h);
//

int16 scriptCanCast(int16 *args) {
	MONOLOG(CanCast);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	assert(caster);
	assert(spell);
	return canCast(caster, spell);
}

int16 scriptCastSpellAtObject(int16 *args) {
	MONOLOG(CastSpellAtObject);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	GameObject  *target = GameObject::objectAddress(*args++);
	assert(caster);
	assert(spell);
	assert(target);
	castSpell(caster, target, spell);
	return 0;
}

int16 scriptCastSpellAtActor(int16 *args) {
	MONOLOG(CastSpellAtActor);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	GameObject  *target = GameObject::objectAddress(*args++);
	assert(caster);
	assert(spell);
	assert(target);
	castSpell(caster, target, spell);
	return 0;
}

int16 scriptCastSpellAtWorld(int16 *args) {
	MONOLOG(CastSpellAtWorld);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	assert(caster);
	assert(spell);
	castUntargetedSpell(caster, spell);
	return 0;
}

int16 scriptCastSpellAtTAG(int16 *args) {
	MONOLOG(CastSpellAtTAG);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	ActiveItem  *ai     = ActiveItem::activeItemAddress(*args++);
	assert(caster);
	assert(spell);
	assert(ai);
	castSpell(caster, ai, spell);
	return 0;
}

int16 scriptCastSpellAtTile(int16 *args) {
	MONOLOG(CastSpellAtTile);
	GameObject  *caster = GameObject::objectAddress(*args++);
	SkillProto  *spell  = skillProtoFromID(*args++);
	int16       u    = *args++; // << kTileUVShift;
	int16       v    = *args++; // << kTileUVShift;
	int16       h    = *args++;
	Location l = Location(TilePoint(u, v, h), Nothing);
	assert(caster);
	assert(spell);
	castSpell(caster, l, spell);
	return 0;
}

//-----------------------------------------------------------------------
//	Function to select a site for a monster actor or object.
//	returns it's result by filling in the threadCoords therad var.
//	If the return height is negative, then it failed.
//		int "C" SelectNearbySite(   int centerU,
//									int centerV,
//									int centerZ,
//									int centerWorld,
//									int minDist,
//									int maxDist,
//									int offScreenOnly );

int16 scriptSelectNearbySite(int16 *args) {
	MONOLOG(SelectNearbySite);
	TilePoint       tp;

	tp = selectNearbySite(args[3],
	                      TilePoint(args[0], args[1], args[2]),
	                      args[4],
	                      args[5],
	                      args[6]);

	if (tp == Nowhere) return 0;
	scriptCallFrame     &scf = thisThread->_threadArgs;

	scf.coords = tp;
	return true;
}

//-----------------------------------------------------------------------
//	Pick a random actor who is not dead
//		GameObject id pickRandomLivingActor( ... );

int16 scriptPickRandomLivingActor(int16 *args) {
	MONOLOG(PickRandomLivingActor);
	int             livingCount = 0,
	                i;

	for (i = 0; i < thisThread->_argCount; i++) {
		if (isActor(args[i])) {
			Actor       *a = (Actor *)GameObject::objectAddress(args[i]);

			if (!a->isDead()) livingCount++;
		}
	}

	if (livingCount <= 0) return Nothing;

	livingCount = g_vm->_rnd->getRandomNumber(livingCount - 1);

	for (i = 0; i < thisThread->_argCount; i++) {
		if (isActor(args[i])) {
			Actor       *a = (Actor *)GameObject::objectAddress(args[i]);

			if (!a->isDead()) {
				if (livingCount == 0) return args[i];
				livingCount--;
			}
		}
	}

	return Nothing;
}

//-----------------------------------------------------------------------
//	Create a new mission object
//		int "c" newMission( GameObject id generatorID );

int16 scriptNewMission(int16 *args) {
	MONOLOG(NewMission);
	ActiveMission   *am = ActiveMission::newMission(args[0], args[1]);

	return am ? am->getMissionID() : -1;
}

//-----------------------------------------------------------------------
//	Find a mission by generator id
//		int "c" findMission( GameObject id generatorID );

int16 scriptFindMission(int16 *args) {
	MONOLOG(FindMission);
	return ActiveMission::findMission(args[0]);
}

//-----------------------------------------------------------------------
//	Set the speed of a tile cycling range
//		void "c" setTileCycleSpeed( int range, int speed );

int16 scriptSetTileCycleSpeed(int16 *args) {
	MONOLOG(SetTileCycleSpeed);
	extern CyclePtr  _cycleList;          // list of tile cycling info

	TileCycleData   &tcd = _cycleList[args[0]];

	tcd._cycleSpeed = args[1];

	return 0;
}

//-----------------------------------------------------------------------
//	Set the state of a tile cycling range
//		void "c" setTileCycleState( int range, int state );

int16 scriptSetTileCycleState(int16 *args) {
	MONOLOG(SetTileCycleState);
	extern CyclePtr  _cycleList;          // list of tile cycling info

	TileCycleData   &tcd = _cycleList[args[0]];

	tcd._currentState = args[1];
	tcd._counter = 0;

	return 0;
}

//-----------------------------------------------------------------------
//	Search a region of the tile map to see if any of the indicated
//	objects are within that region.
//		int "c" searchRegion(   GameObject id world,
//								int uMin. int vMin,
//								int uMax, int vMax, ... );

int16 scriptSearchRegion(int16 *args) {
	MONOLOG(SearchRegion);
	GameWorld       *worldPtr;
	ObjectID        searchObj;
	int             count = 0;
	TilePoint       minP,
	                maxP;

	//  Get a pointer to the world
	assert(isWorld(args[0]));
	worldPtr = (GameWorld *)GameObject::objectAddress(args[0]);
	assert(worldPtr != nullptr);

	minP.u = MIN(args[1], args[3]);
	minP.v = MIN(args[2], args[4]);
	minP.z = -128;
	maxP.u = MAX(args[1], args[3]);
	maxP.v = MAX(args[2], args[4]);
	maxP.z = 127;

	//  Set up an iterator
	RegionalObjectIterator  iter(worldPtr, minP, maxP);

	//  Iterate through the search region
	for (searchObj = iter.first(nullptr);
	        searchObj != Nothing;
	        searchObj = iter.next(nullptr)) {
		//  Starting from the 5th argument, until we reach _argCount,
		//  see if the iterated object matches one in the arg list
		for (int i = 5; i < thisThread->_argCount; i++) {
			if (args[i] == searchObj) {
				count++;
				break;
			}
		}
	}

	//  Return number of items found
	return count;
}

//-----------------------------------------------------------------------
//	Helper function: Returns the number of objects in a TileRegion

int countObjectsInRegion(GameWorld *worldPtr, TileRegion &tr) {
	ObjectID        searchObj;
	int             count;

	//  Count how many objects are in the first region
	RegionalObjectIterator  iter(worldPtr,
	                             tr.min,
	                             tr.max);

	for (searchObj = iter.first(nullptr), count = 0;
	        searchObj != Nothing;
	        searchObj = iter.next(nullptr)) {
		count++;
	}

	return count;
}

//-----------------------------------------------------------------------
//	Helper Function: Returns a list of objects in a tile region

void listObjectsInRegion(
    GameWorld       *worldPtr,
    TileRegion      &tr,
    ObjectID        *list) {
	ObjectID        searchObj;

	//  Count how many objects are in the first region
	RegionalObjectIterator  iter(worldPtr,
	                             tr.min,
	                             tr.max);

	for (searchObj = iter.first(nullptr);
	        searchObj != Nothing;
	        searchObj = iter.next(nullptr)) {
		*list++ = searchObj;
	}
}

//-----------------------------------------------------------------------
//	Swap all of the objects within two regions
//		void "c" swapRegions(   GameObject id world1, int u1, int v1,
//								GameObject id world2, int u2, int v2,
//								int uSize, int vSize );

int16 scriptSwapRegions(int16 *args) {
	MONOLOG(SwapRegions);

	ObjectID        worldID1 = args[0],
	                worldID2 = args[3];
	GameWorld       *worldPtr1,
	                *worldPtr2;
	ObjectID        *objArray1,
	                *objArray2;
	int             objCount1,
	                objCount2;
	TileRegion      region1,
	                region2;

	assert(isWorld(worldID1));
	assert(isWorld(worldID2));

	worldPtr1 = (GameWorld *)GameObject::objectAddress(worldID1);
	worldPtr2 = (GameWorld *)GameObject::objectAddress(worldID2);

	assert(worldPtr1 != nullptr);
	assert(worldPtr2 != nullptr);

	region1.min.u = args[1];
	region1.min.v = args[2];
	region1.min.z = -128;
	region1.max.u = args[1] + ABS(args[6]);
	region1.max.v = args[2] + ABS(args[7]);
	region1.max.z = 127;

	region2.min.u = args[4];
	region2.min.v = args[5];
	region2.min.z = -128;
	region2.max.u = args[4] + ABS(args[6]);
	region2.max.v = args[5] + ABS(args[7]);
	region2.max.z = 127;

	//  Count how many objects are in each region
	objCount1 = countObjectsInRegion(worldPtr1, region1);
	objCount2 = countObjectsInRegion(worldPtr2, region2);

	//  Allocate an array to hold object ID's for each region
	objArray1 = new ObjectID[objCount1];
	assert(objArray1);
	objArray2 = new ObjectID[objCount2];
	assert(objArray2);

	//  Get a list of the objects in each region
	listObjectsInRegion(worldPtr1, region1, objArray1);
	listObjectsInRegion(worldPtr2, region2, objArray2);

	int i;

	//  Move all the objects in the first list to region 2
	for (i = 0; i < objCount1; i++) {
		GameObject      *obj = GameObject::objectAddress(objArray1[i]);
		Location    loc;
		TilePoint   tp;

		tp = obj->getLocation();

		loc._context = worldID2;
		loc.u = tp.u + region2.min.u - region1.min.u;
		loc.v = tp.v + region2.min.v - region1.min.v;
		loc.z = tp.z;

		obj->move(loc);
	}

	//  Move all the objects in the second list to region 1
	for (i = 0; i < objCount2; i++) {
		GameObject      *obj = GameObject::objectAddress(objArray2[i]);
		Location    loc;
		TilePoint   tp;

		tp = obj->getLocation();

		loc._context = worldID1;
		loc.u = tp.u + region1.min.u - region2.min.u;
		loc.v = tp.v + region1.min.v - region2.min.v;
		loc.z = tp.z;

		obj->move(loc);
	}

	delete[] objArray1;
	delete[] objArray2;

	return 0;
}

//-----------------------------------------------------------------------
//	Temporarily disable the updating of the tile display
//		void "c" lockTiles( int lockState );

extern bool tileLockFlag;

int16 scriptLockTiles(int16 *args) {
	MONOLOG(LockTiles);
	if (args[0] == false) tileLockFlag = false;
	else tileLockFlag = true;

	return 0;
}

//-----------------------------------------------------------------------
//	Get the current attitude of a faction
//		int "c" getFactionTally( actor.faction, int column );

int16 scriptGetFactionTally(int16 *args) {
	MONOLOG(GetFactionTally);
	return GetFactionTally(args[0], (enum factionTallyTypes)args[1]);
}

//-----------------------------------------------------------------------
//	Adjust the attitude of a faction
//		int "c" addFactionTally( actor.faction, int column, int amount );

int16 scriptAddFactionTally(int16 *args) {
	MONOLOG(AddFactionTally);
	return AddFactionTally(args[0], (enum factionTallyTypes)args[1], args[2]);
}

//-----------------------------------------------------------------------
//	Return the count of how many temp actors of this type exist.
//		int "c" numTempActors( int protoNum );

extern int16 actorProtoCount;
extern int16 objectProtoCount;

int16 scriptNumTempActors(int16 *args) {
	MONOLOG(NumTempActors);

	assert(args[0] >= 0);
	assert(args[0] < actorProtoCount);

	return getTempActorCount(args[0]);
}

//-----------------------------------------------------------------------
//	Return the base price of an object, given the prototype
//		int16 "c" getObjectBasePrice( int protoNum );

int16 scriptGetObjectBasePrice(int16 *args) {
	MONOLOG(GetBaseObjectPrice);

	assert(args[0] >= 0);
	assert(args[0] < objectProtoCount);

	return g_vm->_objectProtos[args[0]]->price;
}

//-----------------------------------------------------------------------
//	Win the game
//		int16 "c" gotoWinMode( void );

int16 scriptGotoWinMode(int16 *args) {
	MONOLOG(gotoWinMode);
	int16 winType = args[0];
	setWintroMode(winType);
	return 0;
}


//-----------------------------------------------------------------------
//	open automap
//		void "c" openAutoMap( void );

int16 scriptOpenAutoMap(int16 *args) {
	MONOLOG(openAutoMap);
	openAutoMap();
	return 0;
}


//-----------------------------------------------------------------------
//  play a video
//  void "C" scriptPlayVideo( string );

int16 scriptPlayVideo(int16 *args) {
	MONOLOG(PlaySound);
	char        *sID = STRING(args[0]);

	openVidBox(sID);

	return 0;
}

//-----------------------------------------------------------------------
//	Returns the horizontal distance between two objects
//		int "c" distanceBetween( GameObject id obj1, GameObject id obj2 );

int16 scriptDistanceBetween(int16 *args) {
	MONOLOG(distanceBetween);

	assert((isObject(args[0]) || isActor(args[0]))
	       && (isObject(args[1]) || isActor(args[1])));

	GameObject      *obj1 = GameObject::objectAddress(args[0]),
	                 *obj2 = GameObject::objectAddress(args[1]);

	return (obj1->getLocation() - obj2->getLocation()).quickHDistance();
}

//-----------------------------------------------------------------------
//	Transport the center actor and all banded brothers who have a path
//	to the center actor
//		void "c" transportCenterBand( GameObject id context, int u, int v, int z )

int16 scriptTransportCenterBand(int16 *args) {
	MONOLOG(transportCenterBand);

	assert(isWorld(args[0]));

	transportCenterBand(Location(args[1], args[2], args[3], args[0 ]));

	return 0;
}

//-----------------------------------------------------------------------
//	Lock the user interface
//		void "c" lockUI( int locked )

int16 scriptLockUI(int16 *args) {
	MONOLOG(lockUI);

	noStickyMap();
	LockUI(args[0]);
	return 0;
}

//-----------------------------------------------------------------------
//	Return number of pending speeches in speech queue
//		int "c" pendingSpeeches( void );

int16 scriptPendingSpeeches(int16 *args) {
	MONOLOG(PendingSpeeches);

	return speechList.activeCount();
}

//-----------------------------------------------------------------------
//	Redraw the main tile display
//		void "c" drawFrame( void );

int16 scriptDrawFrame(int16 *) {
	MONOLOG(DrawFrame);

	drawMainDisplay();
	return 0;
}

//-----------------------------------------------------------------------
//	Fade down the palette - duh
//		void "c" fadeDown( void );

int16 scriptFadeDown(int16 *) {
	MONOLOG(FadeDown);

	fadeDown();
	return 0;
}

//-----------------------------------------------------------------------
//	Fade up the palette
//		void "c" fadeUp( void );

int16 scriptFadeUp(int16 *) {
	MONOLOG(FadeUp);

	fadeUp();
	return 0;
}

//-----------------------------------------------------------------------
//	Enable or disable script task switching.  Returns previous synchronous
//	setting.
//		int "c" setSynchronous( int val );

int16 scriptSetSynchronous(int16 *args) {
	MONOLOG(SetSynchronous);

	int16       oldVal = (thisThread->_flags & Thread::kTFSynchronous) != 0;

	if (args[0])
		thisThread->_flags |= Thread::kTFSynchronous;
	else
		thisThread->_flags &= ~Thread::kTFSynchronous;

	return oldVal;
}

//-----------------------------------------------------------------------
//	Multiplication with overflow protection
//		int "c" bigMul( int m1, int m2, int d );

int16 scriptBigMul(int16 *args) {
	MONOLOG(BigMul);

	long        result = (long)args[0] * (long)args[1];

	if (args[2] == 0) result = 0;
	else result /= args[2];

	result = clamp((short)minint16, (long)result, (short)maxint16);

	return (int16)result;
}

//-----------------------------------------------------------------------
//	Global script call table

C_Call *globalCFuncList[] = {
	scriptWriteMessage,
	scriptStatus,
	scriptErrorDialog,
	scriptMessageDialog,
	scriptChoiceDialog,
	scriptPlacard,
	scriptLockDisplay,
	scriptSetGameMode,

	scriptWait,
	scriptWaitFrames,

	scriptPlaySong,
	scriptPlayFX,

	scriptObject2Actor,

	scriptGenericCast,
	scriptGenericCast,
	scriptGenericCast,
	scriptGenericCast,
	scriptGenericCast,
	scriptGenericCast,

	scriptWriteLog,
	scriptWriteObject,
	scriptMakeObject,
	scriptDeleteObject,
	scriptMakeActor,
	scriptGetCenterActor,

	scriptPlaySound,
	scriptPlayVoice,
	scriptPlayMusic,
	scriptPlayLoop,
	scriptResID,

	scriptGetHour,
	scriptGetFrameInHour,
	scriptGetRandomBetween,
	scriptIsContaining,
	scriptPlayLongSound,
	scriptWorldNum2Object,

	scriptAppendBookText,
	scriptAppendBookTextF,
	scriptAppendBookText,   //  scriptAppendScrollText,
	scriptAppendBookTextF,  //  scriptAppendScrollText,

	scriptCastSpellAtObject,
	scriptCastSpellAtActor,
	scriptCastSpellAtWorld,
	scriptCastSpellAtTAG,
	scriptCastSpellAtTile,

	scriptAssertEvent,

	scriptSelectNearbySite,

	scriptPickRandomLivingActor,
	scriptNewMission,
	scriptFindMission,

	scriptSetTileCycleSpeed,
	scriptSetTileCycleState,

	scriptSearchRegion,
	scriptSwapRegions,
	scriptLockTiles,

	scriptGetFactionTally,
	scriptAddFactionTally,

	scriptNumTempActors,
	scriptGetObjectBasePrice,

	scriptGotoWinMode,
	scriptOpenAutoMap,

	scriptCanCast,

	scriptPlayVideo,

	scriptDistanceBetween,

	scriptTransportCenterBand,
	scriptLockUI,

	scriptPendingSpeeches,

	scriptDrawFrame,
	scriptFadeDown,
	scriptFadeUp,

	scriptSetSynchronous,

	scriptPlaySoundAt,
	scriptPlaySoundFrom,
	scriptPlayLoopAt,

	scriptBigMul,
};

CallTable   globalCFuncs = { globalCFuncList, ARRAYSIZE(globalCFuncList), 0 };

} // end of namespace Saga2
