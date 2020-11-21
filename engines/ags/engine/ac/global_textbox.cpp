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

#include "ac/global_textbox.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/string.h"
#include "ac/textbox.h"
#include "gui/guimain.h"
#include "gui/guitextbox.h"

using namespace AGS::Common;

extern GameSetupStruct game;

void SetTextBoxFont(int guin, int objn, int fontnum) {

	if ((guin < 0) | (guin >= game.numgui)) quit("!SetTextBoxFont: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!SetTextBoxFont: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUITextBox)
		quit("!SetTextBoxFont: specified control is not a text box");

	GUITextBox *guit = (GUITextBox *)guis[guin].GetControl(objn);
	TextBox_SetFont(guit, fontnum);
}

void GetTextBoxText(int guin, int objn, char *txbuf) {
	VALIDATE_STRING(txbuf);
	if ((guin < 0) | (guin >= game.numgui)) quit("!GetTextBoxText: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!GetTextBoxText: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUITextBox)
		quit("!GetTextBoxText: specified control is not a text box");

	GUITextBox *guisl = (GUITextBox *)guis[guin].GetControl(objn);
	TextBox_GetText(guisl, txbuf);
}

void SetTextBoxText(int guin, int objn, const char *txbuf) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!SetTextBoxText: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!SetTextBoxText: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUITextBox)
		quit("!SetTextBoxText: specified control is not a text box");

	GUITextBox *guisl = (GUITextBox *)guis[guin].GetControl(objn);
	TextBox_SetText(guisl, txbuf);
}
