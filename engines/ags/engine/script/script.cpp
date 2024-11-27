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

#include "ags/engine/script/script.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_dialog.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_hotspot.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_video.h"
#include "ags/engine/ac/inv_window.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_object.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/media/video/video.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/util/string_compat.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

static bool DoRunScriptFuncCantBlock(ccInstance *sci, NonBlockingScriptFunction *funcToRun, bool hasTheFunc);
static char scfunctionname[MAX_FUNCTION_NAME_LEN + 1];

int run_dialog_request(int parmtr) {
	_GP(play).stop_dialog_at_end = DIALOG_RUNNING;
	RuntimeScriptValue params[]{ parmtr };
	RunScriptFunction(_G(gameinst).get(), "dialog_request", 1, params);

	if (_GP(play).stop_dialog_at_end == DIALOG_STOP) {
		_GP(play).stop_dialog_at_end = DIALOG_NONE;
		return -2;
	}
	if (_GP(play).stop_dialog_at_end >= DIALOG_NEWTOPIC) {
		int tval = _GP(play).stop_dialog_at_end - DIALOG_NEWTOPIC;
		_GP(play).stop_dialog_at_end = DIALOG_NONE;
		return tval;
	}
	if (_GP(play).stop_dialog_at_end >= DIALOG_NEWROOM) {
		int roomnum = _GP(play).stop_dialog_at_end - DIALOG_NEWROOM;
		_GP(play).stop_dialog_at_end = DIALOG_NONE;
		NewRoom(roomnum);
		return -2;
	}
	_GP(play).stop_dialog_at_end = DIALOG_NONE;
	return -1;
}

void run_function_on_non_blocking_thread(NonBlockingScriptFunction *funcToRun) {

	update_script_mouse_coords();

	int room_changes_was = _GP(play).room_changes;
	funcToRun->atLeastOneImplementationExists = false;

	// run modules
	// modules need a forkedinst for this to work
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		funcToRun->moduleHasFunction[i] = DoRunScriptFuncCantBlock(_GP(moduleInstFork)[i].get(), funcToRun, funcToRun->moduleHasFunction[i]);

		if (room_changes_was != _GP(play).room_changes)
			return;
	}

	funcToRun->globalScriptHasFunction = DoRunScriptFuncCantBlock(_G(gameinstFork).get(), funcToRun, funcToRun->globalScriptHasFunction);

	if (room_changes_was != _GP(play).room_changes || _G(abort_engine))
		return;

	funcToRun->roomHasFunction = DoRunScriptFuncCantBlock(_G(roominstFork).get(), funcToRun, funcToRun->roomHasFunction);
}

int run_interaction_event(const ObjectEvent &obj_evt, Interaction *nint, int evnt, int chkAny, bool isInv) {

	if (evnt < 0 || (size_t)evnt >= nint->Events.size() ||
	        (nint->Events[evnt].Response.get() == nullptr) || (nint->Events[evnt].Response->Cmds.size() == 0)) {
		// no response defined for this event
		// If there is a response for "Any Click", then abort now so as to
		// run that instead
		if (chkAny < 0);
		else if ((size_t)chkAny < nint->Events.size() &&
		         (nint->Events[chkAny].Response.get() != nullptr) && (nint->Events[chkAny].Response->Cmds.size() > 0))
			return 0;

		// Otherwise, run unhandled_event
		run_unhandled_event(obj_evt, evnt);

		return 0;
	}

	if (_GP(play).check_interaction_only) {
		_GP(play).check_interaction_only = 2;
		return -1;
	}

	int cmdsrun = 0, retval = 0;
	// Right, so there were some commands defined in response to the event.
	retval = run_interaction_commandlist(obj_evt, nint->Events[evnt].Response.get(), &nint->Events[evnt].TimesRun, &cmdsrun);

	if (_G(abort_engine))
		return -1;

	// An inventory interaction, but the wrong item was used
	if ((isInv) && (cmdsrun == 0))
		run_unhandled_event(obj_evt, evnt);

	return retval;
}

