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

#ifndef AGS_ENGINE_SCRIPT_SCRIPT_H
#define AGS_ENGINE_SCRIPT_SCRIPT_H

#include "ags/std/vector.h"

#include "ags/shared/game/roomstruct.h" // MAX_ROOM_OBJECTS
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/script/executingscript.h"
#include "ags/engine/script/nonblockingscriptfunction.h"
#include "ags/engine/ac/dynobj/scriptsystem.h"
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
int     run_interaction_script(InteractionScripts *nint, int evnt, int chkAny = -1, int isInv = 0);
int     create_global_script();
void    cancel_all_scripts();

ccInstance *GetScriptInstanceByType(ScriptInstType sc_inst);
// Queues a script function to be run either called by the engine or from another script
void    QueueScriptFunction(ScriptInstType sc_inst, const char *fn_name, size_t param_count = 0,
	const RuntimeScriptValue &p1 = RuntimeScriptValue(), const RuntimeScriptValue &p2 = RuntimeScriptValue());
// Try to run a script function right away
void    RunScriptFunction(ScriptInstType sc_inst, const char *fn_name, size_t param_count = 0,
	const RuntimeScriptValue &p1 = RuntimeScriptValue(), const RuntimeScriptValue &p2 = RuntimeScriptValue());

int     RunScriptFunctionIfExists(ccInstance *sci, const char *tsname, int numParam, const RuntimeScriptValue *params);
int     RunTextScript(ccInstance *sci, const char *tsname);
int     RunTextScriptIParam(ccInstance *sci, const char *tsname, const RuntimeScriptValue &iparam);
int     RunTextScript2IParam(ccInstance *sci, const char *tsname, const RuntimeScriptValue &iparam, const RuntimeScriptValue &param2);

int     PrepareTextScript(ccInstance *sci, const char **tsname);
bool    DoRunScriptFuncCantBlock(ccInstance *sci, NonBlockingScriptFunction *funcToRun, bool hasTheFunc);

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


extern ExecutingScript scripts[MAX_SCRIPT_AT_ONCE];
extern ExecutingScript *curscript;

extern PScript gamescript;
extern PScript dialogScriptsScript;
extern ccInstance *gameinst, *roominst;
extern ccInstance *dialogScriptsInst;
extern ccInstance *gameinstFork, *roominstFork;

extern int num_scripts;
extern int post_script_cleanup_stack;

extern int inside_script, in_graph_script;
extern int no_blocking_functions; // set to 1 while in rep_Exec_always

extern NonBlockingScriptFunction repExecAlways;
extern NonBlockingScriptFunction lateRepExecAlways;
extern NonBlockingScriptFunction getDialogOptionsDimensionsFunc;
extern NonBlockingScriptFunction renderDialogOptionsFunc;
extern NonBlockingScriptFunction getDialogOptionUnderCursorFunc;
extern NonBlockingScriptFunction runDialogOptionMouseClickHandlerFunc;
extern NonBlockingScriptFunction runDialogOptionKeyPressHandlerFunc;
extern NonBlockingScriptFunction runDialogOptionRepExecFunc;

extern ScriptSystem scsystem;

extern std::vector<PScript> scriptModules;
extern std::vector<ccInstance *> moduleInst;
extern std::vector<ccInstance *> moduleInstFork;
extern std::vector<RuntimeScriptValue> moduleRepExecAddr;
extern int numScriptModules;

// TODO: find out if these extra arrays are really necessary. This may be remains from the
// time when the symbol import table was holding raw pointers to char array.
extern std::vector<AGS::Shared::String> characterScriptObjNames;
extern AGS::Shared::String objectScriptObjNames[MAX_ROOM_OBJECTS];
extern std::vector<AGS::Shared::String> guiScriptObjNames;

} // namespace AGS3

#endif
