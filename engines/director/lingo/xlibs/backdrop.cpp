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
#include "director/lingo/xlibs/backdrop.h"

/**************************************************
 *
 * USED IN:
 * A Trip To The Sky
 * Die Hexenakademie
 *
 **************************************************/

/*
--  Backdrop XObject
--  Draws a backdrop behind the Director stage
--  Version 2.0.1, September 27, 1994
--  ©1993-94 Electronic Ink
I      mNew
X      mDispose
X      mShow
X      mHide
X      mPaint
V      mSetColor, index [or] r,g,b
V      mSetBgColor, index [or] r,g,b
V      mSetPattern, patNum [or] patName
XI     mSetPPat, ppatID
V      mSetPicture, castPict [or] pictID [or] pictFile
XI     mHideInBack, trueOrFalse
XI     mHideMessages, trueOrFalse
XS     mRegister, serialNumber
 */

namespace Director {

const char *const BackdropXObj::xlibName = "Backdrop";
const XlibFileDesc BackdropXObj::fileNames[] = {
	{ "Backdrop",		nullptr },
	{ "backdrop.obj",	nullptr },
	{ "Backdrop XObj",	nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				BackdropXObj::m_new,		 0, 0,	400 },
	{ "dispose",				BackdropXObj::m_dispose,		 0, 0,	400 },
	{ "show",				BackdropXObj::m_show,		 0, 0,	400 },
	{ "hide",				BackdropXObj::m_hide,		 0, 0,	400 },
	{ "paint",				BackdropXObj::m_paint,		 0, 0,	400 },
	{ "setColor",				BackdropXObj::m_setColor,		 0, 0,	400 },
	{ "setBgColor",				BackdropXObj::m_setBgColor,		 0, 0,	400 },
	{ "setPattern",				BackdropXObj::m_setPattern,		 0, 0,	400 },
	{ "setPPat",				BackdropXObj::m_setPPat,		 1, 1,	400 },
	{ "setPicture",				BackdropXObj::m_setPicture,		 0, 0,	400 },
	{ "hideInBack",				BackdropXObj::m_hideInBack,		 1, 1,	400 },
	{ "hideMessages",				BackdropXObj::m_hideMessages,		 1, 1,	400 },
	{ "register",				BackdropXObj::m_register,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BackdropXObject::BackdropXObject(ObjectType ObjectType) :Object<BackdropXObject>("Backdrop") {
	_objType = ObjectType;
}

void BackdropXObj::open(ObjectType type, const Common::Path &path) {
    BackdropXObject::initMethods(xlibMethods);
    BackdropXObject *xobj = new BackdropXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BackdropXObj::close(ObjectType type) {
    BackdropXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BackdropXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BackdropXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(BackdropXObj::m_dispose)
XOBJSTUBNR(BackdropXObj::m_show)
XOBJSTUBNR(BackdropXObj::m_hide)
XOBJSTUBNR(BackdropXObj::m_paint)
XOBJSTUB(BackdropXObj::m_setColor, 0)
XOBJSTUB(BackdropXObj::m_setBgColor, 0)
XOBJSTUB(BackdropXObj::m_setPattern, 0)
XOBJSTUBNR(BackdropXObj::m_setPPat)
XOBJSTUB(BackdropXObj::m_setPicture, 0)
XOBJSTUBNR(BackdropXObj::m_hideInBack)
XOBJSTUBNR(BackdropXObj::m_hideMessages)
XOBJSTUBNR(BackdropXObj::m_register)

}