// Returns 0 normally, or -1 to indicate that the NewInteraction has
// become invalid and don't run another interaction on it
// (eg. a room change occurred)
int run_interaction_script(const ObjectEvent &obj_evt, InteractionScripts *nint, int evnt, int chkAny) {

	if (evnt < 0 || static_cast<size_t>(evnt) >= nint->ScriptFuncNames.size() || nint->ScriptFuncNames[evnt].IsEmpty()) {
		// no response defined for this event
		// If there is a response for "Any Click", then abort now so as to
		// run that instead
		if (chkAny < 0);
		else if (!nint->ScriptFuncNames[chkAny].IsEmpty())
			return 0;

		// Otherwise, run unhandled_event
		run_unhandled_event(obj_evt, evnt);
		return 0;
	}

	if (_GP(play).check_interaction_only) {
		_GP(play).check_interaction_only = 2;  // CHECKME: wth is "2"?
		return -1;
	}

	const int room_was = _GP(play).room_changes;

	// TODO: find a way to generalize all the following hard-coded behavior

	// Character or Inventory require a global script call
	const ScriptInstType inst_type = (strstr(obj_evt.BlockName.GetCStr(), "character") != nullptr) || (strstr(obj_evt.BlockName.GetCStr(), "inventory") != nullptr) ?
		kScInstGame : kScInstRoom;

	// Room events do not require additional params
	if ((strstr(obj_evt.BlockName.GetCStr(), "room") != nullptr)) {
		QueueScriptFunction(inst_type, nint->ScriptFuncNames[evnt].GetCStr());
	}
	// Regions only require 1 param - dynobj ref
	else if ((strstr(obj_evt.BlockName.GetCStr(), "region") != nullptr)) {
		QueueScriptFunction(inst_type, nint->ScriptFuncNames[evnt].GetCStr(), 1, &obj_evt.DynObj);
	}
	// Other types (characters, objects, invitems, hotspots) require
	// 2 params - dynobj ref and the interaction mode (aka verb)
	else {
		RuntimeScriptValue params[]{obj_evt.DynObj, RuntimeScriptValue().SetInt32(obj_evt.Mode)};
		QueueScriptFunction(inst_type, nint->ScriptFuncNames[evnt].GetCStr(), 2, params);
	}

	// if the room changed within the action
	if (room_was != _GP(play).room_changes)
		return -1;
	return 0;
}

int create_global_script() {
	constexpr int kscript_create_error = -3;

	ccSetOption(SCOPT_AUTOIMPORT, 1);

	// NOTE: this function assumes that the module lists have their elements preallocated!

	std::vector<ccInstance *> all_insts; // gather all to resolve exports below
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		_GP(moduleInst)[i] = ccInstance::CreateFromScript(_GP(scriptModules)[i]);
		if (!_GP(moduleInst)[i])
			return kscript_create_error;
		all_insts.push_back(_GP(moduleInst)[i].get()); // this is only for temp reference
	}

	_G(gameinst) = ccInstance::CreateFromScript(_GP(gamescript));
	if (!_G(gameinst))
		return kscript_create_error;
	all_insts.push_back(_G(gameinst).get()); // this is only for temp reference

	if (_GP(dialogScriptsScript)) {
		_G(dialogScriptsInst) = ccInstance::CreateFromScript(_GP(dialogScriptsScript));
		if (!_G(dialogScriptsInst))
			return kscript_create_error;
		all_insts.push_back(_G(dialogScriptsInst).get()); // this is only for temp reference
	}

	// Resolve the script imports after all the scripts have been loaded
	for (auto &inst : all_insts) {
		if (!inst->ResolveScriptImports(inst->instanceof.get()))
			return kscript_create_error;
		if (!inst->ResolveImportFixups(inst->instanceof.get()))
			return kscript_create_error;
	}

	// Create the forks for 'repeatedly_execute_always' after resolving
	// because they copy their respective originals including the resolve information
	for (size_t module_idx = 0; module_idx < _G(numScriptModules); module_idx++) {
		auto fork = _GP(moduleInst)[module_idx]->Fork();
		if (!fork)
			return kscript_create_error;

		_GP(moduleInstFork)[module_idx] = std::move(fork);
		_GP(moduleRepExecAddr)[module_idx] = _GP(moduleInst)[module_idx]->GetSymbolAddress(REP_EXEC_NAME);
	}

	_G(gameinstFork) = _G(gameinst)->Fork();
	if (_G(gameinstFork) == nullptr)
		return kscript_create_error;

	ccSetOption(SCOPT_AUTOIMPORT, 0);
	return 0;
}

void cancel_all_scripts() {
	for (int i = 0; i < _G(num_scripts); ++i) {
		auto &sc = _G(scripts)[i];
		if (sc.inst) {
			(sc.forkedInst) ? sc.inst->AbortAndDestroy() : sc.inst->Abort();
		}
		sc.numanother = 0;
	}
	_G(num_scripts) = 0;
	// in case the script is running on non-blocking thread (rep-exec-always etc)
	auto inst = ccInstance::GetCurrentInstance();
	if (inst)
		inst->Abort();
}

