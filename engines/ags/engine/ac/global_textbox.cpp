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

#include "ags/engine/ac/global_textbox.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/textbox.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_textbox.h"

namespace AGS3 {

using namespace AGS::Shared;



void SetTextBoxFont(int guin, int objn, int fontnum) {

	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!SetTextBoxFont: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!SetTextBoxFont: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUITextBox)
		quit("!SetTextBoxFont: specified control is not a text box");

	GUITextBox *guit = (GUITextBox *)_GP(guis)[guin].GetControl(objn);
	TextBox_SetFont(guit, fontnum);
}

void GetTextBoxText(int guin, int objn, char *txbuf) {
	VALIDATE_STRING(txbuf);
	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!GetTextBoxText: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!GetTextBoxText: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUITextBox)
		quit("!GetTextBoxText: specified control is not a text box");

	GUITextBox *guisl = (GUITextBox *)_GP(guis)[guin].GetControl(objn);
	TextBox_GetText(guisl, txbuf);
}

void SetTextBoxText(int guin, int objn, const char *txbuf) {
	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!SetTextBoxText: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!SetTextBoxText: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUITextBox)
		quit("!SetTextBoxText: specified control is not a text box");

	GUITextBox *guisl = (GUITextBox *)_GP(guis)[guin].GetControl(objn);
	TextBox_SetText(guisl, txbuf);
}

} // namespace AGS3
