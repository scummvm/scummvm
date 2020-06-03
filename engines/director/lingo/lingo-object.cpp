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
#include "director/lingo/lingo-code.h"
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

Object *Object::clone() {
	Object *res = new Object;
	res->name = name;
	res->type = type;
	res->disposed = disposed;
	res->prototype = this;
	res->properties = properties;
	res->methods = methods;
	res->inheritanceLevel = inheritanceLevel + 1;
	res->scriptContext = scriptContext;
	if (objArray) {
		res->objArray = new Common::HashMap<uint, Datum>(*objArray);
	}
	return res;
}

Symbol Object::getMethod(const Common::String methodName) {
	if (disposed) {
		error("Method '%s' called on disposed object '%s'", methodName.c_str(), name->c_str());
	}
	if (methods.contains(methodName)) {
		return methods[methodName];
	}
	if (g_lingo->_methods.contains(methodName)) {
		return g_lingo->_methods[methodName];
	}
	return Symbol();
}

bool Object::hasVar(const Common::String varName) {
	if (disposed) {
		error("Variable '%s' accessed on disposed object '%s'", varName.c_str(), name->c_str());
	}
	// Factory object instance vars are accessed like normal vars
	// Script object properties cannot be accessed like normal vars until D5
	if (type == kScriptObj && g_lingo->_vm->getVersion() < 5) {
		return false;
	}
	return properties.contains(varName);
}

Symbol &Object::getVar(const Common::String varName) {
	return properties[varName];
}

void LM::m_dispose(int nargs) {
	g_lingo->_currentMeObj->disposed = true;
}

void LM::m_get(int nargs) {
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	if (g_lingo->_currentMeObj->objArray->contains(index)) {
		g_lingo->push((*g_lingo->_currentMeObj->objArray)[index]);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_put(int nargs) {
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	(*g_lingo->_currentMeObj->objArray)[index] = value;
}

void LM::m_new(int nargs) {
	// This is usually be overridden by a user-defined mNew
	Datum res;
	res.type = OBJECT;
	res.u.obj = g_lingo->_currentMeObj;
	g_lingo->push(res);
}

}
