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
#include "saga2/objects.h"
#include "saga2/sensor.h"
#include "saga2/player.h"
#include "saga2/tile.h"
#include "saga2/savefile.h"

namespace Saga2 {

/* ===================================================================== *
   SensorList management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Allocate a new SensorList

void newSensorList(SensorList *s) {
	g_vm->_sensorListList.push_back(s);
}

//----------------------------------------------------------------------
//	Deallocate an SensorList

void deleteSensorList(SensorList *s) {
	g_vm->_sensorListList.remove(s);
}

/* ===================================================================== *
   Sensor management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Allocate a new Sensor

void newSensor(Sensor *s) {
	g_vm->_sensorList.push_back(s);

	s->checkCtr = sensorCheckRate;
}

//----------------------------------------------------------------------
//	Allocate a new Sensor with a specified starting check counter

void newSensor(Sensor *s, int16 ctr) {
	newSensor(s);

	s->checkCtr = ctr;
}

//----------------------------------------------------------------------
//	Deallocate a Sensor

void deleteSensor(Sensor *p) {
	g_vm->_sensorList.remove(p);
}

//----------------------------------------------------------------------
//	Reconstruct a Sensor from an archive buffer

void *constructSensor(int16 ctr, void *buf) {
	int16           type;
	Sensor          *sensor = nullptr;
	SensorList      *sl;

	//  Get the sensor type
	type = *((int16 *)buf);
	buf = (int16 *)buf + 1;

	switch (type) {
	case protaganistSensor:
		sensor = new ProtaganistSensor(&buf, ctr);
		break;

	case specificObjectSensor:
		sensor = new SpecificObjectSensor(&buf, ctr);
		break;

	case objectPropertySensor:
		sensor = new ObjectPropertySensor(&buf, ctr);
		break;

	case specificActorSensor:
		sensor = new SpecificActorSensor(&buf, ctr);
		break;

	case actorPropertySensor:
		sensor = new ActorPropertySensor(&buf, ctr);
		break;

	case eventSensor:
		sensor = new EventSensor(&buf, ctr);
		break;
	}

	assert(sensor != nullptr);

	//  Get the sensor list
	sl = fetchSensorList(sensor->getObject());

	assert(sl != nullptr);

	//  Append this Sensor to the sensor list
	sl->_list.push_back(sensor);

	return buf;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive the specified Sensor in
//	an archive buffer

int32 sensorArchiveSize(Sensor *sensor) {
	assert(sensor != NULL);

	return      sizeof(int16)                //  Type
	            +   sensor->archiveSize();
}

//----------------------------------------------------------------------
//	Archive the specified Sensor in an archive buffer

void *archiveSensor(Sensor *sensor, void *buf) {
	assert(sensor != NULL);

	//  Store the sensor type
	*((int16 *)buf) = sensor->getType();
	buf = (int16 *)buf + 1;

	//  Let the sensor store its data in the buffer
	buf = sensor->archive(buf);

	return buf;
}

//----------------------------------------------------------------------

void checkSensors(void) {
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		Sensor *sensor = *it;

		if (--sensor->checkCtr <= 0) {
			assert(sensor->checkCtr == 0);

			SenseInfo   info;
			GameObject  *senseobj = sensor->getObject();
			uint32      sFlags = nonActorSenseFlags;
			if (isActor(senseobj)) {
				Actor *a = (Actor *)senseobj;
				sFlags = a->enchantmentFlags;
			}

			if (sensor->check(info, sFlags)) {
				assert(info.sensedObject != NULL);
				assert(isObject(info.sensedObject) || isActor(info.sensedObject));

				sensor->getObject()->senseObject(sensor->thisID(), info.sensedObject->thisID());
			}

			sensor->checkCtr = sensorCheckRate;
		}
	}
}

//----------------------------------------------------------------------

void assertEvent(const GameEvent &ev) {
	assert(ev.directObject != NULL);
	assert(isObject(ev.directObject) || isActor(ev.directObject));

	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		Sensor *sensor = *it;

		if (sensor->evaluateEvent(ev)) {
			sensor->getObject()->senseEvent(
			    sensor->thisID(),
			    ev.type,
			    ev.directObject->thisID(),
			    ev.indirectObject != NULL
			    ?   ev.indirectObject->thisID()
			    :   Nothing);
		}
	}
}

//----------------------------------------------------------------------
//	Initialize the sensors

void initSensors(void) {
	//  Nothing to do
	assert(sizeof(ProtaganistSensor) <= maxSensorSize);
	assert(sizeof(SpecificObjectSensor) <= maxSensorSize);
	assert(sizeof(ObjectPropertySensor) <= maxSensorSize);
	assert(sizeof(SpecificActorSensor) <= maxSensorSize);
	assert(sizeof(ActorPropertySensor) <= maxSensorSize);
	assert(sizeof(EventSensor) <= maxSensorSize);
}

//----------------------------------------------------------------------
//	Save all active sensors in a save file

void saveSensors(SaveFileConstructor &saveGame) {
	warning("STUB: saveSensort()");
#if 0
	int16                   g_vm->_sensorListCount = 0,
	                        sensorCount = 0;

	SensorListHolder        *listHolder;
	SensorHolder            *sensorHolder;

	void                    *archiveBuffer,
	                        *bufferPtr;
	int32                   archiveBufSize = 0;

	//  Add the sizes of the sensor list count an sensor count
	archiveBufSize += sizeof(g_vm->_sensorListCount) + sizeof(sensorCount);

	//  Tally the sensor lists
	for (listHolder = (SensorListHolder *)g_vm->_sensorListList.first();
	        listHolder != NULL;
	        listHolder = (SensorListHolder *)listHolder->next())
		g_vm->_sensorListCount++;

	//  Add the total archive size of all of the sensor lists
	archiveBufSize += g_vm->_sensorListCount * SensorList::archiveSize();

	//  Tally the sensors and add the archive size of each
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		sensorCount++;
		archiveBufSize += sizeof((*it)->checkCtr) + sensorArchiveSize(*it);
	}

	//  Allocate an archive buffer
	archiveBuffer = RNewPtr(archiveBufSize, NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate sensor archive buffer");

	bufferPtr = archiveBuffer;

	//  Store the sensor list count and sensor count
	*((int16 *)bufferPtr)      = g_vm->_sensorListCount;
	*((int16 *)bufferPtr + 1)  = sensorCount;
	bufferPtr = (int16 *)bufferPtr + 2;

	//  Archive all sensor lists
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); ++it) {
		bufferPtr = (*it)->archive(bufferPtr);

	//  Archive all sensors
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		*((int16 *)bufferPtr) = (*it)->checkCtr;
		bufferPtr = (int16 *)bufferPtr + 1;

		bufferPtr = archiveSensor(*it, bufferPtr);
	}

	assert(bufferPtr == &((uint8 *)archiveBuffer)[archiveBufSize]);

	//  Write the data to the save file
	saveGame.writeChunk(
	    MakeID('S', 'E', 'N', 'S'),
	    archiveBuffer,
	    archiveBufSize);

	RDisposePtr(archiveBuffer);
#endif
}

//----------------------------------------------------------------------
//	Load sensors from a save file

void loadSensors(SaveFileReader &saveGame) {
	warning("STUB: loadSensort()");
#if 0
	int16       i,
	            g_vm->_sensorListCount,
	            sensorCount;

	void        *archiveBuffer,
	            *bufferPtr;

	//  Allocate a buffer in which to read the archive data
	archiveBuffer = RNewPtr(saveGame.getChunkSize(), NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate sensor archive buffer");

	//  Read the data
	saveGame.read(archiveBuffer, saveGame.getChunkSize());

	bufferPtr = archiveBuffer;

	//  Get the sensor list count and sensor count
	g_vm->_sensorListCount = *((int16 *)bufferPtr);
	sensorCount     = *((int16 *)bufferPtr + 1);
	bufferPtr = (int16 *)bufferPtr + 2;

	//  Restore all sensor lists
	for (i = 0; i < g_vm->_sensorListCount; i++)
		new SensorList(&bufferPtr);

	//  Restore all sensors
	for (i = 0; i < sensorCount; i++) {
		int16       ctr;

		ctr = *((int16 *)bufferPtr);
		bufferPtr = (int16 *)bufferPtr + 1;

		bufferPtr = constructSensor(ctr, bufferPtr);
	}

	assert(bufferPtr == &((uint8 *)archiveBuffer)[saveGame.getChunkSize()]);

	RDisposePtr(archiveBuffer);
#endif
}

//----------------------------------------------------------------------
//	Cleanup the active sensors

void cleanupSensors(void) {
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); ++it)
		delete *it;

	g_vm->_sensorListList.clear();

	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it)
		delete *it;

	g_vm->_sensorList.clear();
}

//----------------------------------------------------------------------
//	Fetch a specified object's SensorList

SensorList *fetchSensorList(GameObject *obj) {
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); ++it) {
		if ((*it)->getObject() == obj)
			return *it;
	}

	return NULL;
}

/* ===================================================================== *
   SensorList member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

SensorList::SensorList(void **buf) {
	ObjectID        *bufferPtr = (ObjectID *)*buf;

	assert(isObject(*bufferPtr) || isActor(*bufferPtr));

	obj = GameObject::objectAddress(*bufferPtr);

	*buf = bufferPtr + 1;

	newSensorList(this);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *SensorList::archive(void *buf) {
	*((ObjectID *)buf) = obj->thisID();
	buf = (ObjectID *)buf + 1;

	return buf;
}

/* ===================================================================== *
   Sensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

Sensor::Sensor(void **buf, int16 ctr) {
	void        *bufferPtr = *buf;

	assert(isObject(*((ObjectID *)bufferPtr))
	       ||  isActor(*((ObjectID *)bufferPtr)));

	//  Restore the object pointer
	obj = GameObject::objectAddress(*((ObjectID *)bufferPtr));
	bufferPtr = (ObjectID *)bufferPtr + 1;

	//  Restore the ID
	id = *((SensorID *)bufferPtr);
	bufferPtr = (SensorID *)bufferPtr + 1;

	//  Restore the range
	range = *((int16 *)bufferPtr);
	bufferPtr = (int16 *)bufferPtr + 1;

	*buf = bufferPtr;

	newSensor(this, ctr);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 Sensor::archiveSize(void) {
	return      sizeof(ObjectID)         //  obj ID
	            +   sizeof(id)
	            +   sizeof(range);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *Sensor::archive(void *buf) {
	//  Store the object's ID
	*((ObjectID *)buf) = obj->thisID();
	buf = (ObjectID *)buf + 1;

	//  Store the sensor ID
	*((SensorID *)buf) = id;
	buf = (SensorID *)buf + 1;

	//  Store the range
	*((int16 *)buf) = range;
	buf = (int16 *)buf + 1;

	return buf;
}

/* ===================================================================== *
   ProtaganistSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 ProtaganistSensor::getType(void) {
	return protaganistSensor;
}

//----------------------------------------------------------------------
//	Determine if the object can sense what it's looking for

bool ProtaganistSensor::check(SenseInfo &info, uint32 senseFlags) {
	static PlayerActorID    playerActorIDs[] = {
		FTA_JULIAN,
		FTA_PHILIP,
		FTA_KEVIN,
	};

	int16       i;
	bool        objIsActor = isActor(getObject());

	for (i = 0; i < (long)ARRAYSIZE(playerActorIDs); i++) {
		Actor   *protag =
		    getPlayerActorAddress(playerActorIDs[i])->getActor();

		assert(isActor(protag));

		//  Skip this protaganist if they're dead
		if (protag->isDead())
			continue;

		if (senseFlags & (1 << actorBlind))
			continue;

		//  This extra test is a HACK to ensure that the center actor
		//  will be able to sense a protaganist even if the protaganist
		//  is invisible.
		if (!objIsActor || getObject() != getCenterActor()) {
			if (!(senseFlags & actorSeeInvis)
			        &&  protag->hasEffect(actorInvisible))
				continue;
		}

		//  Skip if out of range
		if (getRange() != 0
		        &&  !getObject()->inRange(protag->getLocation(), getRange()))
			continue;

		//  Skip if we're checking for an actor and the protaganist is
		//  not in sight or not under the same roof
		if (objIsActor
		        && (!underSameRoof(getObject(), protag)
		            ||  !lineOfSight(getObject(), protag, terrainTransparent)))
			continue;

		info.sensedObject = protag;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------
//	Evaluate an event to determine if the object is waiting for it

bool ProtaganistSensor::evaluateEvent(const GameEvent &) {
	return false;
}

/* ===================================================================== *
   ObjectSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Determine if the object can sense what it's looking for

bool ObjectSensor::check(SenseInfo &info, uint32 senseFlags) {
	bool                    objIsActor = isActor(getObject());
	CircularObjectIterator  iter(
	    getObject()->world(),
	    getObject()->getLocation(),
	    getRange() != 0 ? getRange() : kTileUVSize * kPlatformWidth * 8);
	GameObject              *objToTest;
	iter.first(&objToTest);
	bool                    objToTestIsActor = isActor(objToTest);

	for (iter.first(&objToTest);
	        objToTest != NULL;
	        iter.next(&objToTest)) {
		if (senseFlags & (1 << actorBlind))
			continue;

		//  This extra test is a HACK to ensure that the center actor
		//  will be able to sense a protaganist even if the protaganist
		//  is invisible.
		if (objToTestIsActor
		        && (!objIsActor
		            ||  getObject() != getCenterActor()
		            ||  !isPlayerActor((Actor *)objToTest))) {
			Actor *a = (Actor *) objToTest;
			if (!(senseFlags & actorSeeInvis) && a->hasEffect(actorInvisible))
				continue;
		}
		//  Skip if object is out of range
		if (getRange() != 0
		        &&  !getObject()->inRange(objToTest->getLocation(), getRange()))
			continue;

		//  Skip if object is not what we're looking for
		if (!isObjectSought(objToTest))
			continue;

		//  Skip if we're checking for an actor and the protaganist is
		//  not in sight or not under the same roof
		if (objIsActor
		        && (!underSameRoof(getObject(), objToTest)
		            ||  !lineOfSight(getObject(), objToTest, terrainTransparent)))
			continue;

		info.sensedObject = objToTest;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------
//	Evaluate an event to determine if the object is waiting for it

bool ObjectSensor::evaluateEvent(const GameEvent &) {
	return false;
}

/* ===================================================================== *
   SpecificObjectSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

SpecificObjectSensor::SpecificObjectSensor(void **buf, int16 ctr) :
	ObjectSensor(buf, ctr) {
	ObjectID    *bufferPtr = (ObjectID *)*buf;

	//  Restore the sought object's ID
	soughtObjID = *bufferPtr++;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 SpecificObjectSensor::archiveSize(void) {
	return ObjectSensor::archiveSize() + sizeof(soughtObjID);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *SpecificObjectSensor::archive(void *buf) {
	//  Let the base class archive its data
	buf = ObjectSensor::archive(buf);

	//  Store the sought object's ID
	*((ObjectID *)buf) = soughtObjID;
	buf = (ObjectID *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 SpecificObjectSensor::getType(void) {
	return specificObjectSensor;
}

//----------------------------------------------------------------------
//	Determine if the object can sense what it's looking for

bool SpecificObjectSensor::check(SenseInfo &info, uint32 senseFlags) {
	assert(soughtObjID != Nothing);
	assert(isObject(soughtObjID) || isActor(soughtObjID));

	GameObject      *soughtObject = GameObject::objectAddress(soughtObjID);
	bool            objIsActor = isActor(getObject());

	if (senseFlags & (1 << actorBlind))
		return false;

	//  This extra test is a HACK to ensure that the center actor
	//  will be able to sense a protaganist even if the protaganist
	//  is invisible.
	if (isActor(soughtObject)
	        && (!objIsActor
	            ||  getObject() != getCenterActor()
	            ||  !isPlayerActor((Actor *)soughtObject))) {
		Actor *a = (Actor *) soughtObject;
		if (!(senseFlags & actorSeeInvis) && a->hasEffect(actorInvisible))
			return false;
	}

	if (getRange() != 0
	        &&  !getObject()->inRange(soughtObject->getLocation(),  getRange()))
		return false;

	if (objIsActor
	        && (!underSameRoof(getObject(), soughtObject)
	            ||  !lineOfSight(getObject(), soughtObject, terrainTransparent)))
		return false;

	info.sensedObject = soughtObject;
	return true;
}

//----------------------------------------------------------------------
//	Determine if an object meets the search criteria

bool SpecificObjectSensor::isObjectSought(GameObject *obj_) {
	assert(isObject(obj_) || isActor(obj_));
	assert(soughtObjID != Nothing);
	assert(isObject(soughtObjID) || isActor(soughtObjID));

	return obj_ == GameObject::objectAddress(soughtObjID);
}

/* ===================================================================== *
   ObjectPropertySensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

ObjectPropertySensor::ObjectPropertySensor(void **buf, int16 ctr) :
	ObjectSensor(buf, ctr) {
	ObjectPropertyID    *bufferPtr = (ObjectPropertyID *)*buf;

	//  Restore the object property ID
	objectProperty = *bufferPtr++;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 ObjectPropertySensor::archiveSize(void) {
	return ObjectSensor::archiveSize() + sizeof(objectProperty);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *ObjectPropertySensor::archive(void *buf) {
	//  Let the base class archive its data
	buf = ObjectSensor::archive(buf);

	//  Store the object property's ID
	*((ObjectPropertyID *)buf) = objectProperty;
	buf = (ObjectPropertyID *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 ObjectPropertySensor::getType(void) {
	return objectPropertySensor;
}

//----------------------------------------------------------------------
//	Determine if an object meets the search criteria

bool ObjectPropertySensor::isObjectSought(GameObject *obj_) {
	assert(isObject(obj_) || isActor(obj_));

	return obj_->hasProperty(*getObjProp(objectProperty));
}

/* ===================================================================== *
   ActorSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Determine if an object meets the search criteria

bool ActorSensor::isObjectSought(GameObject *obj_) {
	assert(isObject(obj_) || isActor(obj_));

	//  Only actors need apply
	return isActor(obj_) && isActorSought((Actor *)obj_);
}

/* ===================================================================== *
   SpecificActorSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

SpecificActorSensor::SpecificActorSensor(void **buf, int16 ctr) : ActorSensor(buf, ctr) {
	ObjectID        *bufferPtr = (ObjectID *)*buf;

	assert(isActor(*bufferPtr));

	//  Restore the sought actor pointer
	soughtActor = (Actor *)GameObject::objectAddress(*bufferPtr++);

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 SpecificActorSensor::archiveSize(void) {
	return ActorSensor::archiveSize() + sizeof(ObjectID);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *SpecificActorSensor::archive(void *buf) {
	//  Let the base class archive its data
	buf = ActorSensor::archive(buf);

	//  Store the sought actor's ID
	*((ObjectID *)buf) = soughtActor->thisID();
	buf = (ObjectID *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 SpecificActorSensor::getType(void) {
	return specificActorSensor;
}

//----------------------------------------------------------------------
//	Determine if the object can sense what it's looking for

bool SpecificActorSensor::check(SenseInfo &info, uint32 senseFlags) {
	assert(isActor(soughtActor));
	bool        objIsActor = isActor(getObject());

	if (senseFlags & (1 << actorBlind))
		return false;

	//  This extra test is a HACK to ensure that the center actor
	//  will be able to sense a protaganist even if the protaganist
	//  is invisible.
	if (!objIsActor
	        ||  getObject() != getCenterActor()
	        ||  !isPlayerActor(soughtActor)) {
		if (!(senseFlags & actorSeeInvis) && soughtActor->hasEffect(actorInvisible))
			return false;
	}

	if (getRange() != 0
	        &&  !getObject()->inRange(soughtActor->getLocation(), getRange()))
		return false;

	if (objIsActor
	        && (!underSameRoof(getObject(), soughtActor)
	            ||  !lineOfSight(getObject(), soughtActor, terrainTransparent)))
		return false;

	info.sensedObject = soughtActor;
	return true;
}

//----------------------------------------------------------------------
//	Determine if an actor meets the search criteria

bool SpecificActorSensor::isActorSought(Actor *a) {
	return a == soughtActor;
}

/* ===================================================================== *
   ActorPropertySensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

ActorPropertySensor::ActorPropertySensor(void **buf, int16 ctr) : ActorSensor(buf, ctr) {
	ActorPropertyID     *bufferPtr = (ActorPropertyID *)*buf;

	//  Restore the actor property's ID
	actorProperty = *bufferPtr++;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 ActorPropertySensor::archiveSize(void) {
	return ActorSensor::archiveSize() + sizeof(actorProperty);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *ActorPropertySensor::archive(void *buf) {
	//  Let the base class archive its data
	buf = ActorSensor::archive(buf);

	//  Store the actor property's ID
	*((ActorPropertyID *)buf) = actorProperty;
	buf = (ActorPropertyID *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 ActorPropertySensor::getType(void) {
	return actorPropertySensor;
}

//----------------------------------------------------------------------
//	Determine if an actor meets the search criteria

bool ActorPropertySensor::isActorSought(Actor *a) {
	return a->hasProperty(*getActorProp(actorProperty));
}

/* ===================================================================== *
   EventSensor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

EventSensor::EventSensor(
    GameObject      *o,
    SensorID        sensorID,
    int16           rng,
    int16           type) :
	Sensor(o, sensorID, rng),
	eventType(type) {
}

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

EventSensor::EventSensor(void **buf, int16 ctr) : Sensor(buf, ctr) {
	int16       *bufferPtr = (int16 *)*buf;

	//  Restore the event type
	eventType = *bufferPtr++;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 EventSensor::archiveSize(void) {
	return Sensor::archiveSize() + sizeof(eventType);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *EventSensor::archive(void *buf) {
	//  Let the base class archive its data
	buf = Sensor::archive(buf);

	//  Store the event type
	*((int16 *)buf) = eventType;
	buf = (int16 *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this sensor

int16 EventSensor::getType(void) {
	return eventSensor;
}

//----------------------------------------------------------------------
//	Determine if the object can sense what it's looking for

bool EventSensor::check(SenseInfo &, uint32) {
	return false;
}

//----------------------------------------------------------------------
//	Evaluate an event to determine if the object is waiting for it

bool EventSensor::evaluateEvent(const GameEvent &event) {
	return      event.type == eventType
	            &&  getObject()->world() == event.directObject->world()
	            && (getRange() != 0
	                ?   getObject()->inRange(
	                    event.directObject->getLocation(),
	                    getRange())
	                :   true)
	            && (!isActor(getObject())
	                || (underSameRoof(getObject(), event.directObject)
	                    &&  lineOfSight(
	                        getObject(),
	                        event.directObject,
	                        terrainTransparent)));
}

} // end of namespace Saga2
