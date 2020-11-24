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

//=============================================================================
//
// Plugin system functions.
//
//=============================================================================

#ifndef AGS_ENGINE_PLUGIN_PLUGIN_BUILTIN_H
#define AGS_ENGINE_PLUGIN_PLUGIN_BUILTIN_H

namespace AGS3 {

#define PLUGIN_FILENAME_MAX (49)

class IAGSEngine;

//  Initial implementation for apps to register their own inbuilt plugins

struct InbuiltPluginDetails {
	char      filename[PLUGIN_FILENAME_MAX + 1];
	void (*engineStartup)(IAGSEngine *);
	void (*engineShutdown)();
	int (*onEvent)(int, int);
	void (*initGfxHook)(const char *driverName, void *data);
	int (*debugHook)(const char *whichscript, int lineNumber, int reserved);
};

// Register a builtin plugin.
int pl_register_builtin_plugin(InbuiltPluginDetails const &details);

} // namespace AGS3

#endif
