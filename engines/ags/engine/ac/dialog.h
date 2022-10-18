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

#ifndef AGS_ENGINE_AC_DIALOG_H
#define AGS_ENGINE_AC_DIALOG_H

#include "ags/lib/std/vector.h"
#include "ags/shared/ac/dialog_topic.h"
#include "ags/engine/ac/dynobj/script_dialog.h"

namespace AGS3 {

int  Dialog_GetID(ScriptDialog *sd);
int  Dialog_GetOptionCount(ScriptDialog *sd);
int  Dialog_GetShowTextParser(ScriptDialog *sd);
const char *Dialog_GetOptionText(ScriptDialog *sd, int option);
int  Dialog_DisplayOptions(ScriptDialog *sd, int sayChosenOption);
int  Dialog_GetOptionState(ScriptDialog *sd, int option);
int  Dialog_HasOptionBeenChosen(ScriptDialog *sd, int option);
void Dialog_SetOptionState(ScriptDialog *sd, int option, int newState);
void Dialog_Start(ScriptDialog *sd);

void do_conversation(int dlgnum);
int  show_dialog_options(int dlgnum, int sayChosenOption, bool runGameLoopsInBackground);

} // namespace AGS3

#endif
