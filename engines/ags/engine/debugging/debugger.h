//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_DEBUGGER_H
#define __AC_DEBUGGER_H

#include "util/string.h"

struct IAGSEditorDebugger;
struct ScriptPosition;

extern int editor_debugging_enabled;
extern int editor_debugging_initialized;
extern char editor_debugger_instance_token[100];
extern IAGSEditorDebugger *editor_debugger;
extern int break_on_next_script_step;

int check_for_messages_from_editor();
bool send_message_to_editor(const char *msg);
bool send_exception_to_editor(const char *qmsg);
// Returns current script's location and callstack
AGS::Common::String get_cur_script(int numberOfLinesOfCallStack);
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

enum FPSDisplayMode
{
    kFPS_Hide = 0,    // hid by the script/user command
    kFPS_Display = 1, // shown by the script/user command
    kFPS_Forced = 2   // forced shown by the engine arg
};

extern float fps;
extern FPSDisplayMode display_fps;
extern int debug_flags;

#endif // __AC_DEBUGGER_H
