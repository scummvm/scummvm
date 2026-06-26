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
#include "director/lingo/xtras/d/directemail.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9
 *
 **************************************************/

/*
-- xtra DirectEmail
-- DirectEmail Xtra (TM) version 1.5.1
--
-- Developed by Tomer Berda, DirectXtras Inc.
--
new object me, string MailServer, string MailboxName, string Password
forget object me
SendEmail object me, string From, string To, string CC, string BCC, string Subject, string Body, string Attachment
EmailDone object me
EmailErrorCode object me
EmailErrorMessage object me
EmailAbort object me
* RegisterDirectEmail integer SerialNumber
 */

namespace Director {

const char *DirectEmailXtra::xlibName = "DirectEmail";
const XlibFileDesc DirectEmailXtra::fileNames[] = {
	{ "directemail",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DirectEmailXtra::m_new,		 3, 3,	500 },
	{ "forget",				DirectEmailXtra::m_forget,		 0, 0,	500 },
	{ "SendEmail",				DirectEmailXtra::m_SendEmail,		 7, 7,	500 },
	{ "EmailDone",				DirectEmailXtra::m_EmailDone,		 0, 0,	500 },
	{ "EmailErrorCode",				DirectEmailXtra::m_EmailErrorCode,		 0, 0,	500 },
	{ "EmailErrorMessage",				DirectEmailXtra::m_EmailErrorMessage,		 0, 0,	500 },
	{ "EmailAbort",				DirectEmailXtra::m_EmailAbort,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "RegisterDirectEmail", DirectEmailXtra::m_RegisterDirectEmail, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirectEmailXtraObject::DirectEmailXtraObject(ObjectType ObjectType) :Object<DirectEmailXtraObject>("DirectEmail") {
	_objType = ObjectType;
}

bool DirectEmailXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirectEmailXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirectEmailXtra::xlibName);
	warning("DirectEmailXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirectEmailXtra::open(ObjectType type, const Common::Path &path) {
    DirectEmailXtraObject::initMethods(xlibMethods);
    DirectEmailXtraObject *xobj = new DirectEmailXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DirectEmailXtra::close(ObjectType type) {
    DirectEmailXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DirectEmailXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirectEmailXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DirectEmailXtra::m_forget, 0)
XOBJSTUB(DirectEmailXtra::m_SendEmail, 0)
XOBJSTUB(DirectEmailXtra::m_EmailDone, 0)
XOBJSTUB(DirectEmailXtra::m_EmailErrorCode, 0)
XOBJSTUB(DirectEmailXtra::m_EmailErrorMessage, 0)
XOBJSTUB(DirectEmailXtra::m_EmailAbort, 0)
XOBJSTUB(DirectEmailXtra::m_RegisterDirectEmail, 0)

}
