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

#include "saga2/saga2.h"
#include "saga2/objects.h"
#include "saga2/sensor.h"
#include "saga2/player.h"
#include "saga2/tile.h"

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

void readSensor(int16 ctr, Common::InSaveFile *in) {
	int16 type;
	Sensor *sensor = nullptr;
	SensorList *sl;

	//  Get the sensor type
	type = in->readSint16LE();
	debugC(3, kDebugSaveload, "type = %d", type);

	switch (type) {
	case protaganistSensor:
		sensor = new ProtaganistSensor(in, ctr);
		break;

	case specificObjectSensor:
		sensor = new SpecificObjectSensor(in, ctr);
		break;

	case objectPropertySensor:
		sensor = new ObjectPropertySensor(in, ctr);
		break;

	case specificActorSensor:
		sensor = new SpecificActorSensor(in, ctr);
		break;

	case actorPropertySensor:
		sensor = new ActorPropertySensor(in, ctr);
		break;

	case eventSensor:
		sensor = new EventSensor(in, ctr);
		break;
	}

	assert(sensor != nullptr);

	//  Get the sensor list
	sl = fetchSensorList(sensor->getObject());

	assert(sl != nullptr);

	//  Append this Sensor to the sensor list
	sl->_list.push_back(sensor);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive the specified Sensor in
//	an archive buffer

int32 sensorArchiveSize(Sensor *sensor) {
	assert(sensor != NULL);

	return      sizeof(int16)                //  Type
	            +   sensor->archiveSize();
}

void writeSensor(Sensor *sensor, Common::OutSaveFile *out) {
	assert(sensor != NULL);

	//  Store the sensor type
	out->writeSint16LE(sensor->getType());
	debugC(3, kDebugSaveload, "type = %d", sensor->getType());

	//  Let the sensor store its data in the buffer
	sensor->write(out);
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

static int getSensorListID(SensorList *t) {
	int i = 0;
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); it++, i++) {
		if ((*it) == t)
			return i;
	}
	return -1;
}

static int getSensorID(Sensor *t) {
	int i = 0;
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); it++, i++) {
		if ((*it) == t)
			return i;
	}
	return -1;
}


void saveSensors(Common::OutSaveFile *out) {
	debugC(2, kDebugSaveload, "Saving Sensors");

	int16 sensorListCount = 0,
	      sensorCount = 0;

	int32 archiveBufSize = 0;

	//  Add the sizes of the sensor list count an sensor count
	archiveBufSize += sizeof(sensorListCount) + sizeof(sensorCount);

	//  Tally the sensor lists
	sensorListCount = g_vm->_sensorListList.size();

	//  Add the total archive size of all of the sensor lists
	archiveBufSize += sensorListCount * SensorList::archiveSize();

	//  Tally the sensors and add the archive size of each
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		sensorCount++;
		archiveBufSize += sizeof((*it)->checkCtr) + sensorArchiveSize(*it);
	}

	out->write("SENS", 4);
	out->writeUint32LE(archiveBufSize);

	//  Store the sensor list count and sensor count
	out->writeSint16LE(sensorListCount);
	out->writeSint16LE(sensorCount);

	debugC(3, kDebugSaveload, "... sensorListCount = %d", sensorListCount);
	debugC(3, kDebugSaveload, "... sensorCount = %d", sensorCount);

	//  Archive all sensor lists
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); ++it) {
		debugC(3, kDebugSaveload, "Saving SensorList %d", getSensorListID(*it));
		(*it)->write(out);
	}

	//  Archive all sensors
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); ++it) {
		debugC(3, kDebugSaveload, "Saving Sensor %d", getSensorID(*it));
		out->writeSint16LE((*it)->checkCtr);
		debugC(3, kDebugSaveload, "... ctr = %d", (*it)->checkCtr);

		writeSensor(*it, out);
	}
}

void loadSensors(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading Sensors");

	int16 sensorListCount,
	      sensorCount;

	//  Get the sensor list count and sensor count
	sensorListCount = in->readSint16LE();
	sensorCount = in->readSint16LE();
	debugC(3, kDebugSaveload, "... sensorListCount = %d", sensorListCount);
	debugC(3, kDebugSaveload, "... sensorCount = %d", sensorCount);

	//  Restore all sensor lists
	for (int i = 0; i < sensorListCount; i++) {
		debugC(3, kDebugSaveload, "Loading SensorList %d", i);
		new SensorList(in);
	}

	//  Restore all sensors
	for (int i = 0; i < sensorCount; i++) {
		int16 ctr;

		debugC(3, kDebugSaveload, "Loading Sensor %d", i);
		ctr = in->readSint16LE();
		debugC(3, kDebugSaveload, "... ctr = %d", ctr);

		readSensor(ctr, in);
	}
}

//----------------------------------------------------------------------
//	Cleanup the active sensors

