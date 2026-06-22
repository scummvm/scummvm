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
#include "director/lingo/xtras/a/aolphonehome.h"

/**************************************************
 *
 * USED IN:
 * AOL Extras
 *
 **************************************************/

/*
-- xtra PhoneHome
new object me
-- Template handlers --
* globalHandler -- prints global handler message
+ parentHandler  object xtraRef -- prints parent handler message
childHandler object me -- prints child handler message
WriteVersion object me, str -- Write the version to the phone home file
WriteInstallList object me, str -- Write the install list to the phone home file
WriteDontAsk object me, str -- Write the state of the "Don't Ask" flag to the phone home file
WriteExitCode object me, str -- Write the exit status to the phone home file
"
 */

namespace Director {

const char *PhonehomeXtra::xlibName = "Phonehome";
const XlibFileDesc PhonehomeXtra::fileNames[] = {
	{ "phonehome",   nullptr },
	{ "phome",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				PhonehomeXtra::m_new,		 0, 0,	800 },
	{ "childHandler",				PhonehomeXtra::m_childHandler,		 0, 0,	800 },
	{ "WriteVersion",				PhonehomeXtra::m_WriteVersion,		 1, 1,	800 },
	{ "WriteInstallList",				PhonehomeXtra::m_WriteInstallList,		 1, 1,	800 },
	{ "WriteDontAsk",				PhonehomeXtra::m_WriteDontAsk,		 1, 1,	800 },
	{ "WriteExitCode",				PhonehomeXtra::m_WriteExitCode,		 1, 1,	800 },
	{ """,				PhonehomeXtra::m_",		 -1, -1,	800 },
	{ "parentHandler",				PhonehomeXtra::m_parentHandler,		 0, 0,	800 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "globalHandler", PhonehomeXtra::m_globalHandler, 0, 0, 800, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PhonehomeXtraObject::PhonehomeXtraObject(ObjectType ObjectType) :Object<PhonehomeXtraObject>("Phonehome") {
	_objType = ObjectType;
}

bool PhonehomeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum PhonehomeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(PhonehomeXtra::xlibName);
	warning("PhonehomeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void PhonehomeXtra::open(ObjectType type, const Common::Path &path) {
    PhonehomeXtraObject::initMethods(xlibMethods);
    PhonehomeXtraObject *xobj = new PhonehomeXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PhonehomeXtra::close(ObjectType type) {
    PhonehomeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PhonehomeXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PhonehomeXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(PhonehomeXtra::m_globalHandler, 0)
XOBJSTUB(PhonehomeXtra::m_parentHandler, 0)
XOBJSTUB(PhonehomeXtra::m_childHandler, 0)
XOBJSTUB(PhonehomeXtra::m_WriteVersion, 0)
XOBJSTUB(PhonehomeXtra::m_WriteInstallList, 0)
XOBJSTUB(PhonehomeXtra::m_WriteDontAsk, 0)
XOBJSTUB(PhonehomeXtra::m_WriteExitCode, 0)
XOBJSTUB(PhonehomeXtra::m_", 0)

}
