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
#include "director/lingo/xtras/u/uihelper.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 2/3/4/6 (D9) — shipped as WatcherHelper.x32
 *
 **************************************************/

/*
-- xtra UiHelper
new object me
-- UiHelper Xtra --
*setCurrentCast int castLibNum                              -- sets Director's current cast
*activateWindow int windowId                                -- brings the window to the front
*activateMediaEditor int memberNum, int castLibNum          -- opens a media editor window
*getBehaviorMemRef int channelNum, int behaviorNum          -- get behavior member id at current frame
*getBehaviorInitializers int channelNum, int behaviorNum    -- get behavior initializer string at current frame
*activateScriptEditor int memberNum, int castLibNum, int selectionStart, int selectionEnd -- launch and scroll script editor
*sleep int timeInMs                                         -- make this process sleep a while
*getByte int number, int byteNumber                         -- get a byte of the int (0==low order byte)
*executeCommand string cmd                                  -- execute the command string
*isNt                                                       -- Is this windows NT?
*getLanguage -- What's this machine's language? 0=english,1=japanese,2=french,3=german
openInputFile object me, string path                        --
translateInputFile object me                                --
copyInputToOutput object me                                 --
getToken object me, int wantWhiteSpace, int wantAllComments --
getTokenType object me                                      --
openOutputFile object me, string path                       --
putToken object me, string token                            --
pushPosition object me                                      --
popPosition object me                                       --
tossPosition object me                                      --
getPosition object me                                       --
getHighWater object me                                      --
*stringsEqual string a, string b                            --
calcOffset object me, string a, string b, string c, int d, int e, int f -- finds offset within string region
calcIntersect object me, string a, int b, int c, int d, int e           -- intersects a string region
*getDate -- Returns the current date as [year, month, day]    
*setFileWritable string a                                     
*dbgPropList object                                         --
*showHelpTopic integer tTopicID                             --
 */