ccInstance *GetScriptInstanceByType(ScriptInstType sc_inst) {
	if (sc_inst == kScInstGame)
		return _G(gameinst).get();
	else if (sc_inst == kScInstRoom)
		return _G(roominst).get();
	return nullptr;
}

void QueueScriptFunction(ScriptInstType sc_inst, const char *fn_name, size_t param_count, const RuntimeScriptValue *params) {
	if (_G(inside_script))
		// queue the script for the run after current script is finished
		_G(curscript)->run_another(fn_name, sc_inst, param_count, params);
	else
		// if no script is currently running, run the requested script right away
		RunScriptFunctionAuto(sc_inst, fn_name, param_count, params);
}

static bool DoRunScriptFuncCantBlock(ccInstance *sci, NonBlockingScriptFunction *funcToRun, bool hasTheFunc) {
	if (!hasTheFunc)
		return (false);

	_G(no_blocking_functions)++;
	int result = sci->CallScriptFunction(funcToRun->functionName, funcToRun->numParameters, funcToRun->params);

	if (_G(abort_engine))
		return false;

	if (result == -2) {
		// the function doesn't exist, so don't try and run it again
		hasTheFunc = false;
	} else if ((result != 0) && (result != 100)) {
		quit_with_script_error(funcToRun->functionName);
	} else {
		funcToRun->atLeastOneImplementationExists = true;
	}

	// this might be nested, so don't disrupt blocked scripts
	cc_clear_error();
	_G(no_blocking_functions)--;
	return (hasTheFunc);
}

static int PrepareTextScript(ccInstance *sci, const char **tsname) {
	cc_clear_error();
	// FIXME: try to make it so this function is not called with NULL sci
	if (sci == nullptr) return -1;
	if (sci->GetSymbolAddress(tsname[0]).IsNull()) {
		cc_error("no such function in script");
		return -2;
	}
	if (sci->IsBeingRun()) {
		cc_error("script is already in execution");
		return -3;
	}
	ExecutingScript exscript;
	exscript.inst = sci;
	_G(scripts)[_G(num_scripts)] = std::move(exscript);
	_G(curscript) = &_G(scripts)[_G(num_scripts)];
	_G(num_scripts)++;
	if (_G(num_scripts) >= MAX_SCRIPT_AT_ONCE)
		quit("too many nested text script instances created");
	// in case script_run_another is the function name, take a backup
	snprintf(scfunctionname, sizeof(scfunctionname), "%s", tsname[0]);
	tsname[0] = &scfunctionname[0];
	update_script_mouse_coords();
	_G(inside_script)++;
	return 0;
}

int RunScriptFunction(ccInstance *sci, const char *tsname, size_t numParam, const RuntimeScriptValue *params) {
	int oldRestoreCount = _G(gameHasBeenRestored);
	// TODO: research why this is really necessary, and refactor to avoid such hacks!
	// First, save the current ccError state
	// This is necessary because we might be attempting
	// to run Script B, while Script A is still running in the
	// background.
	// If CallInstance here has an error, it would otherwise
	// also abort Script A because ccError is a global variable.
	ScriptError cachedCcError = cc_get_error();

	cc_clear_error();
	int toret = PrepareTextScript(sci, &tsname);
	if (toret) {
		cc_error(cachedCcError);
		return -18;
	}
	cc_clear_error();
	toret = _G(curscript)->inst->CallScriptFunction(tsname, numParam, params);

	// 100 is if Aborted (eg. because we are LoadAGSGame'ing)
	if (!_G(abort_engine) && (toret != 0) && (toret != -2) && (toret != 100)) {
		quit_with_script_error(tsname);
	}

	_G(post_script_cleanup_stack)++;

	if (_G(post_script_cleanup_stack) > 50)
		quitprintf("!post_script_cleanup call stack exceeded: possible recursive function call? running %s", tsname);

	post_script_cleanup();

	_G(post_script_cleanup_stack)--;

	// restore cached error state
	cc_error(cachedCcError);

	// if the game has been restored, ensure that any further scripts are not run
	if ((oldRestoreCount != _G(gameHasBeenRestored)) && (_G(eventClaimed) == EVENT_INPROGRESS))
		_G(eventClaimed) = EVENT_CLAIMED;

	return toret;
}