void cleanupSensors(void) {
	Common::List<SensorList *>::iterator sensorListNextIt;
	for (Common::List<SensorList *>::iterator it = g_vm->_sensorListList.begin(); it != g_vm->_sensorListList.end(); it = sensorListNextIt) {
		sensorListNextIt = it;
		sensorListNextIt++;
		delete *it;
	}

	Common::List<Sensor *>::iterator sensorNextIt;
	for (Common::List<Sensor *>::iterator it = g_vm->_sensorList.begin(); it != g_vm->_sensorList.end(); it = sensorNextIt) {
		sensorNextIt = it;
		sensorNextIt++;
		delete *it;
	}
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

SensorList::SensorList(Common::InSaveFile *in) {
	ObjectID id = in->readUint16LE();

	assert(isObject(id) || isActor(id));

	obj = GameObject::objectAddress(id);

	newSensorList(this);
}

void SensorList::write(Common::OutSaveFile *out) {
	out->writeUint16LE(obj->thisID());
}

/* ===================================================================== *
   Sensor member functions
 * ===================================================================== */

Sensor::Sensor(Common::InSaveFile *in, int16 ctr) {
	ObjectID objID = in->readUint16LE();

	assert(isObject(objID) || isActor(objID));

	//  Restore the object pointer
	obj = GameObject::objectAddress(objID);

	//  Restore the ID
	id = in->readSint16LE();

	//  Restore the range
	range = in->readSint16LE();

	newSensor(this, ctr);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 Sensor::archiveSize(void) {
	return      sizeof(ObjectID)         //  obj ID
	            +   sizeof(id)
	            +   sizeof(range);
}

void Sensor::write(Common::OutSaveFile *out) {
	//  Store the object's ID
	out->writeUint16LE(obj->thisID());

	//  Store the sensor ID
	out->writeSint16LE(id);

	//  Store the range
	out->writeSint16LE(range);
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

SpecificObjectSensor::SpecificObjectSensor(Common::InSaveFile *in, int16 ctr) :
	ObjectSensor(in, ctr) {
	debugC(3, kDebugSaveload, "Loading SpecificObjectSensor");

	//  Restore the sought object's ID
	soughtObjID = in->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 SpecificObjectSensor::archiveSize(void) {
	return ObjectSensor::archiveSize() + sizeof(soughtObjID);
}

void SpecificObjectSensor::write(Common::OutSaveFile *out) {
	debugC(3, kDebugSaveload, "Saving SpecificObjectSensor");

	//  Let the base class archive its data
	ObjectSensor::write(out);

	//  Store the sought object's ID
	out->writeUint16LE(soughtObjID);
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

ObjectPropertySensor::ObjectPropertySensor(Common::InSaveFile *in, int16 ctr) :
	ObjectSensor(in, ctr) {
	debugC(3, kDebugSaveload, "Loading ObjectPropertySensor");

	//  Restore the object property ID
	objectProperty = in->readSint16LE();;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 ObjectPropertySensor::archiveSize(void) {
	return ObjectSensor::archiveSize() + sizeof(objectProperty);
}

void ObjectPropertySensor::write(Common::OutSaveFile *out) {
	debugC(3, kDebugSaveload, "Saving ObjectPropertySensor");

	//  Let the base class archive its data
	ObjectSensor::write(out);

	//  Store the object property's ID
	out->writeSint16LE(objectProperty);
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

SpecificActorSensor::SpecificActorSensor(Common::InSaveFile *in, int16 ctr) : ActorSensor(in, ctr) {
	debugC(3, kDebugSaveload, "Loading SpecificActorSensor");
	ObjectID actorID = in->readUint16LE();

	assert(isActor(actorID));

	//  Restore the sought actor pointer
	soughtActor = (Actor *)GameObject::objectAddress(actorID);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 SpecificActorSensor::archiveSize(void) {
	return ActorSensor::archiveSize() + sizeof(ObjectID);
}

void SpecificActorSensor::write(Common::OutSaveFile *out) {
	debugC(3, kDebugSaveload, "Saving SpecificActorSensor");

	//  Let the base class archive its data
	ActorSensor::write(out);

	//  Store the sought actor's ID
	out->writeUint16LE(soughtActor->thisID());
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

ActorPropertySensor::ActorPropertySensor(Common::InSaveFile *in, int16 ctr) : ActorSensor(in, ctr) {
	debugC(3, kDebugSaveload, "Loading ActorPropertySensor");
	//  Restore the actor property's ID
	actorProperty = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 ActorPropertySensor::archiveSize(void) {
	return ActorSensor::archiveSize() + sizeof(actorProperty);
}

void ActorPropertySensor::write(Common::OutSaveFile *out) {
	debugC(3, kDebugSaveload, "Saving ActorPropertySensor");

	//  Let the base class archive its data
	ActorSensor::write(out);

	//  Store the actor property's ID
	out->writeSint16LE(actorProperty);
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

EventSensor::EventSensor(Common::InSaveFile *in, int16 ctr) : Sensor(in, ctr) {
	debugC(3, kDebugSaveload, "Loading EventSensor");
	//  Restore the event type
	eventType = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 EventSensor::archiveSize(void) {
	return Sensor::archiveSize() + sizeof(eventType);
}

void EventSensor::write(Common::OutSaveFile *out) {
	debugC(3, kDebugSaveload, "Saving EventSensor");

	//  Let the base class archive its data
	Sensor::write(out);

	//  Store the event type
	out->writeSint16LE(eventType);
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
