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
#include "director/lingo/xlibs/window.h"

/**************************************************
 *
 * USED IN:
 * Standard Macromedia Director XObject
 *
 **************************************************/

/*
--Window, Tool, 1.0
--© 1989, 1990 MacroMind, Inc.
--by John Thompson
-----------------------------------------------------------------------
--NOTE: This XObject will not create a window with scroll bars.--
-----------------------------------------------------------------------
-----------------------------------------------------------------------
--=METHODS=--
--
IIS 	mNew, kind, titleString	--Creates a new instance of the XObject.
--kind: #document #noGrowDoc, #dialog, #roundDialog, #plainDialog, #altDialog
X 	mDispose	--Disposes of the instance.
S 	mName	--Returns name of the XObject.
X	mSelect	--Brings to the front
X	mShow	--Shows instance.
X	mHide	--Hides instance.
XII	mMove, hpos, vpos	--Moves to hpos, vpos.
XII	mSize, width, height	--Changes size to width, height.
XIO	mAddPanel, symbol, panelObj	--Adds a panel.
OI	mGetPanel, panelSymbolName	--Gets a panel.
XO	mSetHandler, handlerObject	--Sets the event handler. Args:
O	mGetHandler	--Return the event handler
/X	mIdle
/XI	mActivate, flag
/X	mUpdate
/XII	mMouseUp, eventRecPtr, mdcode
/XII	mMouseDown, eventRecPtr, mdcode
/XI	mKeyDown, theChar
/X	mWindowPtr
/XX	mSetTitle
X	+mDisposeAll	--Closes all windows.
OI	+mNthWindow, n	--Returns the n'th window.
 */

namespace Director {

const char *const WindowXObj::xlibName = "Window";
const XlibFileDesc WindowXObj::fileNames[] = {
	{ "Window",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				WindowXObj::m_new,		 2, 2,	200 },
	{ "dispose",			WindowXObj::m_dispose,	 0, 0,	200 },
	{ "name",				WindowXObj::m_name,		 0, 0,	200 },
	{ "select",				WindowXObj::m_select,	 0, 0,	200 },
	{ "show",				WindowXObj::m_show,		 0, 0,	200 },
	{ "hide",				WindowXObj::m_hide,		 0, 0,	200 },
	{ "move",				WindowXObj::m_move,		 2, 2,	200 },
	{ "size",				WindowXObj::m_size,		 2, 2,	200 },
	{ "addPanel",			WindowXObj::m_addPanel,	 2, 2,	200 },
	{ "getPanel",			WindowXObj::m_getPanel,	 1, 1,	200 },
	{ "setHandler",			WindowXObj::m_setHandler, 1, 1,	200 },
	{ "getHandler",			WindowXObj::m_getHandler, 0, 0,	200 },
	{ "idle",				WindowXObj::m_idle,		 0, 0,	200 },
	{ "activate",			WindowXObj::m_activate,	 1, 1,	200 },
	{ "update",				WindowXObj::m_update,	 0, 0,	200 },
	{ "mouseUp",			WindowXObj::m_mouseUp,	 2, 2,	200 },
	{ "mouseDown",			WindowXObj::m_mouseDown, 2, 2,	200 },
	{ "keyDown",			WindowXObj::m_keyDown,	 1, 1,	200 },
	{ "windowPtr",			WindowXObj::m_windowPtr, 0, 0,	200 },
	{ "setTitle",			WindowXObj::m_setTitle,	 1, 1,	200 },
	{ "disposeAll",			WindowXObj::m_disposeAll, 0, 0,	200 },
	{ "nthWindow",			WindowXObj::m_nthWindow, 1, 1,	200 },
	{ nullptr, nullptr, 0, 0, 0 }
};

WindowXObject::WindowXObject(ObjectType ObjectType) :Object<WindowXObject>("Window") {
	_objType = ObjectType;
}

void WindowXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		WindowXObject::initMethods(xlibMethods);
		WindowXObject *xobj = new WindowXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void WindowXObj::close(ObjectType type) {
	if (type == kXObj) {
		WindowXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void WindowXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("WindowXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(WindowXObj::m_dispose)
XOBJSTUB(WindowXObj::m_name, "")
XOBJSTUBNR(WindowXObj::m_select)
XOBJSTUBNR(WindowXObj::m_show)
XOBJSTUBNR(WindowXObj::m_hide)
XOBJSTUBNR(WindowXObj::m_move)
XOBJSTUBNR(WindowXObj::m_size)
XOBJSTUBNR(WindowXObj::m_addPanel)
XOBJSTUB(WindowXObj::m_getPanel, 0)
XOBJSTUBNR(WindowXObj::m_setHandler)
XOBJSTUB(WindowXObj::m_getHandler, 0)
XOBJSTUBNR(WindowXObj::m_idle)
XOBJSTUBNR(WindowXObj::m_activate)
XOBJSTUBNR(WindowXObj::m_update)
XOBJSTUBNR(WindowXObj::m_mouseUp)
XOBJSTUBNR(WindowXObj::m_mouseDown)
XOBJSTUBNR(WindowXObj::m_keyDown)
XOBJSTUBNR(WindowXObj::m_windowPtr)
XOBJSTUBNR(WindowXObj::m_setTitle)
XOBJSTUBNR(WindowXObj::m_disposeAll)
XOBJSTUB(WindowXObj::m_nthWindow, 0)

}
