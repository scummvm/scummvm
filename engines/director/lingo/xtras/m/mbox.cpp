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
#include "director/lingo/xtras/m/mbox.h"

/**************************************************
 *
 * USED IN:
 * getaheadmath
 *
 **************************************************/

/*
-- xtra MBoxXtra
new object me
forget object me  	--(do't call directly) dispose of the text to speech object
-- Global Handlers --
* OKMsgBox string msg
* YESNOMsgBox string msg
* OKCANCELMsgBox string msg
* RETRYCANCELMsgBox string msg
* YESNOCANCELMsgBox string msg
* DiskSpaceAvailable string volume
* WindowsFileLimit integer numfiles
* WindowsFileAttr string fileName,integer attribute
 */

namespace Director {

const char *MBoxXtra::xlibName = "MBoxXtra";
const XlibFileDesc MBoxXtra::fileNames[] = {
	{ "mboxx",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MBoxXtra::m_new,		 0, 0,	500 },
	{ "forget",				MBoxXtra::m_forget,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "OKMsgBox", MBoxXtra::m_OKMsgBox, 1, 1, 500, HBLTIN },
	{ "YESNOMsgBox", MBoxXtra::m_YESNOMsgBox, 1, 1, 500, HBLTIN },
	{ "OKCANCELMsgBox", MBoxXtra::m_OKCANCELMsgBox, 1, 1, 500, HBLTIN },
	{ "RETRYCANCELMsgBox", MBoxXtra::m_RETRYCANCELMsgBox, 1, 1, 500, HBLTIN },
	{ "YESNOCANCELMsgBox", MBoxXtra::m_YESNOCANCELMsgBox, 1, 1, 500, HBLTIN },
	{ "DiskSpaceAvailable", MBoxXtra::m_DiskSpaceAvailable, 1, 1, 500, HBLTIN },
	{ "WindowsFileLimit", MBoxXtra::m_WindowsFileLimit, 1, 1, 500, HBLTIN },
	{ "WindowsFileAttr", MBoxXtra::m_WindowsFileAttr, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MBoxXtraObject::MBoxXtraObject(ObjectType ObjectType) :Object<MBoxXtraObject>("MBox") {
	_objType = ObjectType;
}

bool MBoxXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MBoxXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MBoxXtra::xlibName);
	warning("MBoxXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MBoxXtra::open(ObjectType type, const Common::Path &path) {
    MBoxXtraObject::initMethods(xlibMethods);
    MBoxXtraObject *xobj = new MBoxXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MBoxXtra::close(ObjectType type) {
    MBoxXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MBoxXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MBoxXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MBoxXtra::m_forget, 0)
XOBJSTUB(MBoxXtra::m_OKMsgBox, 0)
XOBJSTUB(MBoxXtra::m_YESNOMsgBox, 0)
XOBJSTUB(MBoxXtra::m_OKCANCELMsgBox, 0)
XOBJSTUB(MBoxXtra::m_RETRYCANCELMsgBox, 0)
XOBJSTUB(MBoxXtra::m_YESNOCANCELMsgBox, 0)
XOBJSTUB(MBoxXtra::m_DiskSpaceAvailable, 700)
XOBJSTUB(MBoxXtra::m_WindowsFileLimit, 40)
XOBJSTUB(MBoxXtra::m_WindowsFileAttr, 0)

}
