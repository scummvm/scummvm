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

#include "ags/engine/ac/global_label.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/label.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/gui/gui_main.h"

namespace AGS3 {

using namespace AGS::Shared;



void SetLabelColor(int guin, int objn, int colr) {
	if ((guin < 0) | (guin >= _GP(game).numgui))
		quit("!SetLabelColor: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount()))
		quit("!SetLabelColor: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelColor: specified control is not a label");

	GUILabel *guil = (GUILabel *)_GP(guis)[guin].GetControl(objn);
	Label_SetColor(guil, colr);
}

void SetLabelText(int guin, int objn, const char *newtx) {
	VALIDATE_STRING(newtx);
	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!SetLabelText: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!SetLabelTexT: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelText: specified control is not a label");

	GUILabel *guil = (GUILabel *)_GP(guis)[guin].GetControl(objn);
	Label_SetText(guil, newtx);
}

void SetLabelFont(int guin, int objn, int fontnum) {

	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!SetLabelFont: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!SetLabelFont: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUILabel)
		quit("!SetLabelFont: specified control is not a label");

	GUILabel *guil = (GUILabel *)_GP(guis)[guin].GetControl(objn);
	Label_SetFont(guil, fontnum);
}

} // namespace AGS3
