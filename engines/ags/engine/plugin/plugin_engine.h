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

#ifndef __AGS_EE_PLUGIN__PLUGINENGINE_H
#define __AGS_EE_PLUGIN__PLUGINENGINE_H

#include <vector>
#include "game/game_init.h"
#include "game/plugininfo.h"

#define PLUGIN_FILENAME_MAX (49)

class IAGSEngine;
namespace AGS { namespace Common { class Stream; }}
using namespace AGS; // FIXME later

void pl_stop_plugins();
void pl_startup_plugins();
int  pl_run_plugin_hooks (int event, int data);
void pl_run_plugin_init_gfx_hooks(const char *driverName, void *data);
int  pl_run_plugin_debug_hooks (const char *scriptfile, int linenum);
// Tries to register plugins, either by loading dynamic libraries, or getting any kind of replacement
Engine::GameInitError pl_register_plugins(const std::vector<Common::PluginInfo> &infos);
bool pl_is_plugin_loaded(const char *pl_name);

//returns whether _any_ plugins want a particular event
bool pl_any_want_hook(int event);

void pl_set_file_handle(long data, AGS::Common::Stream *stream);
void pl_clear_file_handle();

#endif // __AGS_EE_PLUGIN__PLUGINENGINE_H
