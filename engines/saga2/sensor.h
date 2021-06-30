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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SENSOR_H
#define SAGA2_SENSOR_H

namespace Saga2 {

const uint32 nonActorSenseFlags = actorSeeInvis;

//const size_t maxSensorSize = 24;
const size_t maxSensorSize = 48;

//  This constant represents the maximum sense range for an object.
//  Zero means an infinite range.
const int16 maxSenseRange = 0;

//  Integers representing sensor types
enum SensorType {
	protaganistSensor,
	specificObjectSensor,
	objectPropertySensor,
	specificActorSensor,
	actorPropertySensor,
	eventSensor
};

//  Sensors will be checked every 5 frames
const int16 sensorCheckRate = 5;

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

struct GameEvent;

//  Allocate a new SensorList
void newSensorList(SensorList *s);
//  Deallocate an SensorList
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
void checkSensors(void);
//  Evaluate an event for all active sensors
void assertEvent(const GameEvent &ev);

//  Initialize the sensors
void initSensors(void);
//  Save all active sensors in a save file
void saveSensors(SaveFileConstructor &saveGame);
//  Load sensors from a save file
void loadSensors(SaveFileReader &saveGame);
//  Cleanup the active sensors
void cleanupSensors(void);

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
	GameObject      *obj;

public:
	Common::List<Sensor *> _list;

public:
	//  Constructor -- initial construction
	SensorList(GameObject *o) : obj(o) {
		newSensorList(this);
	}

	~SensorList() {
		deleteSensorList(this);
	}

	//  Constructor -- reconstruct from archive buffer
	SensorList(void **buf);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize(void) {
		return sizeof(ObjectID);
	}

	//  Archive this object in a buffer
	void *archive(void *buf);

	GameObject *getObject(void) {
		return obj;
	}
};

/* ===================================================================== *
   Sensor Class
 * ===================================================================== */

class Sensor {
public:
	GameObject      *obj;
	SensorID        id;
	int16           range;

	int16       checkCtr;

public:
	//  Constructor -- initial construction
	Sensor(GameObject *o, SensorID sensorID, int16 rng) : obj(o), id(sensorID), range(rng) {
		newSensor(this);
	}

	//  Constructor -- reconstruct from an archive buffer
	Sensor(void **buf, int16 ctr);

	//  Virtural destructor
	virtual ~Sensor(void) {
		deleteSensor(this);
	}

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	virtual int32 archiveSize(void);

	//  Archive this object in a buffer
	virtual void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	virtual int16 getType(void) = 0;

	GameObject *getObject(void) {
		return obj;
	}
	SensorID thisID(void) {
		return id;
	}
	int16 getRange(void) {
		return range;
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

	//  Constructor -- reconstruct from an archive buffer
	ProtaganistSensor(void **buf, int16 ctr) : Sensor(buf, ctr) {}

	//  Return an integer representing the type of this sensor
	int16 getType(void);

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

	//  Constructor -- reconstruct from an archive buffer
	ObjectSensor(void **buf, int16 ctr) : Sensor(buf, ctr) {}

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
	ObjectID        soughtObjID;

public:
	//  Constructor -- initial construction
	SpecificObjectSensor(
	    GameObject  *o,
	    SensorID    sensorID,
	    int16       rng,
	    ObjectID    objToSense) :
		ObjectSensor(o, sensorID, rng),
		soughtObjID(objToSense) {
	}

	//  Constructor -- reconstruct from an archive buffer
	SpecificObjectSensor(void **buf, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	int16 getType(void);

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
	ObjectPropertyID    objectProperty;

public:
	//  Constructor -- initial construction
	ObjectPropertySensor(
	    GameObject          *o,
	    SensorID            sensorID,
	    int16               rng,
	    ObjectPropertyID    propToSense) :
		ObjectSensor(o, sensorID, rng),
		objectProperty(propToSense) {
	}

	//  Constructor -- reconstruct from an archive buffer
	ObjectPropertySensor(void **buf, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	int16 getType(void);

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

	//  Constructor -- reconstruct from an archive buffer
	ActorSensor(void **buf, int16 ctr) : ObjectSensor(buf, ctr) {}

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
	Actor       *soughtActor;

public:
	//  Constructor -- initial construction
	SpecificActorSensor(
	    GameObject  *o,
	    SensorID    sensorID,
	    int16       rng,
	    Actor       *actorToSense) :
		ActorSensor(o, sensorID, rng),
		soughtActor(actorToSense) {
	}

	//  Constructor -- reconstruct from an archive buffer
	SpecificActorSensor(void **buf, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	int16 getType(void);

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
	ActorPropertyID     actorProperty;

public:
	//  Constructor -- initial construction
	ActorPropertySensor(
	    GameObject          *o,
	    SensorID            sensorID,
	    int16               rng,
	    ActorPropertyID     propToSense) :
		ActorSensor(o, sensorID, rng),
		actorProperty(propToSense) {
	}

	//  Constructor -- reconstruct from an archive buffer
	ActorPropertySensor(void **buf, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	int16 getType(void);

private:
	//  Determine if an actor meets the search criteria
	bool isActorSought(Actor *a);
};

/* ===================================================================== *
   EventSensor Class
 * ===================================================================== */

class EventSensor : public Sensor {
	int16               eventType;

public:
	//  Constructor -- initial construction
	EventSensor(
	    GameObject      *o,
	    SensorID        sensorID,
	    int16           rng,
	    int16           type);

	//  Constructor -- reconstruct from an archive buffer
	EventSensor(void **buf, int16 ctr);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	//  Return an integer representing the type of this sensor
	int16 getType(void);

	//  Determine if the object can sense what it's looking for
	bool check(SenseInfo &info, uint32 senseFlags);

	//  Evaluate an event to determine if the object is waiting for it
	bool evaluateEvent(const GameEvent &event);
};

} // end of namespace Saga2

#endif
