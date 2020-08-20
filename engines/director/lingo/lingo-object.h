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

#ifndef DIRECTOR_LINGO_OBJECT_H
#define DIRECTOR_LINGO_OBJECT_H

// FIXME: Basic Lingo types like Datum should probably be in a separate, smaller header
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

namespace Director {

struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int version;
};

class AbstractObject {
public:
	virtual ~AbstractObject() {};

	virtual Common::String getName() const = 0;
	virtual ObjectType getObjType() const = 0;
	virtual bool isDisposed() const = 0;
	virtual int *getRefCount() const = 0;
	virtual int getInheritanceLevel() const = 0;

	virtual void setName(const Common::String &name) = 0;
	virtual void dispose() = 0;

	virtual Common::String asString() = 0;
	virtual AbstractObject *clone() = 0;
	virtual Symbol getMethod(const Common::String &methodName) = 0;
	virtual bool hasProp(const Common::String &propName) = 0;
	virtual Datum getProp(const Common::String &propName) = 0;
	virtual bool setProp(const Common::String &propName, const Datum &value) = 0;
	virtual bool hasField(int field) = 0;
	virtual Datum getField(int field) = 0;
	virtual bool setField(int field, const Datum &value) = 0;
};

template <typename Derived>
class Object : public AbstractObject {
public:
	int *_refCount;

protected:
	Object(Common::String objName) {
		_name = objName;
		_objType = kNoneObj;
		_disposed = false;
		_inheritanceLevel = 1;
		_refCount = new int;
		*_refCount = 0;
	};

	Object(const Object &obj) {
		_name = obj._name;
		_objType = obj._objType;
		_disposed = obj._disposed;
		_inheritanceLevel = obj._inheritanceLevel + 1;
		_refCount = new int;
		*_refCount = 0;
	};

public:
	static void initMethods(MethodProto protos[]) {
		_methods = new SymbolHash;
		for (MethodProto *mtd = protos; mtd->name; mtd++) {
			if (mtd->version > g_lingo->_vm->getVersion())
				continue;

			Symbol sym;
			sym.name = new Common::String(mtd->name);
			sym.type = HBLTIN;
			sym.nargs = mtd->minArgs;
			sym.maxArgs = mtd->maxArgs;
			sym.u.bltin = mtd->func;
			(*_methods)[mtd->name] = sym;
		}
	}

	static void cleanupMethods() {
		delete _methods;
		_methods = nullptr;
	}

	virtual ~Object() {
		delete _refCount;
	};

	Common::String getName() const override { return _name; };
	ObjectType getObjType() const override { return _objType; };
	bool isDisposed() const override { return _disposed; };
	int *getRefCount() const override { return _refCount; };
	int getInheritanceLevel() const override { return _inheritanceLevel; };

	void setName(const Common::String &name) override { _name = name; };
	void dispose() override { _disposed = true; };

	Common::String asString() override {
		return Common::String::format("object: #%s %d %p", _name.c_str(), _inheritanceLevel, (void *)this);
	};

	AbstractObject *clone() override {
		return new Derived(static_cast<Derived const &>(*this));
	};

	Symbol getMethod(const Common::String &methodName) override {
		if (_disposed) {
			error("Method '%s' called on disposed object <%s>", methodName.c_str(), asString().c_str());
		}

		Common::String methodId;
		if ((_objType & (kFactoryObj | kXObj)) && methodName.hasPrefixIgnoreCase("m")) {
			methodId = methodName.substr(1);
		} else {
			methodId = methodName;
		}


		Symbol sym;
		if (_methods && _methods->contains(methodId)) {
			sym = (*_methods)[methodId];
			sym.target = this;
			return sym;
		}
		if (g_lingo->_methods.contains(methodId) && (g_lingo->_methods[methodId].targetType & _objType)) {
			sym = g_lingo->_methods[methodId];
			sym.target = this;
			return sym;
		}

		return sym;
	};

	bool hasProp(const Common::String &propName) override {
		return false;
	};
	Datum getProp(const Common::String &propName) override {
		return Datum();
	};
	bool setProp(const Common::String &propName, const Datum &value) override {
		return false;
	};
	bool hasField(int field) override {
		return false;
	};
	Datum getField(int field) override {
		return Datum();
	};
	bool setField(int field, const Datum &value) override {
		return false;
	};

protected:
	static SymbolHash *_methods;
	Common::String _name;
	ObjectType _objType;
	bool _disposed;
	int _inheritanceLevel; // 1 for original object
};

template<typename Derived>
SymbolHash *Object<Derived>::_methods = nullptr;

class ScriptContext : public Object<ScriptContext> {
public:
	LingoArchive *_archive;
	ScriptType _scriptType;
	int _id;
	Common::Array<Common::String> _functionNames; // used by cb_localcall
	SymbolHash _functionHandlers;
	Common::HashMap<uint32, Symbol> _eventHandlers;
	Common::Array<Datum> _constants;
	DatumHash _properties;
	Common::HashMap<uint32, Datum> _objArray;

public:
	ScriptContext(Common::String name, LingoArchive *archive = nullptr, ScriptType type = kNoneScript, int id = 0);
	ScriptContext(const ScriptContext &sc);
	~ScriptContext() override;

	bool isFactory() const { return _objType == kFactoryObj; };
	void setFactory(bool flag) { _objType = flag ? kFactoryObj : kScriptObj; }

	Common::String asString() override;
	Symbol getMethod(const Common::String &methodName) override;
	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	bool setProp(const Common::String &propName, const Datum &value) override;

	Symbol define(Common::String &name, int nargs, ScriptData *code, Common::Array<Common::String> *argNames, Common::Array<Common::String> *varNames);
};

namespace LM {

// predefined methods
void m_describe(int nargs);
void m_dispose(int nargs);
void m_get(int nargs);
void m_instanceRespondsTo(int nargs);
void m_messageList(int nargs);
void m_name(int nargs);
void m_new(int nargs);
void m_perform(int nargs);
void m_put(int nargs);
void m_respondsTo(int nargs);

// window
void m_close(int nargs);
void m_forget(int nargs);
void m_moveToBack(int nargs);
void m_moveToFront(int nargs);
void m_open(int nargs);

} // End of namespace LM

} // End of namespace Director

#endif
