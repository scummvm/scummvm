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

#ifndef SAGA2_PROPERTY_H
#define SAGA2_PROPERTY_H
namespace Saga2 {

/* ===================================================================== *
   Property class template
 * ===================================================================== */

//	This is an abstract property function object class.  A property function
//	object can be used to determine if an object of type T has a certain
//	property.  In order to do this, a pointer to an object of type T is
//	passed to the operator () member function, which returns true if the
//	object has the property and false if it does not.

//	These function objects can be used to process only subset of object of
//	type T which meet certain criteria, without having to know what those
//	criteria are.

template < class T >
class Property {
public:
	virtual ~Property() {}

	virtual bool operator()(T *obj) const = 0;
};

/* ===================================================================== *
   SimpleProperty class template
 * ===================================================================== */

//	This class defines the most simple type of property function object.
//	A simple property is constructed with a pointer to a function which will
//	accept as a parameter a pointer to an object of type T and evaluate the
//	object to determine if it has a certain property.  The only task of the
//	operator () member function is to call this function and return its
//	result.

template < class T >
class SimpleProperty : public Property< T > {
	bool (*propertyFunc)(T *);   //  The pointer to the property
	//  evaluation function

public:
	//  Constructor
	SimpleProperty(bool (*func)(T *)) :
		propertyFunc(func) {
	}

	bool operator()(T *obj) const;
};

template < class T >
bool SimpleProperty< T >::operator()(T *obj) const {
	//  Simply pass this call through to the property evaluation function
	return (*propertyFunc)(obj);
}

/* ===================================================================== *
   CompoundProperty class template
 * ===================================================================== */

//	This is an abstract base class for all compound properties:  properties
//	which are defined in terms of one or more other properties.  This class
//	cosists of a pointer to an array of pointers to Property's and a count
//	of the number of pointers in the array.

template < class T >
class CompoundProperty : public Property< T > {
protected:
	Property< T >   **propertyArray;    //  A pointer to an array of pointers
	//  to Properties.
	uint16          arraySize;          //  The number of elements in the
	//  array

public:
	//  Constructor
	CompoundProperty(Property< T > **array, uint16 size);

	//  Virtual destructor
	virtual ~CompoundProperty();
};

template < class T >
CompoundProperty< T >::CompoundProperty(
    Property< T > **array,
    uint16 size) {
	//  Determine the number of bytes needed to copy the array
	uint16  arrayBytes = sizeof(Property< T > *) * size;

	//  Allocate memory to copy the array.
	propertyArray = (Property< T > **)malloc(arrayBytes);
	assert(propertyArray);

	//  Copy the array
	memcpy(propertyArray, array, arrayBytes);
	arraySize = size;
}


template < class T >
CompoundProperty< T >::~CompoundProperty() {
	//  Free the array memory
	free(propertyArray);
}

/* ===================================================================== *
   PropertyAnd class template
 * ===================================================================== */

//	This class defines an 'and' compound property.  Each of the sub
//	properties in the compound property list must evaluate to true for this
//	function object to evaluate to true.

template < class T >
class PropertyAnd : public CompoundProperty< T > {
public:
	//  Constructor
	PropertyAnd(Property< T > **array, uint16 size) :
		CompoundProperty< T >(array, size) {
	}

	bool operator()(T *obj) const;
};

template < class T >
bool PropertyAnd< T >::operator()(T *obj) const {
#if 0
	uint16  i;

	//  Iterate through each element in the array and if any evaluate to
	//  false, return false immediately.
	for (i = 0; i < arraySize; i++)
		if ((*propertyArray[i])(obj) == false) return false;
#endif
	warning("STUB: PropertyAnd");

	return true;
}

/* ===================================================================== *
   PropertyOr class template
 * ===================================================================== */

//	This class defines an 'or' compound property.  If any of the sub
//	properties in the compound property list evaluate to true this function
//	object will evaluate to true.

template < class T >
class PropertyOr : public CompoundProperty< T > {
public:
	//  Constructor
	PropertyOr(Property< T > **array, uint16 size) :
		CompoundProperty< T >(array, size) {
	}

	bool operator()(T *obj) const;
};

template < class T >
bool PropertyOr< T >::operator()(T *obj) const {
#if 0
	uint16  i;

	//  Iterate through each element in the array and if any evaluate to
	//  true, return true immediately.
	for (i = 0; i < arraySize; i++)
		if ((*propertyArray[i])(obj)) return true;
#endif
	warning("STUB: PropertyOr");

	return false;
}

/* ===================================================================== *
   Object properties
 * ===================================================================== */

class GameObject;

typedef Property< GameObject >          ObjectProperty;
typedef SimpleProperty< GameObject >    SimpleObjectProperty;
typedef PropertyAnd< GameObject >       ObjectPropertyAnd;
typedef PropertyOr< GameObject >        ObjectPropertyOr;

typedef int16 ObjectPropertyID;

enum {
	objPropIDObject,
	objPropIDActor,
	objPropIDWorld,
	objPropIDLocked,
	objPropIDUnlocked,
	objPropIDKey,
	objPropIDPlayerActor,
	objPropIDEnemy,

