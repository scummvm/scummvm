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
#include "director/lingo/xtras/scrnutil.h"

/**************************************************
 *
 * USED IN:
 * amber
 *
 **************************************************/

/*
-- xtra ScrnUtil
-- ScrnUtil Xtra version 1.0 copyright (c) 1996 by g/matter, inc.
-- Programming copyright (c) 1996 Little Planet Publishing
-- For technical support or updates, contact http://www.gmatter.com or support@gmatter.com
--
-- Picture Capture Functions --
* ScreenToClipboard integer left, integer top, integer right, integer bottom
* ScreenToFile integer left, integer top, integer right, integer bottom, string filename

 */

namespace Director {

const char *const ScrnUtilXtra::xlibName = "ScrnUtil";
const XlibFileDesc ScrnUtilXtra::fileNames[] = {
	{ "scrnutil",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "ScreenToClipboard", ScrnUtilXtra::m_ScreenToClipboard, 4, 4, 500, HBLTIN },
	{ "ScreenToFile", ScrnUtilXtra::m_ScreenToFile, 5, 5, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ScrnUtilXtraObject::ScrnUtilXtraObject(ObjectType ObjectType) :Object<ScrnUtilXtraObject>("ScrnUtilXtra") {
	_objType = ObjectType;
}

void ScrnUtilXtra::open(ObjectType type, const Common::Path &path) {
    ScrnUtilXtraObject::initMethods(xlibMethods);
    ScrnUtilXtraObject *xobj = new ScrnUtilXtraObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ScrnUtilXtra::close(ObjectType type) {
    ScrnUtilXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

XOBJSTUB(ScrnUtilXtra::m_ScreenToClipboard, 0)
XOBJSTUB(ScrnUtilXtra::m_ScreenToFile, 0)

}
