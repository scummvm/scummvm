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

/* Memory is a Mac only XObject.
 *
 * Implemented as a no-op, since ScummVM doesn't need to handle memory clears.
 *
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/memoryxobj.h"


namespace Director {

const char *MemoryXObj::xlibName = "Memory";
const char *MemoryXObj::fileNames[] = {
	"Memory XObj",
	0
};

static MethodProto xlibMethods[] = {
	{ "new",					MemoryXObj::m_new,			0,	0,	400 },	// D4
	{ "Clear",					MemoryXObj::m_clear,		0,	0,	400 },	// D4
	{ 0, 0, 0, 0, 0 }
};

void MemoryXObj::open(int type) {
	if (type == kXObj) {
		MemoryXObject::initMethods(xlibMethods);
		MemoryXObject *xobj = new MemoryXObject(kXObj);
		g_lingo->_globalvars[xlibName] = xobj;
	}
}

void MemoryXObj::close(int type) {
	if (type == kXObj) {
		MemoryXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


MemoryXObject::MemoryXObject(ObjectType ObjectType) :Object<MemoryXObject>("MemoryXObj") {
	_objType = ObjectType;
}

void MemoryXObj::m_new(int nargs) {
	// Datum d1 = g_lingo->pop();
	g_lingo->push(g_lingo->_currentMe);
}

void MemoryXObj::m_clear(int nargs) {
	// g_lingo->pop();
}

} // End of namespace Director
