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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xobject/fileio.h"

namespace Director {

static struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	{ "mNew",					FileIO::m_new,				 2, 2, kFactoryObj,			2 },	// D2
	{ 0, 0, 0, 0, 0, 0 }
};

void FileIO::b_openXLib(int nargs) {
	const Common::String name = "FileIO";

	if (g_lingo->_globalvars.contains(name)) {
		warning("FileIO already initialized");
		return;
	}

	Object *obj = new Object(name, kXObj);

	for (MethodProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > g_lingo->_vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = FBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.methodType = mtd->type;
		sym.u.bltin = mtd->func;
		obj->methods[mtd->name] = sym;
	}

	g_lingo->_globalvars[name] = Symbol();
	g_lingo->_globalvars[name].name = new Common::String(name);
	g_lingo->_globalvars[name].global = true;
	g_lingo->_globalvars[name].type = OBJECT;
	g_lingo->_globalvars[name].u.obj = obj;
}

// Initialization/disposal

void FileIO::m_new(int nargs) {
	Datum res;
	res.type = OBJECT;
	res.u.obj = g_lingo->_currentMeObj;
	g_lingo->push(res);
}

} // End of namespace Director
