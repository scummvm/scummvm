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
 * Angel Gate demo
 *
 *************************************/
/*
 * -- Picter, a simple demo XObject, v1.0
 * I           mNew
 * XIII        mGpal, h, l, s
 * XIIIIII     mLine, y1, y2, y3, y4, y5, c
 * II          mGetDate, value
 * X           mClear
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/barakeobj.h"


namespace Director {

const char *const BarakeObj::xlibName = "BarakeObj";
const XlibFileDesc BarakeObj::fileNames[] = {
	{ "BarakeObj",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",			BarakeObj::m_new,		0,	0,	400 },	// D4
	{ "Clear",			BarakeObj::m_clear,		0,	0,	400 },	// D4
	{ "Gpal",			BarakeObj::m_gpal,		3,	3,	400 },	// D4
	{ "Line",			BarakeObj::m_line,		6,	6,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void BarakeObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		BarakeObject::initMethods(xlibMethods);
		BarakeObject *xobj = new BarakeObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void BarakeObj::close(ObjectType type) {
	if (type == kXObj) {
		BarakeObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


BarakeObject::BarakeObject(ObjectType ObjectType) :Object<BarakeObject>("BarakeObj") {
	_objType = ObjectType;
}

void BarakeObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void BarakeObj::m_clear(int nargs) {
}

XOBJSTUBNR(BarakeObj::m_gpal)
XOBJSTUBNR(BarakeObj::m_line)

} // End of namespace Director
