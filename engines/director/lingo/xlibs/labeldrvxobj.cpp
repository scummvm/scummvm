/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

 /*************************************
  *
  * USED IN:
  * Meet Mediaband
  *
  *************************************/

/*
	-- LabelDrv XObject. Version 1.1 6/5/95 greg yachuk
	LabelDrv
	I      mNew          --Creates a new instance of the XObject
	X      mDispose      --Disposes of XObject instance.
	XSS    mSetRange     --Sets the drive letters to begin and end the search for the label. Default is C..Z.
	SS     mGetDrive     --Return the drive letter where the specified label is mounted.
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/labeldrvxobj.h"

namespace Director {

// The name is different from the obj filename.
static const char *xlibName = "LabelDrv";

static MethodProto xlibMethods[] = {
	{ "new",				LabelDrvXObj::m_new,		 0, 0,	400 },	// D4
	{ "SetRange",			LabelDrvXObj::m_setRange,	 2, 2,  400 },	// D4
	{ "GetDrive",			LabelDrvXObj::m_getDrive,	 1, 1,  400 },	// D4
	{ 0, 0, 0, 0, 0 }
};

void LabelDrvXObj::initialize(int type) {
	LabelDrvXObject::initMethods(xlibMethods);
	if (type & kXObj) {
		if (!g_lingo->_globalvars.contains(xlibName)) {
			LabelDrvXObject *xobj = new LabelDrvXObject(kXObj);
			g_lingo->_globalvars[xlibName] = xobj;
		} else {
			warning("LabelDrvXObj already initialized");
		}
	}
}

LabelDrvXObject::LabelDrvXObject(ObjectType ObjectType) :Object<LabelDrvXObject>("LabelDrvXObj") {
	_objType = ObjectType;
}

void LabelDrvXObj::m_new(int nargs) {
	LabelDrvXObject *me = static_cast<LabelDrvXObject *>(g_lingo->_currentMe.u.obj);

	Common::Rect rect;
	me->_range = "C";

	g_lingo->push(g_lingo->_currentMe);
}

void LabelDrvXObj::m_setRange(int nargs) {
	LabelDrvXObject *me = static_cast<LabelDrvXObject *>(g_lingo->_currentMe.u.obj);

	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String from = d1.asString();
	Common::String to = d2.asString();

	me->_range = from; // Store it so we could return value in the requested range
}

void LabelDrvXObj::m_getDrive(int nargs) {
	LabelDrvXObject *me = static_cast<LabelDrvXObject *>(g_lingo->_currentMe.u.obj);

	Datum d1 = g_lingo->pop();

	Common::String label = d1.asString();

	g_lingo->push(Datum(me->_range)); // Always returning first letter
}

} // End of namespace Director
