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

#include "common/scummsys.h"

#if defined(__amigaos4__)

#include "backends/fs/amigaos/amigaos-fs.h"
#include "backends/platform/sdl/amigaos/amigaos.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int main(int argc, char *argv[]) {

	// Update support (AmiUpdate):
	// This will save ScummVM's system application name and add it's binary
	// path to a variable in the platforms native ENV(ARC) system.
	const char *const appname = "ScummVM";

	BPTR lock;
	APTR reqwin;

	// Obtain a lock to it's home directory.
	if ((lock = IDOS->GetProgramDir())) {
		TEXT progpath[2048];
		TEXT apppath[1024] = "AppPaths";

		if (IDOS->DevNameFromLock(lock,	progpath, sizeof(progpath),	DN_FULLPATH)) {
			// Stop any "Please insert volume ..." type system requester.
			reqwin = IDOS->SetProcWindow((APTR)-1);

			// Set the AppPaths variable to the path the binary was run from.
			IDOS->AddPart(apppath, appname, 1024);
			IDOS->SetVar(apppath, progpath, -1, GVF_GLOBAL_ONLY|GVF_SAVE_VAR);

			// Turn system requester back on.
			IDOS->SetProcWindow(reqwin);
		}
	}

	// Set a stack cookie to avoid crashes from a too low stack.
	static const char *stack_cookie __attribute__((used)) = "$STACK: 2048000";

	// Create our OSystem instance.
	g_system = new OSystem_AmigaOS();
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
