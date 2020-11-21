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
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__DIALOG_H
#define __AGS_EE_AC__DIALOG_H

#include <vector>
#include "ac/dynobj/scriptdialog.h"

int  Dialog_GetID(ScriptDialog *sd);
int  Dialog_GetOptionCount(ScriptDialog *sd);
int  Dialog_GetShowTextParser(ScriptDialog *sd);
const char* Dialog_GetOptionText(ScriptDialog *sd, int option);
int  Dialog_DisplayOptions(ScriptDialog *sd, int sayChosenOption);
int  Dialog_GetOptionState(ScriptDialog *sd, int option);
int  Dialog_HasOptionBeenChosen(ScriptDialog *sd, int option);
void Dialog_SetOptionState(ScriptDialog *sd, int option, int newState);
void Dialog_Start(ScriptDialog *sd);

void do_conversation(int dlgnum);
int  show_dialog_options(int dlgnum, int sayChosenOption, bool runGameLoopsInBackground) ;

extern ScriptDialog *scrDialog;

#endif // __AGS_EE_AC__DIALOG_H