void RunScriptFunctionInModules(const char *tsname, size_t param_count, const RuntimeScriptValue *params) {
	for (size_t i = 0; i < _G(numScriptModules); ++i)
		RunScriptFunction(_GP(moduleInst)[i].get(), tsname, param_count, params);
	RunScriptFunction(_G(gameinst).get(), tsname, param_count, params);
}

int RunScriptFunctionInRoom(const char *tsname, size_t param_count, const RuntimeScriptValue *params) {
	// Some room callbacks are considered to be obligatory; for historical reasons these are
	// identified by having no parameters;
	// TODO: this is a hack, this should be defined either by function type, or as an arg
	const bool strict_room_event = (param_count == 0);
	int toret = RunScriptFunction(_G(roominst).get(), tsname, param_count, params);
	// If it's a obligatory room event, and return code means missing function - error
	if (strict_room_event && (toret == -18))
		quitprintf("RunScriptFunction: error %d (%s) trying to run '%s'   (Room %d)",
			toret, cc_get_error().ErrorString.GetCStr(), tsname, _G(displayed_room));
	return toret;
}

// Run non-claimable event in all script modules, except room, break if certain events occurred
static int RunUnclaimableEvent(const char *tsname) {
	const int room_changes_was = _GP(play).room_changes;
	const int restore_game_count_was = _G(gameHasBeenRestored);
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		if (!_GP(moduleRepExecAddr)[i].IsNull())
			RunScriptFunction(_GP(moduleInst)[i].get(), tsname);
		// Break on room change or save restoration
		if ((room_changes_was != _GP(play).room_changes) ||
			(restore_game_count_was != _G(gameHasBeenRestored)))
			return 0;
	}
	return RunScriptFunction(_G(gameinst).get(), tsname);
}

static int RunClaimableEvent(const char *tsname, size_t param_count, const RuntimeScriptValue *params) {
	// Run claimable event chain in script modules and room script
	bool eventWasClaimed;
	int toret = run_claimable_event(tsname, true, param_count, params, &eventWasClaimed);
	// Break on event claim
	if (eventWasClaimed)
		return toret;
	return RunScriptFunction(_G(gameinst).get(), tsname, param_count, params);
}

int RunScriptFunctionAuto(ScriptInstType sc_inst, const char *tsname, size_t param_count, const RuntimeScriptValue *params) {
	// If told to use a room instance, then run only there
	if (sc_inst == kScInstRoom)
		return RunScriptFunctionInRoom(tsname, param_count, params);
	// Rep-exec is only run in script modules, but not room script
	// (because room script has its own callback, attached to event slot)
	if (strcmp(tsname, REP_EXEC_NAME) == 0) {
		return RunUnclaimableEvent(REP_EXEC_NAME);
	}
	// Claimable event is run in all the script modules and room script,
	// before running in the globalscript instance
	if ((strcmp(tsname, _G(tsnames)[kTS_KeyPress]) == 0) || (strcmp(tsname, _G(tsnames)[kTS_MouseClick]) == 0) ||
		(strcmp(tsname, _G(tsnames)[kTS_TextInput]) == 0) || (strcmp(tsname, "on_event") == 0)) {
		return RunClaimableEvent(tsname, param_count, params);
	}
	// Else run on the single chosen script instance
	ccInstance *sci = GetScriptInstanceByType(sc_inst);
	if (!sci)
		return 0;
	return RunScriptFunction(sci, tsname, param_count, params);
}

