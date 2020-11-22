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

#include "ac/global_button.h"
#include "ac/common.h"
#include "ac/button.h"
#include "ac/gamesetupstruct.h"
#include "ac/string.h"
#include "gui/guimain.h"
#include "gui/guibutton.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetupStruct game;

void SetButtonText(int guin, int objn, const char *newtx) {
	VALIDATE_STRING(newtx);
	if ((guin < 0) | (guin >= game.numgui))
		quit("!SetButtonText: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount()))
		quit("!SetButtonText: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUIButton)
		quit("!SetButtonText: specified control is not a button");

	GUIButton *guil = (GUIButton *)guis[guin].GetControl(objn);
	Button_SetText(guil, newtx);
}


void AnimateButton(int guin, int objn, int view, int loop, int speed, int repeat) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!AnimateButton: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!AnimateButton: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUIButton)
		quit("!AnimateButton: specified control is not a button");

	Button_Animate((GUIButton *)guis[guin].GetControl(objn), view, loop, speed, repeat);
}


int GetButtonPic(int guin, int objn, int ptype) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!GetButtonPic: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!GetButtonPic: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUIButton)
		quit("!GetButtonPic: specified control is not a button");
	if ((ptype < 0) | (ptype > 3)) quit("!GetButtonPic: invalid pic type");

	GUIButton *guil = (GUIButton *)guis[guin].GetControl(objn);

	if (ptype == 0) {
		// currently displayed pic
		if (guil->CurrentImage < 0)
			return guil->Image;
		return guil->CurrentImage;
	} else if (ptype == 1) {
		// nomal pic
		return guil->Image;
	} else if (ptype == 2) {
		// mouseover pic
		return guil->MouseOverImage;
	} else { // pushed pic
		return guil->PushedImage;
	}

	quit("internal error in getbuttonpic");
}

void SetButtonPic(int guin, int objn, int ptype, int slotn) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!SetButtonPic: invalid GUI number");
	if ((objn < 0) | (objn >= guis[guin].GetControlCount())) quit("!SetButtonPic: invalid object number");
	if (guis[guin].GetControlType(objn) != kGUIButton)
		quit("!SetButtonPic: specified control is not a button");
	if ((ptype < 1) | (ptype > 3)) quit("!SetButtonPic: invalid pic type");

	GUIButton *guil = (GUIButton *)guis[guin].GetControl(objn);
	if (ptype == 1) {
		Button_SetNormalGraphic(guil, slotn);
	} else if (ptype == 2) {
		// mouseover pic
		Button_SetMouseOverGraphic(guil, slotn);
	} else { // pushed pic
		Button_SetPushedGraphic(guil, slotn);
	}
}

} // namespace AGS3
