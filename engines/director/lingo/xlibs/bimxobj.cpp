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
#include "director/lingo/xlibs/bimxobj.h"

/**************************************************
 *
 * USED IN:
 * Karma: Curse of the 12 Caves
 *
 **************************************************/

/*
 * -- BIM16 External Factory. 14JULY94
 * --BIM
 * I      mNew                                            --Creates a new instance of the XObject
 * ISII   mPlay,    FileName, x, y                        --Play Flc file
 * ISIII  mPlayTo,  FileName, x, y, frame                 --Play Flc file
 * ISIII  mVideo,   FileName, x, y, delay                 --Play Flc file
 * ISIIII mStretch, FileName, destx, desty, destw, desth  --Play Flc file
 * I      mDispose                                        --Disposes of XObject instance
 */

namespace Director {

const char *const BIMXObj::xlibName = "BIM";
const XlibFileDesc BIMXObj::fileNames[] = {
	{ "FLC",	nullptr },
	{ "BIM",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{"new",		BIMXObj::m_new, 0, 0, 400},
	{"play",	BIMXObj::m_play, 3, 3, 400},
	{"playTo",	BIMXObj::m_playTo, 4, 4, 400},
	{"video",	BIMXObj::m_video, 4, 4, 400},
	{"stretch",	BIMXObj::m_stretch, 5, 5, 400},
	{nullptr, nullptr, 0, 0, 0}
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BIMXObject::BIMXObject(const ObjectType objType) :Object<BIMXObject>("BIM") {
	_objType = objType;
}

void BIMXObj::open(ObjectType type, const Common::Path &path) {
    BIMXObject::initMethods(xlibMethods);
    BIMXObject *xobj = new BIMXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BIMXObj::close(ObjectType type) {
    BIMXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();
}

void BIMXObj::m_new(const int nargs) {
	g_lingo->printSTUBWithArglist("BIMXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BIMXObj::m_play, 0)
XOBJSTUB(BIMXObj::m_playTo, 0)
XOBJSTUB(BIMXObj::m_video, 0)
XOBJSTUB(BIMXObj::m_stretch, 0)

}
