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

namespace Director {

static struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int version;
} predefinedMethods[] = {
    // "mAtFrame"                                                   // D2 - XObject or user-defined
    // "mDescribe",			                                        // D2 - XObject
	{ "mDispose",               LM::m_dispose,       0, 0, 2 },	    // D2
	{ "mGet",			        LM::m_get,           1, 1, 2 },	    // D2
	// "mInstanceRespondsTo",                                       // D2 - XObject
	// "mMessageList",			                                    // D2 - XObject
	// "mName",				                                        // D2 - XObject
	{ "mNew",				    LM::m_new,           -1, 0, 2 },    // D2
	// "mPerform",				                                    // D2 - XObject
	{ "mPut",				    LM::m_put,           2, 2, 2 },	    // D2
	// "mRespondsTo",		                                        // D2 - XObject
	{ 0, 0, 0, 0, 0 }
};

void Lingo::initMethods() {
	for (MethodProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = FBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.parens = false;
		sym.u.bltin = mtd->func;
		_methods[mtd->name] = sym;

		// FIXME: Method names can conflict with vars. This won't work all the time.
		Common::String name(mtd->name);
		Datum target(name);
		target.type = VAR;
		Datum source(name);
		source.type = SYMBOL;
		g_lingo->varCreate(name, true);
		g_lingo->varAssign(target, source, true);
	}
}

Symbol Object::getMethod(Common::String &methodName, bool ignorePredefined) {
	if (!ignorePredefined && g_lingo->_methods.contains(methodName)) {
		return g_lingo->_methods[methodName];
	}
	if (methods.contains(methodName)) {
		return methods[methodName];
	}
	// TODO: error handling
	return Symbol();
}

void LM::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("m_dispose", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->pushVoid();
}

void LM::m_get(int nargs) {
	g_lingo->printSTUBWithArglist("m_get", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->pushVoid();
}

void LM::m_put(int nargs) {
	g_lingo->printSTUBWithArglist("m_put", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->pushVoid();
}

void LM::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->pushVoid();
}

}
