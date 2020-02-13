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

#ifndef ULTIMA_SHARED_CORE_MESSAGE_TARGET_H
#define ULTIMA_SHARED_CORE_MESSAGE_TARGET_H

#include "ultima/shared/core/base_object.h"

namespace Ultima {
namespace Shared {

class MessageTarget;
class CMessage;

typedef bool (MessageTarget:: *PMSG)(CMessage *msg);

struct MSGMAP_ENTRY {
	PMSG _fn;
	ClassDefFn _classDef;
};

struct MSGMAP {
	const MSGMAP *(*pFnGetBaseMap)();
	const MSGMAP_ENTRY *lpEntries;
};

#define DECLARE_MESSAGE_MAP \
protected: \
	static const Ultima::Shared::MSGMAP *getThisMessageMap(); \
	virtual const Ultima::Shared::MSGMAP *getMessageMap() const override

#define DECLARE_MESSAGE_MAP_BASE \
protected: \
	static const Ultima::Shared::MSGMAP *getThisMessageMap(); \
	virtual const Ultima::Shared::MSGMAP *getMessageMap() const

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	Ultima::Shared::ClassDef theClass::type() { return Ultima::Shared::ClassDef(#theClass, &baseClass::type); } \
	const Ultima::Shared::MSGMAP *theClass::getMessageMap() const \
		{ return getThisMessageMap(); } \
	const Ultima::Shared::MSGMAP *theClass::getThisMessageMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		typedef bool (theClass::*FNPTR)(Ultima::Shared::CMessage *msg);    \
		static const Ultima::Shared::MSGMAP_ENTRY _messageEntries[] = {

#define ON_MESSAGE(msgClass) \
	{ static_cast<Ultima::Shared::PMSG>((FNPTR)&ThisClass::msgClass), &C##msgClass::type },

#define END_MESSAGE_MAP() \
		{ (Ultima::Shared::PMSG)nullptr, nullptr } \
	}; \
		static const Ultima::Shared::MSGMAP messageMap = \
		{ &TheBaseClass::getThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	}

#define EMPTY_MESSAGE_MAP(theClass, baseClass) \
	Ultima::Shared::ClassDef theClass::type() { return Ultima::Shared::ClassDef(#theClass, &baseClass::type); } \
	const Ultima::Shared::MSGMAP *theClass::getMessageMap() const \
		{ return getThisMessageMap(); } \
	const Ultima::Shared::MSGMAP *theClass::getThisMessageMap() \
	{ \
		typedef baseClass TheBaseClass;					   \
		static const Ultima::Shared::MSGMAP_ENTRY _messageEntries[] = { \
		{ (Ultima::Shared::PMSG)nullptr, nullptr } \
	}; \
		static const Ultima::Shared::MSGMAP messageMap = \
		{ &TheBaseClass::getThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	} \
	enum { DUMMY##theClass }

/**
 * The immediate descendant of the base object, this implements the base class for objects
 * that can receive messages
 */
class MessageTarget: public BaseObject {
	DECLARE_MESSAGE_MAP_BASE;
public:
	CLASSDEF;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
