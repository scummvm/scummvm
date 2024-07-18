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

#ifndef AGS_ENGINE_SCRIPT_SCRIPT_H
#define AGS_ENGINE_SCRIPT_SCRIPT_H

#include "common/std/memory.h"
#include "common/std/vector.h"

#include "ags/engine/script/cc_instance.h"
#include "ags/engine/script/executing_script.h"
#include "ags/engine/script/non_blocking_script_function.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::Interaction;
using AGS::Shared::InteractionCommandList;
using AGS::Shared::InteractionScripts;
using AGS::Shared::InteractionVariable;

#define LATE_REP_EXEC_ALWAYS_NAME "late_repeatedly_execute_always"
#define REP_EXEC_ALWAYS_NAME "repeatedly_execute_always"
#define REP_EXEC_NAME "repeatedly_execute"

int     run_dialog_request(int parmtr);
void    run_function_on_non_blocking_thread(NonBlockingScriptFunction *funcToRun);
int     run_interaction_event(Interaction *nint, int evnt, int chkAny = -1, int isInv = 0);
int     run_interaction_script(InteractionScripts *nint, int evnt, int chkAny = -1);
int     create_global_script();
void    cancel_all_scripts();

ccInstance *GetScriptInstanceByType(ScriptInstType sc_inst);
// Queues a script function to be run either called by the engine or from another script
void    QueueScriptFunction(ScriptInstType sc_inst, const char *fn_name, size_t param_count = 0,
	const RuntimeScriptValue *params = nullptr);
// Try to run a script function on a given script instance
int     RunScriptFunction(ccInstance *sci, const char *tsname, size_t param_count = 0,
	const RuntimeScriptValue *params = nullptr);
// Run a script function in all the regular script modules, in order, where available
// includes globalscript, but not the current room script.
void    RunScriptFunctionInModules(const char *tsname, size_t param_count = 0,
	const RuntimeScriptValue *params = nullptr);
// Run an obligatory script function in the current room script
int     RunScriptFunctionInRoom(const char *tsname, size_t param_count = 0,
	const RuntimeScriptValue *params = nullptr);
// Try to run a script function, guessing the behavior by its name and script instance type;
// depending on the type may run a claimable callback chain
int     RunScriptFunctionAuto(ScriptInstType sc_inst, const char *fn_name, size_t param_count = 0,
	const RuntimeScriptValue *params = nullptr);

// Preallocates script module instances
void	AllocScriptModules();
// Delete all the script instance objects
void	FreeAllScriptInstances();
// Delete only the current room script instance
void	FreeRoomScriptInstance();
// Deletes all the global scripts and modules;
// this frees all of the bytecode and runtime script memory.
void	FreeGlobalScripts();

AGS::Shared::String GetScriptName(ccInstance *sci);

//=============================================================================

char *make_ts_func_name(const char *base, int iii, int subd);
// Performs various updates to the game after script interpreter returns control to the engine.
// Executes actions and does changes that are not executed immediately at script command, for
// optimisation and other reasons.
void    post_script_cleanup();
void    quit_with_script_error(const char *functionName);
int     get_nivalue(InteractionCommandList *nic, int idx, int parm);
int     run_interaction_commandlist(InteractionCommandList *nicl, int *timesrun, int *cmdsrun);
InteractionVariable *get_interaction_variable(int varindx);
InteractionVariable *FindGraphicalVariable(const char *varName);
void    run_unhandled_event(int evnt);
void    can_run_delayed_command();

// Gets current running script position
bool    get_script_position(ScriptPosition &script_pos);
AGS::Shared::String cc_get_callstack(int max_lines = INT_MAX);

// [ikm] we keep ccInstances saved in unique_ptrs globally for now
// (e.g. as opposed to shared_ptrs), because the script handling part of the
// engine is quite fragile and prone to errors whenever the instance is not
// **deleted** in precise time. This is related to:
// - ccScript's "instances" counting, which affects script exports reg/unreg;
// - loadedInstances array.
// One of the examples is the save restoration, that may occur in the midst
// of a post-script cleanup process, whilst the engine's stack still has
// references to the ccInstances that are going to be deleted on cleanup.
// Ideally, this part of the engine should be refactored awhole with a goal
// to make it safe and consistent.
typedef std::unique_ptr<ccInstance> UInstance;

} // namespace AGS3

#endif
