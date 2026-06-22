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
#include "director/lingo/xtras/a/atixglue.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra ATIXGlue
-- ATIXGlue Xtra, for use with Macromedia Director 5 and later.
-- Demo version 1.1.0.3, May 11th, 1999-- not licensed for distribution!
-- 
-- Copyright © 1997-1999, Glenn M. Picher
-- Dirigo Multimedia
-- 142 High Street, Suite 321
-- Portland, ME USA 04101
-- Web: http://www.maine.com/shops/gpicher
-- Email: gpicher@maine.com
-- Voice: 207-761-6535
-- Fax: 207-775-4372
-- 
new object me
-- Note: this Xtra uses only global handlers. No need to create a new object!
-- Also note: all handlers return a string. Check for word 1 'Error:' or 'OK'!
-- Also note: use either the ATIXGlue or ATIXtrol Xtra-- but not both at the
-- same time! In other words, finish with one before using the other.
* atigSetup
-- Sets up the Xtra. Use before any other method. Equivalent to the previous
-- XObject version's mNew method.
* atigCleanup
-- Cleans up the Xtra. Use after all other methods, and before using the
-- ATIXtrol Xtra's features. Equivalent to the former XObject's mDispose.
* atigPreviewOverlay integer l, integer t, integer r, integer b
--  Establishes Video-for-Windows preview overlay window. The window will
--  use left-to-right mirroring, for pose movement to match camera movements,
--  if the video source dialog checkbox is set to mirror. Use pure magenta
--  pixels within the supplied rectangle (which is relative to the top left
--  of the stage) to control which areas show video overlay.
* atigShowVideoSourceDialog
--  Activates Video Source dialog, if the current driver has one.
* atigEndPreview
--  Ends the previously activated preview window.
* atigLoadDriver
--  Loads ATI direct video settings driver (required for the methods below).
* atigSetBrightness integer percent
* atigSetContrast integer percent
* atigSetColor integer percent
--  These three all set video settings in the range 0 to 200 percent of normal.
* atigSetTint integer degrees
--  Sets video settings in the range -180 to 180 degrees from normal.
* atigSetSource string sourceString
-- Sets input source to 'tuner', 'composite', or 'svideo'.
* atigUnloadDriver
--  Unloads ATI direct video setting driver. Take care not to have the ATIXGlue
--  Xtra and the ATIXtrl Xtra both with the driver loaded at the same time!
"
 */

namespace Director {

const char *AtixglueXtra::xlibName = "Atixglue";
const XlibFileDesc AtixglueXtra::fileNames[] = {
	{ "atixglue",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AtixglueXtra::m_new,		 0, 0,	500 },
	{ """,				AtixglueXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "atigSetup", AtixglueXtra::m_atigSetup, 0, 0, 500, HBLTIN },
	{ "atigCleanup", AtixglueXtra::m_atigCleanup, 0, 0, 500, HBLTIN },
	{ "atigPreviewOverlay", AtixglueXtra::m_atigPreviewOverlay, 4, 4, 500, HBLTIN },
	{ "atigShowVideoSourceDialog", AtixglueXtra::m_atigShowVideoSourceDialog, 0, 0, 500, HBLTIN },
	{ "atigEndPreview", AtixglueXtra::m_atigEndPreview, 0, 0, 500, HBLTIN },
	{ "atigLoadDriver", AtixglueXtra::m_atigLoadDriver, 0, 0, 500, HBLTIN },
	{ "atigSetBrightness", AtixglueXtra::m_atigSetBrightness, 1, 1, 500, HBLTIN },
	{ "atigSetContrast", AtixglueXtra::m_atigSetContrast, 1, 1, 500, HBLTIN },
	{ "atigSetColor", AtixglueXtra::m_atigSetColor, 1, 1, 500, HBLTIN },
	{ "atigSetTint", AtixglueXtra::m_atigSetTint, 1, 1, 500, HBLTIN },
	{ "atigSetSource", AtixglueXtra::m_atigSetSource, 1, 1, 500, HBLTIN },
	{ "atigUnloadDriver", AtixglueXtra::m_atigUnloadDriver, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AtixglueXtraObject::AtixglueXtraObject(ObjectType ObjectType) :Object<AtixglueXtraObject>("Atixglue") {
	_objType = ObjectType;
}

bool AtixglueXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AtixglueXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AtixglueXtra::xlibName);
	warning("AtixglueXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AtixglueXtra::open(ObjectType type, const Common::Path &path) {
    AtixglueXtraObject::initMethods(xlibMethods);
    AtixglueXtraObject *xobj = new AtixglueXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AtixglueXtra::close(ObjectType type) {
    AtixglueXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AtixglueXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AtixglueXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AtixglueXtra::m_atigSetup, 0)
XOBJSTUB(AtixglueXtra::m_atigCleanup, 0)
XOBJSTUB(AtixglueXtra::m_atigPreviewOverlay, 0)
XOBJSTUB(AtixglueXtra::m_atigShowVideoSourceDialog, 0)
XOBJSTUB(AtixglueXtra::m_atigEndPreview, 0)
XOBJSTUB(AtixglueXtra::m_atigLoadDriver, 0)
XOBJSTUB(AtixglueXtra::m_atigSetBrightness, 0)
XOBJSTUB(AtixglueXtra::m_atigSetContrast, 0)
XOBJSTUB(AtixglueXtra::m_atigSetColor, 0)
XOBJSTUB(AtixglueXtra::m_atigSetTint, 0)
XOBJSTUB(AtixglueXtra::m_atigSetSource, 0)
XOBJSTUB(AtixglueXtra::m_atigUnloadDriver, 0)
XOBJSTUB(AtixglueXtra::m_", 0)

}
