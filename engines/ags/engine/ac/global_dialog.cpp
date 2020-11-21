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

#include "ac/global_dialog.h"
#include "ac/common.h"
#include "ac/dialog.h"
#include "ac/dialogtopic.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "debug/out.h"
#include "script/script.h"

using namespace AGS::Common;

extern GameSetupStruct game;
extern GameState play;
extern DialogTopic *dialog;

ScriptPosition last_in_dialog_request_script_pos;
void RunDialog(int tum) {
    if ((tum<0) | (tum>=game.numdialog))
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

void SetDialogOption(int dlg, int opt, int onoroff, bool dlg_script)
{
  if ((dlg<0) | (dlg>=game.numdialog))
    quit("!SetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].numoptions))
  {
    // Pre-3.1.1 games had "dialog scripts" that were written in different language and
    // parsed differently; its "option-on/off" commands were more permissive.
    if (dlg_script)
    {
      Debug::Printf(kDbgGroup_Game, kDbgMsg_Error, "SetDialogOption: Invalid option number specified (%d : %d)", dlg, opt);
      return;
    }
    quit("!SetDialogOption: Invalid option number specified");
  }
  opt--;

  dialog[dlg].optionflags[opt]&=~DFLG_ON;
  if ((onoroff==1) & ((dialog[dlg].optionflags[opt] & DFLG_OFFPERM)==0))
    dialog[dlg].optionflags[opt]|=DFLG_ON;
  else if (onoroff==2)
    dialog[dlg].optionflags[opt]|=DFLG_OFFPERM;
}

int GetDialogOption (int dlg, int opt) {
  if ((dlg<0) | (dlg>=game.numdialog))
    quit("!GetDialogOption: Invalid topic number specified");
  if ((opt<1) | (opt>dialog[dlg].numoptions))
    quit("!GetDialogOption: Invalid option number specified");
  opt--;

  if (dialog[dlg].optionflags[opt] & DFLG_OFFPERM)
    return 2;
  if (dialog[dlg].optionflags[opt] & DFLG_ON)
    return 1;
  return 0;
}
