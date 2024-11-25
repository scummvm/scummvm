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

/*************************************
 *
 * USED IN:
 * Chop Suey (Win)
 * Comedians (Mac)
 *
 *************************************/

/*
 * Memory is a Mac only XObject.
 *
 * Implemented as a no-op, since ScummVM doesn't need to handle memory clears.
 *
 *
 * -- Memory XObject
 * -- December 18th, 1992
 * -- Written by Scott Kildall
 * -- 1992 by Macromedia, Inc
 * -- All rights reserved
 * --
 * I mNew
 * X mClear
 * X mCompact
 * X mPurge
 * I mAvailBytes
 * I mAvailBlock
 * I mStackSpace
 * I mGetVM
 * I mGetAddressing
 * I mGetCache
 * XI mSetCache
 * I mGetPhysicalRAM
 * I mGetMMU
 * I mGetLogicalPage
 * I mGetLogicalRAM
 * I mGetLowMemory
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/memoryxobj.h"


namespace Director {

const char *const MemoryXObj::xlibName = "Memory";
const XlibFileDesc MemoryXObj::fileNames[] = {
	{ "Memory XObj",	nullptr },
	{ "Memory",			nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					MemoryXObj::m_new,			0,	0,	300 },	// D3
	{ "Clear",					MemoryXObj::m_clear,		0,	0,	300 },	// D3
	{ "Purge",					MemoryXObj::m_purge,		0,	0,	400 },	// D4
	{ "GetVM",					MemoryXObj::m_getVM,		0,  0,  300 },  // D3
	{ nullptr, nullptr, 0, 0, 0 }
};

void MemoryXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MemoryXObject::initMethods(xlibMethods);
		MemoryXObject *xobj = new MemoryXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MemoryXObj::close(ObjectType type) {
	if (type == kXObj) {
		MemoryXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


MemoryXObject::MemoryXObject(ObjectType ObjectType) :Object<MemoryXObject>("Memory") {
	_objType = ObjectType;
}

void MemoryXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void MemoryXObj::m_clear(int nargs) {
}

void MemoryXObj::m_purge(int nargs) {
}

void MemoryXObj::m_getVM(int nargs) {
	g_lingo->push(Datum(0)); // Chop Suey (Win) and Comedians (Mac) require Virtual Memory to be disabled
}

} // End of namespace Director
