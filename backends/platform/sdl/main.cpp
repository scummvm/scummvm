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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

// Several SDL based ports use a custom main, and hence do not want to compile
// of this file. The following "#if" ensures that.
#if !defined(POSIX) && \
    !defined(WIN32) && \
    !defined(__MAEMO__) && \
    !defined(__SYMBIAN32__) && \
    !defined(_WIN32_WCE) && \
    !defined(__amigaos4__) && \
    !defined(DINGUX) && \
    !defined(CAANOO) && \
    !defined(LINUXMOTO) && \
    !defined(SAMSUNGTV) && \
    !defined(PLAYSTATION3) && \
    !defined(OPENPANDORA)

#include "backends/platform/sdl/sdl.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int main(int argc, char *argv[]) {

	// Create our OSystem instance
	g_system = new OSystem_SDL();
	assert(g_system);

	// Pre initialize the backend
	((OSystem_SDL *)g_system)->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	delete (OSystem_SDL *)g_system;

	return res;
}

#endif
