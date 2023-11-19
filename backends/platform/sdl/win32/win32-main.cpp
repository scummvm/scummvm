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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef WIN32

// Fix for bug #4700 "MSVC compilation broken with r47595":
// We need to keep this on top of the "common/scummsys.h"(base/main.h) include,
// otherwise we will get errors about the windows headers redefining
// "ARRAYSIZE" for example.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "backends/platform/sdl/win32/win32.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"
#include "backends/plugins/win32/win32-provider.h"
#include "base/main.h"

int __stdcall WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetModuleHandle(GetModuleHandle(NULL));
#endif
// HACK: __argc, __argv are broken and return zero when using mingwrt 4.0+ on MinGW
// HACK: MinGW-w64 based toolchains neither feature _argc nor _argv. The 32 bit
// incarnation only defines __MINGW32__. This leads to build breakage due to
// missing declarations. Luckily MinGW-w64 based toolchains define
// __MINGW64_VERSION_foo macros inside _mingw.h, which is included from all
// system headers. Thus we abuse that to detect them.
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
	return main(_argc, _argv);
#else
	return main(__argc, __argv);
#endif
}

int main(int argc, char *argv[]) {
#ifdef UNICODE
	argv = Win32::getArgvUtf8(&argc);
#endif

	// Create our OSystem instance
	g_system = new OSystem_Win32();
	assert(g_system);

	// Pre initialize the backend
	g_system->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new Win32PluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	g_system->destroy();

#ifdef UNICODE
	Win32::freeArgvUtf8(argc, argv);
#endif

	return res;
}

#endif
