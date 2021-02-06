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

#ifndef AGS_ENGINE_DEBUGGING_DEBUGGER_H
#define AGS_ENGINE_DEBUGGING_DEBUGGER_H

#include "ags/shared/util/string.h"

namespace AGS3 {

struct IAGSEditorDebugger;
struct ScriptPosition;

int check_for_messages_from_editor();
bool send_message_to_editor(const char *msg);
bool send_exception_to_editor(const char *qmsg);
// Returns current script's location and callstack
AGS::Shared::String get_cur_script(int numberOfLinesOfCallStack);
bool get_script_position(ScriptPosition &script_pos);
void check_debug_keys();

#define DBG_NOIFACE       1
#define DBG_NODRAWSPRITES 2
#define DBG_NOOBJECTS     4
#define DBG_NOUPDATE      8
#define DBG_NOSFX      0x10
#define DBG_NOMUSIC    0x20
#define DBG_NOSCRIPT   0x40
#define DBG_DBGSCRIPT  0x80
#define DBG_DEBUGMODE 0x100
#define DBG_REGONLY   0x200
#define DBG_NOVIDEO   0x400

enum FPSDisplayMode {
	kFPS_Hide = 0,    // hid by the script/user command
	kFPS_Display = 1, // shown by the script/user command
	kFPS_Forced = 2   // forced shown by the engine arg
};

extern float fps;
extern FPSDisplayMode display_fps;
extern int debug_flags;

} // namespace AGS3

#endif
