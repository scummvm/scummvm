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
 * Night Light (Mac)
 *
 *************************************/
/*
-- HitMap XObject
ISIII		mNew, path, xOffset, yOffset, scale	-- path, xOffset, yOffset, scale
X			mDispose
III		mWhere, x, y						-- Return GWorld Index

-- Hitmap Scale Factor 27.JUL.94
hitmap
S		mName         										--Returns the XObject name (hitmap)
S		mGetSys         									--Returns Windows System Directory Path(hitmap)
ISIII	mNew, hitBitMapfile, hitmaposx, hitmaposy, factor   --Creates new instance of XObject
III	mWhere, xpos, ypos									--coordinate position
X		mDispose												--Dispose of memory allocation
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/hitmap.h"


namespace Director {

const char *const HitMap::xlibName = "HitMap";
const XlibFileDesc HitMap::fileNames[] = {
	{ "HitMap",		nullptr },
	{ "maskXobj",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",	HitMap::m_new,		4,	4,	400 },	// D4
	{ "Where",	HitMap::m_where,	2,	2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void HitMap::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		HitMapObject::initMethods(xlibMethods);
		HitMapObject *xobj = new HitMapObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void HitMap::close(ObjectType type) {
	if (type == kXObj) {
		HitMapObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


HitMapObject::HitMapObject(ObjectType ObjectType) :Object<HitMapObject>("HitMap") {
	_objType = ObjectType;
}

void HitMap::m_new(int nargs) {
	// Common::String hitBitMapfile = g_lingo->pop().asString();
	// int hitmaposx = g_lingo->pop().asInt();
	// int hitmaposy = g_lingo->pop().asInt();
	// int factor = g_lingo->pop().asInt();
	g_lingo->printSTUBWithArglist("HitMap::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void HitMap::m_where(int nargs) {
	// int xpos = g_lingo->pop().asInt();
	// int ypos = g_lingo->pop().asInt();
	g_lingo->printSTUBWithArglist("HitMap::m_where", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

} // End of namespace Director
