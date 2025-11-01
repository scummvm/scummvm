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
#include "director/lingo/xtras/d/displayres.h"

/**************************************************
 *
 * USED IN:
 * I Spy Spooky Mansion
 *
 **************************************************/

/*
-- xtra DisplayRes
-- DisplayRes Xtra, Version 1.0.0.3, 4/27/1997
-- Copyright 1997 Glenn M. Picher, Dirigo Multimedia
--
--Published by g/matter, inc.
--Sales: sales@gmatter.com, (800)933-6223
--Tech support: support@gmatter.com, (415)243-0394
--Web: http://www.gmatter.com
--
--Author: Glenn M. Picher, gpicher@maine.com, (207)767-8015
--Web: http://www.maine.com/shops/gpicher
--
-- Registered version-- thank you! See the 'register' handler.
--
new object me
-- Note: this Xtra uses only global handlers, so there's no need to
-- create any child objects. Just use the handlers listed below.
--
-- All methods return a string. Check the result for word 1 'Error:' to
-- verify proper operation.
--
* dresGetDisplaySettings
-- Provides a string of possible resolutions for the current display,
-- in the format:
--   width height colors frequency interlace grayOrColor capability
-- Reports width in pixels; height in pixels; color depth in bits per pixel: 1 (2
-- colors), 4 (16 colors),  8 (256 colors), 15 (32K colors), 16 (64K colors),
-- 24 (16 million colors), 32 (24 million colors plus 8 bits transparency);
-- vertical refresh rate in cycles per second (a value of 0 or 1 may be returned,
-- which indicates the default hardware refresh); 'interlaced' or 'noninterlaced';
-- 'grayscale' or 'color'; and the capabilites (typically 'dynamic' or 'restart,'
-- but if some error occurs while checking capabilities, could also be 'badFlags',
-- 'failed', 'badMode', 'notUpdated', or 'unknown'). Note that the quality of
-- of Windows display drivers varies wildly. Some drivers may report they can
-- change modes dynamically when they really can't. Some report no error when
-- actually asked to change modes dynamically and unable to do so (I've observed
-- this on Windows NT 3.51). Therefore, it's best to verify that the display really
-- *has* changed with dresGetCurrentDisplay(), after using dresDynamicSetDisplay().
-- NOTE: This method can return more than one line. Each possible display setting
-- is on a different line of the returned string. Based on the results, choose
-- a line number of the new mode you want to set with dresDynamicSetDisplay() (or,
-- if 'capability' indicates a restart is required, dresRestartSetDisplay).
--
* dresDynamicSetDisplay integer lineNumber
-- Change the monitor to use the display settings on the specified line
-- of the information returned by dresGetDisplaySettings. Does not affect
-- registry settings, so the default will be restored on the next restart.
-- It's best to verify proper operation with dresGetCurrentDisplay().
--
* dresRestartSetDisplay integer lineNumber
-- Change the monitor to use the display settings on the specified line
-- of the information returned by dresGetDisplaySettings (updating the
-- default registry settings for the next time the computer restarts).
-- Note that this method *will* change dynamically if it's possible, as
-- well as updating the default registry settings. Use dresGetCurrentDisplay()
-- to verify whether a dynamic change was made.
* dresRestart
-- Restart the machine. Lingo's 'restart' doesn't work on Win95/NT.
* dresShutdown
-- Shut down the machine. Lingo's 'shutdown' doesn't work on Win95/NT.
--
* dresDefaultDisplay
-- Sets the display to the default values stored in the registry. Useful to
-- restore the screen after a dynamic change with dresDynamicSetDisplay.
-- Note that dresRestartSetDisplay() will *change* the defaults, so this method
-- can't be used to restore an old setting after a restart.
* dresGetCurrentDisplay
-- Returns a line of information about the current display settings (in the same
-- format as dresGetDisplaySettings). This is useful to save the existing settings
-- when you change the display, so you can know what to restore it to later.
-- This also returns up-to-date information after a display change, which Lingo's
-- 'the desktopRectList' and 'the colorDepth' will not (in Director 5 at least).
-- Note that the values for 'interlace' and 'grayOrColor' are not meaningful; the value
-- for 'capability' is always 'dynamic' (because by definition the sytem is already
-- using this display mode); and the value for 'frequency' is only meaningful on
-- Windows NT. This is due to the nature of the underlying Windows API functions.
--
+ register object xtraReference, string registrationString
--

 */

namespace Director {

const char *DisplayResXtra::xlibName = "DisplayRes";
const XlibFileDesc DisplayResXtra::fileNames[] = {
	{ "displayres",   nullptr },
	{ "DispRes",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DisplayResXtra::m_new,		 0, 0,	500 },
	{ "register",				DisplayResXtra::m_register,		 1, 1,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "dresGetDisplaySettings", DisplayResXtra::m_dresGetDisplaySettings, 0, 0, 500, HBLTIN },
	{ "dresDynamicSetDisplay", DisplayResXtra::m_dresDynamicSetDisplay, 1, 1, 500, HBLTIN },
	{ "dresRestartSetDisplay", DisplayResXtra::m_dresRestartSetDisplay, 1, 1, 500, HBLTIN },
	{ "dresRestart", DisplayResXtra::m_dresRestart, 0, 0, 500, HBLTIN },
	{ "dresShutdown", DisplayResXtra::m_dresShutdown, 0, 0, 500, HBLTIN },
	{ "dresDefaultDisplay", DisplayResXtra::m_dresDefaultDisplay, 0, 0, 500, HBLTIN },
	{ "dresGetCurrentDisplay", DisplayResXtra::m_dresGetCurrentDisplay, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DisplayResXtraObject::DisplayResXtraObject(ObjectType ObjectType) :Object<DisplayResXtraObject>("DisplayRes") {
	_objType = ObjectType;
}

bool DisplayResXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DisplayResXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DisplayResXtra::xlibName);
	warning("DisplayResXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DisplayResXtra::open(ObjectType type, const Common::Path &path) {
    DisplayResXtraObject::initMethods(xlibMethods);
    DisplayResXtraObject *xobj = new DisplayResXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DisplayResXtra::close(ObjectType type) {
    DisplayResXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DisplayResXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DisplayResXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void DisplayResXtra::m_dresGetDisplaySettings(int nargs) {
	Common::String result = Common::String::format("%d %d %d %d noninterlaced color dynamic", g_director->_wm->getWidth(), g_director->_wm->getHeight(), g_director->_wm->_pixelformat.bytesPerPixel*8, 0);
	g_lingo->push(Datum(result));
}

XOBJSTUB(DisplayResXtra::m_dresDynamicSetDisplay, 0)
XOBJSTUB(DisplayResXtra::m_dresRestartSetDisplay, 0)
XOBJSTUB(DisplayResXtra::m_dresRestart, 0)
XOBJSTUB(DisplayResXtra::m_dresShutdown, 0)
XOBJSTUB(DisplayResXtra::m_dresDefaultDisplay, 0)

void DisplayResXtra::m_dresGetCurrentDisplay(int nargs) {
	Common::String result = Common::String::format("%d %d %d %d noninterlaced color dynamic", g_director->_wm->getWidth(), g_director->_wm->getHeight(), g_director->_wm->_pixelformat.bytesPerPixel*8, 0);
	g_lingo->push(Datum(result));
}


void DisplayResXtra::m_register(int nargs) {
	Datum key = g_lingo->pop();
	debugC(5, kDebugXObj, "DisplayResXtra::m_register: %s", key.asString().c_str());
	g_lingo->push(Datum("OK"));
}

}
