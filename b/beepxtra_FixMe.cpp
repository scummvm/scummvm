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
#include "director/lingo/xtras/b/beepxtra.h"

/**************************************************
 *
 * USED IN:
 * Disney Interactive Learning & Creativity Software Sampler Volume 1
 *
 **************************************************/

/*
-- xtra BeepXtra 
new object me
-- BeepXtra --
beepOne object me, int repeat -- beep n times using default system beep
* beepTwo -- global level beep method: beeps once
+ beepThree  object xtraRef -- xtra level version of above method
* modemCheck -- global level method
* findWindow string class, string caption -- returns hwnd
* moveWindow int hwnd, int x, int y, int cx, int cy --
* topMost int hwnd --
* NoTopMost int hwnd --
* noMinMaxBox int hwnd --
* ModifyWSFlag int hwnd, int flag, int remove --
* showWindow int hwnd, int message --
* SetFocus int hwnd --
* GetDesktopWindow -- returns desktop hwnd
* GetWindowRect int hwnd -- returns a rectangle
* SetWindowText int hwnd, string caption --
* GetWindow int hwnd, int cmd --
* RegQueryValue string hkey, string subkey, string subkey2 -- returns a string
* GetVersion -- returns an int
* WaitForProc string commandline -- 
* SendMessage int hwnd, int msg, int wparam, int lparam -- 
* ReadFile string filename -- 
* SetWindowPos int hwnd, int hwndb, int x, int y, int cx, int cy, int flag -- 
* KillTimer int hwnd, int id -- 
"
 \\ FIXME: There are two versions of the same xtra, even though there identical, even stubbing out into identical CPP \ H files & having the name "BeepXtra" in the table of contents) they have different xlibNames. 
 */

namespace Director {

const char *BeepxtraXtra::xlibName = "Beepxtra";
const XlibFileDesc BeepxtraXtra::fileNames[] = {
	{ "beepxtra",   nullptr },
	{ nullptr,        nullptr },
};

// const char *UtilXtra::xlibName = "Util"; 
// const XlibFileDesc UtilXtra::fileNames[] = {
// 	{ "util",   nullptr },
// 	{ nullptr,        nullptr },
// };

static MethodProto xlibMethods[] = {
	{ "new",				BeepxtraXtra::m_new,		 0, 0,	500 },
	{ "beepOne",				BeepxtraXtra::m_beepOne,		 1, 1,	500 },
	{ """,				BeepxtraXtra::m_",		 -1, -1,	500 },
	{ "beepThree",				BeepxtraXtra::m_beepThree,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "beepTwo", BeepxtraXtra::m_beepTwo, 0, 0, 500, HBLTIN },
	{ "modemCheck", BeepxtraXtra::m_modemCheck, 0, 0, 500, HBLTIN },
	{ "findWindow", BeepxtraXtra::m_findWindow, 2, 2, 500, HBLTIN },
	{ "moveWindow", BeepxtraXtra::m_moveWindow, 5, 5, 500, HBLTIN },
	{ "topMost", BeepxtraXtra::m_topMost, 1, 1, 500, HBLTIN },
	{ "NoTopMost", BeepxtraXtra::m_NoTopMost, 1, 1, 500, HBLTIN },
	{ "noMinMaxBox", BeepxtraXtra::m_noMinMaxBox, 1, 1, 500, HBLTIN },
	{ "ModifyWSFlag", BeepxtraXtra::m_ModifyWSFlag, 3, 3, 500, HBLTIN },
	{ "showWindow", BeepxtraXtra::m_showWindow, 2, 2, 500, HBLTIN },
	{ "SetFocus", BeepxtraXtra::m_SetFocus, 1, 1, 500, HBLTIN },
	{ "GetDesktopWindow", BeepxtraXtra::m_GetDesktopWindow, 0, 0, 500, HBLTIN },
	{ "GetWindowRect", BeepxtraXtra::m_GetWindowRect, 1, 1, 500, HBLTIN },
	{ "SetWindowText", BeepxtraXtra::m_SetWindowText, 2, 2, 500, HBLTIN },
	{ "GetWindow", BeepxtraXtra::m_GetWindow, 2, 2, 500, HBLTIN },
	{ "RegQueryValue", BeepxtraXtra::m_RegQueryValue, 3, 3, 500, HBLTIN },
	{ "GetVersion", BeepxtraXtra::m_GetVersion, 0, 0, 500, HBLTIN },
	{ "WaitForProc", BeepxtraXtra::m_WaitForProc, 1, 1, 500, HBLTIN },
	{ "SendMessage", BeepxtraXtra::m_SendMessage, 4, 4, 500, HBLTIN },
	{ "ReadFile", BeepxtraXtra::m_ReadFile, 1, 1, 500, HBLTIN },
	{ "SetWindowPos", BeepxtraXtra::m_SetWindowPos, 7, 7, 500, HBLTIN },
	{ "KillTimer", BeepxtraXtra::m_KillTimer, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BeepxtraXtraObject::BeepxtraXtraObject(ObjectType ObjectType) :Object<BeepxtraXtraObject>("Beepxtra") {
	_objType = ObjectType;
}

bool BeepxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BeepxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BeepxtraXtra::xlibName);
	warning("BeepxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BeepxtraXtra::open(ObjectType type, const Common::Path &path) {
    BeepxtraXtraObject::initMethods(xlibMethods);
    BeepxtraXtraObject *xobj = new BeepxtraXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BeepxtraXtra::close(ObjectType type) {
    BeepxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BeepxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BeepxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BeepxtraXtra::m_beepOne, 0)
XOBJSTUB(BeepxtraXtra::m_beepTwo, 0)
XOBJSTUB(BeepxtraXtra::m_beepThree, 0)
XOBJSTUB(BeepxtraXtra::m_modemCheck, 0)
XOBJSTUB(BeepxtraXtra::m_findWindow, 0)
XOBJSTUB(BeepxtraXtra::m_moveWindow, 0)
XOBJSTUB(BeepxtraXtra::m_topMost, 0)
XOBJSTUB(BeepxtraXtra::m_NoTopMost, 0)
XOBJSTUB(BeepxtraXtra::m_noMinMaxBox, 0)
XOBJSTUB(BeepxtraXtra::m_ModifyWSFlag, 0)
XOBJSTUB(BeepxtraXtra::m_showWindow, 0)
XOBJSTUB(BeepxtraXtra::m_SetFocus, 0)
XOBJSTUB(BeepxtraXtra::m_GetDesktopWindow, 0)
XOBJSTUB(BeepxtraXtra::m_GetWindowRect, 0)
XOBJSTUB(BeepxtraXtra::m_SetWindowText, 0)
XOBJSTUB(BeepxtraXtra::m_GetWindow, 0)
XOBJSTUB(BeepxtraXtra::m_RegQueryValue, 0)
XOBJSTUB(BeepxtraXtra::m_GetVersion, 0)
XOBJSTUB(BeepxtraXtra::m_WaitForProc, 0)
XOBJSTUB(BeepxtraXtra::m_SendMessage, 0)
XOBJSTUB(BeepxtraXtra::m_ReadFile, 0)
XOBJSTUB(BeepxtraXtra::m_SetWindowPos, 0)
XOBJSTUB(BeepxtraXtra::m_KillTimer, 0)
XOBJSTUB(BeepxtraXtra::m_", 0)

}
