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
#include "director/lingo/xlibs/panel.h"

/**************************************************
 *
 * USED IN:
 * Standard Macromedia Director XObject
 *
 **************************************************/

/*
--Panel, Tool, 1.0
--© 1989, 1990 MacroMind, Inc.
--by John Thompson and Al McNeil
------------------------------------------------------------------
------------------------------------------------------------------
--=METHODS=--
IISIO	mNew, kindSymbol, titleString, myNameSymbol, windowObj	--Creates a new instance.
--kindSymbol is one of: #pushButton, #checkBox, #radioButton, #staticText, #castBitmap
X 	mDispose	--Disposes of the instance.
S 	mName	--Returns the XObject's name.
X	mShow	--Shows instance.
X	mHide	--Hides instance.
X	mUpdate	--Updates the instance.
XII	mMove, hpos, vpos	---Moves to h, v.
XII	mSize, width, height	--Changes size to the specified width, height.
XII	mDrag, startH, startV
XI	mSetValue, value	--Sets panel value.
I	mGetValue	--Returns panel value.
XI	mSetHilite, flag	--Sets hilite state.
I	mGetHilite	--Returns panel hilite state.
XS	mSetTitle, string	--Sets title for control.
S	mGetTitle	--Returns the control's title string.
XII	mMouseDown, loch, locv	--Sends a mousedown at given location in panel.
/X	mBounds, rectPtr	--Returns the bounding rect
 */

namespace Director {

const char *const PanelXObj::xlibName = "Panel";
const XlibFileDesc PanelXObj::fileNames[] = {
	{ "Panel",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PanelXObj::m_new,		 4, 4,	200 },
	{ "dispose",				PanelXObj::m_dispose,		 0, 0,	200 },
	{ "name",				PanelXObj::m_name,		 0, 0,	200 },
	{ "show",				PanelXObj::m_show,		 0, 0,	200 },
	{ "hide",				PanelXObj::m_hide,		 0, 0,	200 },
	{ "update",				PanelXObj::m_update,		 0, 0,	200 },
	{ "move",				PanelXObj::m_move,		 2, 2,	200 },
	{ "size",				PanelXObj::m_size,		 2, 2,	200 },
	{ "drag",				PanelXObj::m_drag,		 2, 2,	200 },
	{ "setValue",				PanelXObj::m_setValue,		 1, 1,	200 },
	{ "getValue",				PanelXObj::m_getValue,		 0, 0,	200 },
	{ "setHilite",				PanelXObj::m_setHilite,		 1, 1,	200 },
	{ "getHilite",				PanelXObj::m_getHilite,		 0, 0,	200 },
	{ "setTitle",				PanelXObj::m_setTitle,		 1, 1,	200 },
	{ "getTitle",				PanelXObj::m_getTitle,		 0, 0,	200 },
	{ "mouseDown",				PanelXObj::m_mouseDown,		 2, 2,	200 },
	{ "bounds",				PanelXObj::m_bounds,		 0, 0,	200 },
	{ nullptr, nullptr, 0, 0, 0 }
};

PanelXObject::PanelXObject(ObjectType ObjectType) :Object<PanelXObject>("Panel") {
	_objType = ObjectType;
}

void PanelXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		PanelXObject::initMethods(xlibMethods);
		PanelXObject *xobj = new PanelXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PanelXObj::close(ObjectType type) {
	if (type == kXObj) {
		PanelXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void PanelXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PanelXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(PanelXObj::m_dispose)
XOBJSTUB(PanelXObj::m_name, "")
XOBJSTUBNR(PanelXObj::m_show)
XOBJSTUBNR(PanelXObj::m_hide)
XOBJSTUBNR(PanelXObj::m_update)
XOBJSTUBNR(PanelXObj::m_move)
XOBJSTUBNR(PanelXObj::m_size)
XOBJSTUBNR(PanelXObj::m_drag)
XOBJSTUBNR(PanelXObj::m_setValue)
XOBJSTUB(PanelXObj::m_getValue, 0)
XOBJSTUBNR(PanelXObj::m_setHilite)
XOBJSTUB(PanelXObj::m_getHilite, 0)
XOBJSTUBNR(PanelXObj::m_setTitle)
XOBJSTUB(PanelXObj::m_getTitle, "")
XOBJSTUBNR(PanelXObj::m_mouseDown)
XOBJSTUBNR(PanelXObj::m_bounds)

}
