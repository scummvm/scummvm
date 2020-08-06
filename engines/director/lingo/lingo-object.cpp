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

#include "common/endian.h"

#include "director/director.h"
#include "director/window.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/xlibs/fileio.h"
#include "director/lingo/xlibs/palxobj.h"
#include "director/lingo/xlibs/flushxobj.h"
#include "director/lingo/xlibs/winxobj.h"

namespace Director {

static struct PredefinedProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	// all except window
	{ "new",					LM::m_new,					-1, 0,	kAllObj, 				2 },	// D2

	// factory and XObject
	{ "describe",				LM::m_describe,				 0, 0,	kXObj,					2 },	// D2
	{ "dispose",				LM::m_dispose,				 0, 0,	kFactoryObj | kXObj,	2 },	// D2
	{ "get",					LM::m_get,					 1, 1,	kFactoryObj,			2 },	// D2
	{ "instanceRespondsTo",		LM::m_instanceRespondsTo,	 1, 1,	kXObj,					3 },		// D3
	{ "messageList",			LM::m_messageList,			 0, 0,	kXObj,					3 },		// D3
	{ "name",					LM::m_name,					 0, 0,	kXObj,					3 },		// D3
	{ "perform",				LM::m_perform,				-1, 0,	kFactoryObj | kXObj, 	3 },		// D3
	{ "put",					LM::m_put,					 2, 2,	kFactoryObj,			2 },	// D2
	{ "respondsTo",				LM::m_respondsTo,			 1, 1,	kXObj,					2 },	// D2

	// script object and Xtra
	{ "birth",					LM::m_new,					-1, 0,	kScriptObj | kXtraObj, 	4 },			// D4

	{ 0, 0, 0, 0, 0, 0 }
};

static MethodProto windowMethods[] = {
	// window / stage
	{ "close",					LM::m_close,				 0, 0,	4 },			// D4
	{ "forget",					LM::m_forget,				 0, 0,	4 },			// D4
	{ "open",					LM::m_open,					 0, 0,	4 },			// D4
	{ "moveToBack",				LM::m_moveToBack,			 0, 0,	4 },			// D4
	{ "moveToFront",			LM::m_moveToFront,			 0, 0,	4 },			// D4
	{ 0, 0, 0, 0, 0 }
};

void Lingo::initMethods() {
	for (PredefinedProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = HBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.targetType = mtd->type;
		sym.u.bltin = mtd->func;
		_methods[mtd->name] = sym;
	}
	Window::initMethods(windowMethods);
}

static struct XLibProto {
	const char *name;
	void (*initializer)(int);
	int type;
	int version;
} xlibs[] = {
	{ "FileIO",					FileIO::initialize,					kXObj | kFactoryObj,	2 },	// D2
	{ "FlushXObj",				FlushXObj::initialize,				kXObj,					4 },	// D4
	{ "PalXObj",				PalXObj:: initialize,				kXObj,					4 }, 	// D4
	{ "winXObj",				RearWindowXObj::initialize,			kXObj,					4 },	// D4
	{ 0, 0, 0, 0 }

};

void Lingo::initXLibs() {
	for (XLibProto *lib = xlibs; lib->name; lib++) {
		if (lib->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(lib->name);
		sym.type = HBLTIN;
		sym.nargs = 0;
		sym.maxArgs = 0;
		sym.targetType = lib->type;
		sym.u.bltin = lib->initializer;
		_xlibInitializers[lib->name] = sym;
	}
}

void Lingo::openXLib(Common::String name, ObjectType type) {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		int pos = name.findLastOf(':');
		name = name.substr(pos + 1, name.size());
	}

	if (_xlibInitializers.contains(name)) {
		Symbol sym = _xlibInitializers[name];
		(*sym.u.bltin)(type);
	} else {
		warning("Unimplemented xlib: '%s'", name.c_str());
	}
}

// Initialization/disposal

void LM::m_new(int nargs) {
	// This is usually overridden by a user-defined mNew
	g_lingo->printSTUBWithArglist("m_new", nargs);
	g_lingo->push(g_lingo->_currentMe);
}

void LM::m_dispose(int nargs) {
	g_lingo->_currentMe.u.obj->dispose();
}

/* ScriptContext */

ScriptContext::ScriptContext(Common::String name, LingoArchive *archive, ScriptType type, int id)
	: Object<ScriptContext>(name), _archive(archive), _scriptType(type), _id(id) {
	_objType = kScriptObj;
}

ScriptContext::ScriptContext(const ScriptContext &sc) : Object<ScriptContext>(sc) {
	_scriptType = sc._scriptType;
	_functionNames = sc._functionNames;
	for (SymbolHash::iterator it = sc._functionHandlers.begin(); it != sc._functionHandlers.end(); ++it) {
		_functionHandlers[it->_key] = it->_value;
		_functionHandlers[it->_key].ctx = this;
	}
	for (Common::HashMap<uint32, Symbol>::iterator it = sc._eventHandlers.begin(); it != sc._eventHandlers.end(); ++it) {
		_eventHandlers[it->_key] = it->_value;
		_eventHandlers[it->_key].ctx = this;
	}
	_constants = sc._constants;
	_properties = sc._properties;

	_archive = sc._archive;
	_id = sc._id;
}

