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
#include "director/lingo/xtras/a/avicreator.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra AviCreator
-- AviCreator Xtra v0.1 (c) 2007 Valentin Schmidt
-- contact: fluxus@freenet.de
new object me
avi_open object me, string filename, integer fps, *fourCC, keyFrameEvery, quality, bytesPerSecond
avi_addFrameFromBmp object me, string bmpfile
avi_addFrameFromImage object me, object image
avi_close object me
"
 */

namespace Director {

const char *AvicreatorXtra::xlibName = "Avicreator";
const XlibFileDesc AvicreatorXtra::fileNames[] = {
	{ "avicreator",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AvicreatorXtra::m_new,		 0, 0,	1000 },
	{ "avi_open",				AvicreatorXtra::m_avi_open,		 6, 6,	1000 },
	{ "avi_addFrameFromBmp",				AvicreatorXtra::m_avi_addFrameFromBmp,		 1, 1,	1000 },
	{ "avi_addFrameFromImage",				AvicreatorXtra::m_avi_addFrameFromImage,		 1, 1,	1000 },
	{ "avi_close",				AvicreatorXtra::m_avi_close,		 0, 0,	1000 },
	{ """,				AvicreatorXtra::m_",		 -1, -1,	1000 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AvicreatorXtraObject::AvicreatorXtraObject(ObjectType ObjectType) :Object<AvicreatorXtraObject>("Avicreator") {
	_objType = ObjectType;
}

bool AvicreatorXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AvicreatorXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AvicreatorXtra::xlibName);
	warning("AvicreatorXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AvicreatorXtra::open(ObjectType type, const Common::Path &path) {
    AvicreatorXtraObject::initMethods(xlibMethods);
    AvicreatorXtraObject *xobj = new AvicreatorXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AvicreatorXtra::close(ObjectType type) {
    AvicreatorXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AvicreatorXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AvicreatorXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AvicreatorXtra::m_avi_open, 0)
XOBJSTUB(AvicreatorXtra::m_avi_addFrameFromBmp, 0)
XOBJSTUB(AvicreatorXtra::m_avi_addFrameFromImage, 0)
XOBJSTUB(AvicreatorXtra::m_avi_close, 0)
XOBJSTUB(AvicreatorXtra::m_", 0)

}
