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
#include "director/lingo/xlibs/s/smallutil.h"

/**************************************************
 *
 * USED IN:
 * flipper
 *
 **************************************************/

/*
-- SmallUtil, xobject  08Aug95 PF
-- ©1995 - Interactive Multimedia Unit
-- Central Queensland University
-- CodeWarrior version, 16May95 EA
-- xobject framework, 29nov94 JT
--
I    mNew                           -- Create an instance of SmallUtil
X    mDispose                       -- Destroy the instance of SmallUtil
I    mQTVersion                     -- Return QT Version
S    mQTVersionasText               -- Return QT Version as Text "."delimited
I    mSMVersion                     -- Return Sound Manager Version
S    mSMVersionasText               -- Return Sound Manager Version "." delimited
S    mFontList                      -- Return List of FOND's
S    mVolumeList                    -- Return List of Mounted Volumes
 */

namespace Director {

const char *SmallUtilXObj::xlibName = "SmallUtil";
const XlibFileDesc SmallUtilXObj::fileNames[] = {
	{ "SmallUtil",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SmallUtilXObj::m_new,		 0, 0,	400 },
	{ "dispose",				SmallUtilXObj::m_dispose,		 0, 0,	400 },
	{ "qTVersion",				SmallUtilXObj::m_qTVersion,		 0, 0,	400 },
	{ "qTVersionasText",				SmallUtilXObj::m_qTVersionasText,		 0, 0,	400 },
	{ "sMVersion",				SmallUtilXObj::m_sMVersion,		 0, 0,	400 },
	{ "sMVersionasText",				SmallUtilXObj::m_sMVersionasText,		 0, 0,	400 },
	{ "fontList",				SmallUtilXObj::m_fontList,		 0, 0,	400 },
	{ "volumeList",				SmallUtilXObj::m_volumeList,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SmallUtilXObject::SmallUtilXObject(ObjectType ObjectType) :Object<SmallUtilXObject>("SmallUtil") {
	_objType = ObjectType;
}

void SmallUtilXObj::open(ObjectType type, const Common::Path &path) {
    SmallUtilXObject::initMethods(xlibMethods);
    SmallUtilXObject *xobj = new SmallUtilXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SmallUtilXObj::close(ObjectType type) {
    SmallUtilXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SmallUtilXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SmallUtilXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(SmallUtilXObj::m_dispose)
XOBJSTUB(SmallUtilXObj::m_qTVersion, 0)
XOBJSTUB(SmallUtilXObj::m_qTVersionasText, "2.5")
XOBJSTUB(SmallUtilXObj::m_sMVersion, 0)
XOBJSTUB(SmallUtilXObj::m_sMVersionasText, "3.2")
XOBJSTUB(SmallUtilXObj::m_fontList, "")
XOBJSTUB(SmallUtilXObj::m_volumeList, "")

}
