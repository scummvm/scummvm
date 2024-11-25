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
#include "director/lingo/xlibs/mystisle.h"

/**************************************************
 *
 * USED IN:
 * Mysterious Island: A Race Against Time and Hot Lava
 *
 **************************************************/

/*
-- MystIsle External Factory, v. 1.0 by David Jackson-Shields
--MYSTISLE
I      mNew                     --Creates new instance
X      mDispose                 --Disposes of XObject instance
S      mName                    --Returns the XObject name (MystIsle)
I      mGetLastError            --Returns most recent error code
I      mGetSysVersion           --returns MS Windows version as an integer
I      mGetQTVersion            --returns QuickTime version as an integer
I      mVerifyUnlocked          --verify startup volume is writable (for Prefs)
S      mGetPrefsPath            --return path of Windows directory
IPP    mDiagnostic, picHdl1, picHdl2 --for testing during development
 */

namespace Director {

const char *const MystIsleXObj::xlibName = "MystIsle";
const XlibFileDesc MystIsleXObj::fileNames[] = {
	{ "MYSTISLE",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MystIsleXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MystIsleXObj::m_dispose,		 0, 0,	400 },
	{ "name",				MystIsleXObj::m_name,		 0, 0,	400 },
	{ "getLastError",				MystIsleXObj::m_getLastError,		 0, 0,	400 },
	{ "getSysVersion",				MystIsleXObj::m_getSysVersion,		 0, 0,	400 },
	{ "getQTVersion",				MystIsleXObj::m_getQTVersion,		 0, 0,	400 },
	{ "verifyUnlocked",				MystIsleXObj::m_verifyUnlocked,		 0, 0,	400 },
	{ "getPrefsPath",				MystIsleXObj::m_getPrefsPath,		 0, 0,	400 },
	{ "diagnostic",				MystIsleXObj::m_diagnostic,		 2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MystIsleXObject::MystIsleXObject(ObjectType ObjectType) :Object<MystIsleXObject>("MystIsle") {
	_objType = ObjectType;
}

void MystIsleXObj::open(ObjectType type, const Common::Path &path) {
    MystIsleXObject::initMethods(xlibMethods);
    MystIsleXObject *xobj = new MystIsleXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MystIsleXObj::close(ObjectType type) {
    MystIsleXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MystIsleXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MystIsleXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MystIsleXObj::m_dispose)
XOBJSTUB(MystIsleXObj::m_name, "MystIsle")
XOBJSTUB(MystIsleXObj::m_getLastError, 0)

void MystIsleXObj::m_getSysVersion(int nargs) {
	// Checks the installed OS version, return minimum expected version
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(g_director->getPlatform() == Common::kPlatformWindows ? 310 : 700));
}

void MystIsleXObj::m_getQTVersion(int nargs) {
	// Checks the installed QuickTime version, return minimum expected version
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(g_director->getPlatform() == Common::kPlatformWindows ? 110 : 200));
}

XOBJSTUB(MystIsleXObj::m_verifyUnlocked, 0)  // Check whether the drive is writable (locked) or read-only (unlocked), return unlocked
XOBJSTUB(MystIsleXObj::m_getPrefsPath, "")
XOBJSTUB(MystIsleXObj::m_diagnostic, 0)
}