ScriptContext::~ScriptContext() {}

Common::String ScriptContext::asString() {
	return Common::String::format("script: #%s %d %p", _name.c_str(), _inheritanceLevel, (void *)this);
}

Symbol ScriptContext::getMethod(const Common::String &methodName) {
	Symbol sym;

	if (_functionHandlers.contains(methodName)) {
		sym = _functionHandlers[methodName];
		sym.target = this;
		return sym;
	}

	sym = Object<ScriptContext>::getMethod(methodName);
	if (sym.type != VOIDSYM)
		return sym;

	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			// ancestor method
			debugC(3, kDebugLingoExec, "Calling method '%s' on ancestor: <%s>", methodName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->getMethod(methodName);
		}
	}

	return sym;
}

bool ScriptContext::hasProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return true;
	}
	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			return _properties["ancestor"].u.obj->hasProp(propName);
		}
	}
	return false;
}

Datum ScriptContext::getProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return _properties[propName];
	}
	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->getProp(propName);
		}
	}
	return _properties[propName]; // return new property
}

bool ScriptContext::setProp(const Common::String &propName, const Datum &value) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		_properties[propName] = value;
		return true;
	}
	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->setProp(propName, value);
		}
	}
	return false;
}

// Object array

void LM::m_get(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_currentMe.u.obj);
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	if (me->_objArray.contains(index)) {
		g_lingo->push(me->_objArray[index]);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_put(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_currentMe.u.obj);
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	me->_objArray[index] = value;
}

// Other

void LM::m_perform(int nargs) {
	// Lingo doesn't seem to bother cloning the object when
	// mNew is called with mPerform
	AbstractObject *me = g_lingo->_currentMe.u.obj;
	Datum methodName = g_lingo->_stack.remove_at(g_lingo->_stack.size() - nargs); // Take method name out of stack
	nargs -= 1;
	Symbol funcSym = me->getMethod(*methodName.u.s);
	LC::call(funcSym, nargs, true);
}

// XObject

void LM::m_describe(int nargs) {
	warning("STUB: m_describe");
}

void LM::m_instanceRespondsTo(int nargs) {
	AbstractObject *me = g_lingo->_currentMe.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_messageList(int nargs) {
	warning("STUB: m_messageList");
	g_lingo->push(Datum(""));
}

void LM::m_name(int nargs) {
	AbstractObject *me = g_lingo->_currentMe.u.obj;
	g_lingo->push(me->getName());
}

void LM::m_respondsTo(int nargs) {
	AbstractObject *me = g_lingo->_currentMe.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	// TODO: Check inheritance level
	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

// Window

Common::String Window::asString() {
	return "window \"" + getName() + "\"";
}

bool Window::hasProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	return g_lingo->_theEntityFields.contains(fieldName);
}

Datum Window::getProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return getField(g_lingo->_theEntityFields[fieldName]->field);
	}

	warning("Window::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

bool Window::setProp(const Common::String &propName, const Datum &value) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return setField(g_lingo->_theEntityFields[fieldName]->field, value);
	}

	warning("Window::setProp: unknown property '%s'", propName.c_str());
	return false;
}

Datum Window::getField(int field) {
	switch (field) {
	case kTheTitle:
		return getTitle();
	case kTheTitleVisible:
		return isTitleVisible();
	case kTheVisible:
		return isVisible();
	case kTheWindowType:
		return getWindowType();
	default:
		warning("Window::getField: unhandled field '%s'", g_lingo->field2str(field));
		return Datum();
	}
}

bool Window::setField(int field, const Datum &value) {
	switch (field) {
	case kTheTitle:
		setTitle(value.asString());
		return true;
	case kTheTitleVisible:
		setTitleVisible(value.asInt());
		return true;
	case kTheVisible:
		setVisible(value.asInt());
		return true;
	case kTheWindowType:
		setWindowType(value.asInt());
		return true;
	default:
		warning("Window::setField: unhandled field '%s'", g_lingo->field2str(field));
		return false;
	}
}

void LM::m_close(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_currentMe.u.obj);
	me->setVisible(false);
}

void LM::m_forget(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_currentMe.u.obj);
	DatumArray *windowList = g_lingo->_windowList.u.farr;

	uint i;
	for (i = 0; i < windowList->size(); i++) {
		if ((*windowList)[i].type != OBJECT || (*windowList)[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>((*windowList)[i].u.obj);
		if (window == me)
			break;
	}

	if (i < windowList->size())
		windowList->remove_at(i);

	// remove me from global vars
	for (DatumHash::iterator it = g_lingo->_globalvars.begin(); it != g_lingo->_globalvars.end(); ++it) {
		if (it->_value.type != OBJECT || it->_value.u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>((*windowList)[i].u.obj);
		if (window == me)
			g_lingo->_globalvars[it->_key] = 0;
	}
}

void LM::m_open(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_currentMe.u.obj);
	me->setVisible(true);
}

void LM::m_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LM::m_moveToFront(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToFront", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
