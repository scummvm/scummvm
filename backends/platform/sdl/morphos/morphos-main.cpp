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

#include "common/scummsys.h"

#if defined(__MORPHOS__)

#include "backends/fs/morphos/morphos-fs.h"
#include "backends/platform/sdl/morphos/morphos.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int main(int argc, char *argv[]) {

	// Set a stack cookie to avoid crashes from a too low stack.
	static const char *stack_cookie __attribute__((used)) = "$STACK: 4096000";

	// Create our OSystem instance.
	g_system = new OSystem_MorphOS();
	assert(g_system);

	// Pre-initialize the backend.
	g_system->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point.
	int res = scummvm_main(argc, argv);

	// Free OSystem.
	g_system->destroy();

	return res;
}

#endif
