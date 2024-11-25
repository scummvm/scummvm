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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/memcheckxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MemCheck External Factory. 16/03/95 - AAF - New
MemCheck
IS    mNew, virtualFile --Creates a new instance of the XObject
X     mDispose --Disposes of XObject instance
I     mMemoryCheck --Checks memory status
X     mMemoryPurge --Purges memory

-- Memory Check XObject - aep 95.06.20
IS mNew, msg -- Open the XObject
X mDispose -- Dispose of XObject
I mMemoryCheck -- Return memory info
X mMemoryPurge -- Clear chunk o' mem
 */

namespace Director {

const char *const MemCheckXObj::xlibName = "MemCheck";
const XlibFileDesc MemCheckXObj::fileNames[] = {
	{ "MemCheck",		nullptr }, // Jewels of the Oracle - Win
	{ "MemCheck.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MemCheckXObj::m_new,		 1, 1,	400 },
	{ "dispose",				MemCheckXObj::m_dispose,		 0, 0,	400 },
	{ "memoryCheck",				MemCheckXObj::m_memoryCheck,		 0, 0,	400 },
	{ "memoryPurge",				MemCheckXObj::m_memoryPurge,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MemCheckXObject::MemCheckXObject(ObjectType ObjectType) :Object<MemCheckXObject>("MemCheck") {
	_objType = ObjectType;
}

void MemCheckXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MemCheckXObject::initMethods(xlibMethods);
		MemCheckXObject *xobj = new MemCheckXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MemCheckXObj::close(ObjectType type) {
	if (type == kXObj) {
		MemCheckXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MemCheckXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MemCheckXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MemCheckXObj::m_dispose)
XOBJSTUB(MemCheckXObj::m_memoryCheck, 0)
XOBJSTUBNR(MemCheckXObj::m_memoryPurge)

}
