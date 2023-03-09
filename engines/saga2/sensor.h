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

#ifndef SAGA2_SENSOR_H
#define SAGA2_SENSOR_H

namespace Saga2 {

const uint32 kNonActorSenseFlags = kActorSeeInvis;

//const size_t maxSensorSize = 24;
const size_t kMaxSensorSize = 48;

//  This constant represents the maximum sense range for an object.
//  Zero means an infinite range.
const int16 kMaxSenseRange = 0;

//  Integers representing sensor types
enum SensorType {
	kProtaganistSensor,
	kSpecificObjectSensor,
	kObjectPropertySensor,
	kSpecificActorSensor,
	kActorPropertySensor,
	kEventSensor
};

//  Sensors will be checked every 5 frames
const int16 kSensorCheckRate = 5;

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

struct GameEvent;

//  Allocate a new SensorList
void newSensorList(SensorList *s);
//  Deallocate a SensorList
void deleteSensorList(SensorList *p);

//  Fetch a specified object's SensorList
SensorList *fetchSensorList(GameObject *obj);

//  Allocate a new Sensor
void newSensor(Sensor *s);
//  Allocate a new Sensor with a specified starting check counter
void newSensor(Sensor *s, int16 ctr);
//  Deallocate a Sensor
void deleteSensor(Sensor *p);

//  Check all active sensors
void checkSensors();
//  Evaluate an event for all active sensors
void assertEvent(const GameEvent &ev);

//  Initialize the sensors
void initSensors();
void saveSensors(Common::OutSaveFile *outS);
void loadSensors(Common::InSaveFile *in);
//  Cleanup the active sensors
void cleanupSensors();

/* ===================================================================== *
   GameEvent struct
 * ===================================================================== */

struct GameEvent {
	int16           type;
	GameObject      *directObject,
	                *indirectObject;
};

/* ===================================================================== *
   SenseInfo struct
 * ===================================================================== */

struct SenseInfo {
	GameObject      *sensedObject;
};

/* ===================================================================== *
   SensorList Class
 * ===================================================================== */

class SensorList {
	GameObject      *_obj;

public:
	Common::List<Sensor *> _list;

public:
	//  Constructor -- initial construction
	SensorList(GameObject *o) : _obj(o) {
		newSensorList(this);
		debugC(1, kDebugSensors, "Adding SensorList %p to %d (%s) (total %d)",
		       (void *)this, o->thisID(), o->objName(), _list.size());
	}

	~SensorList() {
		deleteSensorList(this);
		debugC(1, kDebugSensors, "Deleting SensorList %p of %d (%s) (total %d)",
		       (void *)this, _obj->thisID(), _obj->objName(), _list.size());
	}

	SensorList(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize() {
		return sizeof(ObjectID);
	}

	void write(Common::MemoryWriteStreamDynamic *out);

	GameObject *getObject() {
		return _obj;
	}
};

/* ===================================================================== *
   Sensor Class
 * ===================================================================== */

class Sensor {
public:
	GameObject      *_obj;
	SensorID        _id;
	int16           _range;

	int16       _checkCtr;
	bool _active;

public:
	//  Constructor -- initial construction
	Sensor(GameObject *o, SensorID sensorID, int16 rng) : _obj(o), _id(sensorID), _range(rng), _active(true) {
		newSensor(this);
		SensorList *sl = fetchSensorList(o);
		debugC(1, kDebugSensors, "Adding Sensor %p to %d (%s) (list = %p, total = %d)",
		       (void *)this, o->thisID(), o->objName(), (void *)sl, (sl != nullptr) ? sl->_list.size() : -1);
	}

	Sensor(Common::InSaveFile *in, int16 ctr);

	//  Virtural destructor
	virtual ~Sensor() {
		deleteSensor(this);
		SensorList *sl = fetchSensorList(_obj);
		debugC(1, kDebugSensors, "Deleting Sensor %p of %d (%s) (list = %p, total = %d)",
		       (void *)this, _obj->thisID(), _obj->objName(), (void *)sl, (sl != nullptr) ? sl->_list.size() : -1);
	}

	virtual void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	virtual int16 getType() = 0;

	GameObject *getObject() {
		return _obj;
	}
	SensorID thisID() {
		return _id;
	}
	int16 getRange() {
		return _range;
	}

	//  Determine if the object can sense what it's looking for
	virtual bool check(SenseInfo &info, uint32 senseFlags) = 0;

	//  Evaluate an event to determine if the object is waiting for it
	virtual bool evaluateEvent(const GameEvent &event) = 0;
};

/* ===================================================================== *
   ProtaganistSensor Class
 * ===================================================================== */

class ProtaganistSensor : public Sensor {
public:
	//  Constructor -- initial construction
	ProtaganistSensor(GameObject *o, SensorID sensorID, int16 rng) :
		Sensor(o, sensorID, rng) {
	}

