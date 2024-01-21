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
#include "director/lingo/xlibs/processxobj.h"

/**************************************************
 *
 * USED IN:
 * chopsuey
 *
 **************************************************/

/*
-- ProcessXObj, Kills a specified process
-- ©1994 Image Technologies, Inc.
-- Written by: Steve Kos
X	 mNew           -- inits mem space
X	 mDispose
ISS mFindPSN		-- finds the psn
X	 mKillProcess	-- kills this process
 */

namespace Director {

const char *ProcessXObj::xlibName = "ProcessXObj";
const char *ProcessXObj::fileNames[] = {
	"ProcessXObj",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",				ProcessXObj::m_new,		 0, 0,	400 },
	{ "dispose",				ProcessXObj::m_dispose,		 0, 0,	400 },
	{ "findPSN",				ProcessXObj::m_findPSN,		 2, 2,	400 },
	{ "killProcess",				ProcessXObj::m_killProcess,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

ProcessXObject::ProcessXObject(ObjectType ObjectType) :Object<ProcessXObject>("ProcessXObj") {
	_objType = ObjectType;
}

void ProcessXObj::open(int type) {
	if (type == kXObj) {
		ProcessXObject::initMethods(xlibMethods);
		ProcessXObject *xobj = new ProcessXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void ProcessXObj::close(int type) {
	if (type == kXObj) {
		ProcessXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void ProcessXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ProcessXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ProcessXObj::m_dispose)
XOBJSTUB(ProcessXObj::m_findPSN, 0)
XOBJSTUBNR(ProcessXObj::m_killProcess)

}
