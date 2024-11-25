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

/*************************************
 *
 * USED IN:
 * Alice: An Interactive Museum
 *
 *************************************/

/*
 * Widget External Factory
 *
 * Widget
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mGetPro             --
 * I      mAskQuit            --
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/widgetxobj.h"

namespace Director {


const char *const WidgetXObj::xlibName = "Widget";
const XlibFileDesc WidgetXObj::fileNames[] = {
	{ "widget",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				WidgetXObj::m_new,			0, 0,	400 },	// D4
	{ "Dispose",			WidgetXObj::m_dispose,		0, 0,	400 },	// D4
	{ "GetPro",				WidgetXObj::m_getPro,		0, 0,	400 },	// D4
	{ "AskQuit",			WidgetXObj::m_askQuit,		0, 0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void WidgetXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		WidgetXObject::initMethods(xlibMethods);
		WidgetXObject *xobj = new WidgetXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void WidgetXObj::close(ObjectType type) {
	if (type == kXObj) {
		WidgetXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

WidgetXObject::WidgetXObject(ObjectType ObjectType) :Object<WidgetXObject>("Widget") {
	_objType = ObjectType;
}

void WidgetXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void WidgetXObj::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("WidgetXObj::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void WidgetXObj::m_getPro(int nargs) {
	// seems to want a disk drive letter
	g_lingo->push(Datum("D"));
}

XOBJSTUB(WidgetXObj::m_askQuit, 0)

} // End of namespace Director
