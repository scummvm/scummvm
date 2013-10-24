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
 */

#include "prince/debugger.h"
#include "prince/prince.h"

namespace Prince {

Debugger::Debugger(PrinceEngine *vm) : GUI::Debugger(), _vm(vm) {
    DCmd_Register("continue",       WRAP_METHOD(Debugger, Cmd_Exit));
    DCmd_Register("setflag",        WRAP_METHOD(Debugger, Cmd_SetFlag));
    DCmd_Register("getflag",        WRAP_METHOD(Debugger, Cmd_GetFlag));
    DCmd_Register("clearflag",      WRAP_METHOD(Debugger, Cmd_ClearFlag));
    DCmd_Register("viewflc",        WRAP_METHOD(Debugger, Cmd_ViewFlc));
}

static int strToInt(const char *s) {
    if (!*s)
        // No string at all
        return 0;
    else if (toupper(s[strlen(s) - 1]) != 'H')
        // Standard decimal string
        return atoi(s);

    // Hexadecimal string
    uint tmp = 0;
    int read = sscanf(s, "%xh", &tmp);
    if (read < 1)
        error("strToInt failed on string \"%s\"", s);
    return (int)tmp;
}

/*
 * This command sets a flag
 */
bool Debugger::Cmd_SetFlag(int argc, const char **argv) {
    // Check for a flag to set
    if (argc != 2) {
        DebugPrintf("Usage: %s <flag number>\n", argv[0]);
        return true;
    }

    int flagNum = strToInt(argv[1]);
    //g_globals->setFlag(flagNum);
    return true;
}

/*
 * This command gets the value of a flag
 */
bool Debugger::Cmd_GetFlag(int argc, const char **argv) {
    // Check for an flag to display
    if (argc != 2) {
        DebugPrintf("Usage: %s <flag number>\n", argv[0]);
        return true;
    }

    int flagNum = strToInt(argv[1]);
    //DebugPrintf("Value: %d\n", g_globals->getFlag(flagNum));
    return true;
}

/*
 * This command clears a flag
 */
bool Debugger::Cmd_ClearFlag(int argc, const char **argv) {
    // Check for a flag to clear
    if (argc != 2) {
        DebugPrintf("Usage: %s <flag number>\n", argv[0]);
        return true;
    }

    int flagNum = strToInt(argv[1]);
    //g_globals->clearFlag(flagNum);
    return true;
}

/*
 * This command starts new flc anim
 */
bool Debugger::Cmd_ViewFlc(int argc, const char **argv) {
    // Check for a flag to clear
    if (argc != 2) {
        DebugPrintf("Usage: %s <anim number>\n", argv[0]);
        return true;
    }

    int flagNum = strToInt(argv[1]);
    _vm->loadAnim(flagNum);
    return true;
}
} 
