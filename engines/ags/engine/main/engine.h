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

#ifndef AGS_ENGINE_MAIN_ENGINE_H
#define AGS_ENGINE_MAIN_ENGINE_H

#include "ags/shared/util/ini_util.h"

namespace AGS3 {

const char *get_engine_name();
const char *get_engine_version();
void        show_preload();
void        engine_init_game_settings();
int         initialize_engine(const AGS::Shared::ConfigTree &startup_opts);

struct ScreenSetup;
// Try to set new graphics mode deduced from given configuration;
// if requested mode fails, tries to find any compatible mode close to the
// requested one.
bool        engine_try_set_gfxmode_any(const ScreenSetup &setup);
// Tries to switch between fullscreen and windowed mode; uses previously saved
// setup if it is available, or default settings for the new mode
bool        engine_try_switch_windowed_gfxmode();
// Shutdown graphics mode (used before shutting down tha application)
void        engine_shutdown_gfxmode();

using AGS::Shared::String;
// Defines a package file location
struct PackLocation {
	String Name; // filename, for the reference or to use as an ID
	String Path; // full path
};
// Game resource paths
struct ResourcePaths {
	String       DataDir;    // path to the data directory
	PackLocation GamePak;    // main game package
	PackLocation AudioPak;   // audio package
	PackLocation SpeechPak;  // voice-over package
};
extern ResourcePaths ResPaths;

// Register a callback that will be called before engine is initialised.
// Used for apps to register their own plugins and other configuration
typedef void (*t_engine_pre_init_callback)(void);
extern void engine_set_pre_init_callback(t_engine_pre_init_callback callback);

} // namespace AGS3

#endif
