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

#include "backends/fs/amigaos4/amigaos4-fs.h"
#include "backends/platform/sdl/amigaos/amigaos.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int main(int argc, char *argv[]) {

	// The following will gather the application name and add the install path
	// to a variable in AmigaOS4's ENV(ARC) system. It will be placed in AppPaths
	// so that ScummVM can become AmiUpdate aware
	const char *const appname = "ScummVM";

	BPTR lock;
	APTR oldwin;

	// Obtain a lock to the home directory
	if ((lock = IDOS->GetProgramDir())) {
		TEXT progpath[2048];
		TEXT apppath[1024] = "AppPaths";

		if (IDOS->DevNameFromLock(lock,
			progpath,
			sizeof(progpath),
			DN_FULLPATH)) {

			// Stop any "Insert volume..." type requesters
			oldwin = IDOS->SetProcWindow((APTR)-1);

			// Finally, set the variable to the path the executable was run from
			IDOS->AddPart( apppath, appname, 1024);
			IDOS->SetVar( apppath, progpath, -1, GVF_GLOBAL_ONLY|GVF_SAVE_VAR );

			// Turn system requesters back on
			IDOS->SetProcWindow( oldwin );
		}
	}

	// Set up a stack cookie to avoid crashes from a stack set too low
	static const char *stack_cookie __attribute__((used)) = "$STACK: 600000";

	// Create our OSystem instance
	g_system = new OSystem_AmigaOS();
	assert(g_system);

	// Pre initialize the backend
	((OSystem_AmigaOS *)g_system)->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point
	int res = scummvm_main(argc, argv);

	// Free OSystem
	g_system->destroy();

	return res;
}

#endif