void AllocScriptModules() {
	// NOTE: this preallocation possibly required to safeguard some algorithms
	_GP(moduleInst).resize(_G(numScriptModules));
	_GP(moduleInstFork).resize(_G(numScriptModules));
	_GP(moduleRepExecAddr).resize(_G(numScriptModules));
	_GP(repExecAlways).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(lateRepExecAlways).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(getDialogOptionsDimensionsFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(renderDialogOptionsFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(getDialogOptionUnderCursorFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionMouseClickHandlerFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionKeyPressHandlerFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionTextInputHandlerFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionRepExecFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionCloseFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	for (auto &val : _GP(moduleRepExecAddr)) {
		val.Invalidate();
	}
}

void FreeAllScriptInstances() {
	ccInstance::FreeInstanceStack();
	FreeRoomScriptInstance();

	// NOTE: don't know why, but Forks must be deleted prior to primary inst,
	// or bad things will happen; TODO: investigate and make this less fragile
	_G(gameinstFork).reset();
	_G(gameinst).reset();
	_G(dialogScriptsInst).reset();
	_GP(moduleInstFork).clear();
	_GP(moduleInst).clear();
}

void FreeRoomScriptInstance() {
	// NOTE: don't know why, but Forks must be deleted prior to primary inst,
	// or bad things will happen; TODO: investigate and make this less fragile
	_G(roominstFork).reset();
	_G(roominst).reset();
}

void FreeGlobalScripts() {
	_G(numScriptModules) = 0;

	_GP(gamescript).reset();
	_GP(scriptModules).clear();
	_GP(dialogScriptsScript).reset();

	_GP(repExecAlways).moduleHasFunction.clear();
	_GP(lateRepExecAlways).moduleHasFunction.clear();
	_GP(getDialogOptionsDimensionsFunc).moduleHasFunction.clear();
	_GP(renderDialogOptionsFunc).moduleHasFunction.clear();
	_GP(getDialogOptionUnderCursorFunc).moduleHasFunction.clear();
	_GP(runDialogOptionMouseClickHandlerFunc).moduleHasFunction.clear();
	_GP(runDialogOptionKeyPressHandlerFunc).moduleHasFunction.clear();
	_GP(runDialogOptionTextInputHandlerFunc).moduleHasFunction.clear();
	_GP(runDialogOptionRepExecFunc).moduleHasFunction.clear();
	_GP(runDialogOptionCloseFunc).moduleHasFunction.clear();
}

String GetScriptName(ccInstance *sci) {
	// TODO: have script name a ccScript's member?
	// TODO: check script modules too?
	if (!sci)
		return "Not in a script";
	else if (sci->instanceof == _GP(gamescript))
		return "Global script";
	else if (sci->instanceof == _GP(thisroom).CompiledScript)
		return String::FromFormat("Room %d script", _G(displayed_room));
	return "Unknown script";
}

//=============================================================================


char bname[MAX_FUNCTION_NAME_LEN + 1], bne[MAX_FUNCTION_NAME_LEN + 1];
char *make_ts_func_name(const char *base, int iii, int subd) {
	int err = snprintf(bname, MAX_FUNCTION_NAME_LEN, base, iii);
	if (err >= (int)sizeof(bname))
		debug_script_warn("Function name length limit exceeded: %s (%d)", base, iii);
	err = snprintf(bne, MAX_FUNCTION_NAME_LEN, "%s_%c", bname, subd + 'a');
	if (err >= (int)sizeof(bne))
		debug_script_warn("Function name length limit exceeded: %s", bname);
	return &bne[0];
}

void post_script_cleanup() {
	// should do any post-script stuff here, like go to new room
	if (cc_has_error())
		quit(cc_get_error().ErrorString);

	ExecutingScript copyof;
	if (_G(num_scripts) > 0) {  // save until the end of function
		copyof = std::move(_G(scripts)[_G(num_scripts) - 1]);
		copyof.forkedInst.reset(); // don't need it further
		_G(num_scripts)--;
	}
	_G(inside_script)--;

	if (_G(num_scripts) > 0)
		_G(curscript) = &_G(scripts)[_G(num_scripts) - 1];
	else {
		_G(curscript) = nullptr;
	}
	//  if (abort_executor) user_disabled_data2=aborted_ip;

	int old_room_number = _G(displayed_room);

	// FIXME: sync audio in case any screen changing or time-consuming post-script actions were scheduled
	if (copyof.numPostScriptActions > 0) {
		sync_audio_playback();
	}

	// run the queued post-script actions
	for (int ii = 0; ii < copyof.numPostScriptActions; ii++) {
		int thisData = copyof.postScriptActionData[ii];

		switch (copyof.postScriptActions[ii]) {
		case ePSANewRoom:
			// only change rooms when all scripts are done
			if (_G(num_scripts) == 0) {
				new_room(thisData, _G(playerchar));
				// don't allow any pending room scripts from the old room
				// in run_another to be executed
				return;
			} else
				_G(curscript)->queue_action(ePSANewRoom, thisData, "NewRoom");
			break;
		case ePSAInvScreen:
			invscreen();
			break;
		case ePSARestoreGame:
			cancel_all_scripts();
			try_restore_save(thisData);
			return;
		case ePSARestoreGameDialog:
			restore_game_dialog();
			return;
		case ePSARunAGSGame:
			cancel_all_scripts();
			_G(load_new_game) = thisData;
			return;
		case ePSARunDialog:
			do_conversation(thisData);
			break;
		case ePSARestartGame:
			cancel_all_scripts();
			restart_game();
			return;
		case ePSASaveGame:
			save_game(thisData, copyof.postScriptSaveSlotDescription[ii]);
			break;
		case ePSASaveGameDialog:
			save_game_dialog();
			break;
		default:
			quitprintf("undefined post script action found: %d", copyof.postScriptActions[ii]);
		}
		// if the room changed in a conversation, for example, abort
		if (old_room_number != _G(displayed_room) || _G(abort_engine)) {
			return;
		}
	}

	if (copyof.numPostScriptActions > 0) {
		sync_audio_playback();
	}

	for (int jj = 0; jj < copyof.numanother; jj++) {
		old_room_number = _G(displayed_room);
		QueuedScript &script = copyof.ScFnQueue[jj];
		RunScriptFunctionAuto(script.Instance, script.FnName.GetCStr(), script.ParamCount, script.Params);
		if (script.Instance == kScInstRoom && script.ParamCount == 1) {
			// some bogus hack for "on_call" event handler
			_GP(play).roomscript_finished = 1;
		}

		// if they've changed rooms, cancel any further pending scripts
		if ((_G(displayed_room) != old_room_number) || (_G(load_new_game)))
			break;
	}

}

void quit_with_script_error(const char *functionName) {
	// TODO: clean up the error reporting logic. Now engine will append call
	// stack info in quit_check_for_error_state() but only in case of explicit
	// script error ("!" type), and not in other case.
	const auto &error = cc_get_error();
	if (error.IsUserError)
		quitprintf("!Error running function '%s':\n%s", functionName, error.ErrorString.GetCStr());
	else
		quitprintf("Error running function '%s':\n%s\n\n%s", functionName,
			error.ErrorString.GetCStr(), error.CallStack.GetCStr());
}

int get_nivalue(InteractionCommandList *nic, int idx, int parm) {
	if (nic->Cmds[idx].Data[parm].Type == AGS::Shared::kInterValVariable) {
		// return the value of the variable
		return get_interaction_variable(nic->Cmds[idx].Data[parm].Value)->Value;
	}
	return nic->Cmds[idx].Data[parm].Value;
}

InteractionVariable *get_interaction_variable(int varindx) {

	if ((varindx >= LOCAL_VARIABLE_OFFSET) && ((size_t)varindx < LOCAL_VARIABLE_OFFSET + _GP(thisroom).LocalVariables.size()))
		return &_GP(thisroom).LocalVariables[varindx - LOCAL_VARIABLE_OFFSET];

	if ((varindx < 0) || (varindx >= _G(numGlobalVars)))
		quit("!invalid interaction variable specified");

	return &_G(globalvars)[varindx];
}

InteractionVariable *FindGraphicalVariable(const char *varName) {
	int ii;
	for (ii = 0; ii < _G(numGlobalVars); ii++) {
		if (_G(globalvars)[ii].Name.CompareNoCase(varName) == 0)
			return &_G(globalvars)[ii];
	}
	for (size_t i = 0; i < _GP(thisroom).LocalVariables.size(); ++i) {
		if (_GP(thisroom).LocalVariables[i].Name.CompareNoCase(varName) == 0)
			return &_GP(thisroom).LocalVariables[i];
	}
	return nullptr;
}

#define IPARAM1 get_nivalue(nicl, i, 0)
#define IPARAM2 get_nivalue(nicl, i, 1)
#define IPARAM3 get_nivalue(nicl, i, 2)
#define IPARAM4 get_nivalue(nicl, i, 3)
#define IPARAM5 get_nivalue(nicl, i, 4)

struct TempEip {
	int oldval;
	TempEip(int newval) {
		oldval = get_our_eip();
		set_our_eip(newval);
	}
	~TempEip() {
		set_our_eip(oldval);
	}
};

// the 'cmdsrun' parameter counts how many commands are run.
// if a 'Inv Item Was Used' check does not pass, it doesn't count
// so cmdsrun remains 0 if no inventory items matched
int run_interaction_commandlist(const ObjectEvent &obj_evt, InteractionCommandList *nicl, int *timesrun, int *cmdsrun) {
	if (nicl == nullptr)
		return -1;

	const char *evblockbasename = obj_evt.BlockName.GetCStr();
	const int evblocknum = obj_evt.BlockID;
	for (size_t i = 0; i < nicl->Cmds.size(); i++) {
		cmdsrun[0] ++;
		int room_was = _GP(play).room_changes;

		switch (nicl->Cmds[i].Type) {
		case 0:  // Do nothing
			break;
		case 1: { // Run script
			TempEip tempip(4001);
			RuntimeScriptValue rval_null;
			if ((strstr(evblockbasename, "character") != nullptr) || (strstr(evblockbasename, "inventory") != nullptr)) {
				// Character or Inventory (global script)
				const char *torun = make_ts_func_name(evblockbasename, evblocknum, nicl->Cmds[i].Data[0].Value);
				// we are already inside the mouseclick event of the script, can't nest calls
				QueueScriptFunction(kScInstGame, torun);
			} else {
				// Other (room script)
				const char *torun = make_ts_func_name(evblockbasename, evblocknum, nicl->Cmds[i].Data[0].Value);
				QueueScriptFunction(kScInstRoom, torun);
			}
			break;
		}
		case 2:  // Add score (first time)
			if (timesrun[0] > 0)
				break;
			timesrun[0] ++;
			// fall through
		case 3:  // Add score
			GiveScore(IPARAM1);
			break;
		case 4:  // Display Message
			/*        if (comprdata<0)
			_G(display_message_aschar)=evb->data[ss];*/
			DisplayMessage(IPARAM1);
			break;
		case 5:  // Play Music
			PlayMusicResetQueue(IPARAM1);
			break;
		case 6:  // Stop Music
			stopmusic();
			break;
		case 7:  // Play Sound
			play_sound(IPARAM1);
			break;
		case 8:  // Play Flic
			PlayFlic(IPARAM1, IPARAM2);
			break;
		case 9: { // Run Dialog
			RunDialog(IPARAM1);
			// if they changed room within the dialog script,
			// the interaction command list is no longer valid
			if (room_was != _GP(play).room_changes)
				return -1;
		}
		break;
		case 10: // Enable Dialog Option
			SetDialogOption(IPARAM1, IPARAM2, 1);
			break;
		case 11: // Disable Dialog Option
			SetDialogOption(IPARAM1, IPARAM2, 0);
			break;
		case 12: // Go To Screen
			Character_ChangeRoomAutoPosition(_G(playerchar), IPARAM1, IPARAM2);
			return -1;
		case 13: // Add Inventory
			add_inventory(IPARAM1);
			break;
		case 14: // Move Object
			MoveObject(IPARAM1, IPARAM2, IPARAM3, IPARAM4);
			// if they want to wait until finished, do so
			if (IPARAM5)
				GameLoopUntilNotMoving(&_G(objs)[IPARAM1].moving);
			break;
		case 15: // Object Off
			ObjectOff(IPARAM1);
			break;
		case 16: // Object On
			ObjectOn(IPARAM1);
			break;
		case 17: // Set Object View
			SetObjectView(IPARAM1, IPARAM2);
			break;
		case 18: // Animate Object
			AnimateObject4(IPARAM1, IPARAM2, IPARAM3, IPARAM4);
			break;
		case 19: // Move Character
			if (IPARAM4)
				MoveCharacterBlocking(IPARAM1, IPARAM2, IPARAM3, 0);
			else
				MoveCharacter(IPARAM1, IPARAM2, IPARAM3);
			break;
		case 20: // If Inventory Item was used
			if (_GP(play).usedinv == IPARAM1) {
				if (_GP(game).options[OPT_NOLOSEINV] == 0)
					lose_inventory(_GP(play).usedinv);
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			} else
				cmdsrun[0] --;
			break;
		case 21: // if player has inventory item
			if (_G(playerchar)->inv[IPARAM1] > 0)
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		case 22: // if a character is moving
			if (_GP(game).chars[IPARAM1].walking)
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		case 23: // if two variables are equal
			if (IPARAM1 == IPARAM2)
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		case 24: // Stop character walking
			StopMoving(IPARAM1);
			break;
		case 25: // Go to screen at specific co-ordinates
			NewRoomEx(IPARAM1, IPARAM2, IPARAM3);
			return -1;
		case 26: // Move NPC to different room
			if (!is_valid_character(IPARAM1))
				quit("!Move NPC to different room: invalid character specified");
			_GP(game).chars[IPARAM1].room = IPARAM2;
			break;
		case 27: // Set character view
			SetCharacterView(IPARAM1, IPARAM2);
			break;
		case 28: // Release character view
			ReleaseCharacterView(IPARAM1);
			break;
		case 29: // Follow character
			FollowCharacter(IPARAM1, IPARAM2);
			break;
		case 30: // Stop following
			FollowCharacter(IPARAM1, -1);
			break;
		case 31: // Disable hotspot
			DisableHotspot(IPARAM1);
			break;
		case 32: // Enable hotspot
			EnableHotspot(IPARAM1);
			break;
		case 33: // Set variable value
			get_interaction_variable(nicl->Cmds[i].Data[0].Value)->Value = IPARAM2;
			break;
		case 34: // Run animation
			AnimateCharacter4(IPARAM1, IPARAM2, IPARAM3, 0);
			GameLoopUntilValueIsZero(&_GP(game).chars[IPARAM1].animating);
			break;
		case 35: // Quick animation
			SetCharacterView(IPARAM1, IPARAM2);
			AnimateCharacter4(IPARAM1, IPARAM3, IPARAM4, 0);
			GameLoopUntilValueIsZero(&_GP(game).chars[IPARAM1].animating);
			ReleaseCharacterView(IPARAM1);
			break;
		case 36: // Set idle animation
			SetCharacterIdle(IPARAM1, IPARAM2, IPARAM3);
			break;
		case 37: // Disable idle animation
			SetCharacterIdle(IPARAM1, -1, -1);
			break;
		case 38: // Lose inventory item
			lose_inventory(IPARAM1);
			break;
		case 39: // Show GUI
			InterfaceOn(IPARAM1);
			break;
		case 40: // Hide GUI
			InterfaceOff(IPARAM1);
			break;
		case 41: // Stop running more commands
			return -1;
		case 42: // Face location
			FaceLocation(IPARAM1, IPARAM2, IPARAM3);
			break;
		case 43: // Pause command processor
			scrWait(IPARAM1);
			break;
		case 44: // Change character view
			ChangeCharacterView(IPARAM1, IPARAM2);
			break;
		case 45: // If player character is
			if (GetPlayerCharacter() == IPARAM1)
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		case 46: // if cursor mode is
			if (GetCursorMode() == IPARAM1)
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		case 47: // if player has been to room
			if (HasBeenToRoom(IPARAM1))
				if (run_interaction_commandlist(obj_evt, nicl->Cmds[i].Children.get(), timesrun, cmdsrun))
					return -1;
			break;
		default:
			quit("unknown new interaction command");
			break;
		}

		if (_G(abort_engine))
			return -1;

		// if the room changed within the action, nicl is no longer valid
		if (room_was != _GP(play).room_changes)
			return -1;
	}
	return 0;

}

// check and abort game if the script is currently
// inside the rep_exec_always function
void can_run_delayed_command() {
	if (_G(no_blocking_functions))
		quit("!This command cannot be used within non-blocking events such as " REP_EXEC_ALWAYS_NAME);
}

void run_unhandled_event(const ObjectEvent &obj_evt, int evnt) {

	if (_GP(play).check_interaction_only)
		return;

	const char *evblockbasename = obj_evt.BlockName.GetCStr();
	const int evblocknum = obj_evt.BlockID;
	int evtype = 0;

	if (ags_strnicmp(evblockbasename, "hotspot", 7) == 0) evtype = 1;
	else if (ags_strnicmp(evblockbasename, "object", 6) == 0) evtype = 2;
	else if (ags_strnicmp(evblockbasename, "character", 9) == 0) evtype = 3;
	else if (ags_strnicmp(evblockbasename, "inventory", 9) == 0) evtype = 5;
	else if (ags_strnicmp(evblockbasename, "region", 6) == 0)
		return;  // no unhandled_events for regions

	// clicked Hotspot 0, so change the type code
	if ((evtype == 1) & (evblocknum == 0) & (evnt != 0) & (evnt != 5) & (evnt != 6))
		evtype = 4;
	if ((evtype == 1) & ((evnt == 0) | (evnt == 5) | (evnt == 6)))
		;  // character stands on hotspot, mouse moves over hotspot, any click
	else if ((evtype == 2) & (evnt == 4));  // any click on object
	else if ((evtype == 3) & (evnt == 4));  // any click on character
	else if (evtype > 0) {
		can_run_delayed_command();
		RuntimeScriptValue params[] = { evtype, evnt };
		QueueScriptFunction(kScInstGame, "unhandled_event", 2, params);
	}
}

bool get_script_position(ScriptPosition &script_pos) {
	ccInstance *cur_instance = ccInstance::GetCurrentInstance();
	if (cur_instance) {
		cur_instance->GetScriptPosition(script_pos);
		return true;
	}
	return false;
}

String cc_format_error(const String &message) {
	if (_G(currentline) > 0)
		return String::FromFormat("Error (line %d): %s", _G(currentline), message.GetCStr());
	else
		return String::FromFormat("Error (line unknown): %s", message.GetCStr());
}

} // namespace AGS3
