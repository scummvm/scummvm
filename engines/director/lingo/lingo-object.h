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

#ifndef DIRECTOR_LINGO_OBJECT_H
#define DIRECTOR_LINGO_OBJECT_H

#include "director/lingo/lingo.h"

namespace Director {

struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int version;
};

struct XlibFileDesc {
	const char *name;		// Base file name for the Xlib file in the original
	const char *gameId;		// GameId or nullptr if applicable to all
};

class AbstractObject {
public:
	virtual ~AbstractObject() {};

	virtual Common::String getName() const = 0;
	virtual ObjectType getObjType() const = 0;
	virtual bool isDisposed() const = 0;
	virtual int *getRefCount() const = 0;
	virtual void incRefCount() = 0;
	virtual void decRefCount() = 0;
	virtual int getInheritanceLevel() const = 0;

	virtual void setName(const Common::String &name) = 0;
	virtual void dispose() = 0;

	virtual Common::String asString() = 0;
	virtual AbstractObject *clone() = 0;
	virtual Symbol getMethod(const Common::String &methodName) = 0;
	virtual bool hasProp(const Common::String &propName) = 0;
	virtual Datum getProp(const Common::String &propName) = 0;
	virtual Common::String getPropAt(uint32 index) = 0;
	virtual uint32 getPropCount() = 0;
	virtual bool setProp(const Common::String &propName, const Datum &value, bool force = false) = 0;
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
	static void initMethods(const MethodProto protos[]) {
		if (_methods) {
			warning("Object::initMethods: Methods already initialized");
			return;
		}

		_methods = new SymbolHash;
		for (const MethodProto *mtd = protos; mtd->name; mtd++) {
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
	void incRefCount() override { *_refCount += 1; };
	virtual void decRefCount() override {
		*_refCount -= 1;
		if (*_refCount <= 0)
			delete this;
	};
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
		Symbol sym;

		if (_disposed) {
			warning("Method '%s' called on disposed object <%s>, returning VOID", methodName.c_str(), asString().c_str());
			return sym;
		}

		Common::String methodId;
		if ((_objType & (kFactoryObj | kXObj)) && methodName.hasPrefixIgnoreCase("m")) {
			methodId = methodName.substr(1);
		} else {
			methodId = methodName;
		}

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
	Common::String getPropAt(uint32 index) override {
		return Common::String();
	};
	uint32 getPropCount() override {
		return 0;
	};
	bool setProp(const Common::String &propName, const Datum &value, bool force = false) override {
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
	ScriptType _scriptType;
	int _id;
	Common::Array<Common::String> _functionNames; // used by cb_localcall
	Common::HashMap<Common::String, Common::Array<uint32>> _functionByteOffsets;
	SymbolHash _functionHandlers;
	Common::HashMap<uint32, Symbol> _eventHandlers;
	Common::Array<Datum> _constants;
	Common::HashMap<uint32, Datum> _objArray;
	MethodHash _methodNames;
	Common::SharedPtr<Node> _assemblyAST;	// Optionally contains AST when we compile Lingo

private:
	DatumHash _properties;
	Common::Array<Common::String> _propertyNames;
	bool _onlyInLctxContexts = false;

public:
	ScriptContext(Common::String name, ScriptType type = kNoneScript, int id = 0);
	ScriptContext(const ScriptContext &sc);
	~ScriptContext() override;

	bool isFactory() const { return _objType == kFactoryObj; };
	void setFactory(bool flag) { _objType = flag ? kFactoryObj : kScriptObj; }

	void setOnlyInLctxContexts() { _onlyInLctxContexts = true; }
	bool getOnlyInLctxContexts() { return _onlyInLctxContexts; }

	Common::String asString() override;
	Symbol getMethod(const Common::String &methodName) override;
	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	Common::String getPropAt(uint32 index) override;
	uint32 getPropCount() override;
	bool setProp(const Common::String &propName, const Datum &value, bool force = false) override;

	Symbol define(const Common::String &name, ScriptData *code, Common::Array<Common::String> *argNames, Common::Array<Common::String> *varNames);

	Common::String formatFunctionList(const char *prefix);
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