	objPropIDCount
};

/* ===================================================================== *
   Actor properties
 * ===================================================================== */

class Actor;

typedef Property< Actor >               ActorProperty;
typedef SimpleProperty< Actor >         SimpleActorProperty;
typedef PropertyAnd< Actor >            ActorPropertyAnd;
typedef PropertyOr< Actor >             ActorPropertyOr;

typedef int16 ActorPropertyID;

enum {
	actorPropIDDead,
	actorPropIDCenterActor,
	actorPropIDPlayerActor,
	actorPropIDEnemy,

	actorPropIDCount
};

/* ===================================================================== *
   Tile properties
 * ===================================================================== */

struct TileInfo;

typedef Property< TileInfo >            TileProperty;
typedef SimpleProperty< TileInfo >      SimpleTileProperty;
typedef PropertyAnd< TileInfo >         TilePropertyAnd;
typedef PropertyOr< TileInfo >          TilePropertyOr;

typedef int16 TilePropertyID;

enum {
	tilePropIDHasWater,

	tilePropIDCount
};

/* ===================================================================== *
   MetaTile properties
 * ===================================================================== */

class MetaTile;

/* ===================================================================== *
   MetaTileProperty class
 * ===================================================================== */

//	The MetaTileProperty class hierarchy is similar to the Property template
//	class hierarchy.  The reason that MetaTile's have a separate Property
//	class hierarchy, is because a MetaTile may only be uniquely identified
//	if the location of the MetaTile is specifed, as well as a pointer to the
//	MetaTile structure.  This difference alters the interface of the
//	operator () member function by requiring an additional TilePoint
//	parameter.

class MetaTileProperty {
public:
	virtual ~MetaTileProperty() {}
	virtual bool operator()(
	    MetaTile *mt,
	    int16 mapNum,
	    const TilePoint &tp) const = 0;
};

/* ===================================================================== *
   SimpleMetaTileProperty class
 * ===================================================================== */

class SimpleMetaTileProperty : public MetaTileProperty {
	//  Pointer to the property evaluation function.
	bool (*propertyFunc)(MetaTile *, int16, const TilePoint &);

public:
	//  Constructor
	SimpleMetaTileProperty(bool (*func)(MetaTile *, int16, const TilePoint &)) :
		propertyFunc(func) {
	}

	virtual ~SimpleMetaTileProperty() {}

	bool operator()(
	    MetaTile *mt,
	    int16 mapNum,
	    const TilePoint &tp) const;
};

/* ===================================================================== *
   CompoundMetaTileProperty class
 * ===================================================================== */

class CompoundMetaTileProperty : public MetaTileProperty {
protected:
	MetaTileProperty    **propertyArray;    //  Array of pointers to
	//  MetaTileProperty's
	uint16              arraySize;          //  Elements in the array

public:
	//  Constructor
	CompoundMetaTileProperty(MetaTileProperty **array, uint16 size);

	//  Virtual destructor
	virtual ~CompoundMetaTileProperty();
};

/* ===================================================================== *
   MetaTilePropertyAnd class
 * ===================================================================== */

class MetaTilePropertyAnd : public CompoundMetaTileProperty {
public:
	//  Constructor
	MetaTilePropertyAnd(MetaTileProperty **array, uint16 size) :
		CompoundMetaTileProperty(array, size) {
	}

	bool operator()(
	    MetaTile *mt,
	    int16 mapNum,
	    const TilePoint &tp) const;
};

/* ===================================================================== *
   MetaTilePropertyOr class
 * ===================================================================== */

class MetaTilePropertyOr : public CompoundMetaTileProperty {
public:
	//  Constructor
	MetaTilePropertyOr(MetaTileProperty **array, uint16 size) :
		CompoundMetaTileProperty(array, size) {
	}

	bool operator()(
	    MetaTile *mt,
	    int16 mapNum,
	    const TilePoint &tp) const;
};

typedef int16 MetaTilePropertyID;

enum {
	metaTilePropIDHasWater,

	metaTilePropIDCount
};

bool objIsObject(GameObject *obj);

bool objIsActor(GameObject *obj);

bool objIsWorld(GameObject *obj);

bool objIsLocked(GameObject *obj);

bool objIsUnlocked(GameObject *obj);

bool objIsKey(GameObject *obj);

bool objIsPlayerActor(GameObject *obj);

bool objIsEnemy(GameObject *obj);

bool actorIsDead(Actor *a);

bool actorIsCenterActor(Actor *a);

bool actorIsPlayerActor(Actor *a);

bool actorIsEnemy(Actor *a);

bool tileHasWater(TileInfo *ti);

bool metaTileHasWater(MetaTile *mt, int16 mapNum, const TilePoint &mCoords);

class Properties {
private:
	Common::Array<ObjectProperty *> _objPropArray;
	Common::Array<ActorProperty *> _actorPropArray;
	Common::Array<TileProperty *> _tilePropArray;
	Common::Array<MetaTileProperty *> _metaTilePropArray;

public:

	Properties();
	~Properties();

	const ObjectProperty *getObjProp(ObjectPropertyID id) {
		return _objPropArray[id];
	}

	const ActorProperty *getActorProp(ActorPropertyID id) {
		return _actorPropArray[id];
	}

	const TileProperty *getTileProp(TilePropertyID id) {
		return _tilePropArray[id];
	}

	const MetaTileProperty *getMetaTileProp(MetaTilePropertyID id) {
		return _metaTilePropArray[id];
	}
};

} // end of namespace Saga2

#endif
