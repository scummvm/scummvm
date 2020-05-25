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
 */

#ifndef TITANIC_MESSAGE_TARGET_H
#define TITANIC_MESSAGE_TARGET_H

#include "titanic/core/saveable_object.h"

namespace Titanic {

class CMessageTarget;
class CMessage;

typedef bool (CMessageTarget::*PMSG)(CMessage *msg);

struct MSGMAP_ENTRY {
	PMSG _fn;
	const ClassDef * const *_class;
};

struct MSGMAP {
	const MSGMAP *(* pFnGetBaseMap)();
	const MSGMAP_ENTRY *lpEntries;
};

#define DECLARE_MESSAGE_MAP \
protected: \
	static const MSGMAP *getThisMessageMap(); \
	virtual const MSGMAP *getMessageMap() const override

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const MSGMAP *theClass::getMessageMap() const \
		{ return getThisMessageMap(); } \
	const MSGMAP *theClass::getThisMessageMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		typedef bool (theClass::*FNPTR)(CMessage *msg);    \
		static const MSGMAP_ENTRY _messageEntries[] = {

#define ON_MESSAGE(msgClass) \
	{ static_cast<PMSG>((FNPTR)&ThisClass::msgClass), &C##msgClass::_type },

#define END_MESSAGE_MAP() \
		{ (PMSG)nullptr, nullptr } \
	}; \
		static const MSGMAP messageMap = \
		{ &TheBaseClass::getThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	}

#define EMPTY_MESSAGE_MAP(theClass, baseClass) \
	const MSGMAP *theClass::getMessageMap() const \
		{ return getThisMessageMap(); } \
	const MSGMAP *theClass::getThisMessageMap() \
	{ \
		typedef baseClass TheBaseClass;					   \
		static const MSGMAP_ENTRY _messageEntries[] = { \
		{ (PMSG)nullptr, nullptr } \
	}; \
		static const MSGMAP messageMap = \
		{ &TheBaseClass::getThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	} \
	enum { DUMMY }

class CMessageTarget: public CSaveableObject {
protected:
	static const MSGMAP *getThisMessageMap();
	virtual const MSGMAP *getMessageMap() const;

public:
	CLASSDEF;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_MESSAGE_TARGET_H */
