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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include "backends/platform/sdl/ps3/ps3.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

#include <net/net.h>
#include <sys/process.h>

// Set program stack size from default 64KB to 256KB
SYS_PROCESS_PARAM(1000, 0x40000)

int main(int argc, char *argv[]) {

#if defined(USE_LIBCURL) || defined(USE_SDL_NET)
	netInitialize();
#endif

	// Create our OSystem instance
	g_system = new OSystem_PS3();
	assert(g_system);

	// Pre initialize the backend
	g_system->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	g_system->destroy();

#if defined(USE_LIBCURL) || defined(USE_SDL_NET)
	netDeinitialize();
#endif

	return res;
}
