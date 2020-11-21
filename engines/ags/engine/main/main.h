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

#ifndef AGS_ENGINE_MAIN_MAIN_H
#define AGS_ENGINE_MAIN_MAIN_H

#include "core/platform.h"
#include "util/version.h"

// Current engine version
extern AGS::Common::Version EngineVersion;
// Lowest savedgame version, accepted by this engine
extern AGS::Common::Version SavedgameLowestBackwardCompatVersion;
// Lowest engine version, which would accept current savedgames
extern AGS::Common::Version SavedgameLowestForwardCompatVersion;

//=============================================================================

extern char **global_argv;

// Location of the engine executable
extern AGS::Common::String appDirectory;
// Game path from the startup options (before reading config)
extern AGS::Common::String cmdGameDataPath;

AGS::Common::String GetPathFromCmdArg(int arg_index);

// Startup flags, set from parameters to engine
extern int force_window;
extern int override_start_room;
extern bool justRegisterGame;
extern bool justUnRegisterGame;
extern bool justTellInfo;
extern const char *loadSaveGameOnStartup;

extern int psp_video_framedrop;
extern int psp_audio_enabled;
extern int psp_midi_enabled;
extern int psp_ignore_acsetup_cfg_file;
extern int psp_clear_cache_on_room_change;

extern int psp_midi_preload_patches;
extern int psp_audio_cachesize;
extern char psp_game_file_name[];
extern char psp_translation[];

void main_print_help();

int ags_entry_point(int argc, char *argv[]);

#endif
