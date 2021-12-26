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

#ifndef AGS_PLUGINS_PLUGIN_BASE_H
#define AGS_PLUGINS_PLUGIN_BASE_H

#include "ags/shared/util/string.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/textconsole.h"

namespace AGS3 {

class IAGSEditor;
class IAGSEngine;

namespace Plugins {

#define SCRIPT_METHOD(NAME, PROC) addMethod(#NAME, &PROC)

#define SCRIPT_HASH_MACRO(TheClass, BaseClass, RegisterMethod) \
	private: \
		typedef void (TheClass::*MethodPtr)(ScriptMethodParams &params); \
		Common::HashMap<Common::String, MethodPtr> _methods; \
		inline void addMethod(const Common::String &name, MethodPtr fn) { \
			_methods[name] = fn; \
			_engine->RegisterMethod(name.c_str(), this); \
		} \
	public: \
		void execMethod(const Common::String &name, ScriptMethodParams &params) override { \
			if (_methods.contains(name)) \
				(this->*_methods[name])(params); \
			else \
				BaseClass::execMethod(name, params); \
		}
#define SCRIPT_HASH(TheClass) SCRIPT_HASH_MACRO(TheClass, PluginBase, RegisterScriptFunction)
#define BUILT_IN_HASH(TheClass) SCRIPT_HASH_MACRO(TheClass, ScriptContainer, RegisterBuiltInFunction)
#define SCRIPT_HASH_SUB(TheClass, BaseClass) SCRIPT_HASH_MACRO(TheClass, BaseClass, RegisterScriptFunction)

inline float PARAM_TO_FLOAT(int32 xi) {
	float x;
	memcpy(&x, &xi, sizeof(float));
	return x;
}

inline int32 PARAM_FROM_FLOAT(float x) {
	int32 xi;
	memcpy(&xi, &x, sizeof(float));
	return xi;
}

#define PARAMS1(T1, N1) \
	T1 N1 = (T1)params[0]
#define PARAMS2(T1, N1, T2, N2) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]
#define PARAMS3(T1, N1, T2, N2, T3, N3) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]
#define PARAMS4(T1, N1, T2, N2, T3, N3, T4, N4) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]
#define PARAMS5(T1, N1, T2, N2, T3, N3, T4, N4, T5, N5) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]; \
	T5 N5 = (T5)params[4]
#define PARAMS6(T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]; \
	T5 N5 = (T5)params[4]; \
	T6 N6 = (T6)params[5]
#define PARAMS7(T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6, T7, N7) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]; \
	T5 N5 = (T5)params[4]; \
	T6 N6 = (T6)params[5]; \
	T7 N7 = (T7)params[6]
#define PARAMS8(T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6, T7, N7, T8, N8) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]; \
	T5 N5 = (T5)params[4]; \
	T6 N6 = (T6)params[5]; \
	T7 N7 = (T7)params[6]; \
	T8 N8 = (T8)params[7]
#define PARAMS9(T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6, T7, N7, T8, N8, T9, N9) \
	T1 N1 = (T1)params[0]; \
	T2 N2 = (T2)params[1]; \
	T3 N3 = (T3)params[2]; \
	T4 N4 = (T4)params[3]; \
	T5 N5 = (T5)params[4]; \
	T6 N6 = (T6)params[5]; \
	T7 N7 = (T7)params[6]; \
	T8 N8 = (T8)params[7]; \
	T9 N9 = (T9)params[8]

class ScriptMethodParams;
class ScriptContainer;

using string = const char *;
typedef uint32 HWND;

class ScriptMethodParams : public Common::Array<intptr_t> {
public:
	NumberPtr _result;

	ScriptMethodParams();
	ScriptMethodParams(int val1);
	ScriptMethodParams(int val1, int val2);
	ScriptMethodParams(int val1, int val2, int val3);
	ScriptMethodParams(int val1, int val2, int val3, int val4);

	/**
	 * Form of Common::String::format for the parameters array.
	 * @param formatIndex	Param index of the format specifier string
	 */
	Common::String format(int formatIndex);
};

/**
 * Shared base class for plugins and classes exposed to plugins
 */
class ScriptContainer {
public:
	ScriptContainer() {}
	virtual ~ScriptContainer() {}

protected:
	IAGSEngine *_engine = nullptr;
public:
	virtual void AGS_EngineStartup(IAGSEngine *engine) {
		_engine = engine;
	}

	virtual void execMethod(const Common::String &name, ScriptMethodParams &params) {
		error("Plugin does not contain method - %s", name.c_str());
	}
};

/**
 * Base class for the implementation of AGS plugins
 */
class PluginBase: public ScriptContainer {
public:
	PluginBase() {}
	virtual ~PluginBase() {}

	virtual const char *AGS_GetPluginName() = 0;
	virtual int    AGS_PluginV2() const { return 1; }
	virtual int    AGS_EditorStartup(IAGSEditor *) { return 0; }
	virtual void   AGS_EditorShutdown() {}
	virtual void   AGS_EditorProperties(HWND) {}
	virtual int    AGS_EditorSaveGame(char *, int) { return 0; }
	virtual void   AGS_EditorLoadGame(char *, int) {}
	virtual void   AGS_EngineShutdown() {}
	virtual int64 AGS_EngineOnEvent(int, NumberPtr) { return 0; }
	virtual int    AGS_EngineDebugHook(const char *, int, int) { return 0; }
	virtual void   AGS_EngineInitGfx(const char *driverID, void *data) {}
};

class PluginMethod {
private:
	ScriptContainer *_sc;
	Common::String _name;
public:
	PluginMethod() : _sc(nullptr) {}
	PluginMethod(ScriptContainer *sc, const Common::String &name) :
		_sc(sc), _name(name) {
	}

	operator bool() const {
		return _sc != nullptr;
	}

	NumberPtr operator()(ScriptMethodParams &params) const {
		_sc->execMethod(_name, params);
		return params._result;
	}

	NumberPtr operator()(intptr_t val1) const {
		ScriptMethodParams params(val1);
		_sc->execMethod(_name, params);
		return params._result;
	}
	NumberPtr operator()(intptr_t val1, intptr_t val2) const {
		ScriptMethodParams params(val1, val2);
		_sc->execMethod(_name, params);
		return params._result;
	}
	NumberPtr operator()(intptr_t val1, intptr_t val2, intptr_t val3) const {
		ScriptMethodParams params(val1, val2, val3);
		_sc->execMethod(_name, params);
		return params._result;
	}
	NumberPtr operator()(intptr_t val1, intptr_t val2, intptr_t val3, intptr_t val4) const {
		ScriptMethodParams params(val1, val2, val3, val4);
		_sc->execMethod(_name, params);
		return params._result;
	}
};

extern PluginBase *pluginOpen(const char *filename);

extern int pluginClose(Plugins::PluginBase *lib);

extern const char *pluginError();

} // namespace Plugins
} // namespace AGS3

#endif
