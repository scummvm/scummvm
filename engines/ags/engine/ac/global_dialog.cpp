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

#include "ags/shared/ac/global_dialog.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/dialog.h"
#include "ags/shared/ac/dialogtopic.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/debug/debug_log.h"
#include "ags/shared/debug/debugger.h"
#include "ags/shared/debug/out.h"
#include "ags/shared/script/script.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetupStruct game;
extern GameState play;
extern DialogTopic *dialog;

ScriptPosition last_in_dialog_request_script_pos;
void RunDialog(int tum) {
	if ((tum < 0) | (tum >= game.numdialog))
		quit("!RunDialog: invalid topic number specified");

	can_run_delayed_command();

	if (play.stop_dialog_at_end != DIALOG_NONE) {
		if (play.stop_dialog_at_end == DIALOG_RUNNING)
			play.stop_dialog_at_end = DIALOG_NEWTOPIC + tum;
		else
			quitprintf("!RunDialog: two NewRoom/RunDialog/StopDialog requests within dialog; last was called in \"%s\", line %d",
				last_in_dialog_request_script_pos.Section.GetCStr(), last_in_dialog_request_script_pos.Line);
		return;
	}

	get_script_position(last_in_dialog_request_script_pos);

	if (inside_script)
		curscript->queue_action(ePSARunDialog, tum, "RunDialog");
	else
		do_conversation(tum);
}


void StopDialog() {
	if (play.stop_dialog_at_end == DIALOG_NONE) {
		debug_script_warn("StopDialog called, but was not in a dialog");
		debug_script_log("StopDialog called but no dialog");
		return;
	}
	get_script_position(last_in_dialog_request_script_pos);
	play.stop_dialog_at_end = DIALOG_STOP;
}

void SetDialogOption(int dlg, int opt, int onoroff, bool dlg_script) {
	if ((dlg < 0) | (dlg >= game.numdialog))
		quit("!SetDialogOption: Invalid topic number specified");
	if ((opt < 1) | (opt > dialog[dlg].numoptions)) {
		// Pre-3.1.1 games had "dialog scripts" that were written in different language and
		// parsed differently; its "option-on/off" commands were more permissive.
		if (dlg_script) {
			Debug::Printf(kDbgGroup_Game, kDbgMsg_Error, "SetDialogOption: Invalid option number specified (%d : %d)", dlg, opt);
			return;
		}
		quit("!SetDialogOption: Invalid option number specified");
	}
	opt--;

	dialog[dlg].optionflags[opt] &= ~DFLG_ON;
	if ((onoroff == 1) & ((dialog[dlg].optionflags[opt] & DFLG_OFFPERM) == 0))
		dialog[dlg].optionflags[opt] |= DFLG_ON;
	else if (onoroff == 2)
		dialog[dlg].optionflags[opt] |= DFLG_OFFPERM;
}

int GetDialogOption(int dlg, int opt) {
	if ((dlg < 0) | (dlg >= game.numdialog))
		quit("!GetDialogOption: Invalid topic number specified");
	if ((opt < 1) | (opt > dialog[dlg].numoptions))
		quit("!GetDialogOption: Invalid option number specified");
	opt--;

	if (dialog[dlg].optionflags[opt] & DFLG_OFFPERM)
		return 2;
	if (dialog[dlg].optionflags[opt] & DFLG_ON)
		return 1;
	return 0;
}

} // namespace AGS3
