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

#include "ac/global_label.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/label.h"
#include "ac/string.h"
#include "gui/guimain.h"

using namespace AGS::Shared;

extern GameSetupStruct game;

void SetLabelColor(int guin, int objn, int colr) {
	if ((guin < 0) | (guin >= game.numgui))
		quit("!SetLabelColor: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount()))
		quit("!SetLabelColor: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelColor: specified control is not a label");

	GUILabel *guil = (GUILabel *)guis[guin].GetControl(objn);
	Label_SetColor(guil, colr);
}

void SetLabelText(int guin, int objn, const char *newtx) {
	VALIDATE_STRING(newtx);
	if ((guin < 0) | (guin >= game.numgui)) quit("!SetLabelText: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!SetLabelTexT: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelText: specified control is not a label");

	GUILabel *guil = (GUILabel *)guis[guin].GetControl(objn);
	Label_SetText(guil, newtx);
}

void SetLabelFont(int guin, int objn, int fontnum) {

	if ((guin < 0) | (guin >= game.numgui)) quit("!SetLabelFont: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!SetLabelFont: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelFont: specified control is not a label");

	GUILabel *guil = (GUILabel *)guis[guin].GetControl(objn);
	Label_SetFont(guil, fontnum);
}
