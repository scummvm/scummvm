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

#ifndef AGS_PLUGINS_DLL_H
#define AGS_PLUGINS_DLL_H

#include "ags/shared/util/string.h"
#include "ags/engine/plugin/agsplugin.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace AGS3 {
namespace Plugins {

#define DLL_METHOD(NAME) _methods[#NAME] = (void *)&NAME
#define SCRIPT_METHOD(NAME) engine->RegisterScriptFunction(#NAME, (void *)&NAME)

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


using string = const char *;
typedef uint32 HWND;

/**
 * Base class for the implementation of AGS plugin DLLs
 */
class DLL {
protected:
	Common::HashMap<Common::String, void *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _methods;

	static int    AGS_PluginV2() { return 1; }
	static int    AGS_EditorStartup(IAGSEditor *);
	static void   AGS_EditorShutdown();
	static void   AGS_EditorProperties(HWND);
	static int    AGS_EditorSaveGame(char *, int);
	static void   AGS_EditorLoadGame(char *, int);
	static void   AGS_EngineStartup(IAGSEngine *);
	static void   AGS_EngineShutdown();
	static int    AGS_EngineOnEvent(int, int);
	static int    AGS_EngineDebugHook(const char *, int, int);
	static void   AGS_EngineInitGfx(const char *driverID, void *data);
public:
	DLL();

	void *operator[](const Common::String &methodName) const {
		return _methods[methodName];
	}
};

extern void *dlopen(const char *filename);

extern int dlclose(void *lib);

extern void *dlsym(void *lib, const char *method);

extern const char *dlerror();

} // namespace Plugins
} // namespace AGS3

#endif
