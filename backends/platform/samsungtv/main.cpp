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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/main.cpp $
 * $Id: main.cpp 43636 2009-08-22 12:35:49Z sev $
 *
 */

#include "backends/platform/samsungtv/samsungtv.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

#if defined(SAMSUNGTV)

#include <unistd.h>

extern "C" int Game_Main(char *path, char *) {
	chdir(path);

	g_system = new OSystem_SDL_SamsungTV();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(0, 0);
	g_system->quit();	// TODO: Consider removing / replacing this!
	return res;
}

#endif