	ProtaganistSensor(Common::InSaveFile *in, int16 ctr) : Sensor(in, ctr) {
		debugC(3, kDebugSaveload, "Loading ProtagonistSensor");
	}

	//  Return an integer representing the type of this sensor
	int16 getType();

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

	//  Evaluate an event to determine if the object is waiting for it
	bool evaluateEvent(const GameEvent &event);
};

/* ===================================================================== *
   ObjectSensor Class
 * ===================================================================== */

class ObjectSensor : public Sensor {
public:
	//  Constructor -- initial construction
	ObjectSensor(GameObject *o, SensorID sensorID, int16 rng) :
		Sensor(o, sensorID, rng) {
	}

	ObjectSensor(Common::InSaveFile *in, int16 ctr) : Sensor(in, ctr) {}

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

	//  Evaluate an event to determine if the object is waiting for it
	bool evaluateEvent(const GameEvent &event);

private:
	//  Determine if an object meets the search criteria
	virtual bool isObjectSought(GameObject *obj) = 0;
};

/* ===================================================================== *
   SpecificObjectSensor Class
 * ===================================================================== */

class SpecificObjectSensor : public ObjectSensor {
	ObjectID        _soughtObjID;

public:
	//  Constructor -- initial construction
	SpecificObjectSensor(
	    GameObject  *o,
	    SensorID    sensorID,
	    int16       rng,
	    ObjectID    objToSense) :
		ObjectSensor(o, sensorID, rng),
		_soughtObjID(objToSense) {
	}

	SpecificObjectSensor(Common::InSaveFile *in, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	int16 getType();

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

private:
	//  Determine if an object meets the search criteria
	bool isObjectSought(GameObject *obj);
};

/* ===================================================================== *
   ObjectPropertySensor Class
 * ===================================================================== */

class ObjectPropertySensor : public ObjectSensor {
	ObjectPropertyID    _objectProperty;

public:
	//  Constructor -- initial construction
	ObjectPropertySensor(
	    GameObject          *o,
	    SensorID            sensorID,
	    int16               rng,
	    ObjectPropertyID    propToSense) :
		ObjectSensor(o, sensorID, rng),
		_objectProperty(propToSense) {
	}

	ObjectPropertySensor(Common::InSaveFile *in, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	int16 getType();

private:
	//  Determine if an object meets the search criteria
	bool isObjectSought(GameObject *obj);
};

/* ===================================================================== *
   ActorSensor Class
 * ===================================================================== */

class ActorSensor : public ObjectSensor {
public:
	//  Constructor -- initial construction
	ActorSensor(GameObject *o, SensorID sensorID, int16 rng) :
		ObjectSensor(o, sensorID, rng) {
	}

	ActorSensor(Common::InSaveFile *in, int16 ctr) : ObjectSensor(in, ctr) {}

private:
	//  Determine if an object meets the search criteria
	bool isObjectSought(GameObject *obj);

	//  Determine if an actor meets the search criteria
	virtual bool isActorSought(Actor *a) = 0;
};

/* ===================================================================== *
   SpecificActorSensor Class
 * ===================================================================== */

class SpecificActorSensor : public ActorSensor {
	Actor       *_soughtActor;

public:
	//  Constructor -- initial construction
	SpecificActorSensor(
	    GameObject  *o,
	    SensorID    sensorID,
	    int16       rng,
	    Actor       *actorToSense) :
		ActorSensor(o, sensorID, rng),
		_soughtActor(actorToSense) {
	}

	SpecificActorSensor(Common::InSaveFile *in, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	int16 getType();

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

private:
	//  Determine if an actor meets the search criteria
	bool isActorSought(Actor *a);
};

/* ===================================================================== *
   ActorPropertySensor Class
 * ===================================================================== */

class ActorPropertySensor : public ActorSensor {
	ActorPropertyID     _actorProperty;

public:
	//  Constructor -- initial construction
	ActorPropertySensor(
	    GameObject          *o,
	    SensorID            sensorID,
	    int16               rng,
	    ActorPropertyID     propToSense) :
		ActorSensor(o, sensorID, rng),
		_actorProperty(propToSense) {
	}

	ActorPropertySensor(Common::InSaveFile *in, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	int16 getType();

private:
	//  Determine if an actor meets the search criteria
	bool isActorSought(Actor *a);
};

/* ===================================================================== *
   EventSensor Class
 * ===================================================================== */

class EventSensor : public Sensor {
	int16               _eventType;

public:
	//  Constructor -- initial construction
	EventSensor(
	    GameObject      *o,
	    SensorID        sensorID,
	    int16           rng,
	    int16           type);

	EventSensor(Common::InSaveFile *in, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return an integer representing the type of this sensor
	int16 getType();

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

	//  Evaluate an event to determine if the object is waiting for it
	bool evaluateEvent(const GameEvent &event);
};

} // end of namespace Saga2

#endif