namespace Director {

const char *UiHelperXtra::xlibName = "UiHelper";
const XlibFileDesc UiHelperXtra::fileNames[] = {
	{ "WatcherHelper",   nullptr },	// on-disk Xtra filename (WatcherHelper.x32)
	{ "uihelper",        nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				UiHelperXtra::m_new,		 0, 0,	500 },
	{ "openInputFile",				UiHelperXtra::m_openInputFile,		 1, 1,	500 },
	{ "translateInputFile",				UiHelperXtra::m_translateInputFile,		 0, 0,	500 },
	{ "copyInputToOutput",				UiHelperXtra::m_copyInputToOutput,		 0, 0,	500 },
	{ "getToken",				UiHelperXtra::m_getToken,		 2, 2,	500 },
	{ "getTokenType",				UiHelperXtra::m_getTokenType,		 0, 0,	500 },
	{ "openOutputFile",				UiHelperXtra::m_openOutputFile,		 1, 1,	500 },
	{ "putToken",				UiHelperXtra::m_putToken,		 1, 1,	500 },
	{ "pushPosition",				UiHelperXtra::m_pushPosition,		 0, 0,	500 },
	{ "popPosition",				UiHelperXtra::m_popPosition,		 0, 0,	500 },
	{ "tossPosition",				UiHelperXtra::m_tossPosition,		 0, 0,	500 },
	{ "getPosition",				UiHelperXtra::m_getPosition,		 0, 0,	500 },
	{ "getHighWater",				UiHelperXtra::m_getHighWater,		 0, 0,	500 },
	{ "calcOffset",				UiHelperXtra::m_calcOffset,		 6, 6,	500 },
	{ "calcIntersect",				UiHelperXtra::m_calcIntersect,		 5, 5,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "setCurrentCast", UiHelperXtra::m_setCurrentCast, 1, 1, 500, HBLTIN },
	{ "activateWindow", UiHelperXtra::m_activateWindow, 1, 1, 500, HBLTIN },
	{ "activateMediaEditor", UiHelperXtra::m_activateMediaEditor, 2, 2, 500, HBLTIN },
	{ "getBehaviorMemRef", UiHelperXtra::m_getBehaviorMemRef, 2, 2, 500, HBLTIN },
	{ "getBehaviorInitializers", UiHelperXtra::m_getBehaviorInitializers, 2, 2, 500, HBLTIN },
	{ "activateScriptEditor", UiHelperXtra::m_activateScriptEditor, 4, 4, 500, HBLTIN },
	{ "sleep", UiHelperXtra::m_sleep, 1, 1, 500, HBLTIN },
	{ "getByte", UiHelperXtra::m_getByte, 2, 2, 500, HBLTIN },
	{ "executeCommand", UiHelperXtra::m_executeCommand, 1, 1, 500, HBLTIN },
	{ "isNt", UiHelperXtra::m_isNt, 0, 0, 500, HBLTIN },
	{ "getLanguage", UiHelperXtra::m_getLanguage, 0, 0, 500, HBLTIN },
	{ "stringsEqual", UiHelperXtra::m_stringsEqual, 2, 2, 500, HBLTIN },
	{ "getDate", UiHelperXtra::m_getDate, 0, 0, 500, HBLTIN },
	{ "setFileWritable", UiHelperXtra::m_setFileWritable, 1, 1, 500, HBLTIN },
	{ "dbgPropList", UiHelperXtra::m_dbgPropList, 1, 1, 500, HBLTIN },
	{ "showHelpTopic", UiHelperXtra::m_showHelpTopic, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

UiHelperXtraObject::UiHelperXtraObject(ObjectType ObjectType) :Object<UiHelperXtraObject>("UiHelper") {
	_objType = ObjectType;
}

bool UiHelperXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum UiHelperXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(UiHelperXtra::xlibName);
	warning("UiHelperXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void UiHelperXtra::open(ObjectType type, const Common::Path &path) {
    UiHelperXtraObject::initMethods(xlibMethods);
    UiHelperXtraObject *xobj = new UiHelperXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void UiHelperXtra::close(ObjectType type) {
    UiHelperXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void UiHelperXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("UiHelperXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(UiHelperXtra::m_setCurrentCast, 0)
XOBJSTUB(UiHelperXtra::m_activateWindow, 0)
XOBJSTUB(UiHelperXtra::m_activateMediaEditor, 0)
XOBJSTUB(UiHelperXtra::m_getBehaviorMemRef, 0)
XOBJSTUB(UiHelperXtra::m_getBehaviorInitializers, 0)
XOBJSTUB(UiHelperXtra::m_activateScriptEditor, 0)
XOBJSTUB(UiHelperXtra::m_sleep, 0)
XOBJSTUB(UiHelperXtra::m_getByte, 0)
XOBJSTUB(UiHelperXtra::m_executeCommand, 0)
XOBJSTUB(UiHelperXtra::m_isNt, 0)
XOBJSTUB(UiHelperXtra::m_getLanguage, 0)
XOBJSTUB(UiHelperXtra::m_openInputFile, 0)
XOBJSTUB(UiHelperXtra::m_translateInputFile, 0)
XOBJSTUB(UiHelperXtra::m_copyInputToOutput, 0)
XOBJSTUB(UiHelperXtra::m_getToken, 0)
XOBJSTUB(UiHelperXtra::m_getTokenType, 0)
XOBJSTUB(UiHelperXtra::m_openOutputFile, 0)
XOBJSTUB(UiHelperXtra::m_putToken, 0)
XOBJSTUB(UiHelperXtra::m_pushPosition, 0)
XOBJSTUB(UiHelperXtra::m_popPosition, 0)
XOBJSTUB(UiHelperXtra::m_tossPosition, 0)
XOBJSTUB(UiHelperXtra::m_getPosition, 0)
XOBJSTUB(UiHelperXtra::m_getHighWater, 0)
XOBJSTUB(UiHelperXtra::m_stringsEqual, 0)
XOBJSTUB(UiHelperXtra::m_calcOffset, 0)
XOBJSTUB(UiHelperXtra::m_calcIntersect, 0)
XOBJSTUB(UiHelperXtra::m_getDate, 0)
XOBJSTUB(UiHelperXtra::m_setFileWritable, 0)
XOBJSTUB(UiHelperXtra::m_dbgPropList, 0)
XOBJSTUB(UiHelperXtra::m_showHelpTopic, 0)

}
