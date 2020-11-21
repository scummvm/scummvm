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

#ifndef AGS_ENGINE_DEBUGGING_LOG_H
#define AGS_ENGINE_DEBUGGING_LOG_H

#include "script/cc_instance.h"
#include "ac/runtime_defines.h"
#include "ac/gamestate.h"
#include "platform/base/agsplatformdriver.h"
#include "util/ini_util.h"

void init_debug(const AGS::Common::ConfigTree &cfg, bool stderr_only);
void apply_debug_config(const AGS::Common::ConfigTree &cfg);
void shutdown_debug();

void debug_set_console(bool enable);

// debug_script_log prints debug warnings tagged with kDbgGroup_Script,
// prepending it with current room number and script position identification
void debug_script_warn(const char *texx, ...);
// debug_script_log prints debug message tagged with kDbgGroup_Script,
// prepending it with current room number and script position identification
void debug_script_log(const char *msg, ...);
void quitprintf(const char *texx, ...);
bool init_editor_debugging();

// allow LShift to single-step,  RShift to pause flow
void scriptDebugHook(ccInstance *ccinst, int linenum) ;

extern AGS::Common::String debug_line[DEBUG_CONSOLE_NUMLINES];
extern int first_debug_line, last_debug_line, display_console;


extern AGSPlatformDriver *platform;

#endif
