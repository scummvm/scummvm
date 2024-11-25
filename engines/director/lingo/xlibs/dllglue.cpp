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
#include "director/lingo/xlibs/dllglue.h"

/**************************************************
 *
 * USED IN:
 * Virtual Nightclub
 *
 **************************************************/

/*
-- DLLGlue External Factory. Dec 1994 PTH (P.HAMILTON@APPLELINK.COM)
--DLLGlue
ISSSS      mNew lib, proc, ret, args       --Creates a new instance of the XObject
S      mName               --Returns the XObject name (DLLGlue)
V      mCall               --Returns an integer status code
I      mStatus             --Returns an integer status code
SI     mError, code        --Returns an error string (for above codes)
S      mLastError          --Returns last error string
I      mWindowHandle       --Returns handle for DPW stage window
S      mComment            --Return UserCode comment
X      mDebug              --write debugging info to message window
 */

namespace Director {

const char *const DLLGlueXObj::xlibName = "DLLGlue";
const XlibFileDesc DLLGlueXObj::fileNames[] = {
	{ "DLLGLUE",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				DLLGlueXObj::m_new,		 4, 4,	400 },
	{ "name",				DLLGlueXObj::m_name,		 0, 0,	400 },
	{ "call",				DLLGlueXObj::m_call,		 0, 0,	400 },
	{ "status",				DLLGlueXObj::m_status,		 0, 0,	400 },
	{ "error",				DLLGlueXObj::m_error,		 1, 1,	400 },
	{ "lastError",				DLLGlueXObj::m_lastError,		 0, 0,	400 },
	{ "windowHandle",				DLLGlueXObj::m_windowHandle,		 0, 0,	400 },
	{ "comment",				DLLGlueXObj::m_comment,		 0, 0,	400 },
	{ "debug",				DLLGlueXObj::m_debug,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DLLGlueXObject::DLLGlueXObject(ObjectType ObjectType) :Object<DLLGlueXObject>("DLLGlue") {
	_objType = ObjectType;
}

void DLLGlueXObj::open(ObjectType type, const Common::Path &path) {
    DLLGlueXObject::initMethods(xlibMethods);
    DLLGlueXObject *xobj = new DLLGlueXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DLLGlueXObj::close(ObjectType type) {
    DLLGlueXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DLLGlueXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DLLGlueXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DLLGlueXObj::m_name, "")
XOBJSTUB(DLLGlueXObj::m_call, 0)
XOBJSTUB(DLLGlueXObj::m_status, 0)
XOBJSTUB(DLLGlueXObj::m_error, "")
XOBJSTUB(DLLGlueXObj::m_lastError, "")
XOBJSTUB(DLLGlueXObj::m_windowHandle, 0)
XOBJSTUB(DLLGlueXObj::m_comment, "")
XOBJSTUBNR(DLLGlueXObj::m_debug)

}
