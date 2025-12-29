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
#include "director/lingo/xlibs/m/mazexobj.h"

/**************************************************
 *
 * USED IN:
 * wttf
 *
 **************************************************/

/*
--		Maze XObj v 1.09F (c) 1995 Samizdat Productions. All Rights Reserved.
--		written by Christopher P. Kelly
I		mNew
II		mUserClicked where
II		mCheckClick where
S		mGetKeySettings
IS		mSetKeySettings string
S		mGetLocSettings
IS		mSetLocSettings string
I		mGetTemplate
I		mGetKeyAdded
IS		mSetCDPath fullpath
IS		mSetHDPath fullpath
IIIII	mSetMovieBox t,l,b,r
II		mUpdateScreen moveFlag
I		mFixScreen
I		mGetLevel
II		mGetKeyState keyNum
III		mSetKeyState keyNum, state
II		mSetPreclickOption onOrOff
I		mDisposeMem
 */

namespace Director {

const char *MazeXObj::xlibName = "MazeObj";
const XlibFileDesc MazeXObj::fileNames[] = {
	{ "MazeObj",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MazeXObj::m_new,		 0, 0,	400 },
	{ "userClicked",				MazeXObj::m_userClicked,		 1, 1,	400 },
	{ "checkClick",				MazeXObj::m_checkClick,		 1, 1,	400 },
	{ "getKeySettings",				MazeXObj::m_getKeySettings,		 0, 0,	400 },
	{ "setKeySettings",				MazeXObj::m_setKeySettings,		 1, 1,	400 },
	{ "getLocSettings",				MazeXObj::m_getLocSettings,		 0, 0,	400 },
	{ "setLocSettings",				MazeXObj::m_setLocSettings,		 1, 1,	400 },
	{ "getTemplate",				MazeXObj::m_getTemplate,		 0, 0,	400 },
	{ "getKeyAdded",				MazeXObj::m_getKeyAdded,		 0, 0,	400 },
	{ "setCDPath",				MazeXObj::m_setCDPath,		 1, 1,	400 },
	{ "setHDPath",				MazeXObj::m_setHDPath,		 1, 1,	400 },
	{ "setMovieBox",				MazeXObj::m_setMovieBox,		 4, 4,	400 },
	{ "updateScreen",				MazeXObj::m_updateScreen,		 1, 1,	400 },
	{ "fixScreen",				MazeXObj::m_fixScreen,		 0, 0,	400 },
	{ "getLevel",				MazeXObj::m_getLevel,		 0, 0,	400 },
	{ "getKeyState",				MazeXObj::m_getKeyState,		 1, 1,	400 },
	{ "setKeyState",				MazeXObj::m_setKeyState,		 2, 2,	400 },
	{ "setPreclickOption",				MazeXObj::m_setPreclickOption,		 1, 1,	400 },
	{ "disposeMem",				MazeXObj::m_disposeMem,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MazeXObject::MazeXObject(ObjectType ObjectType) :Object<MazeXObject>("MazeObj") {
	_objType = ObjectType;
}

void MazeXObj::open(ObjectType type, const Common::Path &path) {
    MazeXObject::initMethods(xlibMethods);
    MazeXObject *xobj = new MazeXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MazeXObj::close(ObjectType type) {
    MazeXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MazeXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MazeXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MazeXObj::m_userClicked, 0)
XOBJSTUB(MazeXObj::m_checkClick, 0)
XOBJSTUB(MazeXObj::m_getKeySettings, "")
XOBJSTUB(MazeXObj::m_setKeySettings, 0)
XOBJSTUB(MazeXObj::m_getLocSettings, "")
XOBJSTUB(MazeXObj::m_setLocSettings, 0)
XOBJSTUB(MazeXObj::m_getTemplate, 0)
XOBJSTUB(MazeXObj::m_getKeyAdded, 0)
XOBJSTUB(MazeXObj::m_setCDPath, 0)
XOBJSTUB(MazeXObj::m_setHDPath, 0)
XOBJSTUB(MazeXObj::m_setMovieBox, 0)
XOBJSTUB(MazeXObj::m_updateScreen, 0)
XOBJSTUB(MazeXObj::m_fixScreen, 0)
XOBJSTUB(MazeXObj::m_getLevel, 0)
XOBJSTUB(MazeXObj::m_getKeyState, 0)
XOBJSTUB(MazeXObj::m_setKeyState, 0)
XOBJSTUB(MazeXObj::m_setPreclickOption, 0)
XOBJSTUB(MazeXObj::m_disposeMem, 0)

}
