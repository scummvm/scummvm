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
#include "director/lingo/xlibs/fileio.h"

namespace Director {

static struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	{ "describe",				LM::m_describe,				 0, 0,	kXObj,					2 },	// D2
	{ "dispose",				LM::m_dispose,				 0, 0,	kFactoryObj | kXObj,	2 },	// D2
	{ "get",					LM::m_get,					 1, 1,	kFactoryObj,			2 },	// D2
	{ "instanceRespondsTo",		LM::m_instanceRespondsTo,	 1, 1,	kXObj,					3 },		// D3
	{ "messageList",			LM::m_messageList,			 0, 0,	kXObj,					3 },		// D3
	{ "name",					LM::m_name,					 0, 0,	kXObj,					3 },		// D3
	{ "new",					LM::m_new,					-1, 0,	kFactoryObj | kXObj, 	2 },	// D2
	{ "perform",				LM::m_perform,				-1, 0,	kFactoryObj | kXObj, 	3 },		// D3
	{ "put",					LM::m_put,					 2, 2,	kFactoryObj,			2 },	// D2
	{ "respondsTo",				LM::m_respondsTo,			 1, 1,	kXObj,					2 },	// D2
	{ 0, 0, 0, 0, 0, 0 }
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
		sym.targetType = mtd->type;
		sym.u.bltin = mtd->func;
		_methods[mtd->name] = sym;
	}
}

static struct XLibProto {
	const char *name;
	void (*initializer)(int);
	int type;
	int version;
} xlibs[] = {
	{ "FileIO",					FileIO::initialize,					kXObj | kFactoryObj,	2 },	// D2
	{ 0, 0, 0, 0 }
};

void Lingo::initXLibs() {
	for (XLibProto *lib = xlibs; lib->name; lib++) {
		if (lib->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(lib->name);
		sym.type = FBLTIN;
		sym.nargs = 0;
		sym.maxArgs = 0;
		sym.targetType = lib->type;
		sym.u.bltin = lib->initializer;
		_xlibInitializers[lib->name] = sym;
	}

	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		// TODO: Mac executables can contain XObjects in XCOD resources.
		// If a Mac executable exists, check which XObjects should be loaded.
		openXLib("FileIO", kXObj);
	}
}

void Lingo::openXLib(const Common::String &name, ObjectType type) {
	if (_xlibInitializers.contains(name)) {
		Symbol sym = _xlibInitializers[name];
		(*sym.u.bltin)(type);
	} else {
		warning("Unimplemented xlib: '%s'", name.c_str());
	}
}

Object *Object::clone() {
	Object *res = new Object(*name, type);
	res->disposed = disposed;
	res->prototype = this;
	res->properties = properties;
	res->methods = methods;
	res->inheritanceLevel = inheritanceLevel + 1;
	res->scriptContext = scriptContext;
	if (objArray) {
		res->objArray = new Common::HashMap<uint32, Datum>(*objArray);
	}
	return res;
}

Symbol Object::getMethod(const Common::String &methodName) {
	if (disposed) {
		error("Method '%s' called on disposed object <%s>", methodName.c_str(), Datum(this).asString(true).c_str());
	}

	// instance method (factory, script object, and Xtra)
	if ((type | (kFactoryObj & kScriptObj & kXtraObj)) && methods.contains(methodName)) {
		return methods[methodName];
	}

	if ((type & (kFactoryObj | kXObj)) && methodName.hasPrefixIgnoreCase("m")) {
		Common::String shortName = methodName.substr(1);
		// instance method (XObject)
		if (type == kXObj && methods.contains(shortName) && inheritanceLevel > 1) {
			return methods[shortName];
		}
		// predefined method (factory and XObject)
		if (g_lingo->_methods.contains(shortName) && (type & g_lingo->_methods[shortName].type)) {
			return g_lingo->_methods[shortName];
		}
	} else if (type & (kScriptObj | kXtraObj)) {
		// predefined method (script object and Xtra)
		if (g_lingo->_methods.contains(methodName) && (type & g_lingo->_methods[methodName].type)) {
			return g_lingo->_methods[methodName];
		}
	}

	return Symbol();
}

bool Object::hasVar(const Common::String &varName) {
	if (disposed) {
		error("Variable '%s' accessed on disposed object <%s>", varName.c_str(), Datum(this).asString(true).c_str());
	}
	// Factory object instance vars are accessed like normal vars
	// Script object properties cannot be accessed like normal vars until D5
	if (type == kScriptObj && g_lingo->_vm->getVersion() < 5) {
		return false;
	}
	return properties.contains(varName);
}

Symbol &Object::getVar(const Common::String &varName) {
	return properties[varName];
}

// Initialization/disposal

void LM::m_new(int nargs) {
	// This is usually overridden by a user-defined mNew
	g_lingo->printSTUBWithArglist("m_new", nargs);
	g_lingo->push(g_lingo->_currentMe);
}

void LM::m_dispose(int nargs) {
	g_lingo->_currentMe.u.obj->disposed = true;
}

// Object array

void LM::m_get(int nargs) {
	Object *me = g_lingo->_currentMe.u.obj;
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	if (me->objArray->contains(index)) {
		g_lingo->push((*me->objArray)[index]);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_put(int nargs) {
	Object *me = g_lingo->_currentMe.u.obj;
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	(*me->objArray)[index] = value;
}

// Other

void LM::m_perform(int nargs) {
	// Lingo doesn't seem to bother cloning the object when
	// mNew is called with mPerform
	Object *me = g_lingo->_currentMe.u.obj;
	Datum methodName = g_lingo->_stack.remove_at(g_lingo->_stack.size() - nargs); // Take method name out of stack
	nargs -= 1;
	Symbol funcSym = me->getMethod(*methodName.u.s);
	LC::call(funcSym, nargs, g_lingo->_currentMe);
}

// XObject

void LM::m_describe(int nargs) {
	warning("STUB: m_describe");
}

void LM::m_instanceRespondsTo(int nargs) {
	Object *me = g_lingo->_currentMe.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	if (me->methods.contains(methodName)) {
		g_lingo->push(Datum(1));
		return;
	}
	if (g_lingo->_methods.contains(methodName) && (me->type & g_lingo->_methods[methodName].type)) {
		g_lingo->push(Datum(1));
		return;
	}
	g_lingo->push(Datum(0));
}

void LM::m_messageList(int nargs) {
	warning("STUB: m_messageList");
	g_lingo->push(Datum(""));
}

void LM::m_name(int nargs) {
	Object *me = g_lingo->_currentMe.u.obj;
	g_lingo->push(Datum(*me->name));
}

void LM::m_respondsTo(int nargs) {
	Object *me = g_lingo->_currentMe.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	if (me->methods.contains(methodName) && me->inheritanceLevel > 1) {
		g_lingo->push(Datum(1));
		return;
	}
	if (g_lingo->_methods.contains(methodName) && (me->type & g_lingo->_methods[methodName].type)) {
		g_lingo->push(Datum(1));
		return;
	}
	g_lingo->push(Datum(0));
}

} // End of namespace Director
