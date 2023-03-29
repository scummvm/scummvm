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
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_SERIALIZE_CLASS_H
#define HPL_SERIALIZE_CLASS_H

#include "hpl1/engine/system/MemoryManager.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/static_string.h"
#include "hpl1/std/map.h"

class TiXmlElement;

namespace hpl {

/////////////////////////////////////////////////
//// ENGINE VALUE TYPES ///////////////////////////////
/////////////////////////////////////////////////

typedef unsigned int eSerializeType;

#define eSerializeType_Bool (1)
#define eSerializeType_Int32 (2)
#define eSerializeType_Float32 (3)

#define eSerializeType_String (4)

#define eSerializeType_Vector2l (5)
#define eSerializeType_Vector2f (6)
#define eSerializeType_Vector3l (7)
#define eSerializeType_Vector3f (8)

#define eSerializeType_Matrixf (9)

#define eSerializeType_Color (10)

#define eSerializeType_Rect2l (11)
#define eSerializeType_Rect2f (12)

#define eSerializeType_Planef (13)

#define eSerializeType_WString (14)

#define eSerializeType_Class (100)

// Important with this type is that it will only be created
// if the member variable is NULL. Else it is assume that it is
// already created.
// Also, for the class to be a class pointer it has to have a
// constructor without any parameters.
// NULL Values are NOT allowed!
#define eSerializeType_ClassPointer (101)

// User defined types must be after this
#define eSerializeType_Last (200)

#define eSerializeType_NULL (0xFFFF)

/////////////////////////////////////////////////
//// ENGINE MAIN TYPES //////////////////////////
/////////////////////////////////////////////////

typedef unsigned int eSerializeMainType;

#define eSerializeMainType_Variable (1)
#define eSerializeMainType_Array (2)
#define eSerializeMainType_Container (3)

#define eSerializeMainType_NULL (0xFFFF)

/////////////////////////////////////////////////
//// HELPER DEFINES /////////////////////////////
/////////////////////////////////////////////////

#define ClassMemberOffset(aClass, aMember) (size_t(&(((aClass *)1)->aMember)) - 1)
#define ClassMemberSize(aClass, aMember) (sizeof(((aClass *)1)->aMember))

/**
 * This inits a class and must be first in a serializable class
 */
#define kSerializableClassInit(aClass)                      \
public:                                                     \
	const static char *msSerialize_Name;                    \
	const static char *msSerialize_ParentName;              \
	static cSerializeMemberField *mpSerialize_MemberFields; \
	virtual ~aClass() = default;                            \
	virtual tString Serialize_GetTopClass() { return #aClass; }

#define kSerializableClassInit_nodestructor(aClass)         \
public:                                                     \
	const static char *msSerialize_Name;                    \
	const static char *msSerialize_ParentName;              \
	static cSerializeMemberField *mpSerialize_MemberFields; \
	virtual tString Serialize_GetTopClass() { return #aClass; }

/**
 * Declared in the cpp-file of the class to be serialized. Class must have parent!
 */
#define kBeginSerialize(aClass, aParent)                                                                       \
	namespace SerializeNamespace_##aClass {                                                                    \
		extern cSerializeMemberField mvTempMemberFields[];                                                     \
	}                                                                                                          \
	const char *aClass::msSerialize_Name = #aClass;                                                            \
	const char *aClass::msSerialize_ParentName = #aParent;                                                     \
	cSerializeMemberField *aClass::mpSerialize_MemberFields = SerializeNamespace_##aClass::mvTempMemberFields; \
	namespace SerializeNamespace_##aClass {                                                                    \
		typedef aClass tVarClass;                                                                              \
		cSerializeMemberField mvTempMemberFields[] = {

#define kBeginSerializeBase(aClass)                                                                            \
	namespace SerializeNamespace_##aClass {                                                                    \
		extern cSerializeMemberField mvTempMemberFields[];                                                     \
	}                                                                                                          \
	const char *aClass::msSerialize_Name = #aClass;                                                            \
	const char *aClass::msSerialize_ParentName = "";                                                           \
	cSerializeMemberField *aClass::mpSerialize_MemberFields = SerializeNamespace_##aClass::mvTempMemberFields; \
	namespace SerializeNamespace_##aClass {                                                                    \
		typedef aClass tVarClass;                                                                              \
		cSerializeMemberField mvTempMemberFields[] = {

#define kBeginSerializeVirtual(aClass, aParent)                                                                \
	namespace SerializeNamespace_##aClass {                                                                    \
		extern cSerializeMemberField mvTempMemberFields[];                                                     \
	}                                                                                                          \
	const char *aClass::msSerialize_Name = #aClass;                                                            \
	const char *aClass::msSerialize_ParentName = #aParent;                                                     \
	cSerializeMemberField *aClass::mpSerialize_MemberFields = SerializeNamespace_##aClass::mvTempMemberFields; \
	namespace SerializeNamespace_##aClass {                                                                    \
		typedef aClass tVarClass;                                                                              \
		cSerializeMemberField mvTempMemberFields[] = {

#define kBeginSerializeBaseVirtual(aClass)                                                                     \
	namespace SerializeNamespace_##aClass {                                                                    \
		extern cSerializeMemberField mvTempMemberFields[];                                                     \
	}                                                                                                          \
	const char *aClass::msSerialize_Name = #aClass;                                                            \
	const char *aClass::msSerialize_ParentName = "";                                                           \
	cSerializeMemberField *aClass::mpSerialize_MemberFields = SerializeNamespace_##aClass::mvTempMemberFields; \
	namespace SerializeNamespace_##aClass {                                                                    \
		typedef aClass tVarClass;                                                                              \
		cSerializeMemberField mvTempMemberFields[] = {

/**
 *Declared when all variable declarations are over.
 */
#define kEndSerialize()                                     \
	cSerializeMemberField("", 0, 0, eSerializeType_NULL, 0) \
	}                                                       \
	;                                                       \
	}

/**
 * Declared after begin, adds a variable or class.
 */
#define kSerializeVar(aVar, aType) \
	cSerializeMemberField(#aVar, ClassMemberOffset(tVarClass, aVar), ClassMemberSize(tVarClass, aVar), aType, eSerializeMainType_Variable),

/**
 * Declared after begin, adds an array of variables.
 */
#define kSerializeVarArray(aVar, aType, aArraySize) \
	cSerializeMemberField(#aVar, ClassMemberOffset(tVarClass, aVar), ClassMemberSize(tVarClass, aVar), aType, eSerializeMainType_Array, aArraySize),

/**
 * Declared after begin, adds an container of variables.
 */
#define kSerializeVarContainer(aVar, aType) \
	cSerializeMemberField(#aVar, ClassMemberOffset(tVarClass, aVar), ClassMemberSize(tVarClass, aVar), aType, eSerializeMainType_Container),
/**
 * Declared after begin, adds an container of classes.
 */
#define kSerializeClassContainer(aVar, aClass, aType) \
	cSerializeMemberField(#aVar, ClassMemberOffset(tVarClass, aVar), ClassMemberSize(tVarClass, aVar), aType, eSerializeMainType_Container, #aClass),

/////////////////////////////////////////////////
//// CLASSES ////////////////////////////////////
/////////////////////////////////////////////////

//-------------------------------------------------

class cSerializeMemberField {
public:
	constexpr cSerializeMemberField(Hpl1::StaticString asName, size_t alOffset, size_t alSize, eSerializeType alType,
									eSerializeMainType aMainType) : msName(asName), msClassName(""), mlOffset(alOffset), mlSize(alSize), mType(alType), mMainType(aMainType), mlArraySize(0) {
	}

	constexpr cSerializeMemberField(Hpl1::StaticString asName, size_t alOffset, size_t alSize, eSerializeType alType,
									eSerializeMainType aMainType, size_t alArraySize) : msName(asName), msClassName(""), mlOffset(alOffset), mlSize(alSize), mType(alType), mMainType(aMainType), mlArraySize(alArraySize) {
	}

	constexpr cSerializeMemberField(Hpl1::StaticString asName, size_t alOffset, size_t alSize, eSerializeType alType,
									eSerializeMainType aMainType, Hpl1::StaticString asClassName) : msName(asName), msClassName(asClassName), mlOffset(alOffset), mlSize(alSize), mType(alType), mMainType(aMainType), mlArraySize(0) {
	}

	Hpl1::StaticString msName;
	Hpl1::StaticString msClassName;
	size_t mlOffset;
	size_t mlSize;
	eSerializeType mType;
	eSerializeMainType mMainType;
	size_t mlArraySize;
};

//-------------------------------------------------

class iSerializable {
public:
	virtual ~iSerializable() = default;
	virtual tString Serialize_GetTopClass() { return ""; }
};

//-------------------------------------------------

class iSerializableType {
public:
	virtual ~iSerializableType() = default;
	virtual char *ValueToString(void *apVal) = 0;
	virtual void ValueFromString(char *apString, void *apVal) = 0;
};

//-------------------------------------------------

struct cSerializeSavedClass {
public:
	constexpr cSerializeSavedClass() = default;
	constexpr cSerializeSavedClass(const char *asName, const char *asParent,
								   cSerializeMemberField *apMemberFields, size_t alSize,
								   iSerializable *(*apCreateFunc)()) : msName(asName), msParentName(asParent), mpMemberFields(apMemberFields),
																	   mlSize(alSize), mpCreateFunc(apCreateFunc) {}

	const char *msName = "";
	const char *msParentName = "";
	cSerializeMemberField *mpMemberFields = nullptr;
	size_t mlSize = 0;
	iSerializable *(*mpCreateFunc)() = nullptr;
};

//-------------------------------------------------

class cSerializeMemberFieldIterator {
public:
	cSerializeMemberFieldIterator(cSerializeSavedClass *apTopClass);

	bool HasNext();
	cSerializeMemberField *GetNext();

private:
	cSerializeSavedClass *mpSavedClass;
	int mlFieldNum;
};

//-------------------------------------------------

typedef Hpl1::Std::map<Hpl1::StaticString, cSerializeSavedClass> tSerializeSavedClassMap;
typedef tSerializeSavedClassMap::iterator tSerializeSavedClassMapIt;

typedef Common::List<cSerializeSavedClass *> tSerializeSavedClassList;
typedef tSerializeSavedClassList::iterator tSerializeSavedClassListIt;

class cSerializeClass {
public:
	static void initSaveClassesMap();
	static void finalizeSaveClassesMap();

	static void SetLog(bool abX);
	static bool GetLog();

	static void PrintMembers(iSerializable *apData);

	static bool SaveToFile(iSerializable *apData, const tWString &asFile, const tString &asRoot);
	static void SaveToElement(iSerializable *apData, const tString &asName, TiXmlElement *apParent, bool abIsPointer = false);

	static bool LoadFromFile(iSerializable *apData, const tWString &asFile);
	static void LoadFromElement(iSerializable *apData, TiXmlElement *apElement, bool abIsPointer = false);

	static cSerializeSavedClass *GetClass(const tString &asName);

	static cSerializeMemberFieldIterator GetMemberFieldIterator(iSerializable *apData);

	static tString ValueToString(void *apData, size_t alOffset, eSerializeType aType);
	static void StringToValue(void *apData, size_t alOffset, eSerializeType aType,
							  const char *asVal);

private:
	static void SaveVariable(TiXmlElement *apElement, cSerializeMemberField *apField, iSerializable *apData);
	static void SaveArray(TiXmlElement *apElement, cSerializeMemberField *apField, iSerializable *apData);
	static void SaveContainer(TiXmlElement *apElement, cSerializeMemberField *apField, iSerializable *apData);

	static void LoadVariable(TiXmlElement *apElement, iSerializable *apData, cSerializeSavedClass *apClass);
	static void LoadArray(TiXmlElement *apElement, iSerializable *apData, cSerializeSavedClass *apClass);
	static void LoadClass(TiXmlElement *apElement, iSerializable *apData, cSerializeSavedClass *apClass);
	static void LoadClassPointer(TiXmlElement *apElement, iSerializable *apData, cSerializeSavedClass *apClass);
	static void LoadContainer(TiXmlElement *apElement, iSerializable *apData, cSerializeSavedClass *apClass);

	static void FillSaveClassMembersList(tSerializeSavedClassList *apList, cSerializeSavedClass *apClass);
	static void SaveSavedClassMembers(cSerializeSavedClass *apClass, iSerializable *apData);

	static cSerializeMemberField *GetMemberField(const tString &asName, cSerializeSavedClass *apClass);

	static size_t SizeOfType(eSerializeType aType);

	static void SetUpData();

	static char msTempCharArray[2048];

	static bool mbDataSetup;
	static tSerializeSavedClassMap *m_mapSavedClasses;
	static Common::Array<iSerializableType *> mvValueTypes;
};

//-------------------------------------------------

}
#endif // HPL_SERIALIZE_CLASS_H
