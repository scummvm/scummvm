/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_DEBUGGING_DEBUG_LOG_H
#define AGS_ENGINE_DEBUGGING_DEBUG_LOG_H

#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/ini_util.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

struct ccInstance;

void init_debug(const AGS::Shared::ConfigTree &cfg, bool stderr_only);
void apply_debug_config(const AGS::Shared::ConfigTree &cfg);
void shutdown_debug();

// prints debug messages of given type tagged with kDbgGroup_Game,
// prepending it with current room number and script position info
void debug_script_print(AGS::Shared::MessageType mt, const char *msg, ...);
// prints formatted debug warnings tagged with kDbgGroup_Game,
// prepending it with current room number and script position info
void debug_script_warn(const char *msg, ...);
// prints formatted debug message tagged with kDbgGroup_Game,
// prepending it with current room number and script position info
void debug_script_log(const char *msg, ...);

// Connect engine to external debugger, if one is available
bool init_editor_debugging();
// allow LShift to single-step,  RShift to pause flow
void scriptDebugHook(ccInstance *ccinst, int linenum);

} // namespace AGS3

#endif
